//
// Created by AgentOfChaos on 1/31/21.
//

#ifndef HANDMADEHERO_HANDMADE_ENTITY_H

#define InvalidP V2(100000.0f, 100000.0f)

inline bool32
IsSet(sim_entity *Entity, uint32 Flag)
{
    bool32 Result = Entity->Flags & Flag;
    return (Result);
}

inline void
AddFlag(sim_entity *Entity, uint32 Flag)
{
    Entity->Flags |= Flag;
}

inline void
ClearFlag(sim_entity *Entity, uint32 Flag)
{
    Entity->Flags &= ~Flag;
}

inline void
MakeEntityNonSpatial(sim_entity *Entity)
{
    AddFlag(Entity, EntityFlag_NonSpatial);
    Entity->P = InvalidP;
}

inline void
MakeEntitySpatial(sim_entity *Entity, v2 P, v2 deltaP)
{
    ClearFlag(Entity, EntityFlag_NonSpatial);
    Entity->P = P;
    Entity->deltaP = deltaP;
}

#define HANDMADEHERO_HANDMADE_ENTITY_H
#endif //HANDMADEHERO_HANDMADE_ENTITY_H
