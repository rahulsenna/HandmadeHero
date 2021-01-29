//
// Created by AgentOfChaos on 11/20/2020.
//
#include "handmade_platform.h"

#include "handmade.h"
#include "handmade_world.cpp"
#include "handmade_random.h"

internal void
GameOutputSound(game_sound_output_buffer *SoundBuffer, game_state *GameState)
{
    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0;
         SampleIndex < SoundBuffer->SampleCount;
         ++SampleIndex)
    {
#if 0
        int16 ToneVolume = 3000;
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16) (SineValue * (real32) ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
#if 0
        int WavePeriod = SoundBuffer->SamplesPerSecond / 400;
        GameState->tSine += 2.0f * PII32 * 1.0f / (real32) WavePeriod;
        if (GameState->tSine > 2.0f * PII32)
        {
            GameState->tSine -= 2.0f * PII32;
        }
#endif
    }
}

#if 0
void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
    uint8 *Row = (uint8 *) Buffer->Memory;
    for (int y = 0; y < Buffer->Height; ++y)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int x = 0; x < Buffer->Width; ++x)
        {
            uint8 Blue = (uint8) (x + XOffset);
            uint8 Green = (uint8) (y + YOffset);
            *Pixel++ = ((Green << 16) | Blue);
        }
        Row += Buffer->Pitch;
    }
}
#endif

#if HANDMADE_WIN32
#define RED_PLACE 16
#define GREEN_PLACE 8
#define BLUE_PLACE 0
#else
#define RED_PLACE 0
#define GREEN_PLACE 8
#define BLUE_PLACE 16
#endif

internal void
DrawRectangle(game_offscreen_buffer *Buffer, v2 vMin, v2 vMax,
              real32 R, real32 G, real32 B)
{
    int32 MinX = RoundReal32ToInt32(vMin.X);
    int32 MinY = RoundReal32ToInt32(vMin.Y);
    int32 MaxX = RoundReal32ToInt32(vMax.X);
    int32 MaxY = RoundReal32ToInt32(vMax.Y);
    if (MinX < 0)
    {
        MinX = 0;
    }
    if (MinY < 0)
    {
        MinY = 0;
    }
    if (MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    if (MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    uint32 Color = ((RoundReal32ToUInt32(R * 255.0f) << RED_PLACE) |
                    (RoundReal32ToUInt32(G * 255.0f) << GREEN_PLACE) |
                    (RoundReal32ToUInt32(B * 255.0f) << BLUE_PLACE) |
                    (RoundReal32ToUInt32(255.0f) << 24));
    uint8 *Row = ((uint8 *) Buffer->Memory +
                  MinX * Buffer->BytesPerPixel +
                  MinY * Buffer->Pitch);
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }
        Row += Buffer->Pitch;
    }
}

internal void
DrawBitmap(game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY,
           real32 CAlpha = 1.0f)
{
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;

    int32 SourceOffsetX = 0;
    if (MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX = 0;
    }
    int32 SourceOffsetY = 0;
    if (MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY = 0;
    }
    if (MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    if (MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }

    uint32 *SourceRow = Bitmap->Pixels + Bitmap->Width * (Bitmap->Height - 1);
    SourceRow += -Bitmap->Width * SourceOffsetY + SourceOffsetX;
    uint8 *DestRow = ((uint8 *) Buffer->Memory +
                      MinX * Buffer->BytesPerPixel +
                      MinY * Buffer->Pitch);
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Dest = (uint32 *) DestRow;
        uint32 *Source = SourceRow;
        for (int X = MinX; X < MaxX; ++X)
        {
            real32 A = (real32) ((*Source >> 24) & 0xFF) / 255.0f;
            A *= CAlpha;
            real32 SR = (real32) ((*Source >> RED_PLACE) & 0xFF);
            real32 SG = (real32) ((*Source >> GREEN_PLACE) & 0xFF);
            real32 SB = (real32) ((*Source >> BLUE_PLACE) & 0xFF);

            real32 DR = (real32) ((*Dest >> RED_PLACE) & 0xFF);
            real32 DG = (real32) ((*Dest >> GREEN_PLACE) & 0xFF);
            real32 DB = (real32) ((*Dest >> BLUE_PLACE) & 0xFF);

            real32 R = (1.0f - A) * DR + A * SR;
            real32 G = (1.0f - A) * DG + A * SG;
            real32 B = (1.0f - A) * DB + A * SB;

            *Dest = (((uint32) (R + 0.5f) << RED_PLACE) |
                     ((uint32) (G + 0.5f) << GREEN_PLACE) |
                     ((uint32) (B + 0.5f) << BLUE_PLACE) |
                     ((uint32) (255.0f) << 24));
            Dest++;
            Source++;
        }
        DestRow += Buffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}

internal void
InitializeArena(memory_arena *Arena, mem_index Size, uint8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#pragma pack(push, 1)
struct bitmap_header
{
    uint16 FileType;
    uint32 FileSize;
    uint16 FileReserved1;
    uint16 FileReserved2;
    uint32 FileOffBits;

    uint32 Size;
    uint32 Width;
    uint32 Height;
    uint16 Planes;
    uint16 BitCount;

    uint32 Compression;
    uint32 SizeImage;
    int32 XPelsPerMeter;
    int32 YPelsPerMeter;
    uint32 ClrUsed;
    uint32 ClrImportant;
    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
    uint32 AlphaMask;
};
#pragma pack(pop)

internal loaded_bitmap
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename)
{
    debug_read_file_result ReadResult = ReadEntireFile(Thread, Filename);

    loaded_bitmap Result = {};
    if (ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *) ReadResult.Contents;
        uint32 *Pixels = (uint32 *) ((uint8 *) ReadResult.Contents + Header->FileOffBits);
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        Result.Pixels = Pixels;

        Assert(Header->Compression == 3)

        bit_scan_result RedScan = FindLeastSignificantSetBit(Header->RedMask);
        bit_scan_result GreenScan = FindLeastSignificantSetBit(Header->GreenMask);
        bit_scan_result BlueScan = FindLeastSignificantSetBit(Header->BlueMask);
        bit_scan_result AlphaScan = FindLeastSignificantSetBit(Header->AlphaMask);

        Assert(RedScan.Found)
        Assert(GreenScan.Found)
        Assert(BlueScan.Found)
        Assert(AlphaScan.Found)

        int32 RedShift = RED_PLACE - (int32) RedScan.Index;
        int32 GreenShift = GREEN_PLACE - (int32) GreenScan.Index;
        int32 BlueShift = BLUE_PLACE - (int32) BlueScan.Index;
        int32 AlphaShift = 24 - (int32) AlphaScan.Index;

        uint32 *Source = Pixels;
        for (uint32 Y = 0; Y < Header->Height; ++Y)
        {
            for (uint32 X = 0; X < Header->Width; ++X)
            {
                uint32 C = *Source;
                *Source = (RotateLeft(C & Header->RedMask, RedShift)) |
                          (RotateLeft(C & Header->GreenMask, GreenShift)) |
                          (RotateLeft(C & Header->BlueMask, BlueShift)) |
                          (RotateLeft(C & Header->AlphaMask, AlphaShift));
                ++Source;
            }
        }
    }
    return (Result);
}

