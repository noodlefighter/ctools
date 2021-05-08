/*
 * event.h
 *
 *  Created on: 2019-7-19
 *      Author: Ming180
 */

#ifndef EVENT1_H_
#define EVENT1_H_

#include <stdint.h>
#include <pthread.h>
#include <errno.h>

#define EVENT_FLAG_AND      0x01
#define EVENT_FLAG_OR       0x02
#define EVENT_FLAG_CLEAR    0x04

struct event_t {
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	uint32_t value;
};

int event_init(struct event_t *e);
int event_destroy(struct event_t *e);
int event_post(struct event_t *e, uint32_t set);
int event_wait(struct event_t *e, uint32_t set, uint8_t option, uint32_t *recved);
int event_timedwait(struct event_t *e, uint32_t set, uint8_t option, uint32_t timeout, uint32_t *recved);

#endif // EVENT1_H_
