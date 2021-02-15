//
// Created by AgentOfChaos on 11/20/2020.
//
#include "handmade_platform.h"

#include "handmade.h"
#include "handmade_world.cpp"
#include "handmade_sim_region.cpp"
#include "handmade_random.h"
#include "handmade_entity.cpp"

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

struct add_low_entity_result
{
    low_entity *Low;
    uint32 LowIndex;
};

internal add_low_entity_result
AddLowEntity(game_state *GameState, entity_type Type, world_position P)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities))

    uint32 EntityIndex = GameState->LowEntityCount++;

    low_entity *EntityLow = GameState->LowEntities + EntityIndex;
    *EntityLow = {};
    EntityLow->Sim.Type = Type;
    EntityLow->P = NullPosition();

    ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, P);

    add_low_entity_result Result = {};
    Result.Low = EntityLow;
    Result.LowIndex = EntityIndex;
    return (Result);
}

inline void
InitHitPoints(low_entity *EntityLow, uint32 HitPointCount)
{
    Assert(HitPointCount < ArrayCount(EntityLow->Sim.HitPoint))
    EntityLow->Sim.HitPointMax = HitPointCount;

    for (uint32 HitPointIndex = 0; HitPointIndex < EntityLow->Sim.HitPointMax; ++HitPointIndex)
    {
        hit_point *HitPoint = EntityLow->Sim.HitPoint + HitPointIndex;
        HitPoint->Flags = 0;
        HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
    }
}

internal add_low_entity_result
AddWall(game_state *GameState, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Wall, P);

    Entity.Low->Sim.Height = GameState->World->TileSideInMeters;
    Entity.Low->Sim.Width = Entity.Low->Sim.Height;
    AddFlag(&Entity.Low->Sim, EntityFlag_Collides);

    return (Entity);
}

internal add_low_entity_result
AddMonster(game_state *GameState, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Monster, P);

    InitHitPoints(Entity.Low, 3);
    Entity.Low->Sim.Height = 0.5f;
    Entity.Low->Sim.Width = 1.0f;
    AddFlag(&Entity.Low->Sim, EntityFlag_Collides);

    return (Entity);
}

internal add_low_entity_result
AddSword(game_state *GameState)
{
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, NullPosition());

    Entity.Low->Sim.Height = 0.5f;
    Entity.Low->Sim.Width = 1.0f;
    AddFlag(&Entity.Low->Sim, EntityFlag_NonSpatial);

    return (Entity);
}

internal add_low_entity_result
AddFamiliar(game_state *GameState, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Familiar, P);

    Entity.Low->Sim.Height = 0.5f;
    Entity.Low->Sim.Width = 1.0f;

    return (Entity);
}

internal add_low_entity_result
AddPlayer(game_state *GameState)
{
    world_position P = GameState->CameraP;
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Hero, P);

    Entity.Low->Sim.Height = 0.5f;
    Entity.Low->Sim.Width = 1.0f;
    AddFlag(&Entity.Low->Sim, EntityFlag_Collides);
    InitHitPoints(Entity.Low, 3);

    add_low_entity_result Sword = AddSword(GameState);
    Entity.Low->Sim.Sword.Index = Sword.LowIndex;

    if (GameState->CameraFollowingEntityIndex == 0)
    {
        GameState->CameraFollowingEntityIndex = Entity.LowIndex;
    }
    return (Entity);
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

