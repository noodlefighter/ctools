/*
 * tcpserver.h
 *
 *  Created on: 2020-03-11
 *	  Author: ebi
 */
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/time.h>

#include "sktcpserver.h"
#include "log.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

#define server_log(server, lv, fmt, ...) \
	log_printf(lv, "sktcpserver[%s]: " fmt, server->cfg.server_name, ##__VA_ARGS__)
#define client_log(client, lv, fmt, ...) \
	log_printf(lv, "sktcpserver[%s,%p]: " fmt, client->server->cfg.server_name, client, ##__VA_ARGS__)

static int set_fd_non_block(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags < 0) return -1;

	int err = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (err == -1) return -1;

	return 0;
}

static void client_disconnect(struct sktcpserver_client_ctx *client)
{
	if (client->server->cfg.client_disconnect_cb) {
		client->server->cfg.client_disconnect_cb((intptr_t)client, client->pri_data);
	}

	uev_io_stop(&client->recv_watcher);
	if (client->channel_fd >= 0) {
		shutdown(client->channel_fd, SHUT_RD);
		close(client->channel_fd);
		client->channel_fd = -1;
	}
	list_del(&client->node);
	client_log(client, LOG_LEVEL_INFO, "Disconnected\n");
	free(client);
}

static void client_receive_data_handler(uev_t *w, void *arg, int events)
{
	struct sktcpserver_client_ctx *client = arg;
	ssize_t recv_len;

	uint8_t *buf = client->server->rxbuffer;
	int bufsize  = client->server->cfg.buffer_size;

	recv_len = recv(w->fd, buf, bufsize, 0);
	client_log(client, LOG_LEVEL_VERBOSE, "recv_len=%ld\n", recv_len);

	if (recv_len == 0) { // client connection closed
		client_log(client, LOG_LEVEL_INFO, "recv() return 0\n");
		client_disconnect(client);
	} else if (recv_len == -1) { // connection error
		if (errno == EAGAIN) {
			return;
		}
		client_log(client, LOG_LEVEL_INFO, "recv() fail: %s\n", strerror(errno));
		client_disconnect(client);
	} else {
		if (client->server->cfg.client_data_recv_cb) {
			client->server->cfg.client_data_recv_cb((intptr_t)client, client->pri_data, buf, recv_len);
		}
	}
}

static void tcp_conn_handler(uev_t *w, void *arg, int events)
{
	sktcpserver_t *tcp = arg;
	int new_fd = accept(w->fd, NULL, NULL);
	if (new_fd < 0) {
		LOG_ERR("sktcpserver: accept() fail: %s\n", strerror(errno));
		return;
	}

	server_log(tcp, LOG_LEVEL_DEBUG, "Incoming connection ...\n");
	{
		struct sktcpserver_client_ctx *client;
		client = (struct sktcpserver_client_ctx*) malloc(sizeof(struct sktcpserver_client_ctx));
		if (NULL == client) {
			LOG_ERR("sktcpserver: malloc() fail\n");
			shutdown(new_fd, SHUT_RD);
			close(new_fd);
			return;
		}

		{
			void *pri_data = NULL;
			if (tcp->cfg.client_connect_cb && (0 != tcp->cfg.client_connect_cb((intptr_t)client, &pri_data))) { //返回非0值，断开与客户端的连接
				server_log(tcp, LOG_LEVEL_ERROR, "client_connect_cb() refuse client\n");
				free(client);
				return;
			}
			client->pri_data = pri_data;
			client->server = tcp;
			client->channel_fd = new_fd;
			list_add(&client->node, &tcp->client_list);
			uev_io_init(w->ctx, &client->recv_watcher, client_receive_data_handler, client, client->channel_fd, UEV_READ);
		}
		server_log(tcp, LOG_LEVEL_INFO, "Connection established client %p...\n", client);
	}
}

static int tcp_listen(int listen_port, uev_t *listen_watcher, uev_ctx_t *ctx, uev_cb_t conn_cb, void *conn_cb_arg)
{
	int fd = -1;

	struct sockaddr_in serv_addr;
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
	serv_addr.sin_port = htons(listen_port);

	if (0 > (fd = socket(AF_INET, SOCK_STREAM, 0))) {
		LOG_ERR("sktcpserver: socket() fail, %s\n", strerror(errno));
		goto err;
	}
	if (0 > bind(fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))) {
		LOG_ERR("sktcpserver: bind() fail, %s\n", strerror(errno));
		goto err;
	}
	if (0 > listen(fd, SOMAXCONN)) {
		LOG_ERR("sktcpserver: listen() fail, %s\n", strerror(errno));
		goto err;
	}
	if (0 > set_fd_non_block(fd)) {
		LOG_ERR("sktcpserver: set_fd_non_block() fail\n");
		goto err;
	}

	{
		int keepalive = 1;
		int keepidle = 15;
		int keepinterval = 1;
		int keepcount = 15;
		int reuse_opt = 1;
		struct linger sl = { .l_onoff = 1,  .l_linger = 0 };
		setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, (void *)&keepalive , sizeof(keepalive));
		setsockopt(fd, SOL_TCP, TCP_KEEPIDLE, (void *)&keepidle , sizeof(keepidle));
		setsockopt(fd, SOL_TCP, TCP_KEEPINTVL, (void *)&keepinterval , sizeof(keepinterval));
		setsockopt(fd, SOL_TCP, TCP_KEEPCNT, (void *)&keepcount , sizeof(keepcount));
		setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &reuse_opt, sizeof(reuse_opt)); /* ignore "socket already in use" errors */
		setsockopt(fd, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)); /* avoid TIME_WAIT state */
	}

	uev_io_init(ctx, listen_watcher, conn_cb, conn_cb_arg, fd, UEV_READ);
	return fd;
