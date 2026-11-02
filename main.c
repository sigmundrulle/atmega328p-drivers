#include <stddef.h>
#include <avr/interrupt.h>

#include "driver/gpio.h"
#include "driver/timer.h"

/** Configuration */
#define LED_PIN 8U
#define BUTTON_PIN 13U
#define BLINK_INTERVAL 100U
#define DEBOUNCE_TIME 300U

static gpio_t *led             = NULL;
static gpio_t *button          = NULL;
static timer_t *blink_timer    = NULL;
static timer_t *debounce_timer = NULL;

/**
 * @brief Callback for the LED blink timer.
 * Toggles the LED state if the blink timer is currently enabled.
 */
static void led_callback(void)
{
    if (timer_is_enabled(blink_timer))
    {
        gpio_toggle(led);
    }
}

/**
 * @brief Callback for the debounce timer.
 * Re-enables the button interrupt after the debounce period has passed.
 */
static void debounce_callback(void)
{
    timer_stop(debounce_timer);
    gpio_enable_pci(button);
}

/**
 * @brief Button event handler (ISR callback).
 * Toggles the blinking logic and handles software debouncing by 
 * temporarily disabling interrupts.
 */
static void button_event(void)
{
    // Check if the button is pressed (active low)
    if (!gpio_read(button))
    {
        // Disable pin change interrupts on the button's I/O port to prevent bouncing
        gpio_disable_pci(button, true);

        // Toggle the state of the blink timer
        timer_toggle(blink_timer);

        // Ensure LED is turned off immediately if blinking is stopped
        if (!timer_is_enabled(blink_timer))
        {
            gpio_write(led, 0U);
        }

        // Start the debounce timer to unlock interrupts after 300ms
        timer_restart(debounce_timer);
    }
}

int main(void)
{
    /** Initialize GPIO Hardware */
    led    = gpio_new(LED_PIN, GPIO_DIRECTION_OUTPUT, NULL);
    button = gpio_new(BUTTON_PIN, GPIO_DIRECTION_INPUT_PULLUP, button_event);

    // Timer for LED blinking
    blink_timer = timer_new(BLINK_INTERVAL, led_callback);
    
    // Timer for handling button debounce
    debounce_timer = timer_new(DEBOUNCE_TIME, debounce_callback);

    /** Enable System Interrupts */
    gpio_enable_pci(button);
    
    // Enable global interrupts
    sei();

    while (1)
    {
    }

    return 0;
}