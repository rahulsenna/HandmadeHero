//
// Created by AgentOfChaos on 11/20/2020.
//


#ifndef HANDMADEHERO_HANDMADE_H

#define MINIMUM(A, B) ((A < B) ? (A) : (B))
#define MAXIMUM(A, B) ((A > B) ? (A) : (B))

struct memory_arena
{
    mem_index Size;
    uint8     *Base;
    mem_index Used;

    int32 TempCount;
};

inline void
InitializeArena(memory_arena *Arena, mem_index Size, void *Base)
{
    Arena->Size      = Size;
    Arena->Base      = (uint8 *) Base;
    Arena->Used      = 0;
    Arena->TempCount = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count * sizeof(type)))
#define PushSize(Arena, Size) PushSize_(Arena, Size)

inline void *
PushSize_(memory_arena *Arena, mem_index Size)
{
    Assert(Arena->Used + Size <= Arena->Size)
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return (Result);
}

struct temporary_memory
{
    memory_arena *Arena;
    mem_index    Used;
};

inline temporary_memory
BeginTempMemory(memory_arena *Arena)
{
    temporary_memory Result;
    Result.Arena = Arena;
    Result.Used  = Arena->Used;

    ++Result.Arena->TempCount;

    return (Result);
}

inline void
EndTempMemory(temporary_memory TempMem)
{
    memory_arena *Arena = TempMem.Arena;
    Assert(Arena->Used >= TempMem.Used)
    Arena->Used = TempMem.Used;
    Assert(Arena->TempCount > 0)
    --TempMem.Arena->TempCount;
}

inline void
CheckArena(memory_arena *Arena)
{
    Assert(Arena->TempCount == 0)
}

inline void
ZeroSize(mem_index Size, void *Ptr)
{
    uint8 *Byte = (uint8 *) Ptr;
    while (Size--)
    {
        *Byte++ = 0;
    }
}

#define ZeroStruct(Instance) ZeroSize(sizeof(Instance), &(Instance))

#include "handmade_intrinsics.h"
#include "handmade_math.h"
#include "handmade_world.h"
#include "handmade_sim_region.h"
#include "handmade_entity.h"

struct loaded_bitmap
{
    void  *Memory;
    int32 Width;
    int32 Height;
    int32 Pitch;
};

struct hero_bitmaps
{
    v2            Align;
    loaded_bitmap HeroHead;
    loaded_bitmap HeroTorso;
    loaded_bitmap HeroCape;
};

struct move_spec
{
    bool32 UnitMaxAccelVector;
    real32 Speed;
    real32 Drag;
};
#define HIT_POINT_SUB_COUNT 4

struct low_entity
{
    sim_entity     Sim;
    world_position P;
};

struct controlled_hero
{
    uint32 EntityIndex;
    v2     accel;
    v2     dSword;
    real32 dZ;
};

struct pairwise_collision_rule
{
    bool32 CanCollide;
    uint32 StorageIndexA;
    uint32 StorageIndexB;

    pairwise_collision_rule *NextInHash;
};

struct ground_buffer
{
    world_position P;
    loaded_bitmap  Bitmap;
};

struct game_state
{
    memory_arena WorldArena;
    world        *World;

    real32 TypicalFloorHeight;

    uint32         CameraFollowingEntityIndex;
    world_position CameraP;

    controlled_hero ControlledHeroes[ArrayCount(((game_input *) 0)->Controllers)];

    uint32     LowEntityCount;
    low_entity LowEntities[100000];

    loaded_bitmap Grass[2];
    loaded_bitmap Ground[4];
    loaded_bitmap Tuft[3];

    loaded_bitmap Tree;
    loaded_bitmap Sword;
    loaded_bitmap Familiar;
    loaded_bitmap Monster;
    loaded_bitmap MonsterDead;

    loaded_bitmap Stairwell;

    loaded_bitmap Backdrop;
    loaded_bitmap HeroShadow;

    hero_bitmaps HeroBitmaps[4];

    real32 MetersToPixel;
    real32 PixelsToMeters;

    pairwise_collision_rule *CollisionRuleHash[256];
    pairwise_collision_rule *FirstFreeCollisionRule;

    sim_entity_collision_volume_group *NullCollision;
    sim_entity_collision_volume_group *SwordCollision;
    sim_entity_collision_volume_group *StairCollision;
    sim_entity_collision_volume_group *PlayerCollision;
    sim_entity_collision_volume_group *MonsterCollision;
    sim_entity_collision_volume_group *WallCollision;
    sim_entity_collision_volume_group *FamiliarCollision;
    sim_entity_collision_volume_group *StandardRoomCollision;
};

struct transient_state
{
    memory_arena  TranArena;
    uint32        GroundBufferCount;
    ground_buffer *GroundBuffers;
    bool32        IsInitialized;
};

internal low_entity *
GetLowEntity(game_state *GameState, uint32 Index)
{
    low_entity *Result = 0;
    if ((Index > 0) && (Index < GameState->LowEntityCount))
    {
        Result = GameState->LowEntities + Index;
    }
    return (Result);
}

internal void
AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 ShouldCollide);

internal void
ClearCollisionRuleFor(game_state *GameState, uint32 StorageIndex);

#define HANDMADEHERO_HANDMADE_H
#endif //HANDMADEHERO_HANDMADE_H
