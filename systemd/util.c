/***
  This file is part of systemd.

  Copyright 2010 Lennart Poettering

  systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with systemd; If not, see <http://www.gnu.org/licenses/>.
***/

/* stuff imported from systemd without any changes */

#ifndef _GNU_SOURCE
#  define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>
#include <endian.h>
#include <fcntl.h>
#include <unistd.h>
#include <net/if.h>
#include <errno.h>
#include <inttypes.h>
#include <time.h>
#include "util.h"

int unhexchar(char c) {

        if (c >= '0' && c <= '9')
                return c - '0';

        if (c >= 'a' && c <= 'f')
                return c - 'a' + 10;

        if (c >= 'A' && c <= 'F')
                return c - 'A' + 10;

        return -EINVAL;
}

size_t page_size(void) {
        static thread_local size_t pgsz = 0;
        long r;

        if (_likely_(pgsz > 0))
                return pgsz;

        r = sysconf(_SC_PAGESIZE);
        assert(r > 0);

        pgsz = (size_t) r;
        return pgsz;
}


int ppoll_usec(struct pollfd *fds, size_t nfds, usec_t timeout) {
        struct timespec ts;
        int r;

        assert(fds || nfds == 0);

        if (nfds == 0)
                return 0;

        r = ppoll(fds, nfds, timeout == USEC_INFINITY ? NULL : timespec_store(&ts, timeout), NULL);
        if (r < 0)
                return -errno;
        if (r == 0)
                return 0;

        for (size_t i = 0, n = r; i < nfds && n > 0; i++) {
                if (fds[i].revents == 0)
                        continue;
                if (fds[i].revents & POLLNVAL)
                        return -EBADF;
                n--;
        }

        return r;
}

int safe_atou(const char *s, unsigned *ret_u) {
        char *x = NULL;
        unsigned long l;

        assert(s);
        assert(ret_u);

        /* strtoul() is happy to parse negative values, and silently
         * converts them to unsigned values without generating an
         * error. We want a clean error, hence let's look for the "-"
         * prefix on our own, and generate an error. But let's do so
         * only after strtoul() validated that the string is clean
         * otherwise, so that we return EINVAL preferably over
         * ERANGE. */

        errno = 0;
        l = strtoul(s, &x, 0);
        if (errno > 0)
                return -errno;
        if (!x || x == s || *x)
                return -EINVAL;
        if (s[0] == '-')
                return -ERANGE;
        if ((unsigned long) (unsigned) l != l)
                return -ERANGE;

        *ret_u = (unsigned) l;
        return 0;
}

static bool socket_ipv6_is_supported(void) {
        if (access("/proc/net/if_inet6", F_OK) != 0)
                return false;

        return true;
}

static int assign_address(const char *s,
                          uint16_t port,
                          union sockaddr_union *addr, unsigned *addr_len) {
        int r;

        /* IPv4 in w.x.y.z:p notation? */
        r = inet_pton(AF_INET, s, &addr->in.sin_addr);
        if (r < 0)
                return -errno;

        if (r > 0) {
                /* Gotcha, it's a traditional IPv4 address */
                addr->in.sin_family = AF_INET;
                addr->in.sin_port = htobe16(port);
                *addr_len = sizeof(struct sockaddr_in);
        } else {
                unsigned idx;

                if (strlen(s) > IF_NAMESIZE-1)
                        return -EINVAL;

                /* Uh, our last resort, an interface name */
                idx = if_nametoindex(s);
                if (idx == 0)
                        return -EINVAL;

                addr->in6.sin6_family = AF_INET6;
                addr->in6.sin6_port = htobe16(port);
                addr->in6.sin6_scope_id = idx;
                addr->in6.sin6_addr = in6addr_any;
                *addr_len = sizeof(struct sockaddr_in6);
        }

        return 0;
}