err:
	if (fd >= 0) {
		close(fd);
	}
	return -1;
}

int sktcpserver_init(sktcpserver_t *tcp, const struct sktcpserver_config *cfg)
{
	INIT_LIST_HEAD(&tcp->client_list);
	tcp->cfg = *cfg;
	pthread_mutex_init(&tcp->lock, NULL);

	tcp->rxbuffer = malloc(cfg->buffer_size);
	if (NULL == tcp->rxbuffer) {
		server_log(tcp, LOG_LEVEL_ERROR, "buffer_size malloc() failed\n");
		return -1;
	}

	tcp->listen_fd = tcp_listen(tcp->cfg.server_port, &tcp->listen_watcher, tcp->cfg.ctx, tcp_conn_handler, tcp);
	if (tcp->listen_fd < 0) {
		server_log(tcp, LOG_LEVEL_ERROR, "tcp_listen() failed\n");
		return -1;
	}
	return 0;
}

int sktcpserver_fini(sktcpserver_t *tcp)
{
	struct sktcpserver_client_ctx *p = NULL, *pn;

	// disconnect clients
	list_for_each_entry_safe(p, pn, &tcp->client_list, node) {
		client_disconnect(p);
	}

	uev_io_stop(&tcp->listen_watcher);
	if (tcp->listen_fd >= 0) {
		shutdown(tcp->listen_fd, SHUT_RD);
		close(tcp->listen_fd);
	}
	if (NULL != tcp->rxbuffer) {
		free(tcp->rxbuffer);
	}
	return 0;
}

static struct sktcpserver_client_ctx *client_handle_to_ptr(sktcpserver_t *tcp, intptr_t client)
{
	struct sktcpserver_client_ctx *p = NULL;

	list_for_each_entry(p, &tcp->client_list, node) {
		if ((intptr_t)p == client) {
			return (struct sktcpserver_client_ctx *) p;
		}
	}
	return NULL;
}

void *sktcpserver_client_pri_data_get(sktcpserver_t *tcp, intptr_t client)
{
	struct sktcpserver_client_ctx *client_ctx;
	void *pri_data = NULL;

	pthread_mutex_lock(&tcp->lock);
	client_ctx = client_handle_to_ptr(tcp, client);
	if (NULL == client_ctx) {
		goto err_exit;
	}
	pri_data = client_ctx->pri_data;

err_exit:
	pthread_mutex_unlock(&tcp->lock);
	return pri_data;
}

int sktcpserver_send(sktcpserver_t *tcp, intptr_t client, const uint8_t *dat, int dat_len)
{
	struct sktcpserver_client_ctx *p_client;

	pthread_mutex_lock(&tcp->lock);
	p_client = client_handle_to_ptr(tcp, client);
	if (NULL == p_client) {
		server_log(tcp, LOG_LEVEL_ERROR, "send fail, client is NULL\n");
		goto err_exit;
	}

	{
		struct timeval tv;
		fd_set fds;
		int select_ret;
		int fd = p_client->channel_fd;

		tv.tv_sec  = 0;
		tv.tv_usec = 500*1000;
		FD_ZERO(&fds);
		FD_SET(fd, &fds);
		select_ret = select(fd + 1, NULL, &fds, NULL, &tv);
		if (select_ret > 0) {
			if (FD_ISSET(fd, &fds)) {
				if (0 > send(fd, (const void *)dat, dat_len, 0)) {
					client_disconnect(p_client);
					client_log(p_client, LOG_LEVEL_ERROR, "send fail err=%d, disconnect..\n", errno);
					goto err_exit;
				}
			}
		}
		else {
			client_disconnect(p_client);
			client_log(p_client, LOG_LEVEL_ERROR, "select fail, errno=%d\n", errno);
			goto err_exit;
		}
	}

err_exit:
	pthread_mutex_unlock(&tcp->lock);
	return dat_len;
}
