/*
 * queue.c
 *
 *  Created on: 2016-5-4
 *      Author: Ming180
 */

#include "queue.h"
#include "os_cpu.h"
#include <string.h>

// clear - removes all points from the curve
bool queue_clear(queue_t *queue)
{
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();
	queue->num_items = 0;
	queue->head = 0;
	OS_EXIT_CRITICAL();

	return true;
}

// add - adds an item to the buffer.  returns TRUE if successfully added
bool queue_append(queue_t *queue, uint8_t item)
{
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();

	// determine position of new item
	uint16_t tail = queue->head + queue->num_items;

	if (tail >= queue->size) {
		tail -= queue->size;
	}

	// increment number of items
	if (queue->num_items < queue->size) {
		queue->num_items++;
	}
	else {
		// // no room for new items so drop oldest item
		// queue->head++;
		// if (queue->head >= queue->size) {
		// 	queue->head = 0;
		// }

		OS_EXIT_CRITICAL();
		return false;
	}

	// add item to buffer
	queue->buff[tail] = item;
	OS_EXIT_CRITICAL();

	return true;
}

// get - returns the next value in the buffer
bool queue_get(queue_t *queue, uint8_t *item)
{
	OS_CPU_SR cpu_sr;

	OS_ENTER_CRITICAL();

	// return zero if buffer is empty
	if (queue->num_items == 0) {
		OS_EXIT_CRITICAL();
		return false;
	}

	// get next value in buffer
	*item = queue->buff[queue->head];

	// increment to next point
	queue->head++;
	if (queue->head >= queue->size) {
		queue->head = 0;
	}

	// reduce number of items
	queue->num_items--;
	OS_EXIT_CRITICAL();

	// return item
	return true;
}

uint16_t queue_num_items(queue_t *queue)
{
	return queue->num_items;
}

uint16_t queue_num_available(queue_t *queue)
{
	return queue->size - queue->num_items;
}

uint16_t queue_enqueue(queue_t *queue, uint8_t *data, uint16_t size)
{
	OS_CPU_SR cpu_sr;
	uint16_t tail, temp;

	OS_ENTER_CRITICAL();

	if (size <= queue->size - queue->num_items) {

		// determine position of new item
		tail = queue->head + queue->num_items;

		if (tail >= queue->size) {
			tail -= queue->size;
		}

		temp = queue->size - tail;
		if (size < temp) {
			memcpy(&queue->buff[tail], data, size);
		}
		else {
			memcpy(&queue->buff[tail], &data[0]   , temp);
			memcpy(&queue->buff[0]   , &data[temp], size - temp);
		}

		queue->num_items += size;
	}
	else {
		size = 0;
	}

	OS_EXIT_CRITICAL();

	return size;
}

uint16_t queue_peek_consequent(queue_t *queue, uint8_t *data, uint16_t size)
{
	OS_CPU_SR cpu_sr;
	uint16_t tail, temp;

	// 从队列中获取连续的数据，不回环
	OS_ENTER_CRITICAL();

	if (size > queue->num_items) {
		size = queue->num_items;
	}

	// 缓冲边界检测
	tail = queue->head + queue->num_items;
	if (tail > queue->size) {
		temp = queue->size - queue->head;

		if (size > temp) {
			size = temp;
		}
	}

	memcpy(data, &queue->buff[queue->head], size);

	OS_EXIT_CRITICAL();

	return size;
}

// 从队列中获取连续的数据指针，不回环
uint16_t queue_peek_noncopy(queue_t *queue, uint8_t **data)
{
	OS_CPU_SR cpu_sr;
	uint16_t tail, size;

	OS_ENTER_CRITICAL();

	tail = queue->head + queue->num_items;
	if (tail > queue->size) {
		size = queue->size - queue->head;
	}
	else {
		size = queue->num_items;
	}

	*data = &queue->buff[queue->head];
	OS_EXIT_CRITICAL();

	return size;
}

uint16_t queue_release(queue_t *queue, uint16_t size)
{
	OS_CPU_SR cpu_sr;

	// 从队列中获取连续的数据，不回环
	OS_ENTER_CRITICAL();

	if (size > queue->num_items) {
		size = queue->num_items;
	}

	// increment to next point
	queue->head += size;
	if (queue->head >= queue->size) {
		queue->head -= queue->size;
	}

	queue->num_items -= size;

	OS_EXIT_CRITICAL();

	return size;
}
