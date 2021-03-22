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
    EntityType_Null,

    EntityType_Space,

    EntityType_Hero,
    EntityType_Wall,
    EntityType_Monster,
    EntityType_Familiar,
    EntityType_Sword,
    EntityType_Stairwell,
};

struct sim_entity;

union entity_reference
{
    sim_entity *Ptr;
    uint32 Index;
};

enum sim_entity_flags
{
    EntityFlag_Collides = (1 << 0),
    EntityFlag_NonSpatial = (1 << 1),
    EntityFlag_Movable = (1 << 2),
    EntityFlag_ZSupported = (1 << 3),
    EntityFlag_Traversable = (1 << 4),

    EntityFlag_Simming = (1 << 30),
};

struct sim_entity_collision_volume
{
    v3 OffsetP;
    v3 Dim;
};

struct sim_entity_collision_volume_group
{
    sim_entity_collision_volume TotalVolume;
    uint32 VolumeCount;
    sim_entity_collision_volume *Volumes;
};
struct sim_entity
{
    uint32 StorageIndex;
    bool32 Updatable;

    entity_type Type;
    uint32 Flags;

    v3 P;
    v3 deltaP;

    real32 DistanceLimit;

    sim_entity_collision_volume_group *Collision;
    int32 deltaAbsTileZ;

    uint32 HitPointMax;
    hit_point HitPoint[16];

    real32 tBob;

    uint32 FacingDirection;

    uint32 LowEntityIndex;

    uint32 SwordLowIndex;
    entity_reference Sword;

    v2 WalkableDim;
    real32 WalkableHeight;
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
    rectangle3 Bounds;
    rectangle3 UpdatableBounds;

    uint32 MaxEntityCount;
    uint32 EntityCount;
    sim_entity *Entities;

    sim_entity_hash Hash[4096];

    real32 MaxEntityRadius;
    real32 MaxEntityVelocity;
};

#define HANDMADEHERO_HANDMADE_SIM_REGION_H
#endif //HANDMADEHERO_HANDMADE_SIM_REGION_H
