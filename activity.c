/*
 * activity.c
 *
 *  Created on: 2019-06-28
 *      Author: noodlefighter
 */

#include "activity.h"
#include "app_type.h"
#include "log.h"
#include <errno.h>
#include <stdint.h>
#include <string.h>
#include "pool.h"
#include "wof_allocator.h"

#define ACTIVITY_MSG_POOL_ITEMS                100
#define ACTIVITY_MSG_EXEC_ITEM_POOL_ITEMS      20

#define ACT_ATTR_GC         BIT(0)

struct act_msg {
	struct list_head node;
	void            *arg;
	uint16_t         type;
	uint16_t         dealy;
	uint16_t         attr;
};

struct act_msg_exec_item {
	activity_msg_cb_t func;
	void             *arg;
	uint16_t          attr;
};

static pthread_mutex_t act_manager_lock = PTHREAD_MUTEX_INITIALIZER;
static LIST_HEAD(act_list);
static pool_t act_msg_pool;
static pool_t exec_item_pool;
static wof_allocator_t *act_heapmem;

static int msg_enqueue(activity_t *act, int type, void *arg, unsigned int attr)
{
	struct act_msg *msg = NULL;

	msg = pool_malloc(&act_msg_pool);
	if (NULL == msg)
		return -ENOMEM;

	msg->type = type;
	msg->arg    = arg;
	msg->dealy  = 0;
	msg->attr   = attr;
	list_add_tail(&msg->node, &act->msg_queue);

	uev_event_post(&act->event);
	return 0;
}

static struct act_msg *msg_dequeue(activity_t *act)
{
	struct act_msg *msg;

	if (list_empty(&act->msg_queue))
		return NULL;

	msg = list_first_entry(&act->msg_queue, struct act_msg, node);
	list_del(&msg->node);
	return msg;
}

static int activity_set_pendding_exit(activity_t *act)
{
	int ret;

	if (act->state != ACTIVITY_STATE_RUN)
		return -1;

	ret = msg_enqueue(act, ACTIVITY_MSG_STOP, NULL, 0);
	if (0 != ret)
		return ret;

	act->state = ACTIVITY_STATE_PENDING_STOP;
	return 0;
}

static int act_msg_handler(activity_t *act, struct act_msg *msg)
{
//	LOG_DBG("activity: handling msg %p, type=%d, arg=%p\n", msg, msg->type, msg->arg);
	switch (msg->type) {
	case ACTIVITY_MSG_STOP:
		uev_exit(&act->ctx); // 通知uev控制的线程退出
		break;

	case ACTIVITY_MSG_EXEC: {
		struct act_msg_exec_item *exec_item = (struct act_msg_exec_item *)msg->arg;

		ASSERT(exec_item != NULL);
		ASSERT(exec_item->func != NULL);
		exec_item->func(act, ACTIVITY_MSG_EXEC, exec_item->arg);
		if (exec_item->attr & ACT_ATTR_GC) {
			wof_free(act_heapmem, exec_item->arg);
		}
		pool_free(&exec_item_pool, exec_item);
		break;
	}

//	case ACTIVITY_MSG_EVENT: //todo: event
//		break;

	default:
		if (msg->type >= ACTIVITY_MSG_GENERAL) {
			if (act->cfg->user_msg_handler) {
				act->cfg->user_msg_handler(act, msg->type, msg->arg);
			}
		}
		else {
			LOG_ERR("activity: %s recv unknown msg %d, arg %p", act->cfg->name , msg->type, msg->arg);
		}
		break;
	}

	return 0;
}

static void msg_queue_handler(uev_t *w, void *arg, int events)
{
	activity_t *act = (activity_t*) arg;
	struct act_msg *msg;

	(void)events;

//	LOG("msg_queue_handler() trigger!\n");
	while (1) {
		pthread_mutex_lock(&act->lock);
		msg = msg_dequeue(act);
		pthread_mutex_unlock(&act->lock);

		if (msg == NULL) // 读空队列后退出
			break;

		act_msg_handler(act, msg);

		if (msg->attr & ACT_ATTR_GC)
			wof_free(act_heapmem, msg->arg);

		pool_free(&act_msg_pool, msg);
	}
}

