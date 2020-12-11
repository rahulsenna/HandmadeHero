
#pragma clang diagnostic ignored "-Wnull-dereference"
#pragma ide diagnostic ignored "modernize-use-auto"

//
// Created by AgentOfChaos on 11/20/2020.
//

#include "handmade_platform.h"

#ifndef HANDMADEHERO_HANDMADE_H

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

inline game_controller_input *GetController(game_input *Input, int ControllerIndex)
{
    Assert(ControllerIndex < ArrayCount(Input->Controllers))
    game_controller_input *Result = &Input->Controllers[ControllerIndex];
    return Result;
}

struct memory_arena
{
    mem_index Size;
    uint8 *Base;
    mem_index Used;
};

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count * sizeof(type)))

void *
PushSize_(memory_arena *Arena, mem_index Size)
{
    Assert(Arena->Used + Size <= Arena->Size)
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return (Result);
}

#include "handmade_intrinsics.h"
#include "handmade_tile.h"

struct world
{
    tile_map *TileMap;
};

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    uint32 *Pixels;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;
    tile_map_position PlayerP;

    loaded_bitmap Backdrop;
    loaded_bitmap HeroHead;
    loaded_bitmap HeroTorso;
    loaded_bitmap HeroCape;

};

#define HANDMADEHERO_HANDMADE_H
#endif //HANDMADEHERO_HANDMADE_H
