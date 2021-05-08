/*
 * bitmap.h
 *
 *  Created on: 2019-05-26
 *      Author: noodlefighter
 */

#ifndef BITMAP_H_
#define BITMAP_H_

#include <stdint.h>
#include <stdbool.h>
#include <string.h>

static inline
void bitmap_all_set(uint8_t *bitmap, int size, bool enable) {
	memset(bitmap, (enable)?0xFF:0, size);
}

static inline
void bitmap_bit_set(uint8_t *bitmap, int bit, bool enable) {
	if (enable)
		bitmap[bit/8] |= (1 << (bit%8));
	else
		bitmap[bit/8] &= ~(1 << (bit%8));
}

static inline
bool bitmap_bit_get(const uint8_t *bitmap, int bit) {
	return bitmap[bit/8] & (1 << (bit%8));
}

// 从start_bit开始搜索被置位的bit, 返回-1表示搜索完毕
static inline
int bitmap_next_enable_bit_get(const uint8_t *bitmap, int size, int start_bit) {
	int i = start_bit / 8;
	int j = start_bit % 8;
	for (; i < size; i++) {
		if (bitmap[i] == 0)
			continue;
		for (; j < 8; j++) {
			if (bitmap[i] & (1 << j)) {
				return i*8+j;
			}
		}
	}
	return -1;
}

// 遍历被置位的bit
#define bitmap_foreach(bit, bitmap, size) \
			for (bit = 0; (bit=bitmap_next_enable_bit_get(bitmap, size, bit)) >= 0; bit++)



#endif /* BITMAP_H_ */
