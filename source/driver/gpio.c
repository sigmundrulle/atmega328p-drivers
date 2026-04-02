/**
 * @brief Implementation details for ATmega328p GPIO driver.
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <avr/io.h>
#include <avr/interrupt.h>

#include "driver/gpio.h"
#include "driver/utils.h"

/** Number of available pins. */
#define PIN_COUNT 20U

/** Pin offset for I/O port B. */
#define PIN_OFFSET_B 8U

/** Pin offset for I/O port C. */
#define PIN_OFFSET_C 14U

/** Pin offset for I/O port D. */
#define PIN_OFFSET_D 0U

/** Number of avilable ports. */
#define PORT_COUNT 3U

/** Port offset for I/O port B. */
#define PORT_OFFSET_B 0U

/** Port offset for I/O port C. */
#define PORT_OFFSET_C 1U

/** Port offset for I/O port D. */
#define PORT_OFFSET_D 2U

/**
 * @brief GPIO driver structure.
 */
typedef struct gpio
{
    /** Pointer to the data direction register. */
    volatile uint8_t* ddrx;

    /** Pointer to the port register. */
    volatile uint8_t* portx;

    /** Pointer to the pin register. */
    volatile uint8_t* pinx;

    /** Pointer to the pin change interrupt mask register. */
    volatile uint8_t* pcmskx;

    /** Pin change interrupt control bit. */
    uint8_t pcicx;

    /** Pin number on the associated port. */
    uint8_t pin;

    /** Pin ID (for reserving the pin). */
    uint8_t id;
} gpio_t;

/** GPIO callback. */
typedef void (*gpio_callback_t)(void);

/** Pin registry (1 = pin is reserved, 0 = pin is free). */
static uint32_t pin_reg = 0U;

/** Array of callbacks */
static gpio_callback_t callback_reg[PORT_COUNT] = {NULL, NULL, NULL};

// -----------------------------------------------------------------------------
static inline bool is_pin_free(const uint8_t pin)
{
    // Return true if the pin number is valid and the pin is free.
    return PIN_COUNT > pin ? !READ(pin_reg, pin) : false;
}

// -----------------------------------------------------------------------------
static inline bool is_direction_valid(const gpio_direction_t direction)
{
    // Cast to uint8_t to avoid negative integers.
    return (uint8_t)(GPIO_DIRECTION_OUTPUT) >= (uint8_t)(direction);
}

// -----------------------------------------------------------------------------
static inline void invoke_callback(const uint8_t port)
{
    // Invoke callback for I/O port B if registered.
    if ((PORT_COUNT > port) &&  (NULL != callback_reg[port])) { callback_reg[port](); }
}

// -----------------------------------------------------------------------------
static void gpio_init_attributes(gpio_t* self, const uint8_t pin)
{
    // Store pin ID (for pin registration).
    self->id = pin;

    // PIN 0 - 7 => I/O port D, pin = ID.
    if (PIN_OFFSET_B > pin)
    {
        self->ddrx   = &DDRD;
        self->portx  = &PORTD;
        self->pinx   = &PIND;
        self->pin    = pin - PIN_OFFSET_D;
        self->pcmskx = &PCMSK2;
        self->pcicx  = PCIE2;
    }
    // PIN 8 - 13 => I/O port B, pin = ID - 8.
    else if (PIN_OFFSET_C > pin)
    {
        self->ddrx   = &DDRB;
        self->portx  = &PORTB;
        self->pinx   = &PINB;
        self->pin    = pin - PIN_OFFSET_B;
        self->pcmskx = &PCMSK0;
        self->pcicx  = PCIE0;
    }

    // PIN 14 - 19 => I/O port C, pin = ID - 14.
    else if (PIN_COUNT > pin)
    {
        self->ddrx   = &DDRC;
        self->portx  = &PORTC;
        self->pinx   = &PINC;
        self->pin    = pin - PIN_OFFSET_C;
        self->pcmskx = &PCMSK1;
        self->pcicx  = PCIE1;
    }
}

// -----------------------------------------------------------------------------
static void gpio_set_direction(gpio_t* self, const gpio_direction_t direction)
{
    // Set GPIO direction.
    switch (direction)
    {
        case GPIO_DIRECTION_INPUT_PULLUP:
        {
            // Configure as input by clearing the corresponding bit in DDRx.
            // Enable pull-up resistor by setting the corresponding bit in PORTx.
            CLEAR(*self->ddrx, self->pin);
            SET(*self->portx, self->pin);
            break;
        }
        case GPIO_DIRECTION_OUTPUT:
        {
            // Configure as output by setting the corresponding bit in DDRx.
            SET(*self->ddrx, self->pin);
            CLEAR(*self->portx, self->pin);
            break;
        }
        default:
        {
            // Configure as input by clearing the corresponding bit in DDRx.
            CLEAR(*self->portx, self->pin);
            CLEAR(*self->ddrx, self->pin);
            break;
        }
    }
}

