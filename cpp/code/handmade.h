
#pragma clang diagnostic ignored "-Wnull-dereference"
#pragma ide diagnostic ignored "modernize-use-auto"

//
// Created by AgentOfChaos on 11/20/2020.
//


#ifndef HANDMADEHERO_HANDMADE_H

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

struct hero_bitmaps
{
    int32 AlignX;
    int32 AlignY;
    loaded_bitmap HeroHead;
    loaded_bitmap HeroTorso;
    loaded_bitmap HeroCape;
};

struct entity
{
    bool32 Exists;
    tile_map_position P;
    v2 dP;
    uint32 FacingDirection;
    real32 Height, Width;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;

    uint32 CameraFollowingEntityIndex;
    tile_map_position CameraP;

    uint32 PlayerIndexForController[ArrayCount(((game_input *) 0)->Controllers)];
    entity Entities[256];
    uint32 EntityCount;

    loaded_bitmap Tree;
    loaded_bitmap Backdrop;
    loaded_bitmap HeroShadow;
    hero_bitmaps HeroBitmaps[4];
};

#define HANDMADEHERO_HANDMADE_H
#endif //HANDMADEHERO_HANDMADE_H
