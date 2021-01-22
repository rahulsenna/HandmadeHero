#pragma ide diagnostic ignored "bugprone-suspicious-include"
#pragma ide diagnostic ignored "bugprone-incorrect-roundings"

#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "UnusedLocalVariable"
//#pragma ide diagnostic ignored "bugprone-branch-clone"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "modernize-loop-convert"
#pragma ide diagnostic ignored "modernize-deprecated-headers"
#pragma ide diagnostic ignored "modernize-use-nullptr"

//
// Created by AgentOfChaos on 11/20/2020.
//
#include "handmade_platform.h"

#include "handmade_math.h"
#include "handmade.h"
#include "handmade_world.cpp"
#include "handmade_random.h"

void
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
    uint32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                    (RoundReal32ToUInt32(G * 255.0f) << 8) |
                    (RoundReal32ToUInt32(B * 255.0f) << 0));
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
           int32 AlignX = 0, int32 AlignY = 0, real32 CAlpha = 1.0f)
{
    RealX -= (real32) AlignX;
    RealY -= (real32) AlignY;
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
            real32 SR = (real32) ((*Source >> 16) & 0xFF);
            real32 SG = (real32) ((*Source >> 8) & 0xFF);
            real32 SB = (real32) ((*Source >> 0) & 0xFF);

            real32 DR = (real32) ((*Dest >> 16) & 0xFF);
            real32 DG = (real32) ((*Dest >> 8) & 0xFF);
            real32 DB = (real32) ((*Dest >> 0) & 0xFF);

            real32 R = (1.0f - A) * DR + A * SR;
            real32 G = (1.0f - A) * DG + A * SG;
            real32 B = (1.0f - A) * DB + A * SB;

            *Dest = (((uint32) (R + 0.5f) << 16) |
                     ((uint32) (G + 0.5f) << 8) |
                     ((uint32) (B + 0.5f) << 0));
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

        int32 RedShift = 16 - (int32) RedScan.Index;
        int32 GreenShift = 8 - (int32) GreenScan.Index;
        int32 BlueShift = 0 - (int32) BlueScan.Index;
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
        if (GameState->HighEntityCount < ArrayCount(GameState->HighEntities_))
        {
            uint32 HighIndex = GameState->HighEntityCount++;
            EntityHigh = GameState->HighEntities_ + HighIndex;

            world_difference Diff = Subtract(GameState->World,
                                             &EntityLow->P,
                                             &GameState->CameraP);

            EntityHigh->P = Diff.deltaXY;
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

internal entity
GetHighEntity(game_state *GameState, uint32 LowIndex)
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
    for (uint32 EntityIndex = 1; EntityIndex < GameState->HighEntityCount;)
    {
        high_entity *High = GameState->HighEntities_ + EntityIndex;
        High->P += Offset;

        if (IsInRectangle(HighFrequencyBounds, High->P))
        {
            ++EntityIndex;
        } else
        {
            MakeEntityLowFrequency(GameState, High->LowEntityIndex);
        }
    }
}

internal uint32
AddLowEntity(game_state *GameState, entity_type Type)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities))
    uint32 EntityIndex = GameState->LowEntityCount++;

    GameState->LowEntities[EntityIndex] = {};
    GameState->LowEntities[EntityIndex].Type = Type;

    return (EntityIndex);
}

internal uint32
AddWall(game_state *GameState, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    uint32 EntityIndex = AddLowEntity(GameState, EntityType_Wall);
    low_entity *EntityLow = GetLowEntity(GameState, EntityIndex);

    EntityLow->P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);

    EntityLow->Height = GameState->World->TileSideInMeters;
    EntityLow->Width = EntityLow->Height;
    EntityLow->Collides = true;

    return (EntityIndex);
}

