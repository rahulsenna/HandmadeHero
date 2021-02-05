//
// Created by AgentOfChaos on 1/30/21.
//

#ifndef HANDMADEHERO_HANDMADE_SIM_REGION_H

struct hit_point
{
    uint8 Flags;
    uint8 FilledAmount;
};

enum entity_type
{
    EntityType_Hero,
    EntityType_Wall,
    EntityType_Monster,
    EntityType_Familiar,
    EntityType_Sword,
    EntityType_Null,
};

struct sim_entity;

union entity_reference
{
    sim_entity *Ptr;
    uint32 Index;
};

enum sim_entity_flags
{
    EntityFlag_Collides = (1 << 1),
    EntityFlag_NonSpatial = (1 << 2),

    EntityFlag_Simming = (1 << 30),
};

struct sim_entity
{
    uint32 StorageIndex;

    bool32 Updatable;

    entity_type Type;
    uint32 Flags;

    v2 P;
    v2 deltaP;

    real32 Z;
    real32 deltaZ;

    real32 DistanceLimit;

    uint32 ChunkZ;

    real32 Height, Width;
    int32 deltaAbsTileZ;

    uint32 HitPointMax;
    hit_point HitPoint[16];

    real32 tBob;

    uint32 FacingDirection;

    uint32 LowEntityIndex;

    uint32 SwordLowIndex;
    entity_reference Sword;
};

struct sim_entity_hash
{
    sim_entity *Ptr;
    uint32 Index;
};

struct sim_region
{
    world *World;
    world_position Origin;
    rectangle2 Bounds;
    rectangle2 UpdatableBounds;

    uint32 MaxEntityCount;
    uint32 EntityCount;
    sim_entity *Entities;

    sim_entity_hash Hash[4096];
};

#define HANDMADEHERO_HANDMADE_SIM_REGION_H
#endif //HANDMADEHERO_HANDMADE_SIM_REGION_H