/*
 * app_utils.c
 *
 *  Created on: 2020-05-05
 *      Author: ebi
 */
#include "app_utils.h"

#include <ctype.h>

static uint8_t bcd2ascii[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
static uint8_t ascii2bcd1[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
static uint8_t ascii2bcd2[6]  = {0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F};
static uint8_t ascii2bcd3[6]  = {0X0a, 0X0b, 0X0c, 0X0d, 0X0e, 0X0f};

uint32_t asc2bcd(uint8_t *bcd, const int8_t *asc, int32_t len)
{
    uint8_t c = 0;
    uint8_t index = 0;
    uint8_t i = 0;

    len >>= 1;

    for(; i < len; i++) {
        //first BCD
        if(isupper(*asc)){
            index = *asc - 'A';
            c  = ascii2bcd2[index] << 4;
        } else if(isdigit(*asc)) {
            index = *asc - '0';
            c  = ascii2bcd1[index] << 4;
        }else if(islower(*asc)) {
            index = *asc - 'a';
            c  = ascii2bcd3[index] << 4;
        }
        asc++;

        //second BCD
        if(isupper(*asc)) {
            index = *asc - 'A';
            c  |= ascii2bcd2[index];
        } else if(isdigit(*asc)) {
            index = *asc - '0';
            c  |= ascii2bcd1[index];
        }else if(islower(*asc)) {
            index = *asc - 'a';
            c  |= ascii2bcd3[index];
        }
        asc++;

        *bcd++ = c;
    }

    return 0;
}

int8_t *bcd2asc(int8_t *asc, const uint8_t *bcd, uint32_t len)
{
    uint8_t c = 0;
    uint8_t i;
    int8_t* ptr = asc;

    for(i = 0; i < len; i++) {
        //first BCD
        c = *bcd >> 4;
        *ptr++ = bcd2ascii[c];

        //second
        c = *bcd & 0x0f;
        *ptr++ = bcd2ascii[c];
        bcd++;
    }

    return asc;
}

#include <stdio.h>
#include <unistd.h>
#define __USE_XOPEN_EXTENDED
#include <ftw.h>

static int remove_recursive_cb(const char *fpath, const struct stat *sb, int typeflag, struct FTW *ftwbuf)
{
    return remove(fpath);
}

int remove_recursive(const char *path)
{
    return nftw(path, remove_recursive_cb, 64, FTW_DEPTH | FTW_PHYS);
}

int check_file_exists(const char * filename)
{
	struct stat buffer;
	return (0 == stat(filename, &buffer));
}
