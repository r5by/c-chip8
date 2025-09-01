#include "timer.h"

bool regs_tick_timers(Registers* regs,
                      uint64_t elapsed_ns,
                      bool* out_start_beep,
                      bool* out_stop_beep)
{
    if (!regs) return false;
    static uint64_t acc_ns = 0;
    static bool prev_st_nonzero = false; // to active 'beep' at first frame

    if (out_start_beep) *out_start_beep = false;
    if (out_stop_beep)  *out_stop_beep  = false;

    acc_ns += elapsed_ns;
    const uint64_t step_ns = 1000000000ull / 60ull;

    while (acc_ns >= step_ns) {
        acc_ns -= step_ns;
        if (regs->DT > 0) regs->DT--;
        if (regs->ST > 0) regs->ST--;
    }

    bool cur_st_nonzero = (regs->ST > 0);

    if (!prev_st_nonzero && cur_st_nonzero && out_start_beep) *out_start_beep = true;
    if ( prev_st_nonzero && !cur_st_nonzero && out_stop_beep)  *out_stop_beep  = true;

    bool changed = (prev_st_nonzero != cur_st_nonzero);
    prev_st_nonzero = cur_st_nonzero;
    return changed;
}