internal low_entity *
GetLowEntity(game_state *GameState, uint32 Index)
{
    low_entity *Result = 0;
    if ((Index > 0) && (Index < GameState->LowEntityCount))
    {
        Result = GameState->LowEntities + Index;
    }
    return (Result);
}

inline v2
GetCameraSpaceP(game_state *GameState, low_entity *EntityLow)
{
    world_difference Diff = Subtract(GameState->World,
                                     &EntityLow->P,
                                     &GameState->CameraP);
    v2 Result = Diff.deltaXY;
    return (Result);
}

inline high_entity *
MakeEntityHighFrequency(game_state *GameState, low_entity *EntityLow, uint32 LowIndex, v2 CameraSpaceP)
{
    high_entity *EntityHigh = 0;
    Assert(EntityLow->HighEntityIndex == 0)
    if (EntityLow->HighEntityIndex == 0)
    {
        if (GameState->HighEntityCount < ArrayCount(GameState->HighEntities_))
        {
            uint32 HighIndex = GameState->HighEntityCount++;
            EntityHigh = GameState->HighEntities_ + HighIndex;

            EntityHigh->P = CameraSpaceP;
            EntityHigh->deltaP = V2(0, 0);
            EntityHigh->ChunkZ = EntityLow->P.ChunkZ;
            EntityHigh->FacingDirection = 0;
            EntityHigh->LowEntityIndex = LowIndex;

            EntityLow->HighEntityIndex = HighIndex;
        } else
        {
            InvalidCodePath
        }
    }
    return (EntityHigh);
}

inline high_entity *
MakeEntityHighFrequency(game_state *GameState, uint32 LowIndex)
{
    high_entity *EntityHigh = 0;
    low_entity *EntityLow = &GameState->LowEntities[LowIndex];
    if (EntityLow->HighEntityIndex)
    {
        EntityHigh = GameState->HighEntities_ + EntityLow->HighEntityIndex;
    } else
    {
        v2 CameraSpaceP = GetCameraSpaceP(GameState, EntityLow);
        EntityHigh = MakeEntityHighFrequency(GameState, EntityLow, LowIndex, CameraSpaceP);
    }
    return (EntityHigh);
}

internal entity
ForceEntityIntoHigh(game_state *GameState, uint32 LowIndex)
{
    entity Result = {};

    if ((LowIndex > 0) && (LowIndex < GameState->LowEntityCount))
    {
        Result.LowIndex = LowIndex;
        Result.Low = GameState->LowEntities + LowIndex;
        Result.High = MakeEntityHighFrequency(GameState, LowIndex);
    }

    return (Result);
}

inline void
MakeEntityLowFrequency(game_state *GameState, uint32 LowIndex)
{
    low_entity *EntityLow = &GameState->LowEntities[LowIndex];
    uint32 HighIndex = EntityLow->HighEntityIndex;
    if (HighIndex)
    {
        uint32 LastHighIndex = GameState->HighEntityCount - 1;
        if (HighIndex != LastHighIndex)
        {
            high_entity *LastEntity = GameState->HighEntities_ + LastHighIndex;
            high_entity *DelEntity = GameState->HighEntities_ + HighIndex;
            *DelEntity = *LastEntity;
            GameState->LowEntities[LastEntity->LowEntityIndex].HighEntityIndex = HighIndex;
        }
        --GameState->HighEntityCount;
        EntityLow->HighEntityIndex = 0;
    }
}

