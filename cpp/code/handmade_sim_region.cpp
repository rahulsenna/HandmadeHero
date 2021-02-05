//
// Created by AgentOfChaos on 1/30/21.
//


inline void
StoreEntityReference(entity_reference *Ref)
{
    if (Ref->Ptr != 0)
    {
        Ref->Index = Ref->Ptr->StorageIndex;
    }
}

internal sim_entity_hash *
GetHashFromStorageIndex(sim_region *SimRegion, uint32 StorageIndex)
{
    Assert(StorageIndex)

    sim_entity_hash *Result = 0;

    uint32 HashValue = StorageIndex;

    for (uint32 Offset = 0;
         Offset < ArrayCount(SimRegion->Hash);
         ++Offset)
    {
        sim_entity_hash *Entry = SimRegion->Hash +
                                 ((HashValue + Offset) & (ArrayCount(SimRegion->Hash) - 1));

        if ((Entry->Index == StorageIndex) || (Entry->Index == 0))
        {
            Result = Entry;
            break;
        }
    }

    return (Result);
}

internal sim_entity *
AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v2 *SimP);

inline v2
GetSimSpaceP(sim_region *SimRegion, low_entity *Stored)
{
    v2 Result = InvalidP;
    if (!IsSet(&Stored->Sim, EntityFlag_NonSpatial))
    {
        world_difference Diff = Subtract(SimRegion->World, &Stored->P, &SimRegion->Origin);
        Result = Diff.deltaXY;
    }
    return (Result);
}

inline void
LoadEntityReference(game_state *GameState, sim_region *SimRegion, entity_reference *Ref)
{
    if (Ref->Index)
    {
        sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, Ref->Index);
        if (Entry->Ptr == 0)
        {
            Entry->Index = Ref->Index;
            low_entity *LowEntity = GetLowEntity(GameState, Ref->Index);
            v2 P = GetSimSpaceP(SimRegion, LowEntity);
            Entry->Ptr = AddEntity(GameState, SimRegion, Ref->Index, LowEntity, &P);
        }

        Ref->Ptr = Entry->Ptr;
    }
}

internal sim_entity *
AddEntityRaw(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source)
{
    Assert(StorageIndex)
    sim_entity *Entity = 0;

    sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, StorageIndex);
    if (Entry->Ptr == 0)
    {
        if (SimRegion->EntityCount < SimRegion->MaxEntityCount)
        {
            Entity = SimRegion->Entities + SimRegion->EntityCount++;

            Entry->Index = StorageIndex;
            Entry->Ptr = Entity;

            if (Source)
            {
                *Entity = Source->Sim;
                LoadEntityReference(GameState, SimRegion, &Entity->Sword);

                Assert(!IsSet(&Source->Sim, EntityFlag_Simming))
                AddFlag(&Source->Sim, EntityFlag_Simming);
            }

            Entity->StorageIndex = StorageIndex;
            Entity->Updatable = false;
        } else
        {
            InvalidCodePath
        }
    }

    return (Entity);
}

internal sim_entity *
AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v2 *SimP)
{
    sim_entity *Dest = AddEntityRaw(GameState, SimRegion, StorageIndex, Source);

    if (Dest)
    {
        if (SimP)
        {
            Dest->P = *SimP;
            Dest->Updatable = IsInRectangle(SimRegion->UpdatableBounds, Dest->P);
        } else
        {
            Dest->P = GetSimSpaceP(SimRegion, Source);
        }
    }

    return (Dest);
}

