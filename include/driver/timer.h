#ifndef TIMER_H_
#define TIMER_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Timer driver for ATmega328p.
 */
typedef struct timer timer_t;

/** Alias for timer callbacks. */
typedef void (*timer_callback)(void);

timer_t* timer_new(uint32_t timeout_ms, timer_callback callback);

void timer_delete(timer_t** self);

bool timer_is_enabled(const timer_t* self);

void timer_start(timer_t* self);

void timer_stop(timer_t* self);

void timer_toggle(timer_t* self);

void timer_restart(timer_t* self);

void timer_reset(timer_t* self);

uint32_t timer_timeout_ms(const timer_t* self);

void timer_set_timeout_ms(timer_t* self, uint32_t timeout_ms);

#endif /* TIMER_H_ */