inline void
OffsetAndCheckFrequencyByArea(game_state *GameState, v2 Offset, rectangle2 HighFrequencyBounds)
{
    for (uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount;)
    {
        high_entity *High = GameState->HighEntities_ + HighEntityIndex;
        low_entity *Low = GameState->LowEntities + High->LowEntityIndex;

        High->P += Offset;

        if (IsValid(Low->P) && IsInRectangle(HighFrequencyBounds, High->P))
        {
            ++HighEntityIndex;
        } else
        {
            Assert(GameState->LowEntities[High->LowEntityIndex].HighEntityIndex == HighEntityIndex)
            MakeEntityLowFrequency(GameState, High->LowEntityIndex);
        }
    }
}

struct add_low_entity_result
{
    low_entity *Low;
    uint32 LowIndex;
};

internal add_low_entity_result
AddLowEntity(game_state *GameState, entity_type Type, world_position *P)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities))

    uint32 EntityIndex = GameState->LowEntityCount++;

    low_entity *EntityLow = GameState->LowEntities + EntityIndex;
    *EntityLow = {};
    EntityLow->Type = Type;

    ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, 0, P);

    add_low_entity_result Result = {};
    Result.Low = EntityLow;
    Result.LowIndex = EntityIndex;
    return (Result);
}

inline void
InitHitPoints(low_entity *EntityLow, uint32 HitPointCount)
{
    Assert(HitPointCount < ArrayCount(EntityLow->HitPoint))
    EntityLow->HitPointMax = HitPointCount;

    for (int HitPointIndex = 0; HitPointIndex < EntityLow->HitPointMax; ++HitPointIndex)
    {
        hit_point *HitPoint = EntityLow->HitPoint + HitPointIndex;
        HitPoint->Flags = 0;
        HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
    }
}

internal add_low_entity_result
AddWall(game_state *GameState, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Wall, &P);

    Entity.Low->Height = GameState->World->TileSideInMeters;
    Entity.Low->Width = Entity.Low->Height;
    Entity.Low->Collides = true;

    return (Entity);
}

internal add_low_entity_result
AddMonster(game_state *GameState, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Monster, &P);

    InitHitPoints(Entity.Low, 3);
    Entity.Low->Height = 0.5f;
    Entity.Low->Width = 1.0f;
    Entity.Low->Collides = true;

    return (Entity);
}

internal add_low_entity_result
AddSword(game_state *GameState)
{
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, 0);

    Entity.Low->Height = 0.5f;
    Entity.Low->Width = 1.0f;
    Entity.Low->Collides = false;

    return (Entity);
}

internal add_low_entity_result
AddFamiliar(game_state *GameState, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Familiar, &P);

    Entity.Low->Height = 0.5f;
    Entity.Low->Width = 1.0f;
    Entity.Low->Collides = false;

    return (Entity);
}

internal add_low_entity_result
AddPlayer(game_state *GameState)
{
    world_position P = GameState->CameraP;
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Hero, &P);

    Entity.Low->Height = 0.5f;
    Entity.Low->Width = 1.0f;
    Entity.Low->Collides = true;
    InitHitPoints(Entity.Low, 3);

    add_low_entity_result Sword = AddSword(GameState);
    Entity.Low->SwordLowIndex = Sword.LowIndex;

    if (GameState->CameraFollowingEntityIndex == 0)
    {
        GameState->CameraFollowingEntityIndex = Entity.LowIndex;
    }
    return (Entity);
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
                *tMin = Maximum(0.0f, tResult - tEpsilon);
                Hit = true;
            }
        }
    }
    return (Hit);
}