internal sim_region *
BeginSim(game_state *GameState, memory_arena *SimArena, world *World, world_position Origin, rectangle2 Bounds)
{

    sim_region *SimRegion = PushStruct(SimArena, sim_region);
    ZeroStruct(SimRegion->Hash);

    real32 UpdateSafetyMargin = 1.0f;

    SimRegion->World = World;
    SimRegion->Origin = Origin;
    SimRegion->Bounds = Bounds;
    SimRegion->UpdatableBounds = Bounds;
    SimRegion->Bounds = AddRadiusTo(SimRegion->UpdatableBounds, UpdateSafetyMargin, UpdateSafetyMargin);

    SimRegion->MaxEntityCount = 4096;
    SimRegion->EntityCount = 0;
    SimRegion->Entities = PushArray(SimArena, SimRegion->MaxEntityCount, sim_entity);

    world_position MinChunkP = MapIntoChunkSpace(SimRegion->World, SimRegion->Origin, GetMinCorner(SimRegion->Bounds));
    world_position MaxChunkP = MapIntoChunkSpace(SimRegion->World, SimRegion->Origin, GetMaxCorner(SimRegion->Bounds));

    for (int32 ChunkY = MinChunkP.ChunkY;
         ChunkY <= MaxChunkP.ChunkY;
         ++ChunkY)
    {
        for (int32 ChunkX = MinChunkP.ChunkX;
             ChunkX <= MaxChunkP.ChunkX;
             ++ChunkX)
        {
            world_chunk *Chunk = GetWorldChunk(SimRegion->World, ChunkX, ChunkY, SimRegion->Origin.ChunkZ);

            if (Chunk)
            {
                for (world_entity_block *Block = &Chunk->FirstBlock;
                     Block;
                     Block = Block->Next)
                {
                    for (uint32 EntityIndex = 0;
                         EntityIndex < Block->EntityCount;
                         ++EntityIndex)
                    {
                        uint32 LowEntityIndex = Block->LowEntityIndex[EntityIndex];
                        low_entity *Low = GameState->LowEntities + LowEntityIndex;

                        if (!IsSet(&Low->Sim, EntityFlag_NonSpatial))
                        {
                            v2 SimSpaceP = GetSimSpaceP(SimRegion, Low);
                            if (IsInRectangle(SimRegion->Bounds, SimSpaceP))
                            {
                                AddEntity(GameState, SimRegion, LowEntityIndex, Low, &SimSpaceP);
                            }
                        }
                    }
                }
            }
        }
    }

    return (SimRegion);
}

internal void
EndSim(game_state *GameState, sim_region *SimRegion)
{
    sim_entity *Entity = SimRegion->Entities;

    for (uint32 EntityIndex = 0;
         EntityIndex < SimRegion->EntityCount;
         ++EntityIndex, ++Entity)
    {
        low_entity *Stored = GameState->LowEntities + Entity->StorageIndex;

        Assert(IsSet(&Stored->Sim, EntityFlag_Simming))

        Stored->Sim = *Entity;

        Assert(!IsSet(&Stored->Sim, EntityFlag_Simming))

        StoreEntityReference(&Stored->Sim.Sword);

        world_position NewP = IsSet(Entity, EntityFlag_NonSpatial) ?
                              NullPosition() :
                              MapIntoChunkSpace(GameState->World, SimRegion->Origin, Entity->P);
        ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity->StorageIndex,
                             Stored, NewP);

        if (Entity->StorageIndex == GameState->CameraFollowingEntityIndex)
        {
            world_position NewCameraP = GameState->CameraP;
            NewCameraP.ChunkZ = Stored->P.ChunkZ;

#if 0
            if (CameraFollowingEntity.High->P.X > (9.0f * World->TileSidpeInMeters))
            {
                NewCameraP.ChunkX += 17;
            }
            if (CameraFollowingEntity.High->P.X < (-9.0f * World->TileSideInMeters))
            {
                NewCameraP.ChunkX -= 17;
            }
            if (CameraFollowingEntity.High->P.Y > (5.0f * World->TileSideInMeters))
            {
                NewCameraP.ChunkY += 9;
            }
            if (CameraFollowingEntity.High->P.Y < (-5.0f * World->TileSideInMeters))
            {
                NewCameraP.ChunkY -= 9;
            }
#else

            NewCameraP = Stored->P;
#endif
            GameState->CameraP = NewCameraP;
        }
    }
}

