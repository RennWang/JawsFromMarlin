#ifndef WATCHDOG_H
#define WATCHDOG_H

// intialise watch dog with a 1 sec interrupt time
void watchdog_init();
// pad the dog/reset watchdog. MUST be called at least every second after the first watchdog_init or avr will go into emergency procedures..
void watchdog_reset();

#endif /* WATCHDOG_H */