//todo: 处理延迟队列
//static void delay_msg_queue_handler(uev_t *w, void *arg, int events)
//{
//	activity_t *act = (activity_t*) arg;
//
//	LOG_DBG("activity_delay_msg_handler()\n");
//
//}

static void* activity_thread_entry(void* arg)
{
	activity_t *act = (activity_t*) arg;

	if (act->cfg->init)
		act->cfg->init(act);

	// blocking running
	(void)uev_run(&act->ctx, 0);

	if (act->cfg->fini)
		act->cfg->fini(act);

	// exit, do clean
	pthread_mutex_lock(&act->lock);
	uev_event_stop(&act->event);
	uev_event_stop(&act->timer);
	act->state = ACTIVITY_STATE_STOP;
	pthread_mutex_unlock(&act->lock);
	return NULL;
}

static int activity_run_local(activity_t *act)
{
	int ret, rc = 0;

	if (act->state == ACTIVITY_STATE_RUN)
		return 0;
	else if (act->state != ACTIVITY_STATE_STOP)
		return -1;

	ret = uev_init(&act->ctx);
	if (0 != ret) {
		rc = ret;
		goto err_exit1;
	}

	ret = uev_event_init(&act->ctx, &act->event, msg_queue_handler, act);
	if (0 != ret) {
		rc = ret;
		goto err_exit2;
	}

//	uev_timer_init(&act->ctx, &act->timer, delay_msg_queue_handler, act, 0, 0);

	ret = pthread_create(&act->thread, NULL, activity_thread_entry, act);
	if (0 != ret) {
		rc = ret;
		goto err_exit3;
	}
	act->state = ACTIVITY_STATE_RUN;
	return 0;

err_exit3:
	uev_event_stop(&act->event);
err_exit2:
	uev_exit(&act->ctx);
err_exit1:
	return rc;
}

static activity_t *activity_get_local(const char *act_name)
{
	activity_t *p, *ret_act = NULL;
	list_for_each_entry(p, &act_list, node) {
		if (0 == strcmp(p->cfg->name, act_name)) {
			ret_act = p;
			break;
		}
	}
	return ret_act;
}

int activity_run(activity_t *act)
{
	int rc;

	pthread_mutex_lock(&act_manager_lock);
	rc = activity_run_local(act);
	pthread_mutex_unlock(&act_manager_lock);
	return rc;
}

int activity_reg(activity_t *act, const struct activity_cfg *act_cfg)
{
	int ret, rc = 0;

	if (!act || !act_cfg->name)
		return -EINVAL;

	pthread_mutex_lock(&act_manager_lock);

	// redefine check by name
	if (NULL != activity_get_local(act_cfg->name)) {
		rc = -1;
		LOG_ERR("activity: redefine %s\n", act_cfg->name);
		goto exit;
	}

	// init object
	memset(act, 0, sizeof(*act));
	pthread_mutex_init(&act->lock, NULL);
	INIT_LIST_HEAD(&act->msg_queue);
//	INIT_LIST_HEAD(&act->delay_msg_queue);
	act->cfg = act_cfg;
	list_add(&act->node, &act_list);
	act->state = ACTIVITY_STATE_STOP;
	ret = activity_run_local(act);
	if (0 != ret) {
		rc = ret;
		goto exit;
	}

exit:
	pthread_mutex_unlock(&act_manager_lock);
	return rc;
}

activity_t *activity_get(const char *act_name)
{
	activity_t *ret_act = NULL;

	if (!act_name)
		return NULL;

	pthread_mutex_lock(&act_manager_lock);
	ret_act = activity_get_local(act_name);
	pthread_mutex_unlock(&act_manager_lock);
	return ret_act;
}

int activity_stop(activity_t *act, int wait_finish)
{
	int ret;

	pthread_mutex_lock(&act->lock);
	ret = activity_set_pendding_exit(act);
	pthread_mutex_unlock(&act->lock);
	if (0 != ret)
		return ret;

	if (wait_finish)
		pthread_join(act->thread, NULL);

	return 0;
}

