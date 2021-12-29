//
// Created by AgentOfChaos on 12/7/2020.
//

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <limits.h>
#include <float.h>

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
#elif COMPILER_LLVM
#include <x86intrin.h>
#else
#error SSE2 Not available for this compiler yet!!!
#endif

#define WIDTH           960
#define HEIGHT          540

#define internal        static
#define global_variable static
#define local_persist   static
#define Pi32            3.14159265359f
typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef int8_t  s8;
typedef int16_t s16;
typedef int32_t s32;
typedef int64_t s64;
typedef int32_t b32;

typedef intptr_t sptr;
typedef uintptr_t uptr;

typedef float  r32;
typedef double r64;

#define Real32Maximum FLT_MAX

typedef size_t mem_index;

#ifndef HANDMADEHERO_HANDMADE_PLATFORM_H

#if HANDMADE_SLOW
#define Assert(Expression) \
    if (!(Expression)) { *(int *) 0 = 0; }
#else
#define Assert(Expression)
#endif

#define InvalidCodePath Assert(!"InvalidCodePath");
#define InvalidDefaultCase \
    default: {             \
        InvalidCodePath    \
    }                      \
    break

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
#define Kilobytes(Value)  ((Value) *1024)
#define Megabytes(Value)  (Kilobytes(Value) * 1024)
#define Gigabytes(Value)  (Megabytes(Value) * 1024)
#define Terabytes(Value)  (Gigabytes(Value) * 1024)

inline u32 SafeTruncateUInt64(u64 Value)
{
    Assert(Value <= 0xFFFFFFFF)
    u32 Result = (u32) Value;
    return (Result);
}

struct debug_read_file_result
{
    void *Contents;
    u32   ContentsSize;
};

struct thread_context
{
    int Placeholder;
};

#if HANDMADE_INTERNAL
#define DEBUG_PLATFORM_READ_ENTIRE_FILE(FunctionName) \
    debug_read_file_result FunctionName(thread_context *Thread, char *Filename)
#define DEBUG_PLATFORM_WRITE_ENTIRE_FILE(FunctionName) \
    b32 FunctionName(thread_context *Thread, char *Filename, u32 FileSize, void *Memory)
#define DEBUG_PLATFORM_FREE_FILE_MEMORY(FunctionName) void FunctionName(thread_context *Thread, void *Memory)

typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);

typedef DEBUG_PLATFORM_FREE_FILE_MEMORY(debug_platform_free_file_memory);

typedef DEBUG_PLATFORM_WRITE_ENTIRE_FILE(debug_platform_write_entire_file);

#endif

enum
{
    /* 0 */ DebugCycleCounter_GameUpdateAndRender,
    /* 1 */ DebugCycleCounter_RenderGroupToOutput,
    /* 2 */ DebugCycleCounter_DrawRectangleSlowly,
    /* 3 */ DebugCycleCounter_DrawRectangleHopefullyQuickly,
    /* 4 */ DebugCycleCounter_ProcessPixel,

    DebugCycleCounter_Count,
};

typedef struct debug_cycle_counter
{
    u64 CycleCount;
    u32 HitCount;
} debug_cycle_counter;
extern struct game_memory *DebugGlobalMemory;

#if _MSC_VER
#define BEGIN_TIMED_BLOCK(ID) u64 StartCycleCount##ID = __rdtsc();
#define END_TIMED_BLOCK(ID)                                                                            \
    DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; \
    ++DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount;
#define END_TIMED_BLOCK_COUNTED(ID, Count)                                                             \
    DebugGlobalMemory->Counters[DebugCycleCounter_##ID].CycleCount += __rdtsc() - StartCycleCount##ID; \
    DebugGlobalMemory->Counters[DebugCycleCounter_##ID].HitCount += (Count);
#else
#define BEGIN_TIMED_BLOCK(ID)
#define END_TIMED_BLOCK(ID)
#endif

struct game_memory
{
    b32   IsInitialized;
    u64   PermanentStorageSize;
    void *PermanentStorage;

    u64   TransientStorageSize;
    void *TransientStorage;

    debug_platform_read_entire_file * DEBUGPlatformReadEntireFile;
    debug_platform_write_entire_file *DEBUGPlatformWriteEntireFile;
    debug_platform_free_file_memory * DEBUGPlatformFreeFileMemory;

#if HANDMADE_INTERNAL
    debug_cycle_counter Counters[256];
#endif
};

#define BYTES_PER_PIXEL 4
struct game_offscreen_buffer
{
    void *Memory;
    int   Width;
    int   Height;
    int   Pitch;
};

struct game_sound_output_buffer
{
    s16 *Samples;
    int  SamplesPerSecond;
    int  SampleCount;
};

struct game_button_state
{
    int HalfTransitionCount;
    b32 EndedDown;
};

struct game_controller_input
{
    b32 IsConnected;
    b32 IsAnalog;
    r32 StickAverageX;
    r32 StickAverageY;

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
    int               MouseX, MouseY, MouseZ;

    b32 ExecutableReloaded;

    r32                   deltatForFrame;
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
#endif//HANDMADEHERO_HANDMADE_PLATFORM_H
