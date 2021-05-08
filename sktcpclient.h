/*
 * sktcpclient.h
 *
 *  Created on: 2020-06-23
 *      Author: ebi
 */
#ifndef SKTCPCLIENT_H_
#define SKTCPCLIENT_H_

#include <stdint.h>
#include <stdbool.h>
#include <pthread.h>
#include "uev.h"

typedef struct sktcpclient sktcpclient_t;

struct sktcpclient_config {
	uev_ctx_t *ctx;
	int buffer_size;
	int (*recv_cb)(sktcpclient_t *tcp, const uint8_t *dat, int dat_len);
};

enum sktcpclient_state {
	SKTCPCLIENT_STATE_DISCONNECTED,
	SKTCPCLIENT_STATE_CONNECTED,
};

struct sktcpclient {
	pthread_mutex_t lock;
	struct sktcpclient_config cfg;
	int fd;
	uev_t uev_watcher;
	uint8_t *rxbuffer; // buffer size: cfg.buffer_size
	enum sktcpclient_state state;
};


#ifdef __cplusplus
extern "C"
{
#endif


int sktcpclient_init(sktcpclient_t *tcp, const struct sktcpclient_config *cfg);
int sktcpclient_fini(sktcpclient_t *tcp);

static inline enum sktcpclient_state sktcpclient_state(sktcpclient_t *tcp) {
	return tcp->state;
}

int sktcpclient_connect(sktcpclient_t *tcp, const char *ip_addr, int port);
int sktcpclient_disconnect(sktcpclient_t *tcp);
int sktcpclient_send(sktcpclient_t *tcp, const uint8_t *dat, int dat_len);

// 一组用于流控的函数，stop()暂停接收数据，start()重新开始接收数据，active()检查当前状态
static inline int sktcpclient_recv_stop(sktcpclient_t *tcp) { return uev_io_stop(&tcp->uev_watcher); }
static inline int sktcpclient_recv_start(sktcpclient_t *tcp) { return uev_io_start(&tcp->uev_watcher); }
static inline bool sktcpclient_recv_active(sktcpclient_t *tcp) { return uev_io_active(&tcp->uev_watcher); }

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SKTCPSERVER_H_ */
