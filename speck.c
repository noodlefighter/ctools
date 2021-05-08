// SPDX-License-Identifier: GPL-2.0
/*
 * Speck: a lightweight block cipher
 *
 * Copyright (c) 2018 Google, Inc
 *
 * Speck has 10 variants, including 5 block sizes.  For now we only implement
 * the variants Speck128/128, Speck128/192, Speck128/256, Speck64/96, and
 * Speck64/128.   Speck${B}/${K} denotes the variant with a block size of B bits
 * and a key size of K bits.  The Speck128 variants are believed to be the most
 * secure variants, and they use the same block size and key sizes as AES.  The
 * Speck64 variants are less secure, but on 32-bit processors are usually
 * faster.  The remaining variants (Speck32, Speck48, and Speck96) are even less
 * secure and/or not as well suited for implementation on either 32-bit or
 * 64-bit processors, so are omitted.
 *
 * Reference: "The Simon and Speck Families of Lightweight Block Ciphers"
 * https://eprint.iacr.org/2013/404.pdf
 *
 * In a correspondence, the Speck designers have also clarified that the words
 * should be interpreted in little-endian format, and the words should be
 * ordered such that the first word of each block is 'y' rather than 'x', and
 * the first key word (rather than the last) becomes the first round key.
 */

#include <stdint.h>
#include <stdbool.h>
//#include "arch_wrapper.h"
#include "speck.h"

/**
 * rol32 - rotate a 32-bit value left
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline uint32_t rol32(uint32_t word, uint8_t shift)
{
    return (word << shift) | (word >> ((-shift) & 31));
}

/**
 * ror32 - rotate a 32-bit value right
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline uint32_t ror32(uint32_t word, uint8_t shift)
{
    return (word >> shift) | (word << (32 - shift));
}

static inline uint32_t get_unaligned_le32(const uint8_t *p)
{
    return p[0] | (uint16_t)p[1] << 8 | (uint32_t)p[2] << 16 | (uint32_t)p[3] << 24;
}

/*
static inline void put_unaligned_le16(uint16_t val, uint8_t *p)
{
    *p++ = val;
    *p++ = val >> 8;
} */

static inline void put_unaligned_le32(uint32_t val, uint8_t *p)
{
    //put_unaligned_le16(val >> 16, p + 2);
    //put_unaligned_le16(val, p);
    p[0] = val & 0xff;
    p[1] = (val >> 8) & 0xff;
    p[2] = (val >> 16) & 0xff;
    p[3] = val >> 24;
}

/* Speck64 */

static inline void speck64_round(uint32_t *x, uint32_t *y, uint32_t k)
{
    *x = ror32(*x, 8);
    *x += *y;
    *x ^= k;
    *y = rol32(*y, 3);
    *y ^= *x;
}

static inline void speck64_unround(uint32_t *x, uint32_t *y, uint32_t k)
{
    *y ^= *x;
    *y = ror32(*y, 3);
    *x ^= k;
    *x -= *y;
    *x = rol32(*x, 8);
}

void crypto_speck64_cal(const speck64_t *ctx, bool is_encrypt,
                uint8_t *out, const uint8_t *in)
{
    uint32_t y = get_unaligned_le32(in);
    uint32_t x = get_unaligned_le32(in + 4);
    int8_t i;

    if (is_encrypt) {
        for (i = 0; i < SPECK64_96_NROUNDS; i++)
            speck64_round(&x, &y, ctx->round_keys[i]);
    } else {
        for (i = SPECK64_96_NROUNDS - 1; i >= 0; i--)
            speck64_unround(&x, &y, ctx->round_keys[i]);
    }

    put_unaligned_le32(y, out);
    put_unaligned_le32(x, out + 4);
}

void crypto_speck64_setkey(speck64_t *ctx, const uint8_t *key)
{
    uint32_t l[2];
    uint32_t k;
    int8_t i;

    k = get_unaligned_le32(key);
    l[0] = get_unaligned_le32(key + 4);
    l[1] = get_unaligned_le32(key + 8);
    for (i = 0; i < SPECK64_96_NROUNDS; i++) {
        ctx->round_keys[i] = k;
        speck64_round(&l[i % 2], &k, i);
    }
}

