/*
 * sys_tickclk.h
 *
 *  Created on: 2019-04-26
 *      Author: noodlefighter
 */

#ifndef SYS_TICKCLK_H_
#define SYS_TICKCLK_H_

#include <stdint.h>
#include <time.h>

#ifdef __cplusplus
extern "C"
{
#endif

static inline int64_t sys_tickclk(void) {
	struct timespec ts;

	clock_gettime(CLOCK_MONOTONIC, &ts);

	return ((int64_t)ts.tv_sec * 1000 + ts.tv_nsec / 1000000);
}

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SYS_TICKCLK_H_ */
