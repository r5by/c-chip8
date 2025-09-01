#include "beep.h"
#include <SDL3/SDL.h>
#include <math.h>
#include <stdlib.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

struct Beeper {
    SDL_AudioStream* stream;   /* device-bound stream */
    int   sample_rate;
    float phase;
    float phase_inc;           /* 2π f / sample_rate */
    float volume;              /* 0..1 */
    bool  playing;
};

/* SDL3: stream callback, adding additional_amount byte-data to the stream */
static void SDLCALL beeper_stream_cb(void* userdata,
                                     SDL_AudioStream* stream,
                                     int additional_amount,
                                     int total_amount)
{
    (void)total_amount;
    Beeper* b = (Beeper*)userdata;
    if (!b || additional_amount <= 0) return;

    /* target format: F32 mono；additional_amount repr. num of bytes  */
    const int bytes_per_sample = (int)sizeof(float);
    float tmp[4096];  /* 16KB cache */

    while (additional_amount > 0) {
        const int chunk_bytes = (additional_amount > (int)sizeof(tmp))
                                ? (int)sizeof(tmp) : additional_amount;
        const int frames = chunk_bytes / bytes_per_sample;

        if (b->playing) {
            float phase = b->phase;
            const float inc = b->phase_inc;
            const float vol = b->volume;
            for (int i = 0; i < frames; ++i) {
                tmp[i] = sinf(phase) * vol;
                phase += inc;
                if (phase >= 2.0f * (float)M_PI) phase -= 2.0f * (float)M_PI;
            }
            b->phase = phase;
        } else {
            for (int i = 0; i < frames; ++i) tmp[i] = 0.0f;
        }

        SDL_PutAudioStreamData(stream, tmp, chunk_bytes);
        additional_amount -= chunk_bytes;
    }
}

bool beep_init(Beeper** out_beeper, int freq_hz, float volume)
{
    if (!out_beeper) return false;

    Beeper* b = (Beeper*)calloc(1, sizeof(*b));
    if (!b) return false;

    /* source format, SDL shall tranfer it to target device */
    SDL_AudioSpec spec;
    spec.format   = SDL_AUDIO_F32;
    spec.channels = 1;
    spec.freq     = 48000;

    b->stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
                                          &spec, beeper_stream_cb, b);
    if (!b->stream) { free(b); return false; }

    SDL_AudioSpec src, dst;
    if (SDL_GetAudioStreamFormat(b->stream, &src, &dst)) {
        b->sample_rate = src.freq;
    } else {
        b->sample_rate = spec.freq;
    }

    const float f = (float)freq_hz;
    b->phase      = 0.0f;
    b->phase_inc  = (2.0f * (float)M_PI * f) / (float)b->sample_rate;
    b->volume     = (volume < 0.0f) ? 0.0f : (volume > 1.0f ? 1.0f : volume);
    b->playing    = false;

    if (!SDL_ResumeAudioStreamDevice(b->stream)) {
        SDL_DestroyAudioStream(b->stream);
        free(b);
        return false;
    }

    *out_beeper = b;
    return true;
}

void beep_set(Beeper* b, bool on)
{
    if (!b) return;
    if (SDL_LockAudioStream(b->stream)) {
        b->playing = on;
        SDL_UnlockAudioStream(b->stream);
    }
}

void beep_destroy(Beeper* b)
{
    if (!b) return;
    if (b->stream) SDL_DestroyAudioStream(b->stream);
    free(b);
}
