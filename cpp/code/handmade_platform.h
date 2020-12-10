//
// Created by AgentOfChaos on 12/7/2020.
//
#pragma ide diagnostic ignored "OCUnusedMacroInspection"
#pragma ide diagnostic ignored "modernize-deprecated-headers"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#ifdef __cplusplus
extern "C"  {
#endif

#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static
#define Pi32 3.14159265359f
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32_t bool32;

typedef float real32;
typedef double real64;

typedef size_t mem_index;

#ifndef HANDMADEHERO_HANDMADE_PLATFORM_H

#if HANDMADE_SLOW
#define Assert(Expression) if(!(Expression)) { *(int *)0 = 0; }
#else
#define Assert(Expression)
#endif

struct debug_read_file_result
{
    void *Contents;
    uint32 ContentsSize;
};

struct thread_context
{
    int Placeholder;
};

#if HANDMADE_INTERNAL
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(FunctionName) \
debug_read_file_result FunctionName(thread_context *Thread, char *Filename)
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(FunctionName) \
bool32 FunctionName(thread_context *Thread, char *Filename, uint32 FileSize, void *Memory)
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(FunctionName) void FunctionName(thread_context *Thread, void *Memory)

typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

struct game_memory
{
    bool32 IsInitialized;
    uint64 PermanentStorageSize;
    void *PermanentStorage;

    uint64 TransientStorageSize;
    void *TransientStorage;

    debug_platform_read_entire_file *DEBUGPlatformReadEntireFile;
    debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    debug_platform_free_file_memory *DEBUGPlatformFreeFileMemory;
};
struct game_offscreen_buffer
{
    void *Memory;
    int Width;
    int Height;
    int Pitch;
    int BytesPerPixel;
};

struct game_output_sound_buffer
{
    int16 *Samples;
    int SamplesPerSecond;
    int SampleCount;
};

struct game_button_state
{
    int HalfTransitionCount;
    bool32 EndedDown;
};

struct game_controller_input
{
    bool32 IsConnected;
    bool32 IsAnalog;
    real32 StickAverageX;
    real32 StickAverageY;

    union
    {
        game_button_state Buttons[12];
        struct
        {
            game_button_state MoveUp;
            game_button_state MoveDown;
            game_button_state MoveLeft;
            game_button_state MoveRight;

            game_button_state AButton;
            game_button_state BButton;
            game_button_state XButton;
            game_button_state YButton;

            game_button_state LeftShoulder;
            game_button_state RightShoulder;

            game_button_state Start;
            game_button_state Back;
        };
    };
};

struct game_input
{
    game_button_state MouseButtons[5];
    int MouseX, MouseY, MouseZ;

    real32 SecondsToAdvancePerFrame;
    game_controller_input Controllers[5];
};

#define GAME_UPDATE_AND_RENDER(FunctionName) \
void FunctionName(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
#define GAME_GET_SOUND_SAMPLES(FunctionName) \
void FunctionName(thread_context *Thread, game_memory *Memory, game_output_sound_buffer *SoundBuffer)

typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);


#ifdef __cplusplus
  }
#endif

#define HANDMADEHERO_HANDMADE_PLATFORM_H
#endif //HANDMADEHERO_HANDMADE_PLATFORM_H