int activity_msg_send(activity_t *act, int msg, void *arg, int arg_size, int delay)
{
	unsigned int attr = 0;
	int rc = 0;
	void *new_arg;

	if ((!act) || (msg > UINT16_MAX) || (delay > UINT16_MAX))
		return -EINVAL;
	if (delay != 0) //todo: 待实现等待队列
		return -1;

	// arg_size不为0时接管arg内存
	if (arg_size != 0) {
		attr |= ACT_ATTR_GC;
		new_arg = wof_alloc(act_heapmem, arg_size);
		if (NULL == new_arg) {
			return -ENOMEM;
		}
		memcpy(new_arg, arg, arg_size);
		arg = new_arg;
	}

	pthread_mutex_lock(&act->lock);
	if (act->state != ACTIVITY_STATE_RUN) {
		rc = -1;
		goto err_exit;
	}
	if (0 != msg_enqueue(act, msg, arg, attr)) {
		rc = -ENOMEM;
		goto err_exit;
	}
	pthread_mutex_unlock(&act->lock);
	return 0;

err_exit:
	if (arg_size != 0) {
		wof_free(act_heapmem, arg);
	}
	pthread_mutex_unlock(&act->lock);
	return rc;
}

int activity_exec(activity_t *act, activity_msg_cb_t pfunc, void *arg, int arg_size, int delay)
{
	struct act_msg_exec_item *exec_item;
	int ret, rc = 0;
	void *new_arg = NULL;

	if (!act || !pfunc)
		return -EINVAL;

	exec_item = pool_malloc(&exec_item_pool);
	if (NULL == exec_item) {
		rc = -ENOMEM;
		goto err_exit1;
	}
	exec_item->func = pfunc;
	exec_item->arg  = arg;
	exec_item->attr = 0;

	if (arg_size != 0) {
		exec_item->attr |= ACT_ATTR_GC;
		new_arg = wof_alloc(act_heapmem, arg_size);
		if (NULL == new_arg) {
			rc = -ENOMEM;
			goto err_exit2;
		}
		memcpy(new_arg, arg, arg_size);
		exec_item->arg = new_arg;
	}

	ret = activity_msg_send(act, ACTIVITY_MSG_EXEC, exec_item, 0, 0);
	if (0 != ret) {
		rc = ret;
		goto err_exit3;
	}
	return 0;

err_exit3:
	if (arg_size != 0) {
		wof_free(act_heapmem, new_arg);
	}
err_exit2:
	pool_free(&exec_item_pool, exec_item);
err_exit1:
	return rc;
}

static void periodic_event_cb(uev_t *w, void *arg, int events)
{
	activity_t *act = (activity_t *) arg;
	activity_msg_send(act, ACTIVITY_MSG_PERIODIC, NULL, 0, 0);
	uev_timer_start(&act->timer);
}

int activity_periodic_event_enable(activity_t *act, int interval_ms)
{
	uev_timer_init(&act->ctx, &act->timer, periodic_event_cb, act, interval_ms, interval_ms);
	uev_timer_start(&act->timer);
	return 0;
}

int activity_periodic_event_disable(activity_t *act)
{
	uev_timer_stop(&act->timer);
	return 0;
}


int activity_manager_init(void)
{
	int ret;

	ret = pool_init(&act_msg_pool, sizeof(struct act_msg), ACTIVITY_MSG_POOL_ITEMS);
	if (0 != ret)
		return ret;

	ret = pool_init(&exec_item_pool, sizeof(struct act_msg_exec_item), ACTIVITY_MSG_EXEC_ITEM_POOL_ITEMS);
	if (0 != ret)
		return ret;

	act_heapmem = wof_allocator_new();
	if (NULL == act_heapmem)
		return -ENOMEM;

	pthread_mutex_init(&act_manager_lock, NULL);
	return 0;
}

int activity_manager_fini(void)
{
	activity_t *p, *n;

	// set "pendding_exit" state
	list_for_each_entry(p, &act_list, node) {
		pthread_mutex_lock(&p->lock);
		activity_set_pendding_exit(p);
		pthread_mutex_unlock(&p->lock);
	}

	// wait for thread quit, then delete its list node
	list_for_each_entry_safe(p, n, &act_list, node) {
		// pthread_join(p->thread, NULL); // note: uev_run() will not return after uev_exit() called
		list_del(&p->node);
	}

	// clear msg mempool
	pool_deinit(&act_msg_pool);
	pool_deinit(&exec_item_pool);
	wof_allocator_destroy(act_heapmem);
	return 0;
}

