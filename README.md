# GPIO & Timer Drivers for ATmega328P

[![Build](https://github.com/sigmundrulle/gpio-timerdriver/actions/workflows/build.yml/badge.svg)](https://github.com/sigmundrulle/gpio-timerdriver/actions/workflows/build.yml)

Bare-metal C drivers for the ATmega328P (Arduino Uno), written without the Arduino libraries – just registers, interrupts and a small object-style API.

The demo application blinks an LED every 100 ms and lets a push button start/stop the blinking, with a software debounce timer to filter out contact bounce.

## Features

**GPIO driver** (`driver/gpio.h`)
- Maps Arduino pin numbers (0–19) to the correct port registers (B, C, D)
- Input, input with pull-up, and output modes
- Read, write and toggle (toggle uses the hardware `PINx` trick)
- Pin change interrupts with a user callback per I/O port
- Pin reservation – the same pin can't be claimed twice

**Timer driver** (`driver/timer.h`)
- Up to three software timers, each backed by one of the hardware timers (Timer0/1/2) running in CTC mode with a 1 ms tick
- Configurable timeout in milliseconds and a callback that runs on expiry
- Start, stop, toggle, restart, reset and change timeout at runtime
- Input validation: `timer_new()` returns `NULL` for a zero timeout, a missing callback, or when all hardware timers are in use
- Freed timers release their hardware timer so it can be reused

## Design

Both drivers use **opaque structs**: the header only exposes `typedef struct timer timer_t;`, while the struct definition lives in the `.c` file. Application code can only work through the public functions, so the internal representation can change without breaking callers.

The timer driver knows nothing about GPIO. The LED and button logic lives in `main.c` and is connected through callbacks:

```
Button press ──► PCINT ISR ──► button_event()
                                 ├─ toggle blink timer
                                 ├─ turn LED off if blinking stopped
                                 └─ disable button interrupts, start 300 ms debounce timer

Blink timer (100 ms) ──► led_callback()      ──► toggle LED
Debounce timer (300 ms) ──► debounce_callback() ──► re-enable button interrupts
```

## Hardware

| Component    | Arduino pin | AVR pin |
|--------------|-------------|---------|
| LED          | 8           | PB0     |
| Push button  | 13          | PB5     |

The button connects the pin to GND (internal pull-up enabled, active low). Timer settings assume a 16 MHz clock.

## Example

```c
#include "driver/gpio.h"
#include "driver/timer.h"

static gpio_t*  led   = NULL;
static timer_t* blink = NULL;

static void on_blink(void) { gpio_toggle(led); }

int main(void)
{
    led   = gpio_new(8U, GPIO_DIRECTION_OUTPUT, NULL);
    blink = timer_new(100U, on_blink);
    timer_start(blink);

    while (1) {}
}
```

## Project structure

```
include/driver/   Public headers (gpio.h, timer.h, utils.h)
source/driver/    Driver implementations
main.c            Demo application (LED + button + debounce)
Makefile          avr-gcc build
```

## Building

**With avr-gcc (Linux, macOS, WSL):**

```sh
sudo apt install gcc-avr avr-libc binutils-avr   # Debian/Ubuntu
make            # build build/gpio_struct.hex
make flash      # flash to an Arduino Uno (PORT=/dev/ttyACM0 by default)
```

**With Microchip Studio (Atmel Studio 7):** open `gpio_struct.atsln`, build, and flash.

Every push is built by GitHub Actions with warnings treated as errors.

## Background

Built as a course assignment in embedded C programming. Focus areas: register-level hardware access, interrupt handling, opaque data types, input validation and Doxygen documentation.
