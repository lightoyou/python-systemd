/* SPDX-License-Identifier: LGPL-2.1-or-later */
#pragma once

/*
 * fsprg v0.1  -  (seekable) forward-secure pseudorandom generator
 * Copyright © 2012 B. Poettering
 * Contact: fsprg@point-at-infinity.org
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301  USA
 */

#include <inttypes.h>
#include <sys/types.h>
#include <systemd/sd-id128.h>
#include <gcrypt.h>

#include "macro.h"
#include "util.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FSPRG_RECOMMENDED_SECPAR 1536
#define FSPRG_RECOMMENDED_SEEDLEN (96/8)

#define FSS_HEADER_SIGNATURE                                            \
        ((const char[]) { 'K', 'S', 'H', 'H', 'R', 'H', 'L', 'P' })

typedef struct FSSHeader FSSHeader;

typedef uint16_t le16_t;
typedef uint32_t le32_t;
typedef uint64_t le64_t;

struct FSSHeader {
        uint8_t signature[8]; /* "KSHHRHLP" */
        le32_t compatible_flags;
        le32_t incompatible_flags;
        sd_id128_t machine_id;
        sd_id128_t boot_id;    /* last writer */
        le64_t header_size;
        le64_t start_usec;
        le64_t interval_usec;
        le16_t fsprg_secpar;
        le16_t reserved[3];
        le64_t fsprg_state_size;
} _packed_;
typedef struct JournalFile {
        int fd;
        gcry_md_hd_t hmac;
        bool hmac_running;

        FSSHeader *fss_file;
        size_t fss_file_size;

        uint64_t fss_start_usec;
        uint64_t fss_interval_usec;

        void *fsprg_state;
        size_t fsprg_state_size;

        void *fsprg_seed;
        size_t fsprg_seed_size;

} JournalFile;



size_t FSPRG_mskinbytes(unsigned secpar) _const_;
size_t FSPRG_mpkinbytes(unsigned secpar) _const_;
size_t FSPRG_stateinbytes(unsigned secpar) _const_;

/* Setup msk and mpk. Providing seed != NULL makes this algorithm deterministic. */
void FSPRG_GenMK(void *msk, void *mpk, const void *seed, size_t seedlen, unsigned secpar);

/* Initialize state deterministically in dependence on seed. */
/* Note: in case one wants to run only one GenState0 per GenMK it is safe to use
   the same seed for both GenMK and GenState0.
*/
void FSPRG_GenState0(void *state, const void *mpk, const void *seed, size_t seedlen);

void FSPRG_Evolve(void *state);

uint64_t FSPRG_GetEpoch(const void *state) _pure_;

/* Seek to any arbitrary state (by providing msk together with seed from GenState0). */
void FSPRG_Seek(void *state, uint64_t epoch, const void *msk, const void *seed, size_t seedlen);

void FSPRG_GetKey(const void *state, void *key, size_t keylen, uint32_t idx);

#ifdef __cplusplus
}
#endif