internal void
MoveEntity(game_state *GameState, entity Entity, v2 accelOfEntity, move_spec MoveSpec, real32 deltat)
{
    world *TileMap = GameState->World;

    if (MoveSpec.UnitMaxAccelVector)
    {
        real32 accelLength = LengthSq(accelOfEntity);

        if (accelLength > 1.0f)
        {
            accelOfEntity *= 1.0f / SquareRoot(accelLength);
        }
    }

    accelOfEntity *= MoveSpec.Speed;
    accelOfEntity += -MoveSpec.Drag * Entity.High->deltaP;

    v2 OldPlayerP = Entity.High->P;
    v2 Playerdelta = (0.5f * accelOfEntity * Square(deltat) +
                      Entity.High->deltaP * deltat);
    Entity.High->deltaP = accelOfEntity * deltat + Entity.High->deltaP;
    v2 NewPlayerP = OldPlayerP + Playerdelta;

/*
    uint32 MinTileX = Minimum(OldPlayerP.X, NewPlayerP.X);
    uint32 MinTileY = Minimum(OldPlayerP.Y, NewPlayerP.Y);
    uint32 MaxTileX = Maximum(OldPlayerP.X, NewPlayerP.X);
    uint32 MaxTileY = Maximum(OldPlayerP.Y, NewPlayerP.Y);

    uint32 EntityTileWidth = CeilReal32ToInt32(Entity.Low->Width / TileMap->TileSideInMeters);
    uint32 EntityTileHeight = CeilReal32ToInt32(Entity.Low->Height / TileMap->TileSideInMeters);

    MinTileX -= EntityTileWidth;
    MinTileY -= EntityTileHeight;
    MaxTileX += EntityTileWidth;
    MaxTileY += EntityTileHeight;

    uint32 ChunkZ = Entity.Low->P.ChunkZ;
*/

    for (uint8 Iteration = 0; Iteration < 4; ++Iteration)
    {
        real32 tMin = 1.0f;
        v2 WallNormal = {};
        uint32 HitHighEntityIndex = 0;

        v2 DesiredPosition = Entity.High->P + Playerdelta;

        if (Entity.Low->Collides)
        {
            for (uint32 TestHighEntityIndex = 1;
                 TestHighEntityIndex < GameState->HighEntityCount;
                 ++TestHighEntityIndex)
            {
                if (TestHighEntityIndex != Entity.Low->HighEntityIndex)
                {
                    entity TestEntity = {};
                    TestEntity.High = GameState->HighEntities_ + TestHighEntityIndex;
                    TestEntity.LowIndex = TestEntity.High->LowEntityIndex;
                    TestEntity.Low = GameState->LowEntities + TestEntity.LowIndex;
                    if (TestEntity.Low->Collides)
                    {
                        real32 DiameterW = TestEntity.Low->Width + Entity.Low->Width;
                        real32 DiameterH = TestEntity.Low->Height + Entity.Low->Height;

                        v2 MinCorner = -0.5f * V2(DiameterW, DiameterH);
                        v2 MaxCorner = 0.5f * V2(DiameterW, DiameterH);
                        v2 Rel = Entity.High->P - TestEntity.High->P;

                        if (TestWall(MinCorner.X, Playerdelta.X, Playerdelta.Y, Rel.X, Rel.Y, &tMin,
                                     MinCorner.Y, MaxCorner.Y))
                        {
                            WallNormal = V2(-1, 0);
                            HitHighEntityIndex = TestHighEntityIndex;
                        }
                        if (TestWall(MaxCorner.X, Playerdelta.X, Playerdelta.Y, Rel.X, Rel.Y, &tMin,
                                     MinCorner.Y, MaxCorner.Y))
                        {
                            WallNormal = V2(1, 0);
                            HitHighEntityIndex = TestHighEntityIndex;
                        }

                        if (TestWall(MinCorner.Y, Playerdelta.Y, Playerdelta.X, Rel.Y, Rel.X, &tMin,
                                     MinCorner.X, MaxCorner.X))
                        {
                            WallNormal = V2(0, -1);
                            HitHighEntityIndex = TestHighEntityIndex;
                        }
                        if (TestWall(MaxCorner.Y, Playerdelta.Y, Playerdelta.X, Rel.Y, Rel.X, &tMin,
                                     MinCorner.X, MaxCorner.X))
                        {
                            WallNormal = V2(0, 1);
                            HitHighEntityIndex = TestHighEntityIndex;
                        }
                    }
                }
            }
        }

        Entity.High->P += tMin * Playerdelta;
        if (HitHighEntityIndex)
        {
            Entity.High->deltaP = Entity.High->deltaP - (DotProduct(WallNormal, Entity.High->deltaP) * WallNormal);
            Playerdelta = DesiredPosition - Entity.High->P;
            Playerdelta = Playerdelta - (DotProduct(Playerdelta, WallNormal) * WallNormal);
            high_entity *HitHigh = GameState->HighEntities_ + HitHighEntityIndex;
            low_entity *HitLow = GameState->LowEntities + HitHigh->LowEntityIndex;
//            Entity.High->ChunkZ += HitLow->deltaAbsTileZ;
        } else
        {
            break;
        }
    }

    if ((AbsoluteValue(Entity.High->deltaP.X) == 0.0f) && (AbsoluteValue(Entity.High->deltaP.Y) == 0.0f))
    {
    } else if (AbsoluteValue(Entity.High->deltaP.X) > AbsoluteValue(Entity.High->deltaP.Y))
    {
        if (Entity.High->deltaP.X > 0)
        {
            Entity.High->FacingDirection = 2;
        } else
        {
            Entity.High->FacingDirection = 3;
        }
    } else
    {
        if (Entity.High->deltaP.Y > 0)
        {
            Entity.High->FacingDirection = 0;
        } else
        {
            Entity.High->FacingDirection = 1;
        }
    }

    world_position NewP = MapIntoChunkSpace(GameState->World, GameState->CameraP, Entity.High->P);
    ChangeEntityLocation(&GameState->WorldArena, GameState->World, Entity.LowIndex,
                         Entity.Low, &Entity.Low->P, &NewP);
}

