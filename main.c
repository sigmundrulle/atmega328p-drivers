/**
 * @brief Demonstrate GPIO usage with interrupt-driven callback handling.
 */
#include <stddef.h>

#include "driver/gpio.h"

/** Use GPIO pin 9 for the LED. */
#define LED_PIN 9U

/** Use GPIO pin 13 for the button. */
#define BUTTON_PIN 13U

/** GPIO devices (file-global so they can be accessed from the interrupt callback). */
static gpio_t *led, *button;

/**
 * @brief Handle button event.
 *
 *        Toggle the LED when the button is pressed.
 */
static void button_event(void)
{
    if (gpio_read(button)) { gpio_toggle(led); }
}

/**
 * @brief Initialize GPIO and start the application.
 */
int main(void)
{
    // Initialize LED GPIO (output, no callback).
    led = gpio_new(LED_PIN, GPIO_DIRECTION_OUTPUT, NULL);

    // Initialize button GPIO (input with pull-up, use callback).
    button = gpio_new(BUTTON_PIN, GPIO_DIRECTION_INPUT_PULLUP, button_event);

    // Enable pin change interrupt for the button.
    gpio_enable_pci(button);

    // Keep program running.
    while (1) {}
}
