#ifndef TIMERS_H
#define TIMERS_H

#include <stdbool.h>
#include <stdint.h>
#include "regs.h"   // Registers { DT, ST, ... }

/* Advance DT/ST at 60 Hz using elapsed nanoseconds since last call.
 * - Decrements DT and ST (if >0) at 60 Hz.
 * - If ST transitions 0->>0, *out_start_beep = true.
 * - If ST transitions >0->0, *out_stop_beep  = true.
 * Any output pointer can be NULL if caller doesn't care.
 */
bool regs_tick_timers(Registers* regs, uint64_t elapsed_ns,
                      bool* out_start_beep, bool* out_stop_beep);

#endif // TIMERS_H