internal void
SetCamera(game_state *GameState, world_position NewCameraP)
{
    world *World = GameState->World;

    world_difference deltaCameraP = Subtract(World, &NewCameraP, &GameState->CameraP);
    GameState->CameraP = NewCameraP;

    uint32 ChunkSpanX = 17 * 3;
    uint32 ChunkSpanY = 9 * 3;

    rectangle2 CameraBounds = RectCenterDim(V2(0, 0),
                                            World->TileSideInMeters * V2((real32) ChunkSpanX,
                                                                         (real32) ChunkSpanY));

    v2 EntityOffsetForFrame = -deltaCameraP.deltaXY;
    OffsetAndCheckFrequencyByArea(GameState, EntityOffsetForFrame, CameraBounds);

    world_position MinChunkP = MapIntoChunkSpace(World, NewCameraP, GetMinCorner(CameraBounds));
    world_position MaxChunkP = MapIntoChunkSpace(World, NewCameraP, GetMaxCorner(CameraBounds));

    for (int32 ChunkY = MinChunkP.ChunkY;
         ChunkY <= MaxChunkP.ChunkY;
         ++ChunkY)
    {
        for (int32 ChunkX = MinChunkP.ChunkX;
             ChunkX <= MaxChunkP.ChunkX;
             ++ChunkX)
        {
            world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, NewCameraP.ChunkZ);

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
                        if (Low->HighEntityIndex == 0)
                        {
                            v2 CameraSpaceP = GetCameraSpaceP(GameState, Low);

                            if (IsInRectangle(CameraBounds, CameraSpaceP))
                            {
                                MakeEntityHighFrequency(GameState,
                                                        Low,
                                                        LowEntityIndex,
                                                        CameraSpaceP);
                            }
                        }
                    }
                }
            }
        }
    }
}

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
PushPiece(entity_visible_piece_group *Group, loaded_bitmap *Bitmap, v2 Offset, real32 OffsetZ,
          v2 Dim, v2 Align, v4 Color, real32 EntityZC = 1.0f)
{
    Assert(Group->Count < ArrayCount(Group->Pieces))
    entity_visible_piece *Piece = Group->Pieces + Group->Count++;
    Piece->Bitmap = Bitmap;
    Piece->Offset = Group->GameState->MetersToPixel * V2(Offset.X, -Offset.Y) - Align;
    Piece->OffsetZ = Group->GameState->MetersToPixel * OffsetZ;
    Piece->EntityZC = EntityZC;
    Piece->R = Color.R;
    Piece->G = Color.G;
    Piece->B = Color.B;
    Piece->A = Color.A;
    Piece->Dim = Dim;
}

inline void
PushRect(entity_visible_piece_group *Group, v2 Offset, real32 OffsetZ, v2 Dim,
         v4 Color, real32 EntityZC = 1.0f)
{
    PushPiece(Group, 0, Offset, OffsetZ, Dim, V2(0, 0), Color, EntityZC);
}

inline void
PushBitmap(entity_visible_piece_group *Group, loaded_bitmap *Bitmap,
           v2 Offset, real32 OffsetZ, v2 Align, real32 Alpha = 1.0f, real32 EntityZC = 1.0f)
{
    PushPiece(Group, Bitmap, Offset, OffsetZ, V2(0, 0),
              Align, V4(0, 0, 0, Alpha), EntityZC);
}

inline entity
EntityFromHighIndex(game_state *GameState, uint32 HighEntityIndex)
{
    entity Result = {};
    if (HighEntityIndex)
    {
        Assert(HighEntityIndex < ArrayCount(GameState->HighEntities_))
        Result.High = GameState->HighEntities_ + HighEntityIndex;
        Result.LowIndex = Result.High->LowEntityIndex;
        Result.Low = GameState->LowEntities + Result.LowIndex;
    }
    return (Result);
}

inline void
UpdateFamiliar(game_state *GameState, entity Entity, real32 deltat)
{
    entity ClosestHero = {};
    real32 ClosestHeroDSq = Square(10.0f);
    for (uint32 HighEntityIndex = 1;
         HighEntityIndex < GameState->HighEntityCount;
         ++HighEntityIndex)
    {
        entity TestEntity = EntityFromHighIndex(GameState, HighEntityIndex);

        if (TestEntity.Low->Type == EntityType_Hero)
        {
            real32 TestDSq = LengthSq(TestEntity.High->P - Entity.High->P);
            if (TestDSq < ClosestHeroDSq)
            {
                ClosestHero = TestEntity;
                ClosestHeroDSq = TestDSq;
            }
        }
    }
    v2 accelOfEntity = {};
    if (ClosestHero.High && ClosestHeroDSq > Square(3.0f))
    {
        real32 accel = 0.5f;
        real32 OneOverLength = accel / SquareRoot(ClosestHeroDSq);
        accelOfEntity = OneOverLength * (ClosestHero.High->P - Entity.High->P);
    }
    move_spec MoveSpec = DefaultMoveSpec();
    MoveSpec.UnitMaxAccelVector = true;
    MoveSpec.Speed = 70.0f;
    MoveSpec.Drag = 7.0f;
    MoveEntity(GameState, Entity, accelOfEntity, MoveSpec, deltat);
}

inline void
UpdateSword(game_state *GameState, entity Entity, real32 deltat)
{
    move_spec MoveSpec = DefaultMoveSpec();
    MoveSpec.Speed = 0.0f;
    MoveSpec.Drag = 0.0f;

    v2 OldP = Entity.High->P;
    MoveEntity(GameState, Entity, V2(0, 0), MoveSpec, deltat);
    real32 DistanceTraveled = Length(OldP - Entity.High->P);

    Entity.Low->DistanceRemaining -= DistanceTraveled;
    if (Entity.Low->DistanceRemaining < 0.0f)
    {
        ChangeEntityLocation(&GameState->WorldArena, GameState->World,
                             Entity.LowIndex, Entity.Low, &Entity.Low->P, 0);
    }
}

inline void
UpdateMonster(game_state *GameState, entity Entity, real32 Deltat)
{
}

