//
// Created by AgentOfChaos on 12/7/2020.
//

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#ifndef COMPILER_MSVC
#define COMPILER_MSVC 0
#endif

#ifndef COMPILER_LLVM
#define COMPILER_LLVM 0
#endif

#if !COMPILER_MSVC && !COMPILER_LLVM
#if _MSC_VER
#undef COMPILER_MSVC
#define COMPILER_MSVC 1
#else
#undef COMPILER_LLVM
#define COMPILER_LLVM 1
#endif
#endif

#if COMPILER_MSVC
#include <intrin.h>
#endif

#define internal static
#define global_variable static
#define local_persist static
#define PII32 3.14159265359f
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

#define InvalidCodePath Assert(!"InvalidCodePath");

#define ArrayCount(Array) (sizeof(Array)/sizeof((Array)[0]))
#define Kilobytes(Value) ((Value) * 1024)
#define Megabytes(Value) (Kilobytes(Value) * 1024)
#define Gigabytes(Value) (Megabytes(Value) * 1024)
#define Terabytes(Value) (Gigabytes(Value) * 1024)

inline uint32 SafeTruncateUInt64(uint64 Value)
{
    Assert(Value <= 0xFFFFFFFF)
    uint32 Result = (uint32) Value;
    return (Result);
}

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

#define BYTES_PER_PIXEL 4
struct game_offscreen_buffer
{
    void *Memory;
    int32 Width;
    int32 Height;
    int32 Pitch;
};

struct game_sound_output_buffer
{
    int16 *Samples;
    int32 SamplesPerSecond;
    int32 SampleCount;
};

struct game_button_state
{
    int32 HalfTransitionCount;
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

            game_button_state ActionDown;
            game_button_state ActionRight;
            game_button_state ActionLeft;
            game_button_state ActionUp;

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

    real32 deltatForFrame;
    game_controller_input Controllers[5];
};

inline game_controller_input *GetController(game_input *Input, int ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers))
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return Result;
}

#define GAME_UPDATE_AND_RENDER(FunctionName) \
void FunctionName(thread_context *Thread, game_memory *Memory, game_input *Input, game_offscreen_buffer *Buffer)
#define GAME_GET_SOUND_SAMPLES(FunctionName) \
void FunctionName(thread_context *Thread, game_memory *Memory, game_sound_output_buffer *SoundBuffer)

typedef GAME_UPDATE_AND_RENDER(game_update_and_render);

typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);

#ifdef __cplusplus
}
#endif

#define HANDMADEHERO_HANDMADE_PLATFORM_H
#endif //HANDMADEHERO_HANDMADE_PLATFORM_H
