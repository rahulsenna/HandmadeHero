//
// Created by AgentOfChaos on 1/31/21.
//

#ifndef HANDMADEHERO_HANDMADE_ENTITY_H

#define InvalidP V3(100000.0f, 100000.0f, 100000.0f)

inline b32
IsSet(sim_entity *Entity, u32 Flag)
{
    b32 Result = Entity->Flags & Flag;
    return (Result);
}

inline void
AddFlags(sim_entity *Entity, u32 Flag)
{
    Entity->Flags |= Flag;
}

inline void
ClearFlags(sim_entity *Entity, u32 Flag)
{
    Entity->Flags &= ~Flag;
}

inline void
MakeEntityNonSpatial(sim_entity *Entity)
{
    AddFlags(Entity, EntityFlag_NonSpatial);
    Entity->P = InvalidP;
}

inline void
MakeEntitySpatial(sim_entity *Entity, v3 P, v3 deltaP)
{
    ClearFlags(Entity, EntityFlag_NonSpatial);
    Entity->P = P;
    Entity->deltaP = deltaP;
}

inline v3
GetEntityGroundPoint(sim_entity *Entity, v3 ForEntityP)
{
    v3 Result = ForEntityP;
    return (Result);
}

inline v3
GetEntityGroundPoint(sim_entity *Entity)
{
    v3 Result = GetEntityGroundPoint(Entity, Entity->P);
    return (Result);
}

inline r32
GetStairGround(sim_entity *Entity, v3 AtGroundPoint)
{
    Assert(Entity->Type == EntityType_Stairwell)

    rectangle2 RegionRect = RectCenterDim(Entity->P.xy, Entity->WalkableDim);
    v2         Bary       = Clamp01(GetBarycentric(RegionRect, AtGroundPoint.xy));
    r32        Ground     = Entity->P.z + Bary.y * Entity->WalkableHeight;

    return (Ground);
}

#define HANDMADEHERO_HANDMADE_ENTITY_H
#endif //HANDMADEHERO_HANDMADE_ENTITY_H
