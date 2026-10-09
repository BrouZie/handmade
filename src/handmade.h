#ifndef HANDMADE_H
#define HANDMADE_H

#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

#define PI32 3.14159265358979f
#define ARRAY_COUNT(array) (sizeof(array) / sizeof((array)[0]))

typedef float real32;
typedef double real64;
typedef int32_t bool32;


struct GameOffscreenBuffer
{
	// NOTE: Pixels are always 32-bits wide, Memory Order BB GG RR XX
    void* memory;
    int width;
    int height;
    int pitch;
};

struct GameSoundOutputBuffer
{
    int samples_per_second;
    int sample_count;
    int16_t* samples;
};

struct GameButtonState
{
    int half_transition_count;
    bool32 ended_down;
};

struct GameControllerInput
{
    bool32 is_analog;

    real32 start_x;
    real32 start_y;

    real32 end_x;
    real32 end_y;

    real32 min_x;
    real32 min_y;

    real32 max_x;
    real32 max_y;

    union
    {
        GameButtonState buttons[6];
        struct
        {
            GameButtonState up;
            GameButtonState down;
            GameButtonState left;
            GameButtonState right;
            GameButtonState left_shoulder;
            GameButtonState right_shoulder;
        };
    };
};

struct GameInput
{
    GameControllerInput controllers[4];
};

// FOUR THINGS: timing, keyboard/gamepad inputs, bitmap buffer to use and audio buffer to use
internal void game_update(GameOffscreenBuffer* buffer, GameSoundOutputBuffer* sound_buffer, GameInput* new_input);

#endif // HANDMADE_H
