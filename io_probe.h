#pragma once
#include <stdint.h>

typedef struct {
    intptr_t pin;
    void (*io_func)(intptr_t pin, int set);
    volatile int value;
} io_probe_t;

static inline int io_probe_entry(io_probe_t *obj)
{
    if (obj->value == 0) {
        obj->io_func(obj->pin, 1);
    }
    return obj->value++;
}

static inline void io_probe_exit(io_probe_t *obj, int value)
{
    if (--obj->value == 0) {
        obj->io_func(obj->pin, 0);
    }
}

