#pragma once

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

#include <netinet/ip.h>
#include <arpa/inet.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <stdint.h>
#include <poll.h>
#include <stdio.h>
#include <assert.h>
#include <errno.h>
#include "macro.h"

typedef uint64_t usec_t;
typedef uint64_t nsec_t;

#define USEC_INFINITY ((usec_t) UINT64_MAX)
#define USEC_PER_SEC  ((usec_t) 1000000ULL)
#define TIME_T_MAX (time_t)((UINTMAX_C(1) << ((sizeof(time_t) << 3) - 1)) - 1)
#define NSEC_PER_USEC ((nsec_t) 1000ULL)

union sockaddr_union {
        struct sockaddr sa;
        struct sockaddr_in in;
        struct sockaddr_in6 in6;
};

int safe_atou(const char *s, unsigned *ret_u);
int parse_sockaddr(const char *s,
                   union sockaddr_union *addr, unsigned *addr_len);

ssize_t loop_read(int fd, void *buf, size_t nbytes, bool do_poll);

int safe_close(int fd);
int ppoll_usec(struct pollfd *fds, size_t nfds, usec_t timeout);

usec_t timespec_load(const struct timespec *ts) _pure_;

usec_t now(clockid_t clock);
struct timespec* timespec_store(struct timespec *ts, usec_t u);

static inline void _reset_errno_(int *saved_errno) {
        if (*saved_errno < 0) /* Invalidated by UNPROTECT_ERRNO? */
                return;

        errno = *saved_errno;
}

#define PROTECT_ERRNO                                                   \
        _cleanup_(_reset_errno_) _unused_ int _saved_errno_ = errno