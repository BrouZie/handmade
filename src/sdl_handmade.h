#ifndef SDL_HANDMADE_H
#define SDL_HANDMADE_H

#include "handmade.h"

#include <SDL2/SDL.h>
#include <sys/mman.h>

#include <x86intrin.h> // needed for _rdtsc() - profiling


struct SDLOffscreenBuffer
{
	// NOTE: Pixels are always 32-bits wide, Memory Order BB GG RR XX
	SDL_Texture* texture;
	void* memory;
	int width;
	int height;
	int pitch;
};

struct SDLAudioRingBuffer
{
    void *data;
    int size;
    int write_cursor;
    int play_cursor;
};

struct SDLSoundOutput
{
    int samples_per_second;
    int16_t tone_volume;
    uint32_t running_sample_idx;
    int wave_period;
    int bytes_per_sample;
    int secondary_buffer_size;
    real32 t_sine;
    int latency_sample_count;
};

struct SDLWindowDimensions // only used as a helper, not enforced throughout
{
	int width;
	int height;
};


#endif // !SDL_HANDMADE_H
