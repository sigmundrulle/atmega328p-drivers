# **P01** - Timer-driver i C
# TEST LAPTOP
## Syfte
Ni ska i grupper om 1-2 implementera en timer-driver i C i form av en opaque struct med callback:
* Drivern ska kunna skapa 1–3 timers (begränsat av hårdvaran i labbmiljön).
* En knapp (GPIO) ansluten till pin 13 ska användas för att toggla en timer.
* När timern är aktiv ska den toggla en lysdiod (GPIO) ansluten till pin 8 var 100:e ms. 
* När timern är inaktiv ska lysdioden vara släckt.

**VG-krav:** Koden ska vara korrekt strukturerad (som i GPIO-exemplet), robust (valideringar, såsom NULL-checks), och dokumenterad med Doxygen.

---

## Kravspecifikation

### Funktionellt beteende
* Det ska gå att skapa 1–3 timers.
* Knappen ska toggla timern:
  * **Timer ON:** Lysdioden togglas var 100:e ms.
  * **Timer OFF:** Lysdioden ska vara släckt (och förbli så).
* Timern ska stödja callback som körs när timeouten går ut.

### API-krav
* Filstruktur:
  * `driver/timer.h` (publikt API).
  * `driver/timer.c` (implementation).
* `struct timer` ska vara **opaque** i `driver/timer.h`:
  * Endast `typedef struct timer timer_t;` i headern.
  * Den riktiga definitionen ligger i `driver/timer.c`.
* `timer_new()` ska:
  * **inte** acceptera `timeout_ms == 0`.
  * **inte** acceptera `callback == NULL`.
  * Returnera `NULL` om:
    * Timeouten är ogiltig.
    * Callback saknas.
    * Inga timerkretsar finns tillgängliga (resursbrist).

---

## Rekommenderat publikt API (`driver/timer.h`)
Ni får gärna använda detta API som grund. Ni måste dock själva implementera samt dokumentera funktionerna:

```c
#pragma once

#include <stdbool.h>
#include <stdint.h>

/** Timer structure. */
typedef struct timer timer_t;

/** Alias for timer callbacks. */
typedef void (*timer_callback)(void);

timer_t* timer_new(uint32_t timeout_ms, timer_callback callback);
void timer_delete(timer_t** self);

bool timer_is_enabled(const timer_t* self);
void timer_start(timer_t* self);
void timer_stop(timer_t* self);
void timer_toggle(timer_t* self);
void timer_restart(timer_t* self);
void timer_reset(timer_t* self);
uint32_t timer_timeout_ms(const timer_t* self);
void timer_set_timeout_ms(timer_t* self, uint32_t timeout_ms);
```

Gällande funktionerna ovan:
* `timer_new()` skapar en ny timer om:
  * Given timeout är giltig (`0U < timeout_ms`).
  * Callback finns (`NULL != callback`). 
  * En timerkrets finns tillgänglig.
* `timer_delete()` tar bort en timer och frigör allokerade resurser om:
  * Timern är giltig `((NULL != self) && (NULL != *self))`.
  * När en timer tas bort ska dess timerkrets bli tillgänglig igen.
* `timer_is_enabled()` indikerar ifall timern är på eller inte:
  * "Enabled" betyder att timern är igång och kan generera callbacks.
* `timer_start()` startar timern (om redan igång görs ingenting).
* `timer_stop()` stoppar timern (utan att nollställa räknaren).
* `timer_toggle()` togglar timern.
* `timer_reset()` stoppar timern och nollställer räknaren.
* `timer_restart()` nollställer räknaren och startar om timern.
* `timer_timeout_ms()` ger timerns timeout i ms.
* `timer_set_timeout_ms()` sätter ny timeout om denna är giltig (`0U < timeout_ms`):
  * Om timeouten är ogiltig görs ingenting.
  * Om timern är igång och timeout ändras så startar perioden om från 0.

---

## Integrationskrav: knapp + lysdiod

### Önskat beteende
* Knapptryckning:
  * Anropar button-callback och togglar timern.
  * Om timern stängs av: Säkerställ att lysdioden släcks.
* Timer-callback:
  * Toggla lysdioden (endast om timern är aktiv).
  * Callbacken kan anropas manuellt, men ska inte göra något om timern inte är på:
    * Det ska gå att visa detta genom att anropa callbacken i kod när timern är av (ingen LED-toggle).
    * Callbacken ska därför kontrollera timer-status innan den togglar lysdioden.

