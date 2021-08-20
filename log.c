/*
 * log.c
 *
 *  Created on: 2014-3-5
 *      Author: Ming180
 */

#include "log.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>

static int log_lv = LOG_LEVEL_INFO;

void log_level_set(int lv)
{
	log_lv = lv;
}

int log_printf(int lv, const char *fmt, ...)
{
	va_list ap;

	if (lv > log_lv) //not output
		return 0;

	va_start(ap, fmt);
	vfprintf(stderr, fmt, ap); //note: 输出到stderr因为它一般是无缓冲的，能及时输出
	va_end(ap);

	return 0;
}

void log_hexdump(int lv, const void *data, uint32_t size)
{
	uint32_t i;

	for (i = 0; i < size; i++) {
//		if (i && (i % 16 == 0))
//			LOG("\r\n");

		log_printf(lv, "%02X ", (int)((uint8_t*)data)[i]);
	}

	log_printf(lv, "\r\n");
}

void log_assert(const char *p_file, int line)
{
    LOG_FATAL("assert failed in <%s>, line %d\r\n", p_file, line);
    exit(-1); // error exit
}
