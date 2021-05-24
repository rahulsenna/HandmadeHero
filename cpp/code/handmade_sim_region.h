//
// Created by AgentOfChaos on 1/30/21.
//

#ifndef HANDMADEHERO_HANDMADE_SIM_REGION_H

struct hit_point
{
    u8 Flags;
    u8 FilledAmount;
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
    u32        Index;
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


struct test_wall
{
    r32 X;
    r32 DeltaX;
    r32 DeltaY;
    r32 RelX;
    r32 RelY;
    r32 MinY;
    r32 MaxY;
    v3  Normal;
};

struct sim_entity_collision_volume
{
    v3 OffsetP;
    v3 Dim;
};

struct sim_entity_collision_volume_group
{
    sim_entity_collision_volume TotalVolume;
    u32                         VolumeCount;
    sim_entity_collision_volume *Volumes;
};
struct sim_entity
{
    u32 StorageIndex;
    b32 Updatable;

    entity_type Type;
    u32         Flags;

    v3 P;
    v3 deltaP;

    r32 DistanceLimit;

    sim_entity_collision_volume_group *Collision;
    s32                               deltaAbsTileZ;

    u32       HitPointMax;
    hit_point HitPoint[16];

    r32 tBob;

    u32 FacingDirection;

    u32 LowEntityIndex;

    u32              SwordLowIndex;
    entity_reference Sword;

    v2  WalkableDim;
    r32 WalkableHeight;
};

struct sim_entity_hash
{
    sim_entity *Ptr;
    u32        Index;
};

struct sim_region
{
    world *World;
    world_position Origin;
    rectangle3 Bounds;
    rectangle3 UpdatableBounds;

    u32        MaxEntityCount;
    u32        EntityCount;
    sim_entity *Entities;

    sim_entity_hash Hash[4096];

    r32 MaxEntityRadius;
    r32 MaxEntityVelocity;
};

#define HANDMADEHERO_HANDMADE_SIM_REGION_H
#endif //HANDMADEHERO_HANDMADE_SIM_REGION_H