---

## Testfall ni ska kunna visa

### 1. Validering i `timer_new`
* `timer_t* t1 = timer_new(0U, timer0_callback)` → t1 → **NULL**
* `timer_t* t2 = timer_new(100U, NULL)` → t2 → **NULL**
* Skapa fler än tre timers → den sista → **NULL**

Se kodsnutten nedan:

```c
// Assume this function is implemented.
static void timer0_callback(void);
static void timer1_callback(void);
static void timer2_callback(void);
static void timer3_callback(void);

// Invalid parameters, expect 'timer_new()' to return NULL.
{
    timer_t* timer0 = timer_new(0U, timer0_callback);
    timer_t* timer1 = timer_new(100U, NULL);
}

// Valid parameters - expect 'timer_new()' to create three timers.
// The fourth timer should be NULL due to hardware limitations.
{
    timer_t* timer0 = timer_new(10U, timer0_callback);
    timer_t* timer1 = timer_new(20U, timer1_callback);
    timer_t* timer2 = timer_new(30U, timer2_callback);
    timer_t* timer3 = timer_new(40U, timer3_callback);

    // Delete 'timer2', expect 'timer3' to be initialized after invoking 'timer_new()' once more.
    timer_delete(&timer2);
    timer3 = timer_new(40U, timer3_callback);
}

```

### 2. Funktionalitet
* Timer av → Lysdioden släcks.
* Timer på → Lysdioden togglar var 100:e ms.
* Knapptryck togglar läget korrekt (och lysdioden släcks direkt när timern stängs av).

**OBS!** Timer-drivern ska inte känna till GPIO-drivern. Ovanstående ska hanteras i applikationskoden via callbacken och knapp-callbacken (i `main.c`).

---

## Redovisning
Projektet ska redovisas för läraren under lektionstid.

## Utvärdering
Svara kort på följande (muntligt eller i en kort text):
1. Varför är `struct timer` opaque i headern? Vad vinner vi?
2. Varför kräver vi att callback inte får vara `NULL`?
3. Vilka fel ska `timer_new()` upptäcka och hur signaleras dessa?
4. Hur säkerställer ni att max tre timers kan reserveras samtidigt?
5. Hur skiljer sig timer-callbacken från GPIO-callbacken i L02?
6. Vad händer i ert system när timern stängs av – och varför tvingar vi lysdioden till av?

---

## Bedömningskriterier

### G (Godkänd)
* Opaque struct i `driver/timer.h`.
* `timer_new()` validerar timeout och callback samt hanterar resursbrist.
* Timern kan togglas med knapp.
* Lysdioden togglas var 100:e ms när timern är aktiv.
* Lysdioden är släckt när timern är inaktiv.
* Rensning/hantering av resurser (anrop av `timer_delete()`).

### VG (Väl Godkänd)
* Allt för G, plus:
  * Tillägg av ytterligare en timer, tryckknapp samt lysdiod:
    * Vid nedtryckning av tryckknappen ska timern togglas. 
    * När timern är på ska den toggla motsvarande lysdiod.
    * När timern är av ska lysdioden vara släckt.
  * Tillägg av en debounce-timer, som skyddar mot knappstudsar:
    * Vid button event ska interrupt inaktiveras på tryckknapparnas I/O-port. När detta sker ska debounce-timern startas.
    * Debounce-timern ska löpa ut efter 300 ms. När detta sker ska interrupts för tryckknapparna återaktiveras.
  * Korrekt och konsekvent C-stil (som GPIO-exemplet).
  * Tydliga felhanteringar och NULL-checks.
  * Doxygen-dokumentation i `driver/timer.h` och relevanta delar i `driver/timer.c`.
  * Välstrukturerad kod: Tydlig uppdelning mellan publikt API och intern logik.

### Bidrag till kursens slutpoäng
* Betyget **G** ger 1 poäng till kurssammanställningen.
* Betyget **VG** ger 2 poäng till kurssammanställningen.

---

## Tips
* Håll API:et litet men tydligt.
* Använd statiska hjälp-funktioner i `driver/timer.c` för att hålla koden ren.
* Genomför små tester/self-checks tidigt: Resursbrist och argumentvalidering först.

---
