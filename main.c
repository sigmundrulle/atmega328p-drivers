/**
 * @brief Application logic for Timer-driven LED and Button Debounce.
 */
#include <stddef.h>
#include <avr/interrupt.h> // För sei()

#include "driver/gpio.h"
#include "driver/timer.h"

/* --- Konfiguration --- */
#define LED_PIN 8U          // LED enligt instruktion
#define BUTTON_PIN 13U      // Knapp enligt instruktion
#define BLINK_INTERVAL 100U // 100ms blink
#define DEBOUNCE_TIME 300U  // 300ms debounce (VG-krav)

/* --- Globala instanser --- */
static gpio_t *led    = NULL;
static gpio_t *button = NULL;
static timer_t *blink_timer    = NULL;
static timer_t *debounce_timer = NULL;

/**
 * @brief Timer Callback: Togglar lysdioden.
 * Enligt krav: Kontrollerar att timern faktiskt är enabled innan toggle.
 */
static void led_callback(void)
{
    if (timer_is_enabled(blink_timer))
    {
        gpio_toggle(led);
    }
}

/**
 * @brief Debounce Callback (VG-krav):
 * Körs 300ms efter ett knapptryck för att återaktivera interrupts.
 */
static void debounce_callback(void)
{
    timer_stop(debounce_timer);
    gpio_enable_pci(button); // Återaktivera interrupt för knappen
}

/**
 * @brief Button Callback:
 * Togglar blink-timern och hanterar debounce-låsning.
 */
static void button_event(void)
{
    // Läs av knappen (Low = Press pga Pullup)
    if (!gpio_read(button))
    {
        // 1. Inaktivera interrupt på porten (VG-krav)
        // OBS: Detta förutsätter att gpio_disable_pci finns i din driver, 
        // annars kan man manuellt rensa biten i PCICR.
        PCICR &= ~(1 << PCIE0); 

        // 2. Toggle blink-timern
        timer_toggle(blink_timer);

        // 3. Om timern stängdes av -> Släck LED direkt (Krav)
        if (!timer_is_enabled(blink_timer))
        {
            gpio_write(led, 0U);
        }

        // 4. Starta debounce-timern för att låsa upp om 300ms
        timer_restart(debounce_timer);
    }
}

int main(void)
{
    // 1. Initiera hårdvara (GPIO)
    led    = gpio_new(LED_PIN, GPIO_DIRECTION_OUTPUT, NULL);
    button = gpio_new(BUTTON_PIN, GPIO_DIRECTION_INPUT_PULLUP, button_event);

    // 2. Skapa timers
    // Timer för att blinka LED var 100:e ms
    blink_timer = timer_new(BLINK_INTERVAL, led_callback);
    
    // Timer för debounce (VG-krav)
    debounce_timer = timer_new(DEBOUNCE_TIME, debounce_callback);

    // 3. Aktivera knapp-interrupt
    gpio_enable_pci(button);
    
    // Aktivera globala avbrott (viktigt för att timers ska gå)
    sei();

    while (1)
    {
        // Main loopen är tom, allt sköts via callbacks (interrupt-driven)
    }

    return 0;
}