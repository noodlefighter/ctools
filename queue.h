/*
 * queue.h
 *
 *  Created on: 2016-5-4
 *      Author: Ming180
 */

#ifndef QUEUE_H_
#define QUEUE_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
	uint16_t num_items;
	uint16_t head;
	uint16_t size;
	uint8_t *buff;
} queue_t;

#define QUEUE_INIT(_buff, _buff_size)    (queue_t){ .buff = _buff, .size = _buff_size }

bool queue_clear(queue_t *queue);
bool queue_append(queue_t *queue, uint8_t item);
bool queue_get(queue_t *queue, uint8_t *item);
uint16_t queue_num_items(queue_t *queue);
uint16_t queue_num_available(queue_t *queue);
uint16_t queue_enqueue(queue_t *queue, uint8_t *data, uint16_t size);
uint16_t queue_peek_consequent(queue_t *queue, uint8_t *data, uint16_t size);
uint16_t queue_peek_noncopy(queue_t *queue, uint8_t **data);
uint16_t queue_release(queue_t *queue, uint16_t size);

#endif  // QUEUE_H_
