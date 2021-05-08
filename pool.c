#include <string.h>
#include <stdlib.h>
#include <errno.h>

#include "pool.h"

#define POOL_BLOCKS_INITIAL 1

#ifndef max
#define max(a,b) ((a)<(b)?(b):(a))
#endif

int pool_init(pool_t *p, const uint32_t element_size, const uint32_t block_size)
{
	uint32_t i;

	p->element_size = max(element_size, sizeof(struct pool_freed));
	p->block_size = block_size;
	
	pool_free_all(p);

	p->blocks_used = POOL_BLOCKS_INITIAL;
	p->blocks = malloc(sizeof(uint8_t*)* p->blocks_used);
	if (NULL == p->blocks) {
		return -ENOMEM;
	}

	for(i = 0; i < p->blocks_used; ++i)
		p->blocks[i] = NULL;

	return 0;
}

void pool_deinit(pool_t *p)
{
	uint32_t i;
	for(i = 0; i < p->blocks_used; ++i) {
		if(p->blocks[i] == NULL)
			break;
		else
			free(p->blocks[i]);
	}

	free(p->blocks);
}

void *pool_malloc(pool_t *p)
{
	if(p->freed != NULL) {
		void *recycle = p->freed;
		p->freed = p->freed->next_free;
		return recycle;
	}

	if(++p->used == p->block_size) {
		p->used = 0;
		if(++p->block == (int32_t)p->blocks_used) {
			uint32_t i;

			p->blocks_used <<= 1;
			p->blocks = realloc(p->blocks, sizeof(uint8_t*)* p->blocks_used);

			for(i = p->blocks_used >> 1; i < p->blocks_used; ++i)
				p->blocks[i] = NULL;
		}

		if(p->blocks[p->block] == NULL)
			p->blocks[p->block] = malloc(p->element_size * p->block_size);
	}
	
	return p->blocks[p->block] + p->used * p->element_size;
}

void pool_free(pool_t *p, void *ptr)
{
	struct pool_freed *pfreed = p->freed;

	p->freed = ptr;
	p->freed->next_free = pfreed;
}

void pool_free_all(pool_t *p)
{
	p->used = p->block_size - 1;
	p->block = -1;
	p->freed = NULL;
}