internal void
DrawHitPoints(low_entity *LowEntity, entity_visible_piece_group *PieceGroup)
{
    if (LowEntity->HitPointMax >= 1)
    {
        v2 HealthDim = {0.2f, 0.2f};
        real32 SpacingInX = 1.5f * HealthDim.X;

        v2 HitP = {-0.5f * ((real32) (LowEntity->HitPointMax - 1)) * SpacingInX, -0.2f};
        v2 dHitP = {SpacingInX, 0.0f};
        for (uint32 HealthIndex = 0; HealthIndex < LowEntity->HitPointMax; ++HealthIndex)
        {
            hit_point *HitPoint = LowEntity->HitPoint + HealthIndex;
            v4 Color = {1.0f, 0.0f, 0.0f, 1.0f};

            if (HitPoint->FilledAmount == 0)
            {
                Color = V4(0.2f, 0.2f, 0.2f, 1.0f);
            }
            PushRect(PieceGroup, HitP, 0.0f, HealthDim, Color, 0.0f);
            HitP += dHitP;
        }
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        AddLowEntity(GameState, EntityType_Null, 0);
        GameState->HighEntityCount = 1;

        GameState->Backdrop = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "test/test_background.bmp");

        GameState->HeroShadow = DEBUGLoadBMP(Thread,
                                             Memory->DEBUGPlatformReadEntireFile,
                                             "test/test_hero_shadow.bmp");

        GameState->Tree = DEBUGLoadBMP(Thread,
                                       Memory->DEBUGPlatformReadEntireFile,
                                       "test2/tree00.bmp");

        GameState->Sword = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test2/rock03.bmp");
        hero_bitmaps *Bitmap;
        Bitmap = GameState->HeroBitmaps;

        Bitmap->HeroHead = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_back_head.bmp");
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_back_torso.bmp");
        Bitmap->HeroCape = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_back_cape.bmp");
        Bitmap->Align = V2(72, 182);
        ++Bitmap;

        Bitmap->HeroHead = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_front_head.bmp");
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_front_torso.bmp");
        Bitmap->HeroCape = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_front_cape.bmp");
        Bitmap->Align = V2(72, 182);
        ++Bitmap;

        Bitmap->HeroHead = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_right_head.bmp");
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_right_torso.bmp");
        Bitmap->HeroCape = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_right_cape.bmp");
        Bitmap->Align = V2(72, 182);
        ++Bitmap;

        Bitmap->HeroHead = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_left_head.bmp");
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_left_torso.bmp");
        Bitmap->HeroCape = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_left_cape.bmp");
        Bitmap->Align = V2(72, 182);
        ++Bitmap;

        InitializeArena(&GameState->WorldArena,
                        (mem_index) Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *) Memory->PermanentStorage + sizeof(game_state));
        GameState->World = PushStruct(&GameState->WorldArena, world);

        world *World = GameState->World;

        InitializeWorld(World, 1.4f);

        uint32 TilesPerHeight = 9;
        uint32 TilesPerWidth = 17;
        real32 TileSideInPixels = 60;
        GameState->MetersToPixel = TileSideInPixels / World->TileSideInMeters;

        uint32 ScreenBaseX = 0;
        uint32 ScreenBaseY = 0;
        uint32 ScreenBaseZ = 0;

        uint32 ScreenX = ScreenBaseX;
        uint32 ScreenY = ScreenBaseY;

        uint32 RandomNumberIndex = 0;
        uint32 AbsTileZ = ScreenBaseZ;

        bool32 DoorLeft = false;
        bool32 DoorRight = false;
        bool32 DoorTop = false;
        bool32 DoorBottom = false;

        bool32 DoorUp = false;
        bool32 DoorDown = false;

        for (int ScreenIndex = 0; ScreenIndex < 2000; ++ScreenIndex)
        {
            Assert(RandomNumberIndex < ArrayCount(RandomNumberTable))
            uint32 RandomChoice;

//            if (DoorDown || DoorUp)
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
            }
#if 0
            else
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
            }
