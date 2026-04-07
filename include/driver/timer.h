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

/** 
 * @brief Create a new timer instance.
 * 
 * @param[in] timeout_ms The timeout in milliseconds.
 * @param[in] callback The callback function to be called when the timer expires.
 * 
 * @return A pointer to the new timer instance, or NULL if creation fails.
 */
timer_t* timer_new(uint32_t timeout_ms, timer_callback callback);

/** 
 * @brief Delete a timer instance.
 * 
 * @param[in] self A pointer to the timer instance to delete.
 */
void timer_delete(timer_t** self);

/** 
 * @brief Check if a timer instance is enabled.
 * 
 * @param[in] self A pointer to the timer instance.
 * 
 * @return true if the timer is enabled, false otherwise.
 */
bool timer_is_enabled(const timer_t* self);

/** 
 * @brief Start a timer instance.
 * 
 * @param[in] self A pointer to the timer instance to start.
 */
void timer_start(timer_t* self);

/** 
 * @brief Stop a timer instance.
 * 
 * @param[in] self A pointer to the timer instance to stop.
 */
void timer_stop(timer_t* self);

/** 
 * @brief Toggle a timer instance.
 * 
 * @param[in] self A pointer to the timer instance to toggle.
 */
void timer_toggle(timer_t* self);

/** 
 * @brief Restart a timer instance.
 * 
 * @param[in] self A pointer to the timer instance to restart.
 */
void timer_restart(timer_t* self);

/** 
 * @brief Reset a timer instance.
 * 
 * @param[in] self A pointer to the timer instance to reset.
 */
void timer_reset(timer_t* self);

/** 
 * @brief Get the timeout value of a timer instance.
 * 
 * @param[in] self A pointer to the timer instance.
 * 
 * @return The timeout value in milliseconds, or 0 if the timer instance is invalid.
 */
uint32_t timer_timeout_ms(const timer_t* self);

/** 
 * @brief Set the timeout value of a timer instance.
 * 
 * @param[in] self A pointer to the timer instance.
 * @param[in] timeout_ms The new timeout value in milliseconds.
 */
void timer_set_timeout_ms(timer_t* self, uint32_t timeout_ms);

#endif /* TIMER_H_ */