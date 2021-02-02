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

inline void
LoadEntityReference(game_state *GameState, sim_region *SimRegion, entity_reference *Ref)
{
    if (Ref->Index)
    {
        sim_entity_hash *Entry = GetHashFromStorageIndex(SimRegion, Ref->Index);
        if (Entry->Ptr == 0)
        {
            Entry->Index = Ref->Index;
            Entry->Ptr = AddEntity(GameState, SimRegion, Ref->Index, GetLowEntity(GameState, Ref->Index), 0);
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
        } else
        {
            InvalidCodePath
        }
    }

    return (Entity);
}

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

internal sim_entity *
AddEntity(game_state *GameState, sim_region *SimRegion, uint32 StorageIndex, low_entity *Source, v2 *SimP)
{
    sim_entity *Dest = AddEntityRaw(GameState, SimRegion, StorageIndex, Source);

    if (Dest)
    {
        if (SimP)
        {
            Dest->P = *SimP;
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

    SimRegion->World = World;
    SimRegion->Origin = Origin;
    SimRegion->Bounds = Bounds;

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

internal void
MoveEntity(sim_region *SimRegion, sim_entity *Entity, v2 accelOfEntity, move_spec *MoveSpec, real32 deltat)
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
    v2 Playerdelta = (0.5f * accelOfEntity * Square(deltat) +
                      Entity->deltaP * deltat);
    Entity->deltaP = accelOfEntity * deltat + Entity->deltaP;
    v2 NewPlayerP = OldPlayerP + Playerdelta;

    for (uint8 Iteration = 0; Iteration < 4; ++Iteration)
    {
        real32 tMin = 1.0f;
        v2 WallNormal = {};
        sim_entity *HitEntity = 0;

        v2 DesiredPosition = Entity->P + Playerdelta;

        if (IsSet(Entity, EntityFlag_Collides) &&
            !IsSet(Entity, EntityFlag_NonSpatial))
        {
            for (uint32 TestHighEntityIndex = 0;
                 TestHighEntityIndex < SimRegion->EntityCount;
                 ++TestHighEntityIndex)
            {

                sim_entity *TestEntity = SimRegion->Entities + TestHighEntityIndex;

                if (Entity != TestEntity)
                {
                    if (IsSet(TestEntity, EntityFlag_Collides) &&
                        !IsSet(Entity, EntityFlag_NonSpatial))
                    {
                        real32 DiameterW = TestEntity->Width + Entity->Width;
                        real32 DiameterH = TestEntity->Height + Entity->Height;

                        v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
                        v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);
                        v2 Rel = Entity->P - TestEntity->P;

                        if (TestWall(MinCorner.X, Playerdelta.X, Playerdelta.Y, Rel.X, Rel.Y, &tMin,
                                     MinCorner.Y, MaxCorner.Y))
                        {
                            WallNormal = V2(-1, 0);
                            HitEntity = TestEntity;
                        }
                        if (TestWall(MaxCorner.X, Playerdelta.X, Playerdelta.Y, Rel.X, Rel.Y, &tMin,
                                     MinCorner.Y, MaxCorner.Y))
                        {
                            WallNormal = V2(1, 0);
                            HitEntity = TestEntity;
                        }

                        if (TestWall(MinCorner.Y, Playerdelta.Y, Playerdelta.X, Rel.Y, Rel.X, &tMin,
                                     MinCorner.X, MaxCorner.X))
                        {
                            WallNormal = V2(0, -1);
                            HitEntity = TestEntity;
                        }
                        if (TestWall(MaxCorner.Y, Playerdelta.Y, Playerdelta.X, Rel.Y, Rel.X, &tMin,
                                     MinCorner.X, MaxCorner.X))
                        {
                            WallNormal = V2(0, 1);
                            HitEntity = TestEntity;
                        }
                    }
                }
            }
        }

        Entity->P += tMin * Playerdelta;
        if (HitEntity)
        {
            Entity->deltaP = Entity->deltaP - (DotProduct(WallNormal, Entity->deltaP) * WallNormal);
            Playerdelta = DesiredPosition - Entity->P;
            Playerdelta = Playerdelta - (DotProduct(Playerdelta, WallNormal) * WallNormal);
//            Entity->High->ChunkZ += HitLow->deltaAbsTileZ;
        } else
        {
            break;
        }
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