internal bool32
TestWall(real32 WallX, real32 PlayerdeltaX, real32 PlayerdeltaY,
         real32 RelX, real32 RelY, real32 *tMin,
         real32 MinY, real32 MaxY)
{
    bool32 Hit = false;
    real32 tEpsilon = 0.001f;
    if (PlayerdeltaX != 0.0f)
    {
        real32 tResult = (WallX - RelX) / PlayerdeltaX;
        real32 Y = RelY + tResult * PlayerdeltaY;

        if ((tResult >= 0.0f) && (*tMin > tResult))
        {
            if ((Y >= MinY) && (Y <= MaxY))
            {
                *tMin = MAXIMUM(0.0f, tResult - tEpsilon);
                Hit = true;
            }
        }
    }
    return (Hit);
}



internal bool32
ShouldCollide(game_state *GameState, sim_entity *A, sim_entity *B)
{
    bool32 Result = false;

    if (A->StorageIndex > B->StorageIndex)
    {
        sim_entity *Temp = A;
        A = B;
        B = Temp;
    }

    if (!IsSet(A, EntityFlag_NonSpatial) &&
        !IsSet(B, EntityFlag_NonSpatial))
    {
        Result = true;
    }

    uint32 HashBucket = A->StorageIndex & (ArrayCount((GameState->CollisionRuleHash) - 1));
    for (pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashBucket];
         Rule;
         Rule = Rule->NextInHash)
    {
        if ((Rule->StorageIndexA == A->StorageIndex) &&
            (Rule->StorageIndexB == B->StorageIndex))
        {
            Result = Rule->ShouldCollide;
            break;
        }
    }

    return (Result);
}

internal bool32
HandleCollision(sim_entity *A, sim_entity *B)
{
    bool32 StopsOnCollision = false;

    if (A->Type == EntityType_Sword)
    {
        StopsOnCollision = false;
    } else
    {
        StopsOnCollision = true;
    }

    if (A->Type > B->Type)
    {
        sim_entity *Temp = A;
        A = B;
        B = Temp;
    }

    if ((B->Type == EntityType_Sword) &&
        (A->Type == EntityType_Monster))
    {
        if (A->HitPointMax > 0)
        {
            --A->HitPointMax;
        }
    }
    return (StopsOnCollision);
}