// -----------------------------------------------------------------------------
static void gpio_set_callback(gpio_t* self, void (*callback)(void))
{
    // Register callback if valid.
    if (NULL != callback) 
    { 
        const uint8_t offset = self->pcicx;
        callback_reg[offset] = callback;
    }
}

// -----------------------------------------------------------------------------
gpio_t* gpio_new(const uint8_t pin, const gpio_direction_t direction, void (*callback)(void))
{
    // Check the pin, return NULL if reserved or invalid.
    if (!is_pin_free(pin)) { return NULL; }

    // Check the data direction, return a nullptr if invalid.
    if (!is_direction_valid(direction)) { return NULL; }

    // Allocate memory for a new GPIO, return a nullptr on failure.
    gpio_t* self = (gpio_t*)(malloc(sizeof(gpio_t)));
    if (NULL == self) { return NULL; }

    // Initialize GPIO.
    gpio_init_attributes(self, pin);
    gpio_set_direction(self, direction);
    gpio_set_callback(self, callback);

    // Register pin, then return a pointer to the GPIO.
    SET(pin_reg, self->id);
    return self;
}

// -----------------------------------------------------------------------------
void gpio_delete(gpio_t** self)
{
    // Check the GPIO instance, terminate the function if invalid.
    if (NULL == self) { return; }

    // Retrieve the GPIO implementation, terminate the function if invalid.
    gpio_t* gpio = *self;
    if (NULL == gpio) { return; }

    // Reset/clear used hardware (DDRx and PORTx).
    CLEAR(*gpio->ddrx, gpio->pin);
    CLEAR(*gpio->portx, gpio->pin);

    // Release the pin from the pin registry.
    CLEAR(pin_reg, gpio->id);

    // Free allocated memory and set the associated pointer to NULL.
    free(gpio);
    *self = NULL;
}

// -----------------------------------------------------------------------------
void gpio_write(gpio_t* self, const bool state)
{
    // Terminate the function is the GPIO is invalid.
    if (NULL == self) { return; }

    // Check data direction, terminate the function if not configured as output.
    if (!READ(*(self->ddrx), self->pin)) { return; }

    // Check the desired state, set/clear the bit in PORTx.
    if (state) { SET(*self->portx, self->pin); }
    else { CLEAR(*self->portx, self->pin); }
}

// -----------------------------------------------------------------------------
bool gpio_read(const gpio_t* self)
{
    // Read PINx if the GPIO is valid, otherwise return false.
    return NULL != self ? READ(*self->pinx, self->pin) : false;
}

// -----------------------------------------------------------------------------
void gpio_toggle(gpio_t* self)
{
    // Terminate the function is the GPIO is invalid.
    if (NULL == self) { return; }

    // Check data direction, terminate the function if not configured as output.
    if (!READ(*self->ddrx, self->pin)) { return; }

    // Toggle the output by setting the pin in PINx (the hardware will toggle the output).
    SET(*self->pinx, self->pin);
}

// -----------------------------------------------------------------------------
void gpio_enable_pci(gpio_t* self)
{
    // Terminate the function is the GPIO is invalid.
    if (NULL == self) { return; }

    // Enable interrupts on the associated I/O-port.
    SET(PCICR, self->pcicx);

    // Enable interrupts on the pin.
    SET(*self->pcmskx, self->pin);

    // Enable interrupts globally.
    sei();
}

// -----------------------------------------------------------------------------
void gpio_disable_pci(gpio_t* self, bool disable_port)
{
    // Terminate the function is the GPIO is invalid.
    if (NULL == self) { return; }

    // Disable interrupts on the pin.
    CLEAR(*self->pcmskx, self->pin);

    // Disable interrupts on the entire I/O-port if specified.
    if (disable_port) { CLEAR(PCICR, self->pcicx); }
}

// -----------------------------------------------------------------------------
ISR(PCINT0_vect)
{
    // Invoke callback for I/O port B if registered.
    invoke_callback(PORT_OFFSET_B);
}

// -----------------------------------------------------------------------------
ISR(PCINT1_vect)
{
    // Invoke callback for I/O port C if registered.
    invoke_callback(PORT_OFFSET_C);
}

// -----------------------------------------------------------------------------
ISR(PCINT2_vect)
{
    // Invoke callback for I/O port D if registered.
    invoke_callback(PORT_OFFSET_D);
}
