/*
 * logbuffer.h
 *
 *  Created on: 2019-10-23
 *      Author: noodlefighter
 */

#ifndef LOGBUFFER_H_
#define LOGBUFFER_H_

#include <stdint.h>

struct logbuffer {
	char     *mem;
	uint32_t  mem_size;
	uint32_t  tail;
	int       round_flag;  // 已经写回环了
};

#ifdef __cplusplus
extern "C"
{
#endif

int logbuffer_init(struct logbuffer *p_obj, int size);
void logbuffer_deinit(struct logbuffer *p_obj);
int logbuffer_puts(struct logbuffer *p_obj, const void *p_data, int length);
int logbuffer_gets(struct logbuffer *p_obj, void *p_buf, int buf_size);

int logbuffer_gets_to_fd(struct logbuffer *p_obj, int fd);

#ifdef __cplusplus
}
#endif

#endif /* LOGBUFFER_H_ */
