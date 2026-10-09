#include "handmade.h"

#include <math.h>

internal void game_output_sound(GameSoundOutputBuffer* sound_buffer, int tone_hz)
{
    local_persist real32 t_sine;

    int16_t tone_volume = 3000;
    // real32 tone_hz = 261.626;
    int wave_period = sound_buffer->samples_per_second/tone_hz;

    int16_t* sample_out = sound_buffer->samples;
    for (int sample_idx = 0; sample_idx < sound_buffer->sample_count; ++sample_idx)
    {
        real32 sine_value = sinf(t_sine);
        int16_t sample_value = (int16_t)(sine_value * tone_volume);
        *sample_out++ = sample_value;
        *sample_out++ = sample_value;

        t_sine += 2.0f*PI32*1.0f/(real32)wave_period;
    }
}

internal void game_render_gradient(GameOffscreenBuffer* buffer, int blue_offset, int green_offset)
{
    int width  = buffer->width;
    int height = buffer->height;

    uint8_t* row = (uint8_t *)buffer->memory;
    for(int y = 0; y < buffer->height; ++y)
	{
        uint32_t *pixel = (uint32_t *)row;
        for(int x {}; x < buffer->width; ++x)
		{
            uint8_t blue = (x + blue_offset);
            uint8_t green = (y + green_offset);

            *pixel++ = ((green << 8) | blue);
        }

        row += buffer->pitch;
    }
}

internal void game_update(GameOffscreenBuffer* buffer, GameSoundOutputBuffer* sound_buffer, GameInput* new_input)
{
    local_persist int BlueOffset = 0;
    local_persist int GreenOffset = 0;
    local_persist int ToneHz = 256;

    // TODO: Allow sample offsets here for more robust platform options
    game_output_sound(sound_buffer, ToneHz);
    game_render_gradient(buffer, BlueOffset, GreenOffset);
}