#endif
            bool32 ZDoorCreated = false;
            if (RandomChoice == 2)
            {
                ZDoorCreated = true;
                if (AbsTileZ == ScreenBaseZ)
                {
                    DoorUp = true;
                } else
                {
                    DoorDown = true;
                }
            } else if (RandomChoice == 1)
            {
                DoorRight = true;
            } else
            {
                DoorTop = true;
            }
            for (uint32 TileY = 0; TileY < TilesPerHeight; ++TileY)
            {
                for (uint32 TileX = 0; TileX < TilesPerWidth; ++TileX)
                {
                    uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
                    uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;

                    uint32 TileValue = 1;

                    if ((TileX == 0) && (!DoorLeft || (TileY != (TilesPerHeight / 2))))
                    {
                        TileValue = 2;
                    }
                    if ((TileX == (TilesPerWidth - 1)) && (!DoorRight || (TileY != (TilesPerHeight / 2))))
                    {
                        TileValue = 2;
                    }
                    if ((TileY == 0) && (!DoorBottom || (TileX != (TilesPerWidth / 2))))
                    {
                        TileValue = 2;
                    }
                    if ((TileY == (TilesPerHeight - 1)) && (!DoorTop || (TileX != (TilesPerWidth / 2))))
                    {
                        TileValue = 2;
                    }

                    if ((TileX == 10) && (TileY == 6))
                    {
                        if (DoorUp)
                        {
                            TileValue = 3;
                        }
                        if (DoorDown)
                        {
                            TileValue = 4;
                        }
                    }

                    if (TileValue == 2)
                    {
                        AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);
                    }
                }
            }

            DoorLeft = DoorRight;
            DoorBottom = DoorTop;
            DoorRight = false;
            DoorTop = false;

            if (ZDoorCreated)
            {
                DoorUp = !DoorUp;
                DoorDown = !DoorDown;
            } else
            {
                DoorUp = false;
                DoorDown = false;
            }

            if (RandomChoice == 2)
            {
                if (AbsTileZ == ScreenBaseZ)
                {
                    AbsTileZ = ScreenBaseZ + 1;
                } else
                {
                    AbsTileZ = ScreenBaseZ;
                }
            } else if (RandomChoice == 1)
            {
                ScreenX += 1;
            } else
            {
                ScreenY += 1;
            }
        }
        world_position NewCameraP = {};

        uint32 CameraTileX = ScreenBaseX * TilesPerWidth + 17 / 2;
        uint32 CameraTileY = ScreenBaseY * TilesPerHeight + 9 / 2;
        uint32 CameraTileZ = ScreenBaseZ;

        NewCameraP = ChunkPosFromTilePos(GameState->World,
                                         CameraTileX,
                                         CameraTileY,
                                         CameraTileZ);

        AddMonster(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);
        AddFamiliar(GameState, CameraTileX - 2, CameraTileY + 2, CameraTileZ);

        SetCamera(GameState, NewCameraP);

        Memory->IsInitialized = true;
    }

    world *World = GameState->World;

    for (int ControllerIndex = 0;
         ControllerIndex < ArrayCount(Input->Controllers);
         ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        uint32 LowIndex = GameState->PlayerIndexForController[ControllerIndex];
        if (LowIndex == 0)
        {
            if (Controller->Start.EndedDown)
            {
                uint32 EntityIndex = AddPlayer(GameState).LowIndex;
                GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
            }
        } else
        {
            entity ControllingEntity = ForceEntityIntoHigh(GameState, LowIndex);
            v2 accelOfPlayer = {};
            if (Controller->IsAnalog)
            {
                accelOfPlayer = V2(Controller->StickAverageX, Controller->StickAverageY);
            } else
            {
                if (Controller->MoveUp.EndedDown)
                {
                    accelOfPlayer.Y = 1.0f;
                }
                if (Controller->MoveDown.EndedDown)
                {
                    accelOfPlayer.Y = -1.0f;
                }
                if (Controller->MoveLeft.EndedDown)
                {
                    accelOfPlayer.X = -1.0f;
                }
                if (Controller->MoveRight.EndedDown)
                {
                    accelOfPlayer.X = 1.0f;
                }
            }
            if (Controller->ActionDown.EndedDown)
            {
                accelOfPlayer *= 50.0f;
            }

            if (Controller->Start.EndedDown)
            {
                ControllingEntity.High->deltaZ = 4.0f;
            }

            v2 deltaSword = {};
            if (Controller->ActionUp.EndedDown)
            {
                deltaSword = {0.0f, 1.0f};
            }
            if (Controller->ActionDown.EndedDown)
            {
                deltaSword = {0.0f, -1.0f};
            }
            if (Controller->ActionLeft.EndedDown)
            {
                deltaSword = {-1.0f, 0.0f};
            }
            if (Controller->ActionRight.EndedDown)
            {
                deltaSword = {1.0f, 0.0f};
            }
            move_spec MoveSpec = DefaultMoveSpec();
            MoveSpec.UnitMaxAccelVector = true;
            MoveSpec.Speed = 70.0f;
            MoveSpec.Drag = 7.0f;

            MoveEntity(GameState, ControllingEntity, accelOfPlayer, MoveSpec, Input->deltatForFrame);

            if ((deltaSword.X != 0.0f) || (deltaSword.Y != 0.0f))
            {
                low_entity *SwordLow = GetLowEntity(GameState, ControllingEntity.Low->SwordLowIndex);
                if (SwordLow && !IsValid(SwordLow->P))
                {
                    world_position *SwordP = &ControllingEntity.Low->P;
                    ChangeEntityLocation(&GameState->WorldArena, GameState->World,
                                         ControllingEntity.Low->SwordLowIndex,
                                         SwordLow, 0, SwordP);

                    entity Sword = ForceEntityIntoHigh(GameState,
                                                       ControllingEntity.Low->SwordLowIndex);

                    Sword.Low->DistanceRemaining = 5.0f;
                    Sword.High->deltaP = 5.0f * deltaSword;
                }
            }
        }
    }
    entity CameraFollowingEntity = ForceEntityIntoHigh(GameState,
                                                       GameState->CameraFollowingEntityIndex);
    if (CameraFollowingEntity.High)
    {
        world_position NewCameraP = GameState->CameraP;
        NewCameraP.ChunkZ = CameraFollowingEntity.Low->P.ChunkZ;

#if 0
        if (CameraFollowingEntity.High->P.X > (9.0f * World->TileSideInMeters))
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

        NewCameraP = CameraFollowingEntity.Low->P;
#endif

        SetCamera(GameState, NewCameraP);
    }

#if 1
    DrawRectangle(Buffer, V2(0, 0),
                  V2((real32) Buffer->Width, (real32) Buffer->Height),
                  0.1f, 0.2f, 0.1f);
