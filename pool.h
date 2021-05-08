
#ifndef POOL_H_
#define POOL_H_

#include <stdint.h>

struct pool_freed{
	struct pool_freed *next_free;
};

typedef struct {
	uint32_t element_size;
	uint32_t block_size;
	uint32_t used;
	int32_t block;
	struct pool_freed *freed;
	uint32_t blocks_used;
	uint8_t **blocks;
} pool_t;

#ifdef __cplusplus
extern "C"
{
#endif

int pool_init(pool_t *p, const uint32_t element_size, const uint32_t block_size);
void pool_deinit(pool_t *p);

void *pool_malloc(pool_t *p);
void pool_free(pool_t *p, void *ptr);

void pool_free_all(pool_t *p);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* POOL_H_ */
