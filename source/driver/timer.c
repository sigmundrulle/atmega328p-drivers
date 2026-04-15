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

    /** Id, 0,1 or 2, for timer0, timer1 or timer2 respectively */
    uint8_t id;

    /** Callback function to be called when timer expires */
    timer_callback callback;
} timer_t;

static void timer_init_attributes(timer_t* self, const uint8_t id)
{
    self->id = id;

    if (0U == id)
    {
        TCCR0A  = (1 << WGM01);
        TCCR0B  = (1 << CS01) | (1 << CS00);
        OCR0A   = 249;
        TIMSK0 |= (1 << OCIE0A);
    }
    else if (1U == id)
    {
        TCCR1A  = 0U;
        TCCR1B  = (1 << WGM12) | (1 << CS11) | (1 << CS10);
        OCR1A   = 249;
        TIMSK1 |= (1 << OCIE1A);
    }
    else if (2U == id)
    {
        TCCR2A  = (1 << WGM21);
        TCCR2B  = (1 << CS22);
        OCR2A   = 249;
        TIMSK2 |= (1 << OCIE2A); 
    }
    sei();
}

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

    timer_init_attributes(self, (uint8_t)timer_id);

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
    timer->is_enabled = false;

    if (timer->id == 0)      { TIMSK0 &= ~(1 << OCIE0A); }
    else if (timer->id == 1) { TIMSK1 &= ~(1 << OCIE1A); }
    else if (timer->id == 2) { TIMSK2 &= ~(1 << OCIE2A); }

    // Clear the instance from the array of instances.
    instances[timer->id] = NULL; 

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
    if (NULL == self) { return false; }
    return self->is_enabled;
}

// -----------------------------------------------------------------------------
void timer_start(timer_t* self)
{
    // If the timer instance is invalid or already enabled, terminate the function.
    if (NULL == self || self->is_enabled) { return; }

    // Start the timer by setting the enabled state to true.
    self->is_enabled = true;
}

// -----------------------------------------------------------------------------
void timer_stop(timer_t* self)
{
    // If the timer instance is invalid or already disabled, terminate the function.
    if (NULL == self || !self->is_enabled) { return; }

    // Stop the timer by setting the enabled state to false.
    self->is_enabled = false;
}

// -----------------------------------------------------------------------------
void timer_toggle(timer_t* self)
{
    // If the timer instance is invalid, terminate the function.
    if (NULL == self) { return; }

    // Toggle the timer's state
    self->is_enabled = !self->is_enabled;
}

// -----------------------------------------------------------------------------
void timer_restart(timer_t* self)
{
    // If the timer instance is invalid, terminate the function.
    if (NULL == self) { return; }

    // Reset the counter and restart the timer.
    self->elapsed_ms = 0U;
    self->is_enabled = true;
}

// -----------------------------------------------------------------------------
void timer_reset(timer_t* self)
{
    // If the timer instance is invalid, terminate the function.
    if (NULL == self) { return; }

    // Stop the timer and reset the counter.
    self->is_enabled = false;
    self->elapsed_ms = 0U;

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
    // If the timer instance is invalid or new timeout is invalid, terminate the function.
    if (NULL == self || 0U == timeout_ms) { return; }

    // Set new timeout and reset counter.
    self->timeout_ms = timeout_ms;
    self->elapsed_ms = 0U;
}

static void timer_tick(uint8_t id)
{
    timer_t* t = instances[id];
    if (NULL != t && t->is_enabled)
    {
        t->elapsed_ms++;
        if (t->elapsed_ms >= t->timeout_ms)
        {
            t->elapsed_ms = 0U;
            if (NULL != t->callback) { t->callback(); }
        }
    }
}

ISR(TIMER0_COMPA_vect) { timer_tick(0); }
ISR(TIMER1_COMPA_vect) { timer_tick(1); }
ISR(TIMER2_COMPA_vect) { timer_tick(2); }