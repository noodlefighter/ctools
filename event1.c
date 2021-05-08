/*
 * event.c
 *
 *  Created on: 2019-7-19
 *      Author: Ming180
 */

#include "event1.h"

#include <stdio.h>
#include <time.h>

int event_init(struct event_t *e)
{
	if (!e) return EINVAL;

	pthread_mutex_init(&e->mutex, NULL);
	pthread_cond_init(&e->cond, NULL);

	pthread_mutex_lock(&e->mutex);
	e->value = 0;
	pthread_mutex_unlock(&e->mutex);
	return 0;
}

int event_destroy(struct event_t *e)
{
	pthread_cond_destroy(&e->cond);
	pthread_mutex_destroy(&e->mutex);
	return 0;
}

int event_post(struct event_t *e, uint32_t set)
{
	if (!e) return EINVAL;

	pthread_mutex_lock(&e->mutex);
	e->value |= set;
	pthread_cond_broadcast(&e->cond);
	pthread_mutex_unlock(&e->mutex);

	return 0;
}


int event_wait(struct event_t *e, uint32_t set, uint8_t option, uint32_t *recved)
{
	int match = 0, rc = 0;

	if (!e || !recved) return EINVAL;

	pthread_mutex_lock(&e->mutex);

	while (!match && rc == 0) {
		if (option & EVENT_FLAG_AND) {
			if ((e->value & set) == set) {
				match = 1;
				break;
			}
		}
		else if (option & EVENT_FLAG_OR) {
			if (e->value & set) {
				match = 1;
				break;
			}
		}

        rc = pthread_cond_wait(&e->cond, &e->mutex);
	}

	if (match) {
		*recved = e->value & set;

		if (option & EVENT_FLAG_CLEAR) {
			e->value &= ~set;
		}
	}

	pthread_mutex_unlock(&e->mutex);

	return match ? 0 : ETIMEDOUT;
}

int event_timedwait(struct event_t *e, uint32_t set, uint8_t option, uint32_t timeout, uint32_t *recved)
{
	struct timespec ts;
	int match = 0, rc = 0;

	if (!e || !recved) return EINVAL;

	pthread_mutex_lock(&e->mutex);

	if (clock_gettime(CLOCK_REALTIME, &ts) == -1) {
		perror("clock_gettime\n");
		return -1;
	}

	ts.tv_sec  +=  timeout / 1000;
	ts.tv_nsec += (timeout % 1000) * 1000000;

	while (!match && rc == 0) {
		if (option & EVENT_FLAG_AND) {
			if ((e->value & set) == set) {
				match = 1;
			}
		}
		else if (option & EVENT_FLAG_OR) {
			if (e->value & set) {
				match = 1;
			}
		}

        rc = pthread_cond_timedwait(&e->cond, &e->mutex, &ts);
	}

	if (match) {
		*recved = e->value & set;

		if (option & EVENT_FLAG_CLEAR) {
			e->value &= ~set;
		}
	}

	pthread_mutex_unlock(&e->mutex);

	return match ? 0 : ETIMEDOUT;
}
