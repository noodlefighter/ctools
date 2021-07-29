#include "io_probe.h"

void io_func(intptr_t pin, int set)
{
    io_write_pin(pin, set);
}

io_probe_t probe_pin_0 = {
    .pin = 1,
    .io_func = io_func,
};
io_probe_t probe_pin_1 = {
    .pin = 2,
    .io_func = io_func,
};