internal uint32
AddPlayer(game_state *GameState)
{
    uint32 EntityIndex = AddLowEntity(GameState, EntityType_Hero);
    low_entity *EntityLow = GetLowEntity(GameState, EntityIndex);

    EntityLow->P = GameState->CameraP;
    EntityLow->P.Offset_.X = 0.0f;
    EntityLow->P.Offset_.Y = 0.0f;
    EntityLow->Height = 0.5f;
    EntityLow->Width = 1.0f;
    EntityLow->Collides = true;

    if (GameState->CameraFollowingEntityIndex == 0)
    {
        GameState->CameraFollowingEntityIndex = EntityIndex;
    }
    return (EntityIndex);
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
MovePlayer(game_state *GameState, entity Entity, v2 accelOfPlayer, real32 deltat)
{
    world *TileMap = GameState->World;

    real32 accelLength = LengthSq(accelOfPlayer);

    if (accelLength > 1.0f)
    {
        accelOfPlayer *= 1.0f / SquareRoot(accelLength);
    }

    accelOfPlayer *= 70.0f;
    accelOfPlayer += -7.0f * Entity.High->deltaP;

    v2 OldPlayerP = Entity.High->P;
    v2 Playerdelta = (0.5f * accelOfPlayer * Square(deltat) +
                      Entity.High->deltaP * deltat);
    Entity.High->deltaP = accelOfPlayer * deltat + Entity.High->deltaP;
    v2 NewPlayerP = OldPlayerP + Playerdelta;

    /*uint32 MinTileX = Minimum(OldPlayerP.X, NewPlayerP.X);
    uint32 MinTileY = Minimum(OldPlayerP.Y, NewPlayerP.Y);
    uint32 MaxTileX = Maximum(OldPlayerP.X, NewPlayerP.X);
    uint32 MaxTileY = Maximum(OldPlayerP.Y, NewPlayerP.Y);

    uint32 EntityTileWidth = CeilReal32ToInt32(Entity.Low->Width / TileMap->TileSideInMeters);
    uint32 EntityTileHeight = CeilReal32ToInt32(Entity.Low->Height / TileMap->TileSideInMeters);

    MinTileX -= EntityTileWidth;
    MinTileY -= EntityTileHeight;
    MaxTileX += EntityTileWidth;
    MaxTileY += EntityTileHeight;

    uint32 ChunkZ = Entity.Low->P.ChunkZ;*/

    for (uint8 Iteration = 0; Iteration < 4; ++Iteration)
    {
        real32 tMin = 1.0f;
        v2 WallNormal = {};
        uint32 HitHighEntityIndex = 0;

        v2 DesiredPosition = Entity.High->P + Playerdelta;
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

                    v2 MinCorner = -0.5f * v2{DiameterW, DiameterH};
                    v2 MaxCorner = 0.5f * v2{DiameterW, DiameterH};
                    v2 Rel = Entity.High->P - TestEntity.High->P;

                    if (TestWall(MinCorner.X, Playerdelta.X, Playerdelta.Y, Rel.X, Rel.Y, &tMin,
                                 MinCorner.Y, MaxCorner.Y))
                    {
                        WallNormal = v2{-1, 0};
                        HitHighEntityIndex = TestHighEntityIndex;
                    }
                    if (TestWall(MaxCorner.X, Playerdelta.X, Playerdelta.Y, Rel.X, Rel.Y, &tMin,
                                 MinCorner.Y, MaxCorner.Y))
                    {
                        WallNormal = v2{1, 0};
                        HitHighEntityIndex = TestHighEntityIndex;
                    }

                    if (TestWall(MinCorner.Y, Playerdelta.Y, Playerdelta.X, Rel.Y, Rel.X, &tMin,
                                 MinCorner.X, MaxCorner.X))
                    {
                        WallNormal = v2{0, -1};
                        HitHighEntityIndex = TestHighEntityIndex;
                    }
                    if (TestWall(MaxCorner.Y, Playerdelta.Y, Playerdelta.X, Rel.Y, Rel.X, &tMin,
                                 MinCorner.X, MaxCorner.X))
                    {
                        WallNormal = v2{0, 1};
                        HitHighEntityIndex = TestHighEntityIndex;
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
    Entity.Low->P = MapIntoTileSpace(GameState->World, GameState->CameraP, Entity.High->P);
}

internal void
SetCamera(game_state *GameState, world_position NewCameraP)
{
    world *World = GameState->World;

    world_difference deltaCameraP = Subtract(World, &NewCameraP, &GameState->CameraP);
    GameState->CameraP = NewCameraP;

    uint32 TileSpanX = 17 * 3;
    uint32 TileSpanY = 9 * 3;

    rectangle2 CameraBounds = RectCenterDim(V2(0, 0),
                                            World->TileSideInMeters * V2((real32) TileSpanX,
                                                                         (real32) TileSpanY));

    v2 EntityOffsetForFrame = -deltaCameraP.deltaXY;
    OffsetAndCheckFrequencyByArea(GameState, EntityOffsetForFrame, CameraBounds);

#if 0
    int32 MinTileX = NewCameraP.ChunkX - TileSpanX / 2;
    int32 MaxTileX = NewCameraP.ChunkX + TileSpanX / 2;
    int32 MinTileY = NewCameraP.ChunkY - TileSpanY / 2;
    int32 MaxTileY = NewCameraP.ChunkY + TileSpanY / 2;

    for (uint32 EntityIndex = 1; EntityIndex < GameState->LowEntityCount; ++EntityIndex)
    {
        low_entity *Low = GameState->LowEntities + EntityIndex;
        if (Low->HighEntityIndex == 0)
        {
            if ((Low->P.ChunkZ == NewCameraP.ChunkZ) &&
                (Low->P.ChunkX >= MinTileX) &&
                (Low->P.ChunkX <= MaxTileX) &&
                (Low->P.ChunkY >= MinTileY) &&
                (Low->P.ChunkY <= MaxTileY))
            {
                MakeEntityHighFrequency(GameState, EntityIndex);
            }
        }
    }
#endif
}

#pragma clang diagnostic ignored "-Wwritable-strings"
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        AddLowEntity(GameState, EntityType_Null);
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
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
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
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
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
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
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
        Bitmap->AlignX = 72;
        Bitmap->AlignY = 182;
        ++Bitmap;

        InitializeArena(&GameState->WorldArena,
                        (mem_index) Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *) Memory->PermanentStorage + sizeof(game_state));
        GameState->World = PushStruct(&GameState->WorldArena, world);

        world *World = GameState->World;

        InitializeWorld(World, 1.4f);

        uint32 TilesPerHeight = 9;
        uint32 TilesPerWidth = 17;

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
        NewCameraP = ChunkPosFromTilePos(GameState->World, ScreenBaseX * TilesPerWidth + 17 / 2,
                                         ScreenBaseY * TilesPerHeight + 9 / 2,
                                         ScreenBaseZ);

        SetCamera(GameState, NewCameraP);

        Memory->IsInitialized = true;
    }

    world *World = GameState->World;

    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        uint32 LowIndex = GameState->PlayerIndexForController[ControllerIndex];
        if (LowIndex == 0)
        {
            if (Controller->Start.EndedDown)
            {
                uint32 EntityIndex = AddPlayer(GameState);
                GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
            }
        } else
        {
            entity ControllingEntity = GetHighEntity(GameState, LowIndex);
            v2 accelOfPlayer = {};
            if (Controller->IsAnalog)
            {
                accelOfPlayer = v2{Controller->StickAverageX, Controller->StickAverageY};
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
            if (Controller->AButton.EndedDown)
            {
                accelOfPlayer *= 50.0f;
            }

            if (Controller->YButton.EndedDown)
            {
                ControllingEntity.High->deltaZ = 4.0f;
            }
            MovePlayer(GameState, ControllingEntity, accelOfPlayer, Input->deltatForFrame);
        }
    }
    v2 EntityOffsetForFrame = {};
    entity CameraFollowingEntity = GetHighEntity(GameState,
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
    DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);

    real32 ScreenCenterX = (real32) Buffer->Width * 0.5f;
    real32 ScreenCenterY = (real32) Buffer->Height * 0.5f;
    real32 TileSideInPixels = 60;
    real32 MetersToPixel = TileSideInPixels / World->TileSideInMeters;

#if 0
    for (int32 RelRow = -10; RelRow < 10; ++RelRow)
    {
        for (int32 RelColumn = -20; RelColumn < 20; ++RelColumn)
        {
            uint32 Column = (uint32) RelColumn + GameState->CameraP.ChunkX;
            uint32 Row = (uint32) RelRow + GameState->CameraP.ChunkY;

            uint32 TileID = GetTileValue(World, Column, Row, GameState->CameraP.ChunkZ);

            if (TileID > 1)
            {
                real32 Gray = 0.2f;
                if (TileID == 2)
                {
                    Gray = 0.2f;
                }
                if (TileID > 2)
                {
                    Gray = 0.5f;
                }
                if ((Row == GameState->CameraP.ChunkY) && (Column == GameState->CameraP.ChunkX))
                {
                    Gray = 0.0;
                }

                v2 Center = {(ScreenCenterX - (MetersToPixel * GameState->CameraP.Offset_.X)) +
                             ((real32) RelColumn) * TileSideInPixels,
                             (ScreenCenterY + (MetersToPixel * GameState->CameraP.Offset_.Y)) -
                             ((real32) RelRow) * TileSideInPixels};
                v2 TileSide = {0.5f * TileSideInPixels, 0.5f * TileSideInPixels};
                v2 vMin = Center - 0.9f * TileSide;
                v2 vMax = Center + 0.9f * TileSide;

                DrawRectangle(Buffer, vMin, vMax, Gray, Gray, Gray);
            }
        }
    }
#endif

    for (uint32 HighEntityIndex = 1; HighEntityIndex < GameState->HighEntityCount; ++HighEntityIndex)
    {
        high_entity *HighEntity = GameState->HighEntities_ + HighEntityIndex;
        low_entity *LowEntity = GameState->LowEntities + HighEntity->LowEntityIndex;

        HighEntity->P += EntityOffsetForFrame;

        real32 deltat = Input->deltatForFrame;
        real32 accelOfPlayerInZ = -9.8f;
        real32 deltaZ = (0.5f * accelOfPlayerInZ * Square(deltat) + HighEntity->deltaZ * deltat);
        HighEntity->Z += deltaZ;
        HighEntity->deltaZ = accelOfPlayerInZ * deltat + HighEntity->deltaZ;
        if (HighEntity->Z < 0)
        {
            HighEntity->Z = 0;
        }
        real32 CAlpha = 1.0f - 0.5f * HighEntity->Z;
        if (CAlpha < 0)
        {
            CAlpha = 0.0f;
        }

        real32 HeroGroundPointX = ScreenCenterX + HighEntity->P.X * MetersToPixel;
        real32 HeroGroundPointY = ScreenCenterY - HighEntity->P.Y * MetersToPixel;

        real32 Z = -MetersToPixel * HighEntity->Z;
        hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[HighEntity->FacingDirection];

        v2 PlayerLeftTop = {HeroGroundPointX - (0.5f * LowEntity->Width * MetersToPixel),
                            HeroGroundPointY - (0.5f * LowEntity->Height * MetersToPixel)};
        v2 EntityWidthHeight = {LowEntity->Width, LowEntity->Height};

        if (LowEntity->Type == EntityType_Hero)
        {
            DrawBitmap(Buffer, &HeroBitmaps->HeroTorso, HeroGroundPointX, HeroGroundPointY + Z,
                       HeroBitmaps->AlignX, HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &HeroBitmaps->HeroCape, HeroGroundPointX, HeroGroundPointY + Z,
                       HeroBitmaps->AlignX, HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &HeroBitmaps->HeroHead, HeroGroundPointX, HeroGroundPointY + Z,
                       HeroBitmaps->AlignX, HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &GameState->HeroShadow, HeroGroundPointX, HeroGroundPointY,
                       HeroBitmaps->AlignX, HeroBitmaps->AlignY, CAlpha);
        } else
        {
            DrawRectangle(Buffer, PlayerLeftTop,
                          PlayerLeftTop + MetersToPixel * EntityWidthHeight,
                          1.0f, 1.0f, 0.0f);
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(SoundBuffer, GameState);
}