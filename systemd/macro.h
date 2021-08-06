/*-*- Mode: C; c-basic-offset: 8; indent-tabs-mode: nil -*-*/

#pragma once
#include <malloc.h>
/***

  Copyright 2010 Lennart Poettering

  python-systemd is free software; you can redistribute it and/or modify it
  under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation; either version 2.1 of the License, or
  (at your option) any later version.

  python-systemd is distributed in the hope that it will be useful, but
  WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with python-systemd; If not, see <http://www.gnu.org/licenses/>.
***/

#define _const_ __attribute__ ((const))
#define _pure_ __attribute__ ((pure))
#define _unused_ __attribute__((__unused__))
#define _likely_(x) (__builtin_expect(!!(x), 1))

#if __GNUC__ >= 7
#define _fallthrough_ __attribute__((__fallthrough__))
#else
#define _fallthrough_
#endif

/* Define C11 thread_local attribute even on older gcc compiler
 * version */
#ifndef thread_local
/*
 * Don't break on glibc < 2.16 that doesn't define __STDC_NO_THREADS__
 * see http://gcc.gnu.org/bugzilla/show_bug.cgi?id=53769
 */
#if __STDC_VERSION__ >= 201112L && !(defined(__STDC_NO_THREADS__) || (defined(__GNU_LIBRARY__) && __GLIBC__ == 2 && __GLIBC_MINOR__ < 16))
#define thread_local _Thread_local
#else
#define thread_local __thread
#endif
#endif



#define DISABLE_WARNING_MISSING_PROTOTYPES                              \
        _Pragma("GCC diagnostic push");                                 \
        _Pragma("GCC diagnostic ignored \"-Wmissing-prototypes\"")

#define REENABLE_WARNING                                                \
        _Pragma("GCC diagnostic pop")



#define DEFINE_TRIVIAL_CLEANUP_FUNC(type, func)                 \
        static inline void func##p(type *p) {                   \
                if (*p)                                         \
                        func(*p);                               \
        }                                                       \
        struct __useless_struct_to_allow_trailing_semicolon__

#define new0(t, n) ((t*) calloc((n), sizeof(t)))
#define alloca0(n)                                      \
        ({                                              \
                char *_new_;                            \
                size_t _len_ = n;                       \
                _new_ = alloca(_len_);                  \
                (void *) memset(_new_, 0, _len_);       \
        })

#define _cleanup_(x) __attribute__((cleanup(x)))



static inline void freep(void *p) {
        free(*(void**) p);
}


#define _cleanup_free_ _cleanup_(freep)




#if defined(static_assert)
#  define assert_cc(expr)                               \
 static_assert(expr, #expr)
#else
#  define assert_cc(expr)
#endif
