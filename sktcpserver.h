/*
 * tcpserver.h
 *
 *  Created on: 2020-03-11
 *      Author: ebi
 */

#ifndef SKTCPSERVER_H_
#define SKTCPSERVER_H_

#include <pthread.h>

#include "stdint.h"
#include "uev.h"
#include "linux_list.h"

struct sktcpserver_config {
	const char *server_name;
	uev_ctx_t *ctx;
	int buffer_size;
	int server_port;
	int (*client_connect_cb)(intptr_t client, void **pri_data); // return 0 to accpet, or any other value to refuse
	void (*client_disconnect_cb)(intptr_t client, void *pri_data);
	void (*client_data_recv_cb)(intptr_t client, void *pri_data, uint8_t *dat, int dat_len);
};

typedef struct {
	pthread_mutex_t lock;
	struct sktcpserver_config cfg;
	int listen_fd;
	uev_t listen_watcher;
	struct list_head client_list;
	uint8_t *rxbuffer; // buffer size: cfg.buffer_size
} sktcpserver_t;

struct sktcpserver_client_ctx {
	struct list_head node;
	sktcpserver_t *server;
	int channel_fd;
	uev_t recv_watcher;
	void *pri_data;
};

#ifdef __cplusplus
extern "C"
{
#endif

int sktcpserver_init(sktcpserver_t *tcp, const struct sktcpserver_config *cfg);
int sktcpserver_fini(sktcpserver_t *tcp);

void *sktcpserver_client_pri_data_get(sktcpserver_t *tcp, intptr_t client);

int sktcpserver_send(sktcpserver_t *tcp, intptr_t client, const uint8_t *dat, int dat_len); // 阻塞发送

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SKTCPSERVER_H_ */
