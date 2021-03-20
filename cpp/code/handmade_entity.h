//
// Created by AgentOfChaos on 1/31/21.
//

#ifndef HANDMADEHERO_HANDMADE_ENTITY_H

#define InvalidP V3(100000.0f, 100000.0f, 100000.0f)

inline bool32
IsSet(sim_entity *Entity, uint32 Flag)
{
    bool32 Result = Entity->Flags & Flag;
    return (Result);
}

inline void
AddFlags(sim_entity *Entity, uint32 Flag)
{
    Entity->Flags |= Flag;
}

inline void
ClearFlags(sim_entity *Entity, uint32 Flag)
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
GetEntityGroundPoint(sim_entity *Entity)
{
    v3 Result = Entity->P;
    return (Result);
}

inline real32
GetStairGround(sim_entity *Entity, v3 AtGroundPoint)
{
    Assert(Entity->Type == EntityType_Stairwell)

    rectangle2 RegionRect = RectCenterDim(Entity->P.XY, Entity->WalkableDim);
    v2 Bary = Clamp01(GetBarycentric(RegionRect, AtGroundPoint.XY));
    real32 Ground = Entity->P.Z + Bary.Y * Entity->WalkableHeight;

    return (Ground);
}

#define HANDMADEHERO_HANDMADE_ENTITY_H
#endif //HANDMADEHERO_HANDMADE_ENTITY_H
