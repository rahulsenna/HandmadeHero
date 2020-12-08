
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

#include "handmade_intrinsics.h"
#include "handmade_tile.h"

struct memory_arena
{
    mem_index Size;
    uint8 *Base;
    mem_index Used;
};

struct world
{
    tile_map *TileMap;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;
    tile_map_position PlayerP;
};

#define HANDMADEHERO_HANDMADE_H
#endif //HANDMADEHERO_HANDMADE_H