int parse_sockaddr(const char *s,
                   union sockaddr_union *addr, unsigned *addr_len) {

        char *e, *n;
        unsigned u;
        int r;

        if (*s == '[') {
                /* IPv6 in [x:.....:z]:p notation */

                e = strchr(s+1, ']');
                if (!e)
                        return -EINVAL;

                n = strndupa(s+1, e-s-1);

                errno = 0;
                if (inet_pton(AF_INET6, n, &addr->in6.sin6_addr) <= 0)
                        return errno > 0 ? -errno : -EINVAL;

                e++;
                if (*e) {
                        if (*e != ':')
                                return -EINVAL;

                        e++;
                        r = safe_atou(e, &u);
                        if (r < 0)
                                return r;

                        if (u <= 0 || u > 0xFFFF)
                                return -EINVAL;

                        addr->in6.sin6_port = htobe16((uint16_t)u);
                }

                addr->in6.sin6_family = AF_INET6;
                *addr_len = sizeof(struct sockaddr_in6);

        } else {
                e = strchr(s, ':');
                if (e) {
                        r = safe_atou(e+1, &u);
                        if (r < 0)
                                return r;

                        if (u <= 0 || u > 0xFFFF)
                                return -EINVAL;

                        n = strndupa(s, e-s);
                        return assign_address(n, u, addr, addr_len);

                } else {
                        r = safe_atou(s, &u);
                        if (r < 0)
                                return assign_address(s, 0, addr, addr_len);

                        /* Just a port */
                        if (u <= 0 || u > 0xFFFF)
                                return -EINVAL;

                        if (socket_ipv6_is_supported()) {
                                addr->in6.sin6_family = AF_INET6;
                                addr->in6.sin6_port = htobe16((uint16_t)u);
                                addr->in6.sin6_addr = in6addr_any;
                                *addr_len = sizeof(struct sockaddr_in6);
                        } else {
                                addr->in.sin_family = AF_INET;
                                addr->in.sin_port = htobe16((uint16_t)u);
                                addr->in.sin_addr.s_addr = INADDR_ANY;
                                *addr_len = sizeof(struct sockaddr_in);
                        }
                }
        }

        return 0;
}

int fd_wait_for_event(int fd, int event, usec_t timeout) {
        struct pollfd pollfd = {
                .fd = fd,
                .events = event,
        };
        int r;

        r = ppoll_usec(&pollfd, 1, timeout);
        if (r <= 0)
                return r;

        return pollfd.revents;
}


ssize_t loop_read(int fd, void *buf, size_t nbytes, bool do_poll) {
        uint8_t *p = buf;
        ssize_t n = 0;

        assert(fd >= 0);
        assert(buf);

        while (nbytes > 0) {
                ssize_t k;

                k = read(fd, p, nbytes);
                if (k < 0 && errno == EINTR)
                        continue;

                if (k < 0 && errno == EAGAIN && do_poll) {

                        /* We knowingly ignore any return value here,
                         * and expect that any error/EOF is reported
                         * via read() */

                        fd_wait_for_event(fd, POLLIN, (usec_t) -1);
                        continue;
                }

                if (k <= 0)
                        return n > 0 ? n : (k < 0 ? -errno : 0);

                p += k;
                nbytes -= k;
                n += k;
        }

        return n;
}

int safe_close(int fd) {

        /*
         * Like close_nointr() but cannot fail. Guarantees errno is
         * unchanged. Is a NOP with negative fds passed, and returns
         * -1, so that it can be used in this syntax:
         *
         * fd = safe_close(fd);
         */

        if (fd >= 0) {
                PROTECT_ERRNO;
                /* The kernel might return pretty much any error code
                 * via close(), but the fd will be closed anyway. The
                 * only condition we want to check for here is whether
                 * the fd was invalid at all... */

                assert(close_nointr(fd) != -EBADF);
        }

        return -1;
}

usec_t now(clockid_t clock_id) {
        struct timespec ts;
        clock_gettime(clock_id, &ts);
        return timespec_load(&ts);
}

struct timespec *timespec_store(struct timespec *ts, usec_t u)  {
        assert(ts);

        if (u == USEC_INFINITY ||
            u / USEC_PER_SEC >= TIME_T_MAX) {
                ts->tv_sec = (time_t) -1;
                ts->tv_nsec = -1L;
                return ts;
        }

        ts->tv_sec = (time_t) (u / USEC_PER_SEC);
        ts->tv_nsec = (long) ((u % USEC_PER_SEC) * NSEC_PER_USEC);

        return ts;
}

usec_t timespec_load(const struct timespec *ts) {
        assert(ts);

        if (ts->tv_sec < 0 || ts->tv_nsec < 0)
                return USEC_INFINITY;

        if ((usec_t) ts->tv_sec > (UINT64_MAX - (ts->tv_nsec / NSEC_PER_USEC)) / USEC_PER_SEC)
                return USEC_INFINITY;

        return
                (usec_t) ts->tv_sec * USEC_PER_SEC +
                (usec_t) ts->tv_nsec / NSEC_PER_USEC;
}