#else
    DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);
#endif

    real32 ScreenCenterX = (real32) Buffer->Width * 0.5f;
    real32 ScreenCenterY = (real32) Buffer->Height * 0.5f;

    entity_visible_piece_group PieceGroup = {};
    PieceGroup.GameState = GameState;
    for (uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount; ++HighEntityIndex)
    {
        PieceGroup.Count = 0;

        high_entity *HighEntity = GameState->HighEntities_ + HighEntityIndex;
        low_entity *LowEntity = GameState->LowEntities + HighEntity->LowEntityIndex;

        entity Entity = {};
        Entity.LowIndex = HighEntity->LowEntityIndex;
        Entity.Low = LowEntity;
        Entity.High = HighEntity;

#if 0
        v2 PlayerLeftTop = {HeroGroundPointX - (0.5f * LowEntity->Width * MetersToPixel),
                            HeroGroundPointY - (0.5f * LowEntity->Height * MetersToPixel)};
        v2 EntityWidthHeight = {LowEntity->Width, LowEntity->Height};
#endif
        real32 deltat = Input->deltatForFrame;
        real32 ShadowAlpha = 1.0f - 0.5f * HighEntity->Z;
        if (ShadowAlpha < 0)
        {
            ShadowAlpha = 0.0f;
        }

        hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[HighEntity->FacingDirection];
        switch (LowEntity->Type)
        {
            case EntityType_Hero:
            {
                PushBitmap(&PieceGroup, &HeroBitmaps->HeroTorso, V2(0, 0), 0,
                           HeroBitmaps->Align);
                PushBitmap(&PieceGroup, &HeroBitmaps->HeroCape, V2(0, 0), 0,
                           HeroBitmaps->Align);
                PushBitmap(&PieceGroup, &HeroBitmaps->HeroHead, V2(0, 0), 0,
                           HeroBitmaps->Align);
                PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                           HeroBitmaps->Align, ShadowAlpha, 0.0f);

                DrawHitPoints(LowEntity, &PieceGroup);
            }
                break;
            case EntityType_Wall:
            {
                PushBitmap(&PieceGroup, &GameState->Tree, V2(0, 0), 0, V2(40, 80));
            }
                break;

            case EntityType_Sword:
            {
                UpdateSword(GameState, Entity, deltat);
                PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                           HeroBitmaps->Align, ShadowAlpha, 0.0f);
                PushBitmap(&PieceGroup, &GameState->Sword, V2(0, 0), 0, V2(29, 10));
            }
                break;
            case EntityType_Familiar:
            {
                UpdateFamiliar(GameState, Entity, deltat);
                Entity.High->tBob += deltat;
                if (Entity.High->tBob > (2 * PII32))
                {
                    Entity.High->tBob -= (2 * PII32);
                }
                real32 SinBob = Sin(4.0f * Entity.High->tBob);
                PushBitmap(&PieceGroup, &HeroBitmaps->HeroHead, V2(0, 0),
                           0.25f * SinBob, HeroBitmaps->Align);

                PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                           HeroBitmaps->Align, (0.5f * ShadowAlpha) + 0.2f * SinBob, 0.0F);
            }
                break;
            case EntityType_Monster:
            {
                UpdateMonster(GameState, Entity, deltat);
                PushBitmap(&PieceGroup, &HeroBitmaps->HeroCape, V2(0, 0), 0,
                           HeroBitmaps->Align);
                PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                           HeroBitmaps->Align, ShadowAlpha, 0.0f);

                DrawHitPoints(LowEntity, &PieceGroup);
            }
                break;
            default:
            {
                InvalidCodePath
            }
                break;
        }

        real32 accelOfPlayerInZ = -9.8f;
        real32 deltaZ = (0.5f * accelOfPlayerInZ * Square(deltat) + HighEntity->deltaZ * deltat);
        HighEntity->Z += deltaZ;
        HighEntity->deltaZ = accelOfPlayerInZ * deltat + HighEntity->deltaZ;
        if (HighEntity->Z < 0)
        {
            HighEntity->Z = 0;
        }

        real32 EntityGroundPointX = ScreenCenterX + HighEntity->P.X * GameState->MetersToPixel;
        real32 EntityGroundPointY = ScreenCenterY - HighEntity->P.Y * GameState->MetersToPixel;
        real32 EntityZ = -GameState->MetersToPixel * HighEntity->Z;

        for (uint32 PieceIndex = 0;
             PieceIndex < PieceGroup.Count;
             ++PieceIndex)
        {
            entity_visible_piece *Piece = PieceGroup.Pieces + PieceIndex;

            v2 Center = {EntityGroundPointX + Piece->Offset.X,
                         EntityGroundPointY + Piece->Offset.Y + Piece->OffsetZ + EntityZ * Piece->EntityZC};

            if (Piece->Bitmap)
            {
                DrawBitmap(Buffer, Piece->Bitmap, Center.X, Center.Y, Piece->A);
            } else
            {
                v2 HalfDim = 0.5f * GameState->MetersToPixel * Piece->Dim;
                DrawRectangle(Buffer, Center - HalfDim, Center + HalfDim,
                              Piece->R, Piece->G, Piece->B);
            }
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(SoundBuffer, GameState);
}
