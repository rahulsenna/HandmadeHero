//
// Created by AgentOfChaos on 1/31/21.
//

inline move_spec
DefaultMoveSpec()
{
    move_spec MoveSpec;
    MoveSpec.UnitMaxAccelVector = false;
    MoveSpec.Speed = 1.0f;
    MoveSpec.Drag = 0.0f;
    return (MoveSpec);
}

inline void
UpdateFamiliar(sim_region *SimRegion, sim_entity *Entity, real32 deltat)
{
    sim_entity *ClosestHero = 0;
    real32 ClosestHeroDSq = Square(10.0f);

    sim_entity *TestEntity = SimRegion->Entities;

    for (uint32 HighEntityIndex = 1;
         HighEntityIndex < SimRegion->EntityCount;
         ++HighEntityIndex)
    {
        if (TestEntity->Type == EntityType_Hero)
        {
            real32 TestDSq = LengthSq(TestEntity->P - Entity->P);
            if (TestDSq < ClosestHeroDSq)
            {
                ClosestHero = TestEntity;
                ClosestHeroDSq = TestDSq;
            }
        }
    }
    v2 accelOfEntity = {};
    if (ClosestHero && ClosestHeroDSq > Square(3.0f))
    {
        real32 accel = 0.5f;
        real32 OneOverLength = accel / SquareRoot(ClosestHeroDSq);
        accelOfEntity = OneOverLength * (ClosestHero->P - Entity->P);
    }
    move_spec MoveSpec = DefaultMoveSpec();
    MoveSpec.UnitMaxAccelVector = true;
    MoveSpec.Speed = 70.0f;
    MoveSpec.Drag = 7.0f;
    MoveEntity(SimRegion, Entity, accelOfEntity, &MoveSpec, deltat);
}

inline void
UpdateSword(sim_region *SimRegion, sim_entity *Entity, real32 deltat)
{
    if (!IsSet(Entity, EntityFlag_NonSpatial))
    {
        move_spec MoveSpec = DefaultMoveSpec();
        MoveSpec.Speed = 0.0f;
        MoveSpec.Drag = 0.0f;

        v2 OldP = Entity->P;
        MoveEntity(SimRegion, Entity, V2(0, 0), &MoveSpec, deltat);
        real32 DistanceTraveled = Length(OldP - Entity->P);

        Entity->DistanceRemaining -= DistanceTraveled;
        if (Entity->DistanceRemaining < 0.0f)
        {
            MakeEntityNonSpatial(Entity);
        }
    }
}

inline void
UpdateMonster(sim_region *SimRegion, sim_entity *Entity, real32 deltat)
{
}