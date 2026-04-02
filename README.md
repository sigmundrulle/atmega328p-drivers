# L03 – Anteckningar

## Beskrivning
Slutförd implementering av en GPIO-driver med hjälp av en struktur med namnet `gpio_t`.

---

## Filer
* [driver/gpio.h](./include/driver/gpio.h):
    * Innehåller GPIO-driverns publika interface i form av funktioner samt enumerationen `gpio_direction_t`.
    * Strukturen `gpio_t` är fördeklarerad i denna fil för att hålla medlemsvariablerna interna (strukturen är därmed `opaque`).
* [driver/gpio.c](./source/driver/gpio.c):
    * Innehåller implementationsdetaljer för GPIO-drivern, såsom funktionsdefinitioner samt definition av strukturen `gpio`.
* [driver/utils.h](./include/driver/utils.h):
    * Innehåller funktionsliknande makron för att sätta, nollställa, toggla och läsa pinnar i register.
* [main.c](./main.c):
    * Innehåller ett demonstrationsprogram där GPIO-enheter initieras och används.
    * En callback används för att toggla en lysdiod vid nedtryckning av en tryckknapp.

---
