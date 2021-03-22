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
AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v3 *SimP);

inline v3
GetSimSpaceP(sim_region *SimRegion, low_entity *Stored)
{
    v3 Result = InvalidP;
    if (!IsSet(&Stored->Sim, EntityFlag_NonSpatial))
    {
        Result = Subtract(SimRegion->World, &Stored->P, &SimRegion->Origin);
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
            v3 P = GetSimSpaceP(SimRegion, LowEntity);
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
                AddFlags(&Source->Sim, EntityFlag_Simming);
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

inline bool32
EntityOverlapsRectangle(v3 P, sim_entity_collision_volume Volume, rectangle3 Rect)
{
    rectangle3 Grown = AddRadiusTo(Rect, 0.5f * Volume.Dim);
    bool32 Result = IsInRectangle(Grown, P + Volume.OffsetP);
    return (Result);
}

internal sim_entity *
AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v3 *SimP)
{
    sim_entity *Dest = AddEntityRaw(GameState, SimRegion, StorageIndex, Source);

    if (Dest)
    {
        if (SimP)
        {
            Dest->P = *SimP;
            Dest->Updatable = EntityOverlapsRectangle(Dest->P,
                                                      Dest->Collision->TotalVolume,
                                                      SimRegion->UpdatableBounds);
        } else
        {
            Dest->P = GetSimSpaceP(SimRegion, Source);
        }
    }

    return (Dest);
}

internal sim_region *
BeginSim(game_state *GameState, memory_arena *SimArena, world *World,
         world_position Origin, rectangle3 Bounds, real32 Deltat)
{
    sim_region *SimRegion = PushStruct(SimArena, sim_region);
    ZeroStruct(SimRegion->Hash);

    SimRegion->MaxEntityRadius = 5.0f;
    SimRegion->MaxEntityVelocity = 30.0f;
    real32 UpdateSafetyMargin = SimRegion->MaxEntityRadius + Deltat * SimRegion->MaxEntityVelocity;
    real32 UpdateSafetyMarginZ = 1.0f;

    SimRegion->World = World;
    SimRegion->Origin = Origin;
    SimRegion->Bounds = Bounds;
    SimRegion->UpdatableBounds = AddRadiusTo(Bounds, V3(SimRegion->MaxEntityRadius,
                                                        SimRegion->MaxEntityRadius,
                                                        SimRegion->MaxEntityRadius));
    SimRegion->Bounds = AddRadiusTo(SimRegion->UpdatableBounds,
                                    V3(UpdateSafetyMargin, UpdateSafetyMargin, UpdateSafetyMarginZ));

    SimRegion->MaxEntityCount = 4096;
    SimRegion->EntityCount = 0;
    SimRegion->Entities = PushArray(SimArena, SimRegion->MaxEntityCount, sim_entity);

    world_position MinChunkP = MapIntoChunkSpace(SimRegion->World,
                                                 SimRegion->Origin,
                                                 GetMinCorner(SimRegion->Bounds));
    world_position MaxChunkP = MapIntoChunkSpace(SimRegion->World,
                                                 SimRegion->Origin,
                                                 GetMaxCorner(SimRegion->Bounds));
    for (int32 ChunkZ = MinChunkP.ChunkZ;
         ChunkZ <= MaxChunkP.ChunkZ;
         ++ChunkZ)
    {
        for (int32 ChunkY = MinChunkP.ChunkY;
             ChunkY <= MaxChunkP.ChunkY;
             ++ChunkY)
        {
            for (int32 ChunkX = MinChunkP.ChunkX;
                 ChunkX <= MaxChunkP.ChunkX;
                 ++ChunkX)
            {
                world_chunk *Chunk = GetWorldChunk(SimRegion->World, ChunkX, ChunkY, ChunkZ);

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
                                v3 SimSpaceP = GetSimSpaceP(SimRegion, Low);
                                if (EntityOverlapsRectangle(SimSpaceP, Low->Sim.Collision->TotalVolume,
                                                            SimRegion->Bounds))
                                {
                                    AddEntity(GameState, SimRegion, LowEntityIndex, Low, &SimSpaceP);
                                }
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

            real32 CamZOffset = NewCameraP.Offset_.Z;
            NewCameraP = Stored->P;
            NewCameraP.Offset_.Z = CamZOffset;
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
CanCollide(game_state *GameState, sim_entity *A, sim_entity *B)
{
    bool32 Result = false;

    if (A != B)
    {
        if (A->StorageIndex > B->StorageIndex)
        {
            sim_entity *Temp = A;
            A = B;
            B = Temp;
        }

        if (IsSet(A, EntityFlag_Collides) && IsSet(B, EntityFlag_Collides))
        {
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
                    Result = Rule->CanCollide;
                    break;
                }
            }
        }
    }

    return (Result);
}

internal bool32
HandleCollision(game_state *GameState, sim_entity *A, sim_entity *B)
{
    bool32 StopsOnCollision;

    if (A->Type == EntityType_Sword)
    {
        AddCollisionRule(GameState, A->StorageIndex, B->StorageIndex, false);
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

internal bool32
CanOverlap(game_state *GameState, sim_entity *Mover, sim_entity *Region)
{
    bool32 Result = false;

    if (Mover != Region)
    {
        if (Region->Type == EntityType_Stairwell)
        {
            Result = true;
        }
    }

    return (Result);
}

internal void
HandleOverlap(game_state *GameState, sim_entity *Mover, sim_entity *Region, real32 deltat, real32 *Ground)
{
    if (Region->Type == EntityType_Stairwell)
    {
        *Ground = GetStairGround(Region, GetEntityGroundPoint(Mover));
    }
}

internal bool32
SpeculativeCollide(sim_entity *Mover, sim_entity *Region)
{
    bool32 Result = true;

    if (Region->Type == EntityType_Stairwell)
    {
        real32 StepHeight = 0.1f;
#if 0
        Result = (AbsoluteValue(GetEntityGroundPoint(Mover).Z - Ground) > StepHeight) ||
                 ((Bary.Y > 0.1f) && (Bary.Y < 0.9f));
#endif

        v3 MoverGroundPoint = GetEntityGroundPoint(Mover);
        real32 Ground = GetStairGround(Region, MoverGroundPoint);
        Result = (AbsoluteValue(MoverGroundPoint.Z - Ground) > StepHeight);
    }

    return (Result);
}

internal void
MoveEntity(game_state *GameState, sim_region *SimRegion, sim_entity *Entity, v3 accelOfEntity, move_spec *MoveSpec,
           real32 deltat)
{
    Assert(!IsSet(Entity, EntityFlag_NonSpatial))

    world *World = SimRegion->World;

    if (MoveSpec->UnitMaxAccelVector)
    {
        real32 accelLength = LengthSq(accelOfEntity);

        if (accelLength > 1.0f)
        {
            accelOfEntity *= 1.0f / SquareRoot(accelLength);
        }
    }

    accelOfEntity *= MoveSpec->Speed;
    v3 Drag = -MoveSpec->Drag * Entity->deltaP;
    Drag.Z = 0.0f;
    accelOfEntity += Drag;
    if (!IsSet(Entity, EntityFlag_ZSupported))
    {
        accelOfEntity += V3(0, 0, -9.8f);
    }

    v3 OldEntityP = Entity->P;
    v3 EntityDelta = (0.5f * accelOfEntity * Square(deltat) +
                      Entity->deltaP * deltat);
    Entity->deltaP = accelOfEntity * deltat + Entity->deltaP;
    Assert(LengthSq(Entity->deltaP) <= Square(SimRegion->MaxEntityVelocity))
    v3 NewEntityP = OldEntityP + EntityDelta;

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

            v3 WallNormal = {};
            sim_entity *HitEntity = 0;

            v3 DesiredPosition = Entity->P + EntityDelta;

            if (!IsSet(Entity, EntityFlag_NonSpatial))
            {
                for (uint32 TestHighEntityIndex = 0;
                     TestHighEntityIndex < SimRegion->EntityCount;
                     ++TestHighEntityIndex)
                {

                    sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;

                    if (CanCollide(GameState, Entity, TestEntity))
                    {
                        for (uint32 VolumeIndex = 0;
                             VolumeIndex < Entity->Collision->VolumeCount;
                             ++VolumeIndex)
                        {
                            sim_entity_collision_volume *Volume =
                                    Entity->Collision->Volumes + VolumeIndex;

                            for (uint32 TestVolumeIndex = 0;
                                 TestVolumeIndex < TestEntity->Collision->VolumeCount;
                                 ++TestVolumeIndex)
                            {
                                sim_entity_collision_volume *TestVolume =
                                        TestEntity->Collision->Volumes + TestVolumeIndex;

                                v3 MinkowskiDiameter = {TestVolume->Dim.X + Volume->Dim.X,
                                                        TestVolume->Dim.Y + Volume->Dim.Y,
                                                        TestVolume->Dim.Z + Volume->Dim.Z};

                                v3 MinCorner = -0.5f * MinkowskiDiameter;
                                v3 MaxCorner = 0.5f * MinkowskiDiameter;
                                v3 Rel = ((Entity->P + Volume->OffsetP) - (TestEntity->P + TestVolume->OffsetP));

                                if (Rel.Z >= MinCorner.Z && Rel.Z <= MaxCorner.Z)
                                {
                                    real32 tMinTest = tMin;
                                    v3 TestWallNormal = {};
                                    bool32 HitThis = false;

                                    if (TestWall(MinCorner.X, EntityDelta.X, EntityDelta.Y, Rel.X, Rel.Y, &tMinTest,
                                                 MinCorner.Y, MaxCorner.Y))
                                    {
                                        TestWallNormal = V3(-1, 0, 0);
                                        HitThis = true;
                                    }
                                    if (TestWall(MaxCorner.X, EntityDelta.X, EntityDelta.Y, Rel.X, Rel.Y, &tMinTest,
                                                 MinCorner.Y, MaxCorner.Y))
                                    {
                                        TestWallNormal = V3(1, 0, 0);
                                        HitThis = true;
                                    }

                                    if (TestWall(MinCorner.Y, EntityDelta.Y, EntityDelta.X, Rel.Y, Rel.X, &tMinTest,
                                                 MinCorner.X, MaxCorner.X))
                                    {
                                        TestWallNormal = V3(0, -1, 0);
                                        HitThis = true;
                                    }
                                    if (TestWall(MaxCorner.Y, EntityDelta.Y, EntityDelta.X, Rel.Y, Rel.X, &tMinTest,
                                                 MinCorner.X, MaxCorner.X))
                                    {
                                        TestWallNormal = V3(0, 1, 0);
                                        HitThis = true;
                                    }

                                    if (HitThis)
                                    {
                                        v3 TestP = Entity->P + tMinTest * EntityDelta;

                                        if (SpeculativeCollide(Entity, TestEntity))
                                        {
                                            tMin = tMinTest;
                                            WallNormal = TestWallNormal;
                                            HitEntity = TestEntity;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }

            Entity->P += tMin * EntityDelta;
            DistanceRemaining -= tMin * EntityDeltaLength;

            if (HitEntity)
            {
                EntityDelta = DesiredPosition - Entity->P;

                bool32 StopsOnCollision = HandleCollision(GameState, Entity, HitEntity);

                if (StopsOnCollision)
                {
                    Entity->deltaP = Entity->deltaP - (DotProduct(Entity->deltaP, WallNormal) * WallNormal);
                    EntityDelta = EntityDelta - (DotProduct(EntityDelta, WallNormal) * WallNormal);
                }
            } else
            {
                break;
            }
        } else
        {
            break;
        }
    }

    real32 Ground = 0.0f;

    {
        rectangle3 EntityRect = RectCenterDim(Entity->P + Entity->Collision->TotalVolume.OffsetP,
                                              Entity->Collision->TotalVolume.Dim);

        for (uint32 TestHighEntityIndex = 0;
             TestHighEntityIndex < SimRegion->EntityCount;
             ++TestHighEntityIndex)
        {
            sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;

            if (CanOverlap(GameState, Entity, TestEntity))
            {
                rectangle3 TestEntityRect = RectCenterDim(TestEntity->P + TestEntity->Collision->TotalVolume.OffsetP,
                                                          TestEntity->Collision->TotalVolume.Dim);
                if (RectangleIntersect(EntityRect, TestEntityRect))
                {
                    HandleOverlap(GameState, Entity, TestEntity, deltat, &Ground);
                }
            }
        }
    }

    Ground += Entity->P.Z - GetEntityGroundPoint(Entity).Z;
    if ((Entity->P.Z <= Ground) ||
        (IsSet(Entity, EntityFlag_ZSupported) &&
         (Entity->deltaP.Z == 0.0f)))
    {
        Entity->P.Z = Ground;
        Entity->deltaP.Z = 0;
        AddFlags(Entity, EntityFlag_ZSupported);
    } else
    {
        ClearFlags(Entity, EntityFlag_ZSupported);
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