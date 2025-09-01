#ifndef BEEP_H
#define BEEP_H

#include <stdbool.h>
#include <stdint.h>

/* Opaque beeper; SDL3 details are hidden in beep.c */
typedef struct Beeper Beeper;

/* Create a sine-wave beeper (e.g. freq=330, volume=0.10). */
bool beep_init(Beeper** out_beeper, int freq_hz, float volume);

/* Turn beep on/off. */
void beep_set(Beeper* b, bool on);

/* Destroy. */
void beep_destroy(Beeper* b);

#endif /* BEEP_H */
