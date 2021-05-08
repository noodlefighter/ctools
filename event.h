/*
 * event.h
 *
 *  Created on: 2019-06-29
 *      Author: noodlefighter
 */

#ifndef EVENT_H_
#define EVENT_H_

#include "activity.h"

typedef struct {

} event_t;

typedef int (*event_cb_t)(event_t *evt, activity_t *recv_act);


#ifdef __cplusplus
extern "C"
{
#endif

int event_manager_init(void);

int event_reg(event_t *evt, const struct event_cfg evt_cfg);
event_t *event_get(const char *evt_name);

int event_subscribe(event_t *evt, activity_t *recv_act, event_cb_t evt_handler);
int event_raise(event_t *evt, void *arg);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* EVENT_H_ */