internal void
MoveEntity(game_state *GameState, sim_region *SimRegion, sim_entity *Entity, v2 accelOfEntity, move_spec *MoveSpec,
           real32 deltat)
{
    Assert(!IsSet(Entity, EntityFlag_NonSpatial))

    world *TileMap = SimRegion->World;

    if (MoveSpec->UnitMaxAccelVector)
    {
        real32 accelLength = LengthSq(accelOfEntity);

        if (accelLength > 1.0f)
        {
            accelOfEntity *= 1.0f / SquareRoot(accelLength);
        }
    }

    accelOfEntity *= MoveSpec->Speed;
    accelOfEntity += -MoveSpec->Drag * Entity->deltaP;

    v2 OldPlayerP = Entity->P;
    v2 EntityDelta = (0.5f * accelOfEntity * Square(deltat) +
                      Entity->deltaP * deltat);
    Entity->deltaP = accelOfEntity * deltat + Entity->deltaP;
    v2 NewPlayerP = OldPlayerP + EntityDelta;

    real32 accelOfPlayerInZ = -9.8f;
    real32 deltaZ = (0.5f * accelOfPlayerInZ * Square(deltat) + Entity->deltaZ * deltat);
    Entity->Z += deltaZ;
    Entity->deltaZ = accelOfPlayerInZ * deltat + Entity->deltaZ;
    if (Entity->Z < 0)
    {
        Entity->Z = 0;
    }

    real32 DistanceLimit = Entity->DistanceLimit;
    if (DistanceLimit == 0.0f)
    {
        DistanceLimit = 10000.0f;
    }

    real32 DistanceRemaining = DistanceLimit;
    for (uint8 Iteration = 0; Iteration < 4; ++Iteration)
    {
        real32 tMin = 1.0f;

        real32 EntityDeltaLength = Length(EntityDelta);
        if (EntityDeltaLength > 0.0f)
        {
            if (EntityDeltaLength > DistanceRemaining)
            {
                tMin = (DistanceRemaining / EntityDeltaLength);
            }

            v2 WallNormal = {};
            sim_entity *HitEntity = 0;

            v2 DesiredPosition = Entity->P + EntityDelta;

            if (!IsSet(Entity, EntityFlag_NonSpatial))
            {
                for (uint32 TestHighEntityIndex = 0;
                     TestHighEntityIndex < SimRegion->EntityCount;
                     ++TestHighEntityIndex)
                {

                    sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;

                    if (ShouldCollide(GameState, Entity, TestEntity))
                    {
                        real32 DiameterW = TestEntity->Width + Entity->Width;
                        real32 DiameterH = TestEntity->Height + Entity->Height;

                        v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
                        v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);
                        v2 Rel = Entity->P - TestEntity->P;

                        if (TestWall(MinCorner.X, EntityDelta.X, EntityDelta.Y, Rel.X, Rel.Y, &tMin,
                                     MinCorner.Y, MaxCorner.Y))
                        {
                            WallNormal = V2(-1, 0);
                            HitEntity = TestEntity;
                        }
                        if (TestWall(MaxCorner.X, EntityDelta.X, EntityDelta.Y, Rel.X, Rel.Y, &tMin,
                                     MinCorner.Y, MaxCorner.Y))
                        {
                            WallNormal = V2(1, 0);
                            HitEntity = TestEntity;
                        }

                        if (TestWall(MinCorner.Y, EntityDelta.Y, EntityDelta.X, Rel.Y, Rel.X, &tMin,
                                     MinCorner.X, MaxCorner.X))
                        {
                            WallNormal = V2(0, -1);
                            HitEntity = TestEntity;
                        }
                        if (TestWall(MaxCorner.Y, EntityDelta.Y, EntityDelta.X, Rel.Y, Rel.X, &tMin,
                                     MinCorner.X, MaxCorner.X))
                        {
                            WallNormal = V2(0, 1);
                            HitEntity = TestEntity;
                        }
                    }
                }
            }

            Entity->P += tMin * EntityDelta;
            DistanceRemaining -= tMin * EntityDeltaLength;

            if (HitEntity)
            {
                EntityDelta = DesiredPosition - Entity->P;

                bool32 StopsOnCollision = HandleCollision(Entity, HitEntity);
                if (StopsOnCollision)
                {
                    Entity->deltaP = Entity->deltaP - (DotProduct(WallNormal, Entity->deltaP) * WallNormal);
                    EntityDelta = EntityDelta - (DotProduct(EntityDelta, WallNormal) * WallNormal);
                } else
                {
                    AddCollisionRule(GameState, Entity->StorageIndex, HitEntity->StorageIndex, false);
                }


//                Entity->High->ChunkZ += HitLow->deltaAbsTileZ;
            } else
            {
                break;
            }
        } else
        {
            break;
        }
    }

    if (Entity->DistanceLimit != 0.0f)
    {
        Entity->DistanceLimit = DistanceRemaining;
    }

    if ((AbsoluteValue(Entity->deltaP.X) == 0.0f) && (AbsoluteValue(Entity->deltaP.Y) == 0.0f))
    {
    } else if (AbsoluteValue(Entity->deltaP.X) > AbsoluteValue(Entity->deltaP.Y))
    {
        if (Entity->deltaP.X > 0)
        {
            Entity->FacingDirection = 2;
        } else
        {
            Entity->FacingDirection = 3;
        }
    } else
    {
        if (Entity->deltaP.Y > 0)
        {
            Entity->FacingDirection = 0;
        } else
        {
            Entity->FacingDirection = 1;
        }
    }
}