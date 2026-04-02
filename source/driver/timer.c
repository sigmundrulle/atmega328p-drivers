#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "driver/timer.h"
#include "driver/utils.h"


typedef struct timer
{
} timer_t;

typedef void (*timer_callback)(void)
{
}

timer_t* timer_new(uint32_t timeout_ms, timer_callback callback)
{
}

void timer_delete(timer_t** self)
{
}

bool timer_is_enabled(const timer_t* self)
{
}

void timer_start(timer_t* self)
{
}

void timer_stop(timer_t* self)
{
}

void timer_toggle(timer_t* self)
{
}

void timer_restart(timer_t* self)
{
}

void timer_reset(timer_t* self)
{
}

uint32_t timer_timeout_ms(const timer_t* self)
{
}

void timer_set_timeout_ms(timer_t* self, uint32_t timeout_ms)
{
}