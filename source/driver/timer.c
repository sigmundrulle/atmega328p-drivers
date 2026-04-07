#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "driver/timer.h"
#include "driver/utils.h"

static uint8_t timer_reg = 0U; // Timer registry
static timer_t* instances[3] = {NULL, NULL, NULL}; // Array of timer instances (for timer interrupt handlers)

// -----------------------------------------------------------------------------
typedef struct timer
{
    /** Timeout in milliseconds */
    uint32_t timeout_ms;

    /** Elapsed time in milliseconds */
    uint32_t elapsed_ms;

    /** Timer is enabled = true, disabled = false */
    bool is_enabled;

    // Id, 0,1 or 2, for timer0, timer1 or timer2 respectively.
    uint8_t id;

    /** Callback function to be called when timer expires */
    timer_callback callback;
} timer_t;

// -----------------------------------------------------------------------------
timer_t* timer_new(uint32_t timeout_ms, timer_callback callback)
{
    // Check if parameters are valid 
    if (NULL == callback || timeout_ms < 1U) { return NULL; }

    // Find an available timer instance 
    int8_t timer_id = -1;
    for (uint8_t i = 0U; i < 3U; i++)
    {
        if (!READ(timer_reg, i))
        {
            timer_id = i;
            break;
        }
    }

    // If no available timer instance is found, return NULL 
    if (timer_id < 0) { return NULL; }

    timer_t* self = (timer_t*)(malloc(sizeof(timer_t)));
    if (NULL == self) { return NULL; }

    // Initialize the timer instance 
    self->timeout_ms = timeout_ms;
    self->elapsed_ms = 0U;
    self->is_enabled = false;
    self->id         = timer_id;
    self->callback   = callback;

    // Store the instance for interrupt handlers
    instances[timer_id] = self; 

    // Mark the timer as used in the registry
    SET(timer_reg, timer_id); 

    return self;
}

// -----------------------------------------------------------------------------
void timer_delete(timer_t** self)
{
    // Check the GPIO instance, terminate the function if invalid.
    if (NULL == self) { return; }

    // Retrieve the GPIO implementation, terminate the function if invalid.
    timer_t* timer = *self;
    if (NULL == timer) { return; }

    // Reset/clear used hardware.
    

    // Release the pin from the pin registry.
    CLEAR(timer_reg, timer->id);

    // Free allocated memory and set the associated pointer to NULL.
    free(timer);
    *self = NULL;
}

// -----------------------------------------------------------------------------
bool timer_is_enabled(const timer_t* self)
{
    // Return false if the timer instance is invalid, otherwise return the timer's enabled state.
    if (NULL == self) { return true; }
    return self->is_enabled;
}

// -----------------------------------------------------------------------------
void timer_start(timer_t* self)
{
}

// -----------------------------------------------------------------------------
void timer_stop(timer_t* self)
{
}

// -----------------------------------------------------------------------------
void timer_toggle(timer_t* self)
{
}

// -----------------------------------------------------------------------------
void timer_restart(timer_t* self)
{
}

// -----------------------------------------------------------------------------
void timer_reset(timer_t* self)
{
}

// -----------------------------------------------------------------------------
uint32_t timer_timeout_ms(const timer_t* self)
{
    // Return 0 if the timer instance is invalid, otherwise return the timer's timeout value.
    if (NULL == self) { return 0U; }
    return self->timeout_ms;
}

// -----------------------------------------------------------------------------
void timer_set_timeout_ms(timer_t* self, uint32_t timeout_ms)
{
}