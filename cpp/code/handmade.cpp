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
#include "handmade_tile.cpp"
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
        GameState->tSine += 2.0f * Pi32 * 1.0f / (real32) WavePeriod;
        if (GameState->tSine > 2.0f * Pi32)
        {
            GameState->tSine -= 2.0f * Pi32;
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
    int32 MaxX = RoundReal32ToInt32(RealX + (real32) Bitmap->Width);
    int32 MaxY = RoundReal32ToInt32(RealY + (real32) Bitmap->Height);

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

internal void
ChangeEntityResidence(game_state *GameState, uint32 EntityIndex, entity_residence Residence)
{
    if (Residence == EntityResidence_High)
    {
        if (GameState->EntityResidence[EntityIndex] != EntityResidence_High)
        {
            high_entity *EntityHigh = &GameState->HighEntities[EntityIndex];
            dormant_entity *EntityDormant = &GameState->DormantEntities[EntityIndex];

            tile_map_difference Diff = Subtract(GameState->World->TileMap, &EntityDormant->P, &GameState->CameraP);

            EntityHigh->P = Diff.dXY;
            EntityHigh->dP = V2(0, 0);
            EntityHigh->AbsTileZ = EntityDormant->P.AbsTileZ;
            EntityHigh->FacingDirection = 0;
        }
    }
    GameState->EntityResidence[EntityIndex] = Residence;
}

internal entity
GetEntity(game_state *GameState, entity_residence Residence, uint32 Index)
{
    entity Entity = {};

    if ((Index > 0) && (Index < GameState->EntityCount))
    {
        if (GameState->EntityResidence[Index] < Residence)
        {
            ChangeEntityResidence(GameState, Index, Residence);
            Assert(GameState->EntityResidence[Index] >= Residence)
        }
        Entity.Residence = Residence;
        Entity.Dormant = &GameState->DormantEntities[Index];
        Entity.Low = &GameState->LowEntities[Index];
        Entity.High = &GameState->HighEntities[Index];
    }
    return (Entity);
}

internal void
InitializePlayer(game_state *GameState, uint32 EntityIndex)
{
    entity Entity = GetEntity(GameState, EntityResidence_Dormant, EntityIndex);

    Entity.Dormant->P.AbsTileX = 1;
    Entity.Dormant->P.AbsTileY = 3;
    Entity.Dormant->P.Offset_.X = 0.0f;
    Entity.Dormant->P.Offset_.Y = 0.0f;
    Entity.Dormant->Height = 0.5f;
    Entity.Dormant->Width = 1.0f;
    Entity.Dormant->Collides = true;
    ChangeEntityResidence(GameState, EntityIndex, EntityResidence_High);

    if (GetEntity(GameState, EntityResidence_Dormant, GameState->CameraFollowingEntityIndex).Residence
        == EntityResidence_Nonexistent)
    {
        GameState->CameraFollowingEntityIndex = EntityIndex;
    }
}

internal uint32
AddEntity(game_state *GameState)
{
    uint32 EntityIndex = GameState->EntityCount++;
    Assert(GameState->EntityCount < ArrayCount(GameState->HighEntities))
    Assert(GameState->EntityCount < ArrayCount(GameState->LowEntities))
    Assert(GameState->EntityCount < ArrayCount(GameState->DormantEntities))

    GameState->EntityResidence[EntityIndex] = EntityResidence_Dormant;
    GameState->DormantEntities[EntityIndex] = {};
    GameState->LowEntities[EntityIndex] = {};
    GameState->HighEntities[EntityIndex] = {};

    return (EntityIndex);
}

internal bool32
TestWall(real32 WallX, real32 PlayerDeltaX, real32 PlayerDeltaY, real32 RelX, real32 RelY, real32 *tMin,
         real32 MinY, real32 MaxY)
{
    bool32 Hit = false;
    real32 tEpsilon = 0.001f;
    if (PlayerDeltaX != 0.0f)
    {
        real32 tResult = (WallX - RelX) / PlayerDeltaX;
        real32 Y = RelY + tResult * PlayerDeltaY;

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
MovePlayer(game_state *GameState, entity Entity, v2 accelOfPlayer, real32 dt)
{
    tile_map *TileMap = GameState->World->TileMap;

    real32 accelLength = LengthSq(accelOfPlayer);

    if (accelLength > 1.0f)
    {
        accelOfPlayer *= 1.0f / SquareRoot(accelLength);
    }

    accelOfPlayer *= 70.0f;
    accelOfPlayer += -7.0f * Entity.High->dP;

    v2 OldPlayerP = Entity.High->P;
    v2 PlayerDisplacement = (0.5f * accelOfPlayer * Square(dt) +
                             Entity.High->dP * dt);
    Entity.High->dP = accelOfPlayer * dt + Entity.High->dP;
    v2 NewPlayerP = OldPlayerP + PlayerDisplacement;

    /*uint32 MinTileX = Minimum(OldPlayerP.X, NewPlayerP.X);
    uint32 MinTileY = Minimum(OldPlayerP.Y, NewPlayerP.Y);
    uint32 MaxTileX = Maximum(OldPlayerP.X, NewPlayerP.X);
    uint32 MaxTileY = Maximum(OldPlayerP.Y, NewPlayerP.Y);

    uint32 EntityTileWidth = CeilReal32ToInt32(Entity.Dormant->Width / TileMap->TileSideInMeters);
    uint32 EntityTileHeight = CeilReal32ToInt32(Entity.Dormant->Height / TileMap->TileSideInMeters);

    MinTileX -= EntityTileWidth;
    MinTileY -= EntityTileHeight;
    MaxTileX += EntityTileWidth;
    MaxTileY += EntityTileHeight;

    uint32 AbsTileZ = Entity.Dormant->P.AbsTileZ;*/

    real32 tRemaining = 1.0f;
    for (uint8 Iteration = 0; (Iteration < 4) && (tRemaining > 0.0f); ++Iteration)
    {
        real32 tMin = 1.0f;
        v2 WallNormal = {};
        uint32 HitEntityIndex = 0;
        for (uint32 EntityIndex = 1; EntityIndex < GameState->EntityCount; ++EntityIndex)
        {
            entity TestEntity = GetEntity(GameState, EntityResidence_High, EntityIndex);
            if (TestEntity.High != Entity.High)
            {
                if (TestEntity.Dormant->Collides)
                {
                    real32 DiameterW = TestEntity.Dormant->Width + Entity.Dormant->Width;
                    real32 DiameterH = TestEntity.Dormant->Height + Entity.Dormant->Height;

                    v2 MinCorner = -0.5f * v2{DiameterW, DiameterH};
                    v2 MaxCorner = 0.5f * v2{DiameterW, DiameterH};
                    v2 Rel = Entity.High->P - TestEntity.High->P;

                    if (TestWall(MinCorner.X, PlayerDisplacement.X, PlayerDisplacement.Y, Rel.X, Rel.Y, &tMin,
                                 MinCorner.Y, MaxCorner.Y))
                    {
                        WallNormal = v2{-1, 0};
                        HitEntityIndex = EntityIndex;
                    }
                    if (TestWall(MaxCorner.X, PlayerDisplacement.X, PlayerDisplacement.Y, Rel.X, Rel.Y, &tMin,
                                 MinCorner.Y, MaxCorner.Y))
                    {
                        WallNormal = v2{1, 0};
                        HitEntityIndex = EntityIndex;
                    }

                    if (TestWall(MinCorner.Y, PlayerDisplacement.Y, PlayerDisplacement.X, Rel.Y, Rel.X, &tMin,
                                 MinCorner.X, MaxCorner.X))
                    {
                        WallNormal = v2{0, -1};
                        HitEntityIndex = EntityIndex;
                    }
                    if (TestWall(MaxCorner.Y, PlayerDisplacement.Y, PlayerDisplacement.X, Rel.Y, Rel.X, &tMin,
                                 MinCorner.X, MaxCorner.X))
                    {
                        WallNormal = v2{0, 1};
                        HitEntityIndex = EntityIndex;
                    }
                }
            }
        }

        Entity.High->P += tMin * PlayerDisplacement;
        if (HitEntityIndex)
        {
            Entity.High->dP = Entity.High->dP - (DotProduct(WallNormal, Entity.High->dP) * WallNormal);
            PlayerDisplacement = PlayerDisplacement - (DotProduct(PlayerDisplacement, WallNormal) * WallNormal);
            tRemaining -= tMin * tRemaining;

            entity HitEntity = GetEntity(GameState, EntityResidence_Dormant, HitEntityIndex);
            Entity.High->AbsTileZ += HitEntity.Dormant->dAbsTileZ;
        } else
        {
            break;
        }
    }

    if ((AbsoluteValue(Entity.High->dP.X) == 0.0f) && (AbsoluteValue(Entity.High->dP.Y)) == 0.0f)
    {
    } else if (AbsoluteValue(Entity.High->dP.X) > AbsoluteValue(Entity.High->dP.Y))
    {
        if (Entity.High->dP.X > 0)
        {
            Entity.High->FacingDirection = 2;
        } else
        {
            Entity.High->FacingDirection = 3;
        }
    } else
    {
        if (Entity.High->dP.Y > 0)
        {
            Entity.High->FacingDirection = 0;
        } else
        {
            Entity.High->FacingDirection = 1;
        }
    }
    Entity.Dormant->P = MapIntoTileSpace(GameState->World->TileMap, GameState->CameraP, Entity.High->P);
}

//#pragma clang diagnostic ignored "-Wnull-dereference"
#pragma clang diagnostic ignored "-Wwritable-strings"
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        AddEntity(GameState);

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

        Memory->IsInitialized = true;

        GameState->CameraP.AbsTileX = 17 / 2;
        GameState->CameraP.AbsTileY = 9 / 2;

        InitializeArena(&GameState->WorldArena, (mem_index) Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *) Memory->PermanentStorage + sizeof(game_state));
        GameState->World = PushStruct(&GameState->WorldArena, world);

        world *World = GameState->World;
        World->TileMap = PushStruct(&GameState->WorldArena, tile_map);

        tile_map *TileMap = World->TileMap;

        TileMap->TileSideInMeters = 1.4f;

        TileMap->ChunkShift = 4;
        TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;

        TileMap->ChunkDim = (1 << TileMap->ChunkShift);

        TileMap->TileChunkCountX = 128;
        TileMap->TileChunkCountY = 128;
        TileMap->TileChunkCountZ = 2;

        TileMap->TileChunks = PushArray(&GameState->WorldArena,
                                        TileMap->TileChunkCountX *
                                        TileMap->TileChunkCountY *
                                        TileMap->TileChunkCountZ,
                                        tile_chunk);

        uint32 TilesPerHeight = 9;
        uint32 TilesPerWidth = 17;

#if 0
        int ScreenX = INT32_MAX / 2;
        int ScreenY = INT32_MAX / 2;
#else
        int ScreenX = 0;
        int ScreenY = 0;
#endif
        uint32 RandomNumberIndex = 0;
        uint32 AbsTileZ = 0;

        bool32 DoorLeft = false;
        bool32 DoorRight = false;
        bool32 DoorTop = false;
        bool32 DoorBottom = false;

        bool32 DoorUp = false;
        bool32 DoorDown = false;

        for (int ScreenIndex = 0; ScreenIndex < 100; ++ScreenIndex)
        {
            Assert(RandomNumberIndex < ArrayCount(RandomNumberTable))
            uint32 RandomChoice;

            if (DoorDown || DoorUp)
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
            } else
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
            }

            bool32 ZDoorCreated = false;
            if (RandomChoice == 2)
            {
                ZDoorCreated = true;
                if (AbsTileZ == 0)
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

                    SetTileValue(&GameState->WorldArena, World->TileMap,
                                 AbsTileX, AbsTileY, AbsTileZ, TileValue);
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
                if (AbsTileZ == 0)
                {
                    AbsTileZ = 1;
                } else
                {
                    AbsTileZ = 0;
                }
            } else if (RandomChoice == 1)
            {
                ScreenX += 1;
            } else
            {
                ScreenY += 1;
            }
        }
    }

    world *World = GameState->World;
    tile_map *TileMap = World->TileMap;

    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        entity ControllingEntity = GetEntity(GameState, EntityResidence_High,
                                             GameState->PlayerIndexForController[ControllerIndex]);
        if (ControllingEntity.Residence != EntityResidence_Nonexistent)
        {
            v2 accelerationOfPlayer = {};
            if (Controller->IsAnalog)
            {
                accelerationOfPlayer = v2{Controller->StickAverageX, Controller->StickAverageY};
            } else
            {
                if (Controller->MoveUp.EndedDown)
                {
                    accelerationOfPlayer.Y = 1.0f;
                }
                if (Controller->MoveDown.EndedDown)
                {
                    accelerationOfPlayer.Y = -1.0f;
                }
                if (Controller->MoveLeft.EndedDown)
                {
                    accelerationOfPlayer.X = -1.0f;
                }
                if (Controller->MoveRight.EndedDown)
                {
                    accelerationOfPlayer.X = 1.0f;
                }

                if (Controller->AButton.EndedDown)
                {
                    accelerationOfPlayer *= 50.0f;
                }

                if (Controller->YButton.EndedDown)
                {
                    ControllingEntity.High->dZ = 4.0f;
                }
            }
            MovePlayer(GameState, ControllingEntity, accelerationOfPlayer, Input->dtForFrame);
        } else
        {
            if (Controller->Start.EndedDown)
            {
                uint32 EntityIndex = AddEntity(GameState);
//                ControllingEntity = GetEntity(GameState, EntityIndex);
                InitializePlayer(GameState, EntityIndex);
                GameState->PlayerIndexForController[ControllerIndex] = EntityIndex;
            }
        }
    }
    v2 EntityOffsetForFrame = {};
    entity CameraFollowingEntity = GetEntity(GameState, EntityResidence_High, GameState->CameraFollowingEntityIndex);
    if (CameraFollowingEntity.Residence != EntityResidence_Nonexistent)
    {
        tile_map_position OldCameraP = GameState->CameraP;
        GameState->CameraP.AbsTileZ = CameraFollowingEntity.Dormant->P.AbsTileZ;

        if (CameraFollowingEntity.High->P.X > (9.0f * TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileX += 17;
        }
        if (CameraFollowingEntity.High->P.X < (-9.0f * TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileX -= 17;
        }
        if (CameraFollowingEntity.High->P.Y > (5.0f * TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileY += 9;
        }
        if (CameraFollowingEntity.High->P.Y < (-5.0f * TileMap->TileSideInMeters))
        {
            GameState->CameraP.AbsTileY -= 9;
        }

        tile_map_difference dCameraP = Subtract(TileMap, &GameState->CameraP, &OldCameraP);
        EntityOffsetForFrame = -dCameraP.dXY;
    }
    DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);

    real32 ScreenCenterX = (real32) Buffer->Width * 0.5f;
    real32 ScreenCenterY = (real32) Buffer->Height * 0.5f;
    real32 TileSideInPixels = 60;
    real32 MetersToPixel = TileSideInPixels / TileMap->TileSideInMeters;

    for (int32 RelRow = -10; RelRow < 10; ++RelRow)
    {
        for (int32 RelColumn = -20; RelColumn < 20; ++RelColumn)
        {
            uint32 Column = (uint32) RelColumn + GameState->CameraP.AbsTileX;
            uint32 Row = (uint32) RelRow + GameState->CameraP.AbsTileY;

            uint32 TileID = GetTileValue(TileMap, Column, Row, GameState->CameraP.AbsTileZ);

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
                if ((Row == GameState->CameraP.AbsTileY) && (Column == GameState->CameraP.AbsTileX))
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

    for (uint32 EntityIndex = 0; EntityIndex < GameState->EntityCount; ++EntityIndex)
    {
        if (GameState->EntityResidence[EntityIndex] == EntityResidence_High)
        {
            high_entity *HighEntity = &GameState->HighEntities[EntityIndex];
            dormant_entity *DormantEntity = &GameState->DormantEntities[EntityIndex];

            HighEntity->P += EntityOffsetForFrame;

            real32 dt = Input->dtForFrame;
            real32 accelOfPlayerInZ = -9.8f;
            real32 DisplacementInZ = (0.5f * accelOfPlayerInZ * Square(dt) + HighEntity->dZ * dt);
            HighEntity->Z += DisplacementInZ;
            HighEntity->dZ = accelOfPlayerInZ * dt + HighEntity->dZ;
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

            real32 PlayerLeft = HeroGroundPointX - (0.5f * DormantEntity->Width * MetersToPixel);
            real32 PlayerTop = HeroGroundPointY - (0.5f * DormantEntity->Height * MetersToPixel);
            real32 Z = -MetersToPixel * HighEntity->Z;
            hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[HighEntity->FacingDirection];

            DrawBitmap(Buffer, &HeroBitmaps->HeroTorso, HeroGroundPointX, HeroGroundPointY + Z, HeroBitmaps->AlignX,
                       HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &HeroBitmaps->HeroCape, HeroGroundPointX, HeroGroundPointY + Z, HeroBitmaps->AlignX,
                       HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &HeroBitmaps->HeroHead, HeroGroundPointX, HeroGroundPointY + Z, HeroBitmaps->AlignX,
                       HeroBitmaps->AlignY);
            DrawBitmap(Buffer, &GameState->HeroShadow, HeroGroundPointX, HeroGroundPointY, HeroBitmaps->AlignX,
                       HeroBitmaps->AlignY, CAlpha);
        }
    }
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(SoundBuffer, GameState);
}