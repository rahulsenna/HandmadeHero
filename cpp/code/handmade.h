//
// Created by AgentOfChaos on 11/20/2020.
//


#ifndef HANDMADEHERO_HANDMADE_H

#include "handmade_platform.h"

#define MINIMUM(A, B) ((A < B) ? (A) : (B))
#define MAXIMUM(A, B) ((A > B) ? (A) : (B))

struct memory_arena
{
    mem_index Size;
    u8        *Base;
    mem_index Used;

    s32 TempCount;
};

inline void
InitializeArena(memory_arena *Arena, mem_index Size, void *Base)
{
    Arena->Size      = Size;
    Arena->Base      = (u8 *) Base;
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
    u8 *Byte = (u8 *) Ptr;
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
#include "handmade_render_group.h"


struct hero_bitmaps
{
    loaded_bitmap HeroHead;
    loaded_bitmap HeroTorso;
    loaded_bitmap HeroCape;
};

struct move_spec
{
    b32 UnitMaxAccelVector;
    r32 Speed;
    r32 Drag;
};
#define HIT_POINT_SUB_COUNT 4

struct low_entity
{
    sim_entity     Sim;
    world_position P;
};

struct controlled_hero
{
    u32 EntityIndex;
    v2  accel;
    v2  dSword;
    r32 dZ;
};

struct pairwise_collision_rule
{
    b32 CanCollide;
    u32 StorageIndexA;
    u32    StorageIndexB;

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

    r32 TypicalFloorHeight;

    u32            CameraFollowingEntityIndex;
    world_position CameraP;

    controlled_hero ControlledHeroes[ArrayCount(((game_input *) 0)->Controllers)];

    u32        LowEntityCount;
    low_entity LowEntities[100000];

    loaded_bitmap Grass[2];
    loaded_bitmap Ground[4];
    loaded_bitmap Tuft[3];

    loaded_bitmap Tree;
    loaded_bitmap Sword;
    loaded_bitmap Familiar;
    loaded_bitmap Monster;
    loaded_bitmap Stair;
    loaded_bitmap MonsterDead;

    loaded_bitmap Stairwell;

    loaded_bitmap Backdrop;
    loaded_bitmap HeroShadow;

    hero_bitmaps HeroBitmaps[4];

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

    r32 Time;

    loaded_bitmap TestDiffuse;
    loaded_bitmap TestNormal;

};

struct transient_state
{
    memory_arena  TranArena;
    u32           GroundBufferCount;
    ground_buffer *GroundBuffers;
    b32           IsInitialized;

    platform_work_queue *RenderQueue;

    s32 EnvMapWidth;
    s32 EnvMapHeight;
    environment_map EnvMaps[3];
};

internal low_entity *
GetLowEntity(game_state *GameState, u32 Index)
{
    low_entity *Result = 0;
    if ((Index > 0) && (Index < GameState->LowEntityCount))
    {
        Result = GameState->LowEntities + Index;
    }
    return (Result);
}

internal void
AddCollisionRule(game_state *GameState, u32 StorageIndexA, u32 StorageIndexB, b32 ShouldCollide);

internal void
ClearCollisionRuleFor(game_state *GameState, u32 StorageIndex);

global_variable platform_add_entry *PlatformAddEntry;
global_variable platform_complete_all_work *PlatformCompleteAllWork;


#define HANDMADEHERO_HANDMADE_H
#endif //HANDMADEHERO_HANDMADE_H
