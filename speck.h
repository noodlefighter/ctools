// SPDX-License-Identifier: GPL-2.0
/*
 * Common values for the Speck algorithm
 */

#ifndef _CRYPTO_SPECK_H
#define _CRYPTO_SPECK_H

/* Speck64 */
#include <stdint.h>
#include <stdbool.h>

#define SPECK64_BLOCK_SIZE      8

#define SPECK64_96_KEY_SIZE     12
#define SPECK64_96_NROUNDS      26

typedef struct {
    uint32_t round_keys[SPECK64_96_NROUNDS];
} speck64_t;

void crypto_speck64_cal(const speck64_t *ctx, bool is_encrypt,
                        uint8_t *out, const uint8_t *in);

void crypto_speck64_setkey(speck64_t *ctx, const uint8_t *key);

#endif /* _CRYPTO_SPECK_H */
