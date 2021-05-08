/*
 * sktcpclient.h
 *
 *  Created on: 2020-06-23
 *      Author: ebi
 */
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/fcntl.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <sys/time.h>
#include <time.h>
#include <pthread.h>
#include <string.h>

#include "sktcpclient.h"
#include "log.h"

#ifdef DMALLOC
#include <dmalloc.h>
#endif

int sktcpclient_init(sktcpclient_t *tcp, const struct sktcpclient_config *cfg)
{
	if ((NULL == cfg) || (0 == cfg->buffer_size) || (NULL == cfg->ctx))
		return -EINVAL;

	pthread_mutex_init(&tcp->lock, NULL);
	memset(tcp, 0, sizeof(sktcpclient_t));
	tcp->cfg   = *cfg;
	tcp->state = SKTCPCLIENT_STATE_DISCONNECTED;

	tcp->rxbuffer = (uint8_t *) malloc(tcp->cfg.buffer_size);
	if (NULL == tcp->rxbuffer) {
		LOG_ERR("%s:malloc() failed\n", __func__);
		return -ENOMEM;
	}
	return 0;
}

int sktcpclient_fini(sktcpclient_t *tcp)
{
	if (tcp->state == SKTCPCLIENT_STATE_CONNECTED) {
		sktcpclient_disconnect(tcp);
	}
	return 0;
}

static int create_noblock_socket_fd(const char *ip_addr, int port)
{
	struct sockaddr_in servaddr;
	int ret, flags, fd;

	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		LOG_DBG("%s: socket creation failed...\n", __func__);
		return -1;
	}

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family      = AF_INET;
	servaddr.sin_port        = htons(port);
	servaddr.sin_addr.s_addr = inet_addr(ip_addr);
	ret = connect(fd, (struct sockaddr*)&servaddr, sizeof(struct sockaddr));
	if (0 != ret) {
		LOG_DBG("%s: connect failed, %d\n", __func__, ret);
		return ret;
	}

	// set no-block mode
	flags = fcntl(fd, F_GETFL, 0);
	ret = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	if (0 != ret) {
		LOG_DBG("%s: fcntl() failed, %d\n", __func__, ret);
		return ret;
	}

	return fd;
}

static void fd_read_event_handler(uev_t *w, void *arg, int events)
{
	sktcpclient_t *tcp = (sktcpclient_t *) arg;
	int n;

	n = read(tcp->fd, tcp->rxbuffer, tcp->cfg.buffer_size);
	if (n == 0) { // connection closed
		LOG_DBG("sktcpclient: read() return 0\n");
		sktcpclient_disconnect(tcp);
	} else if (n == -1) { // connection error
		if (errno == EAGAIN) {
			return;
		}
		LOG_ERR("sktcpclient: read() fail: %s\n", strerror(errno));
		sktcpclient_disconnect(tcp);
	} else {
		if (tcp->cfg.recv_cb) {
			tcp->cfg.recv_cb(tcp, tcp->rxbuffer, n);
		}
	}
}

int sktcpclient_connect(sktcpclient_t *tcp, const char *ip_addr, int port)
{
	int fd, ret, rc = 0;

	pthread_mutex_lock(&tcp->lock);
	fd = create_noblock_socket_fd(ip_addr, port);
	if (fd < 0) {
		LOG_DBG("sktcpclient: create_noblock_socket_fd() failed %d\n", fd);
		rc = -1;
		goto err_exit;
	}

	ret = uev_io_init(tcp->cfg.ctx, &tcp->uev_watcher, fd_read_event_handler, tcp, fd, UEV_READ);
	if (0 != ret) {
		LOG_DBG("sktcpclient: uev_io_init() failed %d\n", ret);
		rc = -1;
		goto err_exit;
	}

	tcp->fd = fd;
	tcp->state = SKTCPCLIENT_STATE_CONNECTED;
	LOG_DBG("sktcpclient: connected to %s:%d, fd=%d\n", ip_addr, port, fd);

err_exit:
	pthread_mutex_unlock(&tcp->lock);
	return rc;
}

int sktcpclient_disconnect(sktcpclient_t *tcp)
{
	int rc = 0;

	pthread_mutex_lock(&tcp->lock);
	if (tcp->state == SKTCPCLIENT_STATE_CONNECTED) {
		uev_io_stop(&tcp->uev_watcher);
		shutdown(tcp->fd, SHUT_RD);
		close(tcp->fd);
		tcp->state = SKTCPCLIENT_STATE_DISCONNECTED;
		LOG_DBG("sktcpclient: sktcpclient_disconnect()\n");
	}
	else {
		rc = -1;
	}
	pthread_mutex_unlock(&tcp->lock);
	return rc;
}

int sktcpclient_send(sktcpclient_t *tcp, const uint8_t *dat, int dat_len)
{
	int rc = 0;

	pthread_mutex_lock(&tcp->lock);
	if (tcp->state == SKTCPCLIENT_STATE_CONNECTED) {
		rc = write(tcp->fd, dat, dat_len);
	}
	else {
		LOG_VERBOSE("%s: send failed, not connected..\n", __func__);
		rc = -1;
	}
	pthread_mutex_unlock(&tcp->lock);
	return rc;
}

