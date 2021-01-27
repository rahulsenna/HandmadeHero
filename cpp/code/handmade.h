//
// Created by AgentOfChaos on 11/20/2020.
//


#ifndef HANDMADEHERO_HANDMADE_H

#define Minimum(A, B) ((A < B) ? (A) : (B))
#define Maximum(A, B) ((A > B) ? (A) : (B))

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
#include "handmade_world.h"

struct loaded_bitmap
{
    int32 Width;
    int32 Height;
    uint32 *Pixels;
};

struct hero_bitmaps
{
    v2 Align;
    loaded_bitmap HeroHead;
    loaded_bitmap HeroTorso;
    loaded_bitmap HeroCape;
};

struct entity_visible_piece
{
    loaded_bitmap *Bitmap;
    v2 Offset;
    real32 OffsetZ;
    real32 EntityZC;

    real32 R, G, B, A;
    v2 Dim;
};

struct high_entity
{
    v2 P;
    v2 deltaP;
    uint32 ChunkZ;
    uint32 FacingDirection;

    real32 tBob;

    real32 Z;
    real32 deltaZ;

    uint32 LowEntityIndex;
};
enum entity_type
{
    EntityType_Hero,
    EntityType_Wall,
    EntityType_Monster,
    EntityType_Familiar,
    EntityType_Null,
};

#define HIT_POINT_SUB_COUNT 4

struct hit_point
{
    uint8 Flags;
    uint8 FilledAmount;
};
struct low_entity
{
    entity_type Type;

    world_position P;
    real32 Height, Width;
    int32 deltaAbsTileZ;
    bool32 Collides;

    uint32 HighEntityIndex;

    uint32 HitPointMax;
    hit_point HitPoint[16];
};

struct entity
{
    uint32 LowIndex;
    low_entity *Low;
    high_entity *High;
};

struct game_state
{
    memory_arena WorldArena;
    world *World;

    uint32 CameraFollowingEntityIndex;
    world_position CameraP;

    uint32 PlayerIndexForController[ArrayCount(((game_input *) 0)->Controllers)];

    uint32 LowEntityCount;
    low_entity LowEntities[500000];

    loaded_bitmap Tree;

    high_entity HighEntities_[256];
    uint32 HighEntityCount;

    loaded_bitmap Backdrop;
    loaded_bitmap HeroShadow;
    hero_bitmaps HeroBitmaps[4];
    real32 MetersToPixel;
};

struct entity_visible_piece_group
{
    game_state *GameState;
    uint32 Count;
    entity_visible_piece Pieces[16];
};

#define HANDMADEHERO_HANDMADE_H
#endif //HANDMADEHERO_HANDMADE_H