internal void
DrawHitPoints(sim_entity *Entity, entity_visible_piece_group *PieceGroup)
{
    if (Entity->HitPointMax >= 1)
    {
        v2 HealthDim = {0.2f, 0.2f};
        real32 SpacingInX = 1.5f * HealthDim.X;

        v2 HitP = {-0.5f * ((real32) (Entity->HitPointMax - 1)) * SpacingInX, -0.2f};
        v2 dHitP = {SpacingInX, 0.0f};
        for (uint32 HealthIndex = 0; HealthIndex < Entity->HitPointMax; ++HealthIndex)
        {
            hit_point *HitPoint = Entity->HitPoint + HealthIndex;
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

internal void
ClearCollisionRuleFor(game_state *GameState, uint32 StorageIndex)
{

    for (uint32 HashBucket = 0;
         HashBucket < ArrayCount(GameState->CollisionRuleHash);
         ++HashBucket)
    {

        for (pairwise_collision_rule **Rule = &GameState->CollisionRuleHash[HashBucket];
             *Rule;
             )
        {
            if (((*Rule)->StorageIndexA == StorageIndex) ||
                ((*Rule)->StorageIndexB == StorageIndex))
            {
                pairwise_collision_rule *RemovedRule = *Rule;
                *Rule = (*Rule)->NextInHash;

                RemovedRule->NextInHash = GameState->FirstFreeCollisionRule;
                GameState->FirstFreeCollisionRule = RemovedRule;
            }else
            {
                Rule = &(*Rule)->NextInHash;
            }
        }
    }
}

internal void
AddCollisionRule(game_state *GameState, uint32 StorageIndexA, uint32 StorageIndexB, bool32 ShouldCollide)
{
    if (StorageIndexA > StorageIndexB)
    {
        uint32 Temp = StorageIndexA;
        StorageIndexA = StorageIndexB;
        StorageIndexB = Temp;
    }

    pairwise_collision_rule *Found = 0;
    uint32 HashBucket = StorageIndexA & (ArrayCount((GameState->CollisionRuleHash) - 1));
    for (pairwise_collision_rule *Rule = GameState->CollisionRuleHash[HashBucket];
         Rule;
         Rule = Rule->NextInHash)
    {
        if ((Rule->StorageIndexA == StorageIndexA) &&
            (Rule->StorageIndexB == StorageIndexB))
        {
            Found = Rule;
            break;
        }
    }

    if (!Found)
    {
        Found = GameState->FirstFreeCollisionRule;

        if (Found)
        {
            GameState->FirstFreeCollisionRule = Found->NextInHash;
        } else
        {
            Found = PushStruct(&GameState->WorldArena, pairwise_collision_rule);
        }
        Found->NextInHash = GameState->CollisionRuleHash[HashBucket];
        GameState->CollisionRuleHash[HashBucket] = Found;
    }

    if (Found)
    {
        Found->StorageIndexA = StorageIndexA;
        Found->StorageIndexB = StorageIndexB;
        Found->ShouldCollide = ShouldCollide;
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        AddLowEntity(GameState, EntityType_Null, NullPosition());

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
                                        "new/sword.bmp");

        GameState->Familiar = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "new/familiar.bmp");
        GameState->Monster = DEBUGLoadBMP(Thread,
                                          Memory->DEBUGPlatformReadEntireFile,
                                          "new/monster.bmp");
        GameState->MonsterDead = DEBUGLoadBMP(Thread,
                                          Memory->DEBUGPlatformReadEntireFile,
                                          "new/monster_dead.bmp");
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

        GameState->CameraP = NewCameraP;

        AddMonster(GameState, CameraTileX + 2, CameraTileY + 2, CameraTileZ);
        AddFamiliar(GameState, CameraTileX - 2, CameraTileY + 2, CameraTileZ);

        Memory->IsInitialized = true;
    }

    world *World = GameState->World;

    for (int ControllerIndex = 0;
         ControllerIndex < ArrayCount(Input->Controllers);
         ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_hero *ConHero = &GameState->ControlledHeroes[ControllerIndex];
        if (ConHero->EntityIndex == 0)
        {
            if (Controller->Start.EndedDown)
            {
                *ConHero = {};
                ConHero->EntityIndex = AddPlayer(GameState).LowIndex;
            }
        } else
        {
            ConHero->deltaZ = 0;
            ConHero->accel = {};
            ConHero->deltaSword = {};

            if (Controller->IsAnalog)
            {
                ConHero->accel = V2(Controller->StickAverageX, Controller->StickAverageY);
            } else
            {
                if (Controller->MoveUp.EndedDown)
                {
                    ConHero->accel.Y = 1.0f;
                }
                if (Controller->MoveDown.EndedDown)
                {
                    ConHero->accel.Y = -1.0f;
                }
                if (Controller->MoveLeft.EndedDown)
                {
                    ConHero->accel.X = -1.0f;
                }
                if (Controller->MoveRight.EndedDown)
                {
                    ConHero->accel.X = 1.0f;
                }
            }

            if (Controller->ActionDown.EndedDown)
            {
                ConHero->accel *= 50.0f;
            }

            if (Controller->Start.EndedDown)
            {
                ConHero->deltaZ = 4.0f;
            }

            ConHero->deltaSword = {};
            if (Controller->ActionUp.EndedDown)
            {
                ConHero->deltaSword = {0.0f, 1.0f};
            }
            if (Controller->ActionDown.EndedDown)
            {
                ConHero->deltaSword = {0.0f, -1.0f};
            }
            if (Controller->ActionLeft.EndedDown)
            {
                ConHero->deltaSword = {-1.0f, 0.0f};
            }
            if (Controller->ActionRight.EndedDown)
            {
                ConHero->deltaSword = {1.0f, 0.0f};
            }
        }
    }

    uint32 ChunkSpanX = 17 * 3;
    uint32 ChunkSpanY = 9 * 3;

    rectangle2 CameraBounds = RectCenterDim(V2(0, 0),
                                            World->TileSideInMeters * V2((real32) ChunkSpanX,
                                                                         (real32) ChunkSpanY));

    memory_arena SimArena;
    InitializeArena(&SimArena, (mem_index) Memory->TransientStorageSize, Memory->TransientStorage);
    sim_region *SimRegion = BeginSim(GameState, &SimArena,
                                     GameState->World, GameState->CameraP, CameraBounds);

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
    sim_entity *Entity = SimRegion->Entities;

    for (uint32 EntityIndex = 0;
         EntityIndex < SimRegion->EntityCount;
         ++EntityIndex, ++Entity)
    {
        if (Entity->Updatable)
        {
            PieceGroup.Count = 0;

            real32 deltat = Input->deltatForFrame;
            real32 ShadowAlpha = 1.0f - 0.5f * Entity->Z;
            if (ShadowAlpha < 0)
            {
                ShadowAlpha = 0.0f;
            }

            move_spec MoveSpec = DefaultMoveSpec();
            v2 accelOfEntity = {};

            hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[Entity->FacingDirection];
            switch (Entity->Type)
            {
                case EntityType_Hero:
                {
                    for (uint32 ControlIndex = 0;
                         ControlIndex < ArrayCount(GameState->ControlledHeroes);
                         ++ControlIndex)
                    {
                        controlled_hero *ConHero = GameState->ControlledHeroes + ControlIndex;

                        if (Entity->StorageIndex == ConHero->EntityIndex)
                        {
                            if (ConHero->deltaZ != 0.0f)
                            {
                                Entity->deltaZ = ConHero->deltaZ;
                            }

                            MoveSpec.UnitMaxAccelVector = true;
                            MoveSpec.Speed = 70.0f;
                            MoveSpec.Drag = 7.0f;
                            accelOfEntity = ConHero->accel;

                            if ((ConHero->deltaSword.X != 0.0f) || (ConHero->deltaSword.Y != 0.0f))
                            {
                                sim_entity *Sword = Entity->Sword.Ptr;
                                if (Sword && IsSet(Sword, EntityFlag_NonSpatial))
                                {
                                    Sword->DistanceLimit = 5.0f;
                                    MakeEntitySpatial(Sword,
                                                      Entity->P,
                                                      Entity->deltaP + 5.0f * ConHero->deltaSword);
                                    AddCollisionRule(GameState,
                                                     Sword->StorageIndex,
                                                     Entity->StorageIndex,
                                                     false);
                                }
                            }
                        }
                    }

                    PushBitmap(&PieceGroup, &HeroBitmaps->HeroTorso, V2(0, 0), 0,
                               HeroBitmaps->Align);
                    PushBitmap(&PieceGroup, &HeroBitmaps->HeroCape, V2(0, 0), 0,
                               HeroBitmaps->Align);
                    PushBitmap(&PieceGroup, &HeroBitmaps->HeroHead, V2(0, 0), 0,
                               HeroBitmaps->Align);
                    PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                               HeroBitmaps->Align, ShadowAlpha, 0.0f);

                    DrawHitPoints(Entity, &PieceGroup);
                }
                    break;
                case EntityType_Wall:
                {
                    PushBitmap(&PieceGroup, &GameState->Tree,
                               V2(0, 0), 0, V2(40, 80));
                }
                    break;

                case EntityType_Sword:
                {
                    MoveSpec.Speed = 0.0f;
                    MoveSpec.Drag = 0.0f;

                    v2 OldP = Entity->P;
                    if (Entity->DistanceLimit == 0.0f)
                    {
                        ClearCollisionRuleFor(GameState, Entity->StorageIndex);
                        MakeEntityNonSpatial(Entity);
                    }

                    PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                               HeroBitmaps->Align, ShadowAlpha, 0.0f);
                    PushBitmap(&PieceGroup, &GameState->Sword,
                               V2(0, 0), 0, V2(29, 10));
                }
                    break;
                case EntityType_Familiar:
                {
                    sim_entity *ClosestHero = 0;
                    real32 ClosestHeroDSq = Square(10.0f);

                    sim_entity *TestEntity = SimRegion->Entities;

                    for (uint32 HighEntityIndex = 1;
                         HighEntityIndex < SimRegion->EntityCount;
                         ++HighEntityIndex, ++TestEntity)
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
                    if (ClosestHero && ClosestHeroDSq > Square(3.0f))
                    {
                        real32 accel = 0.5f;
                        real32 OneOverLength = accel / SquareRoot(ClosestHeroDSq);
                        accelOfEntity = OneOverLength * (ClosestHero->P - Entity->P);
                    }
                    MoveSpec.UnitMaxAccelVector = true;
                    MoveSpec.Speed = 70.0f;
                    MoveSpec.Drag = 7.0f;

                    Entity->tBob += deltat;

                    if (Entity->tBob > (2 * PII32))
                    {
                        Entity->tBob -= (2 * PII32);
                    }

                    real32 SinBob = Sin(4.0f * Entity->tBob);
                    PushBitmap(&PieceGroup, &GameState->Familiar, V2(0, 0),
                               0.25f * SinBob, HeroBitmaps->Align);

                    PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                               HeroBitmaps->Align, (0.5f * ShadowAlpha) + 0.2f * SinBob, 0.0F);
                }
                    break;
                case EntityType_Monster:
                {

                    if (Entity->HitPointMax < 1)
                    {
                        PushBitmap(&PieceGroup, &GameState->MonsterDead, V2(0, 0), 0,
                                   HeroBitmaps->Align);
                    }else
                    {
                        PushBitmap(&PieceGroup, &GameState->Monster, V2(0, 0), 0,
                                   HeroBitmaps->Align);
                    }

                    PushBitmap(&PieceGroup, &GameState->HeroShadow, V2(0, 0), 0,
                               HeroBitmaps->Align, ShadowAlpha, 0.0f);

                    DrawHitPoints(Entity, &PieceGroup);
                }
                    break;
                default:
                {
                    InvalidCodePath
                }
                    break;
            }

            if (!IsSet(Entity, EntityFlag_NonSpatial))
            {
                MoveEntity(GameState, SimRegion, Entity, accelOfEntity, &MoveSpec, Input->deltatForFrame);
            }

            real32 EntityGroundPointX = ScreenCenterX + Entity->P.X * GameState->MetersToPixel;
            real32 EntityGroundPointY = ScreenCenterY - Entity->P.Y * GameState->MetersToPixel;
            real32 EntityZ = -GameState->MetersToPixel * Entity->Z;

            for (uint32 PieceIndex = 0;
                 PieceIndex < PieceGroup.Count;
                 ++PieceIndex)
            {
                entity_visible_piece *Piece = PieceGroup.Pieces + PieceIndex;

                v2 Center = {EntityGroundPointX + Piece->Offset.X,
                             EntityGroundPointY + Piece->Offset.Y + Piece->OffsetZ + EntityZ *
                             Piece->EntityZC};

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

    EndSim(GameState, SimRegion);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(SoundBuffer, GameState);
}
