/*
 * activity.h
 *
 *  Created on: 2019-06-25
 *      Author: noodlefighter
 */

#ifndef ACTIVITY_H_
#define ACTIVITY_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include <pthread.h>
#include "linux_list.h"
#include "uev.h"

enum {
	ACTIVITY_MSG_NONE = 0,     // 内部消息
	ACTIVITY_MSG_STOP,
	ACTIVITY_MSG_EXEC,
	ACTIVITY_MSG_EVENT,

	ACTIVITY_MSG_GENERAL = 50, // 通用消息
	ACTIVITY_MSG_PERIODIC,

	ACTIVITY_MSG_USER = 100,   // 用户自定义消息
};

enum activity_state {
	ACTIVITY_STATE_NONE,             // 未初始化
	ACTIVITY_STATE_STOP,             // 停止
	ACTIVITY_STATE_RUN,              // 工作中
	ACTIVITY_STATE_PENDING_STOP,     // 正在停止
	ACTIVITY_STATE_NUM,
};

typedef struct activity activity_t;

struct activity {
	uev_ctx_t ctx;
	uev_t     event;
	uev_t     timer;

	struct list_head msg_queue;
//	struct list_head delay_msg_queue;

	struct list_head           node;
	pthread_mutex_t            lock;
	enum activity_state        state;
	const struct activity_cfg *cfg;
	pthread_t                  thread;
};


typedef int (*activity_msg_cb_t)(activity_t *act, int msg, void *arg);

struct activity_cfg {
	char   *name;            /* activity name */

	void (*init)(activity_t *act);
	void (*fini)(activity_t *act);

	activity_msg_cb_t user_msg_handler;

};


// 初始化活动对象管理器
int activity_manager_init(void);

// 反初始化活动对象管理器，停止所有活动对象，等待其控制线程退出
int activity_manager_fini(void);

// 将活动对象注册到活动对象管理器，注册后活动对象将被启动
int activity_reg(activity_t *act, const struct activity_cfg *act_cfg);

// 获取活动对象实例，失败返回NULL
activity_t *activity_get(const char *act_name);

// 获取活动对象当前状态
static inline
enum activity_state activity_state(activity_t *act) {
	return act->state;
}

// 启停活动对象
int activity_run(activity_t *act);
int activity_stop(activity_t *act, int wait_finish);

// 发送msg到活动对象的消息队列, arg_size不为0时自动管理arg内存
int activity_msg_send(activity_t *act, int msg, void *arg, int arg_size, int delay);

// 插入函数到活动对象中执行
int activity_exec(activity_t *act, activity_msg_cb_t pfunc, void *arg, int arg_size, int delay);

int activity_periodic_event_enable(activity_t *act, int interval_ms);
int activity_periodic_event_disable(activity_t *act);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* ACTIVITY_H_ */
