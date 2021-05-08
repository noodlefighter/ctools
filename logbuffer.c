/*
 * logbuffer.c
 *
 *  Created on: 2019-10-23
 *      Author: noodlefighter
 */

#include "logbuffer.h"

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

int logbuffer_init(struct logbuffer *p_obj, int size)
{
	memset(p_obj, 0, sizeof(struct logbuffer));

	p_obj->mem      = (char*) malloc(size);
	p_obj->mem_size = size;
	return 0;
}

void logbuffer_deinit(struct logbuffer *p_obj)
{
	free(p_obj->mem);
}

int logbuffer_puts(struct logbuffer *p_obj, const void *p_data, int length)
{
	const uint8_t *u8dat = (const uint8_t *) p_data;

	// discard some data if it's too big
	if (length > p_obj->mem_size) {
		u8dat += (length - p_obj->mem_size);
		length = p_obj->mem_size;
	}

	if (p_obj->mem_size - p_obj->tail > length) {
		memcpy(&p_obj->mem[p_obj->tail], u8dat, length);
	}
	else {
		memcpy(&p_obj->mem[p_obj->tail], u8dat,
				p_obj->mem_size - p_obj->tail);
		memcpy(&p_obj->mem[0], &u8dat[p_obj->mem_size - p_obj->tail],
				length - (p_obj->mem_size - p_obj->tail));
	}
	p_obj->tail += length;
	if (p_obj->tail >= p_obj->mem_size) {
		p_obj->tail -= p_obj->mem_size;
		p_obj->round_flag = 1;
	}
	return length;
}

int logbuffer_gets(struct logbuffer *p_obj, void *p_buf, int buf_size)
{
	uint8_t *u8buf = (uint8_t *) p_buf;

	if (p_obj->round_flag) {
		memcpy(u8buf, &p_obj->mem[p_obj->tail], p_obj->mem_size - p_obj->tail);
		memcpy(&u8buf[p_obj->mem_size - p_obj->tail], &p_obj->mem[0], p_obj->tail);
		return p_obj->mem_size;
	}
	else {
		memcpy(u8buf, &p_obj->mem[0], p_obj->tail);
		return p_obj->tail;
	}
}

int logbuffer_gets_to_fd(struct logbuffer *p_obj, int fd)
{
	if (p_obj->round_flag) {
		write(fd, &p_obj->mem[p_obj->tail], p_obj->mem_size - p_obj->tail);
		write(fd, &p_obj->mem[0], p_obj->tail);
		return p_obj->mem_size;
	}
	else {
		write(fd, &p_obj->mem[0], p_obj->tail);
		return p_obj->tail;
	}
}
