/**
 * @brief GPIO driver for ATmega328p.
 */
#ifndef GPIO_H_
#define GPIO_H_

#include <stdbool.h>
#include <stdint.h>

/**
 * @brief Enumeration of GPIO directions.
 */
typedef enum gpio_direction
{
    GPIO_DIRECTION_INPUT,        ///< GPIO input without internal pull-up resistor enabled.
    GPIO_DIRECTION_INPUT_PULLUP, ///< GPIO input with internal pull-up resistor enabled.
    GPIO_DIRECTION_OUTPUT,       ///< GPIO output.
} gpio_direction_t;

/**
 * @brief GPIO driver structure.
 */
typedef struct gpio gpio_t;

/**
 * @brief Create a new GPIO.
 * 
 * @param[in] pin Arduino pin number.
 * @param[in] direction Data direction.
 * @param[in] callback Optional callback. Pass NULL if none.
 * 
 * @return The initialized GPIO on success, a nullptr otherwise.
 */
gpio_t* gpio_new(uint8_t pin, gpio_direction_t direction, void (*callback)(void));

/**
 * @brief Delete GPIO instance.
 * 
 *        Release resources allocated for the GPIO.
 *        Set the associated pointer to null.
 * 
 * @param[in, out] self Reference to the GPIO instance.
 */
void gpio_delete(gpio_t** self);

/**
 * @brief Set state of the GPIO.
 * 
 * @note This operation is only supported for outputs.
 * 
 * @param[in, out] self Pointer to the GPIO. 
 * @param[in] state GPIO state (true = enabled, false = disabled).
 */
void gpio_write(gpio_t* self, bool state);

/**
 * @brief Read the state of the GPIO.
 * 
 * @param[in] self Pointer to the GPIO.
 * 
 * @return True if the GPIO is enabled, false otherwise.
 */
bool gpio_read(const gpio_t* self);

/**
 * @brief Toggle state of the GPIO.
 * 
 * @note This operation is only supported for outputs.
 * 
 * @param[in, out] self Pointer to the GPIO. 
 */
void gpio_toggle(gpio_t* self);

/**
 * @brief Enable pin change interrupt for the given GPIO.
 *
 *        Any pending interrupt flag for the I/O port is cleared first, so pin
 *        changes that occurred while interrupts were disabled are ignored.
 *
 * @param[in, out] self Pointer to the GPIO.
 */
void gpio_enable_pci(gpio_t* self);

/**
 * @brief Disable pin change interrupts for the given GPIO.
 * 
 * @param[in, out] self Pointer to the GPIO.
 * @param[in] disable_port True to disable interrupts on the entire I/O port.
 */
void gpio_disable_pci(gpio_t* self, bool disable_port);

#endif /* GPIO_H_ */
