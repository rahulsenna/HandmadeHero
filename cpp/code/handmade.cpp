//
// Created by AgentOfChaos on 11/20/2020.
//

#include "handmade.h"
#include "handmade_render_group.cpp"
#include "handmade_world.cpp"
#include "handmade_sim_region.cpp"
#include "handmade_entity.cpp"
#include "handmade_random.h"

internal void
GameOutputSound(game_sound_output_buffer *SoundBuffer, game_state *GameState)
{
    s16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0;
         SampleIndex < SoundBuffer->SampleCount;
         ++SampleIndex)
    {
#if 0
        s16 ToneVolume = 3000;
        r32 SineValue = sinf(GameState->tSine);
        s16 SampleValue = (s16) (SineValue * (r32) ToneVolume);
#else
        s16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
#if 0
        int WavePeriod = SoundBuffer->SamplesPerSecond / 400;
        GameState->tSine += 2.0f * Pi32 * 1.0f / (r32) WavePeriod;
        if (GameState->tSine > 2.0f * Pi32)
        {
            GameState->tSine -= 2.0f * Pi32;
        }
#endif
    }
}

#if 0
void
RenderWeirdGradient(game_offscreen_buffer *DrawBuffer, int XOffset, int YOffset)
{
    u8 *Row = (u8 *) DrawBuffer->Memory;
    for (int y = 0; y < DrawBuffer->Height; ++y)
    {
        u32 *Pixel = (u32 *) Row;
        for (int x = 0; x < DrawBuffer->Width; ++x)
        {
            u8 Blue = (u8) (x + XOffset);
            u8 Green = (u8) (y + YOffset);
            *Pixel++ = ((Green << 16) | Blue);
        }
        Row += DrawBuffer->Pitch;
    }
}
#endif

#pragma pack(push, 1)
struct bitmap_header
{
    u16 FileType;
    u32 FileSize;
    u16 FileReserved1;
    u16 FileReserved2;
    u32 FileOffBits;

    u32 Size;
    u32 Width;
    u32 Height;
    u16 Planes;
    u16 BitCount;

    u32 Compression;
    u32 SizeImage;
    s32 XPelsPerMeter;
    s32 YPelsPerMeter;
    u32 ClrUsed;
    u32 ClrImportant;
    u32 RedMask;
    u32 GreenMask;
    u32 BlueMask;
    u32 AlphaMask;
};
#pragma pack(pop)

internal loaded_bitmap
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename,
             s32 AlignX, s32 TopDownAlignY)
{
    debug_read_file_result ReadResult = ReadEntireFile(Thread, Filename);

    loaded_bitmap Result = {};
    if (ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *) ReadResult.Contents;
        u32 *          Pixels = (u32 *) ((u8 *) ReadResult.Contents + Header->FileOffBits);

        Result.Width  = Header->Width;
        Result.Height = Header->Height;
        Result.Memory = Pixels;

        Result.AlignPercentage.x = SafeRatio0((r32) AlignX, (r32) Result.Width);
        Result.AlignPercentage.y = SafeRatio0((r32) ((Result.Height - 1) - TopDownAlignY), (r32) Result.Height);
        Result.WidthOverHeight   = SafeRatio0((r32) Result.Width, (r32) Result.Height);

        Assert(Result.Height >= 0);
        Assert(Header->Compression == 3);

        bit_scan_result RedScan   = FindLeastSignificantSetBit(Header->RedMask);
        bit_scan_result GreenScan = FindLeastSignificantSetBit(Header->GreenMask);
        bit_scan_result BlueScan  = FindLeastSignificantSetBit(Header->BlueMask);
        bit_scan_result AlphaScan = FindLeastSignificantSetBit(Header->AlphaMask);

        Assert(RedScan.Found)
        Assert(GreenScan.Found)
        Assert(BlueScan.Found)
        Assert(AlphaScan.Found)

        s32 RedShift   = (s32) RedScan.Index;
        s32 GreenShift = (s32) GreenScan.Index;
        s32 BlueShift  = (s32) BlueScan.Index;
        s32 AlphaShift = (s32) AlphaScan.Index;

        u32 *Source = Pixels;
        for (u32 Y = 0; Y < Header->Height; ++Y)
        {
            for (u32 X = 0; X < Header->Width; ++X)
            {
                u32 C = *Source;

                v4 Texel = {(r32) ((C & Header->RedMask) >> RedShift),
                            (r32) ((C & Header->GreenMask) >> GreenShift),
                            (r32) ((C & Header->BlueMask) >> BlueShift),
                            (r32) ((C & Header->AlphaMask) >> AlphaShift)};

                Texel = SRGB255ToLinear1(Texel);
#if 1
                Texel.rgb *= Texel.a;
#endif
                Texel = Linear1ToSRGB255(Texel);

                *Source = (((u32) (Texel.r + 0.5f) << RED_PLACE) |
                           ((u32) (Texel.g + 0.5f) << GREEN_PLACE) |
                           ((u32) (Texel.b + 0.5f) << BLUE_PLACE)) |
                          ((u32) (Texel.a + 0.5f) << 24);
                ++Source;
            }
        }
    }

    Result.Pitch = Result.Width * BYTES_PER_PIXEL;

#if 0
        Result.Memory = (u8 *) Result.Memory + Result.Pitch * (Result.Height - 1);
        Result.Pitch  = -Result.Pitch;
#endif

    return (Result);
}

internal loaded_bitmap
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename)
{
    loaded_bitmap Result   = DEBUGLoadBMP(Thread, ReadEntireFile, Filename, 0, 0);
    Result.AlignPercentage = V2(.5f, .5f);
    return (Result);
}

struct add_low_entity_result
{
    low_entity *Low;
    u32         LowIndex;
};

internal add_low_entity_result
AddLowEntity(game_state *GameState, entity_type Type, world_position P)
{
    Assert(GameState->LowEntityCount < ArrayCount(GameState->LowEntities))

    u32 EntityIndex = GameState->LowEntityCount++;

    low_entity *EntityLow    = GameState->LowEntities + EntityIndex;
    *EntityLow               = {};
    EntityLow->Sim.Type      = Type;
    EntityLow->Sim.Collision = GameState->NullCollision;
    EntityLow->P             = NullPosition();

    ChangeEntityLocation(&GameState->WorldArena, GameState->World, EntityIndex, EntityLow, P);

    add_low_entity_result Result = {};
    Result.Low                   = EntityLow;
    Result.LowIndex              = EntityIndex;
    return (Result);
}

internal add_low_entity_result
AddGroundedEntity(game_state *GameState, entity_type Type,
                  world_position P, sim_entity_collision_volume_group *Collision)
{
    add_low_entity_result Entity = AddLowEntity(GameState, Type, P);
    Entity.Low->Sim.Collision    = Collision;

    return (Entity);
}

inline void
InitHitPoints(low_entity *EntityLow, u32 HitPointCount)
{
    Assert(HitPointCount < ArrayCount(EntityLow->Sim.HitPoint))
    EntityLow->Sim.HitPointMax = HitPointCount;

    for (u32 HitPointIndex = 0; HitPointIndex < EntityLow->Sim.HitPointMax; ++HitPointIndex)
    {
        hit_point *HitPoint    = EntityLow->Sim.HitPoint + HitPointIndex;
        HitPoint->Flags        = 0;
        HitPoint->FilledAmount = HIT_POINT_SUB_COUNT;
    }
}

inline world_position
ChunkPosFromTilePos(world *World, s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ,
                    v3 AdditionalOffset = V3(0, 0, 0))
{
    world_position BasePos = {};

    r32 TileSideInMeters  = 1.4f;
    r32 TileDepthInMeters = 3.0f;

    v3 TileDim = V3(TileSideInMeters, TileSideInMeters, TileDepthInMeters);
    v3 Offset  = Hadamard(TileDim, V3((r32) AbsTileX, (r32) AbsTileY, (r32) AbsTileZ));

    world_position Result = MapIntoChunkSpace(World, BasePos, Offset + AdditionalOffset);

    Assert(IsCanonical(World, Result.Offset_)) return (Result);
}

internal add_low_entity_result
AddWall(game_state *GameState, s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ)
{
    world_position        P = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity =
    AddGroundedEntity(GameState, EntityType_Wall, P, GameState->WallCollision);

    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);

    return (Entity);
}

internal add_low_entity_result
AddStandardRoom(game_state *GameState, s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ)
{
    world_position        P      = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Space, P,
                                                     GameState->StandardRoomCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Traversable);

    return (Entity);
}

internal add_low_entity_result
AddStair(game_state *GameState, s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ)
{
    world_position        P      = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Stairwell,
                                                     P, GameState->StairCollision);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);
    Entity.Low->Sim.WalkableHeight = GameState->TypicalFloorHeight;
    Entity.Low->Sim.WalkableDim    = Entity.Low->Sim.Collision->TotalVolume.Dim.xy;

    return (Entity);
}

internal add_low_entity_result
AddMonster(game_state *GameState, s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ)
{
    world_position        P      = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Monster,
                                                     P, GameState->MonsterCollision);

    InitHitPoints(Entity.Low, 3);
    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Movable);

    return (Entity);
}

internal add_low_entity_result
AddSword(game_state *GameState)
{
    add_low_entity_result Entity = AddLowEntity(GameState, EntityType_Sword, NullPosition());
    Entity.Low->Sim.Collision    = GameState->SwordCollision;

    AddFlags(&Entity.Low->Sim, EntityFlag_NonSpatial | EntityFlag_Movable);

    return (Entity);
}

internal add_low_entity_result
AddFamiliar(game_state *GameState, s32 AbsTileX, s32 AbsTileY, s32 AbsTileZ)
{
    world_position        P      = ChunkPosFromTilePos(GameState->World, AbsTileX, AbsTileY, AbsTileZ);
    add_low_entity_result Entity = AddGroundedEntity(GameState, EntityType_Familiar,
                                                     P, GameState->FamiliarCollision);

    AddFlags(&Entity.Low->Sim, EntityFlag_Collides);

    return (Entity);
}

internal add_low_entity_result
AddPlayer(game_state *GameState)
{
    world_position        P = GameState->CameraP;
    add_low_entity_result Entity =
    AddGroundedEntity(GameState, EntityType_Hero, P, GameState->PlayerCollision);

    AddFlags(&Entity.Low->Sim, EntityFlag_Collides | EntityFlag_Movable);
    InitHitPoints(Entity.Low, 3);

    add_low_entity_result Sword = AddSword(GameState);
    Entity.Low->Sim.Sword.Index = Sword.LowIndex;

    if (GameState->CameraFollowingEntityIndex == 0)
    {
        GameState->CameraFollowingEntityIndex = Entity.LowIndex;
    }
    return (Entity);
}

internal void
DrawHitPoints(sim_entity *Entity, render_group *PieceGroup)
{
    if (Entity->HitPointMax >= 1)
    {
        v2  HealthDim  = {0.2f, 0.2f};
        r32 SpacingInX = 1.5f * HealthDim.x;

        v2 HitP  = {-0.5f * ((r32) (Entity->HitPointMax - 1)) * SpacingInX, -0.2f};
        v2 dHitP = {SpacingInX, 0.0f};
        for (u32 HealthIndex = 0; HealthIndex < Entity->HitPointMax; ++HealthIndex)
        {
            hit_point *HitPoint = Entity->HitPoint + HealthIndex;
            v4         Color    = {1.0f, 0.0f, 0.0f, 1.0f};

            if (HitPoint->FilledAmount == 0)
            {
                Color = V4(0.2f, 0.2f, 0.2f, 1.0f);
            }
            PushRect(PieceGroup, V3(HitP, 0.0f), HealthDim, Color);
            HitP += dHitP;
        }
    }
}

internal void
ClearCollisionRuleFor(game_state *GameState, u32 StorageIndex)
{

    for (u32 HashBucket = 0;
         HashBucket < ArrayCount(GameState->CollisionRuleHash);
         ++HashBucket)
    {

        for (pairwise_collision_rule **Rule = &GameState->CollisionRuleHash[HashBucket];
             *Rule;)
        {
            if (((*Rule)->StorageIndexA == StorageIndex) ||
                ((*Rule)->StorageIndexB == StorageIndex))
            {
                pairwise_collision_rule *RemovedRule = *Rule;
                *Rule                                = (*Rule)->NextInHash;

                RemovedRule->NextInHash           = GameState->FirstFreeCollisionRule;
                GameState->FirstFreeCollisionRule = RemovedRule;
            } else
            {
                Rule = &(*Rule)->NextInHash;
            }
        }
    }
}

internal void
AddCollisionRule(game_state *GameState, u32 StorageIndexA, u32 StorageIndexB, b32 ShouldCollide)
{
    if (StorageIndexA > StorageIndexB)
    {
        u32 Temp      = StorageIndexA;
        StorageIndexA = StorageIndexB;
        StorageIndexB = Temp;
    }

    pairwise_collision_rule *Found      = 0;
    u32                      HashBucket = StorageIndexA & (ArrayCount((GameState->CollisionRuleHash) - 1));
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
        Found->NextInHash                        = GameState->CollisionRuleHash[HashBucket];
        GameState->CollisionRuleHash[HashBucket] = Found;
    }

    if (Found)
    {
        Found->StorageIndexA = StorageIndexA;
        Found->StorageIndexB = StorageIndexB;
        Found->CanCollide    = ShouldCollide;
    }
}

internal sim_entity_collision_volume_group *
         MakeSimpleGroundedCollision(game_state *GameState, r32 DimX, r32 DimY, r32 DimZ)
{
    sim_entity_collision_volume_group *Group = PushStruct(&GameState->WorldArena,
                                                          sim_entity_collision_volume_group);
    Group->VolumeCount                       = 1;
    Group->Volumes                           = PushArray(&GameState->WorldArena,
                               Group->VolumeCount, sim_entity_collision_volume);
    Group->TotalVolume.Dim                   = V3(DimX, DimY, DimZ);
    Group->TotalVolume.OffsetP               = V3(0, 0, 0.5f * DimZ);
    Group->Volumes[0]                        = Group->TotalVolume;

    return (Group);
}

internal sim_entity_collision_volume_group *
         MakeNullCollision(game_state *GameState)
{
    sim_entity_collision_volume_group *Group = PushStruct(&GameState->WorldArena,
                                                          sim_entity_collision_volume_group);
    Group->VolumeCount                       = 0;
    Group->Volumes                           = 0;
    Group->TotalVolume.Dim                   = V3(0, 0, 0);
    Group->TotalVolume.OffsetP               = V3(0, 0, 0);

    return (Group);
}

internal void
FillGroundChunk(transient_state *TranState, game_state *GameState,
                ground_buffer *GroundBuffer, world_position *ChunkP)
{
    temporary_memory RenderMemory = BeginTempMemory(&TranState->TranArena);

    loaded_bitmap *Buffer      = &GroundBuffer->Bitmap;
    render_group * RenderGroup = AllocateRenderGroup(&TranState->TranArena, Megabytes(4), Buffer->Width, Buffer->Height);

    Buffer->AlignPercentage = V2(.5f, .5f);
    Buffer->WidthOverHeight = 1.f;

    Clear(RenderGroup, V4(1.f, 0.f, 1.f, 1.f));

    GroundBuffer->P = *ChunkP;

#if 0
        r32 Width   = GameState->World->ChunkDimInMeters.x;
        r32 Height  = GameState->World->ChunkDimInMeters.y;
        v2  HalfDim = .5f * V2(Width, Height);
        HalfDim = 2.f * HalfDim;
    
        for (s32 ChunkOffsetY = -1;
             ChunkOffsetY <= 1;
             ++ChunkOffsetY)
        {
            for (s32 ChunkOffsetX = -1;
                 ChunkOffsetX <= 1;
                 ++ChunkOffsetX)
            {
                s32 ChunkX = ChunkP->ChunkX + ChunkOffsetX;
                s32 ChunkY = ChunkP->ChunkY + ChunkOffsetY;
                s32 ChunkZ = ChunkP->ChunkZ;
    
                random_series Series = Seed(139 * ChunkX + 425 * ChunkY + 513 * ChunkZ);
    
                v2 Center = V2(ChunkOffsetX * Width, ChunkOffsetY * Height);
    
                for (u32 GrassIndex = 0; GrassIndex < 100; ++GrassIndex)
                {
                    loaded_bitmap *Stamp;
                    if (RandomChoice(&Series, 2))
                    {
                        Stamp = GameState->Grass + (RandomChoice(&Series, ArrayCount(GameState->Grass)));
                    } else
                    {
                        Stamp = GameState->Ground + (RandomChoice(&Series, ArrayCount(GameState->Ground)));
                    }
                    v2 P = Center + Hadamard(HalfDim, V2(RandomBilateral(&Series), RandomBilateral(&Series)));
                    PushBitmap(RenderGroup, Stamp, V3(P, 0.f), 4.f);
                }
            }
        }
    
        for (s32 ChunkOffsetY = -1;
             ChunkOffsetY <= 1;
             ++ChunkOffsetY)
        {
            for (s32 ChunkOffsetX = -1;
                 ChunkOffsetX <= 1;
                 ++ChunkOffsetX)
            {
                s32 ChunkX = ChunkP->ChunkX + ChunkOffsetX;
                s32 ChunkY = ChunkP->ChunkY + ChunkOffsetY;
                s32 ChunkZ = ChunkP->ChunkZ;
    
                random_series Series = Seed(139 * ChunkX + 425 * ChunkY + 513 * ChunkZ);
    
                v2 Center = V2(ChunkOffsetX * Width, ChunkOffsetY * Height);
    
                for (u32 GrassIndex = 0; GrassIndex < 20; ++GrassIndex)
                {
                    loaded_bitmap *Stamp;
    
                    Stamp = GameState->Tuft + (RandomChoice(&Series, ArrayCount(GameState->Tuft)));
    
                    v2 P = Center + Hadamard(HalfDim, V2(RandomBilateral(&Series), RandomBilateral(&Series)));
                    PushBitmap(RenderGroup, Stamp, V3(P, 0.f), .4f);
                }
            }
        }

#endif
    RenderGroupToOutput(RenderGroup, Buffer);
    EndTempMemory(RenderMemory);
}

internal void
ClearBitmap(loaded_bitmap *Bitmap)
{
    if (Bitmap->Memory)
    {
        s32 TotalBitmapSize = Bitmap->Width * Bitmap->Height * BYTES_PER_PIXEL;
        ZeroSize(TotalBitmapSize, Bitmap->Memory);
    }
}

internal loaded_bitmap
MakeEmptyBitmap(memory_arena *Arena, s32 Width, s32 Height, b32 ClearToZero = true)
{
    loaded_bitmap Result = {};
    Result.Width         = Width;
    Result.Height        = Height;
    Result.Pitch         = Result.Width * BYTES_PER_PIXEL;
    s32 TotalBitmapSize  = Width * Height * BYTES_PER_PIXEL;
    Result.Memory        = PushSize_(Arena, TotalBitmapSize);

    if (ClearToZero)
    {
        ClearBitmap(&Result);
    }

    return (Result);
}

internal void
MakeSphereNormalMap(loaded_bitmap *Bitmap, r32 Roughness)
{
    r32 InvWidth  = 1.f / (r32) (Bitmap->Width - 1);
    r32 InvHeight = 1.f / (r32) (Bitmap->Height - 1);

    u8 *Row = (u8 *) Bitmap->Memory;
    for (s32 Y = 0; Y < Bitmap->Height; ++Y)
    {
        u32 *Pixel = (u32 *) Row;
        for (s32 X = 0; X < Bitmap->Width; ++X)
        {
            v2 BitmapUV = V2(InvWidth * (r32) X, InvHeight * (r32) Y);

            r32 Nx = 2.f * BitmapUV.x - 1.f;
            r32 Ny = 2.f * BitmapUV.y - 1.f;
            r32 Nz = 0.f;

            r32 RootTerm = 1.f - Nx * Nx - Ny * Ny;
            v3  Normal   = V3(0, 0.7071067811865475f, 0.7071067811865475f);
            if (RootTerm >= 0.f)
            {
                Nz     = SquareRoot(RootTerm);
                Normal = V3(Nx, Ny, Nz);
            }

            v4 Color = V4(255.f * (.5f * (Normal.x + 1.f)),
                          255.f * (.5f * (Normal.y + 1.f)),
                          255.f * (.5f * (Normal.z + 1.f)),
                          Roughness * 255.f);

            *Pixel++ = (((u32) (Color.r + 0.5f) << RED_PLACE) |
                        ((u32) (Color.g + 0.5f) << GREEN_PLACE) |
                        ((u32) (Color.b + 0.5f) << BLUE_PLACE)) |
                       ((u32) (Color.a + 0.5f) << 24);
        }
        Row += Bitmap->Pitch;
    }
}

internal void
MakeSphereDiffuseMap(loaded_bitmap *Bitmap)
{
    r32 InvWidth  = 1.f / (r32) (Bitmap->Width - 1);
    r32 InvHeight = 1.f / (r32) (Bitmap->Height - 1);

    u8 *Row = (u8 *) Bitmap->Memory;
    for (s32 Y = 0; Y < Bitmap->Height; ++Y)
    {
        u32 *Pixel = (u32 *) Row;
        for (s32 X = 0; X < Bitmap->Width; ++X)
        {
            v2 BitmapUV = V2(InvWidth * (r32) X, InvHeight * (r32) Y);

            r32 Nx = 2.f * BitmapUV.x - 1.f;
            r32 Ny = 2.f * BitmapUV.y - 1.f;
            r32 Nz = 0.f;

            r32 Alpha    = 0.f;
            r32 RootTerm = 1.f - Nx * Nx - Ny * Ny;
            if (RootTerm >= 0.f)
            {
                Alpha = 1.f;
            }

            v3 BaseColor = V3(0.f, 0.f, 0.f);

            Alpha *= 255.f;
            v4 Color = V4(Alpha * BaseColor.r,
                          Alpha * BaseColor.g,
                          Alpha * BaseColor.b,
                          Alpha);

            *Pixel++ = (((u32) (Color.r + 0.5f) << RED_PLACE) |
                        ((u32) (Color.g + 0.5f) << GREEN_PLACE) |
                        ((u32) (Color.b + 0.5f) << BLUE_PLACE)) |
                       ((u32) (Color.a + 0.5f) << 24);
        }
        Row += Bitmap->Pitch;
    }
}

#if 0

internal void
RequestGroundBuffers(world_position CenterP, rectangle3 Bounds)
{
    Bounds = Offset(Bounds, CenterP.Offset_);
    CenterP.Offset_ = V3(0,0,0);

    DrawGroundChunk(TranState, GameState, TranState->GroundBuffers, &GameState->CameraP);
}

#endif

#if HANDMADE_INTERNAL
game_memory *DebugGlobalMemory;
#endif

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    BEGIN_TIMED_BLOCK(GameUpdateAndRender);
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)

#if HANDMADE_INTERNAL
    DebugGlobalMemory = Memory;
#endif
    u32 GroundBufferWidth  = 256;
    u32 GroundBufferHeight = 256;

    //TODO(rahul): Remove this
    r32 PixelsToMeters = 1.f / 42.f;

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        u32 TilesPerHeight = 9;
        u32 TilesPerWidth  = 17;

        InitializeArena(&GameState->WorldArena,
                        (mem_index) Memory->PermanentStorageSize - sizeof(game_state),
                        (u8 *) Memory->PermanentStorage + sizeof(game_state));

        GameState->World = PushStruct(&GameState->WorldArena, world);

        GameState->TypicalFloorHeight = 3.0f;

        v3 WorldChunkDimInMeters = {PixelsToMeters * (r32) GroundBufferWidth,
                                    PixelsToMeters * (r32) GroundBufferHeight,
                                    GameState->TypicalFloorHeight};

        world *World = GameState->World;

        InitializeWorld(World, WorldChunkDimInMeters);

        AddLowEntity(GameState, EntityType_Null, NullPosition());

        r32 TileSideInMeters  = 1.4f;
        r32 TileDepthInMeters = GameState->TypicalFloorHeight;

        GameState->NullCollision     = MakeNullCollision(GameState);
        GameState->SwordCollision    = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.1f);
        GameState->StairCollision    = MakeSimpleGroundedCollision(GameState, TileSideInMeters,
                                                                2.0f * TileSideInMeters,
                                                                1.1f * TileDepthInMeters);
        GameState->PlayerCollision   = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 1.2f);
        GameState->FamiliarCollision = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);
        GameState->MonsterCollision  = MakeSimpleGroundedCollision(GameState, 1.0f, 0.5f, 0.5f);
        GameState->WallCollision     = MakeSimpleGroundedCollision(GameState, TileSideInMeters,
                                                               TileSideInMeters,
                                                               TileDepthInMeters);

        GameState->StandardRoomCollision =
        MakeSimpleGroundedCollision(GameState,
                                    TilesPerWidth * TileSideInMeters,
                                    TilesPerHeight * TileSideInMeters,
                                    0.9f * TileDepthInMeters);

        GameState->Grass[0] = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "test2/grass00.bmp");

        GameState->Grass[1] = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "test2/grass01.bmp");

        GameState->Ground[0] = DEBUGLoadBMP(Thread,
                                            Memory->DEBUGPlatformReadEntireFile,
                                            "test2/ground00.bmp");
        GameState->Ground[1] = DEBUGLoadBMP(Thread,
                                            Memory->DEBUGPlatformReadEntireFile,
                                            "test2/ground01.bmp");
        GameState->Ground[2] = DEBUGLoadBMP(Thread,
                                            Memory->DEBUGPlatformReadEntireFile,
                                            "test2/ground02.bmp");
        GameState->Ground[3] = DEBUGLoadBMP(Thread,
                                            Memory->DEBUGPlatformReadEntireFile,
                                            "test2/ground03.bmp");

        GameState->Tuft[0] = DEBUGLoadBMP(Thread,
                                          Memory->DEBUGPlatformReadEntireFile,
                                          "test2/tuft00.bmp");
        GameState->Tuft[1] = DEBUGLoadBMP(Thread,
                                          Memory->DEBUGPlatformReadEntireFile,
                                          "test2/tuft01.bmp");
        GameState->Tuft[2] = DEBUGLoadBMP(Thread,
                                          Memory->DEBUGPlatformReadEntireFile,
                                          "test2/tuft02.bmp");

        GameState->Backdrop = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "test/test_background.bmp");

        GameState->HeroShadow = DEBUGLoadBMP(Thread,
                                             Memory->DEBUGPlatformReadEntireFile,
                                             "test/test_hero_shadow.bmp", 72, 182);

        GameState->Tree = DEBUGLoadBMP(Thread,
                                       Memory->DEBUGPlatformReadEntireFile,
                                       "test2/tree00.bmp", 40, 80);

        GameState->Sword = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "new/sword.bmp", 29, 10);

        GameState->Familiar = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "new/familiar.bmp");

        GameState->Stair       = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "new/stair.bmp", 73, 72);
        GameState->Monster     = DEBUGLoadBMP(Thread,
                                          Memory->DEBUGPlatformReadEntireFile,
                                          "new/monster.bmp");
        GameState->MonsterDead = DEBUGLoadBMP(Thread,
                                              Memory->DEBUGPlatformReadEntireFile,
                                              "new/monster_dead.bmp");

        GameState->Stairwell = DEBUGLoadBMP(Thread,
                                            Memory->DEBUGPlatformReadEntireFile,
                                            "test2/rock02.bmp");

        hero_bitmaps *Bitmap;
        Bitmap = GameState->HeroBitmaps;

        Bitmap->HeroHead  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_back_head.bmp", 72, 182);
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_back_torso.bmp", 72, 182);
        Bitmap->HeroCape  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_back_cape.bmp", 72, 182);
        ++Bitmap;

        Bitmap->HeroHead  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_front_head.bmp", 72, 182);
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_front_torso.bmp", 72, 182);
        Bitmap->HeroCape  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_front_cape.bmp", 72, 182);
        ++Bitmap;

        Bitmap->HeroHead  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_right_head.bmp", 72, 182);
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_right_torso.bmp", 72, 182);
        Bitmap->HeroCape  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_right_cape.bmp", 72, 182);
        ++Bitmap;

        Bitmap->HeroHead  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_left_head.bmp", 72, 182);
        Bitmap->HeroTorso = DEBUGLoadBMP(Thread,
                                         Memory->DEBUGPlatformReadEntireFile,
                                         "test/test_hero_left_torso.bmp", 72, 182);
        Bitmap->HeroCape  = DEBUGLoadBMP(Thread,
                                        Memory->DEBUGPlatformReadEntireFile,
                                        "test/test_hero_left_cape.bmp", 72, 182);
        ++Bitmap;

        u32 ScreenBaseX = 0;
        u32 ScreenBaseY = 0;
        u32 ScreenBaseZ = 0;

        u32 ScreenX = ScreenBaseX;
        u32 ScreenY = ScreenBaseY;

        random_series Series   = {1234};
        u32           AbsTileZ = ScreenBaseZ;

        b32 DoorLeft   = false;
        b32 DoorRight  = false;
        b32 DoorTop    = false;
        b32 DoorBottom = false;

        b32 DoorUp   = false;
        b32 DoorDown = false;

        for (u32 ScreenIndex = 0; ScreenIndex < 2000; ++ScreenIndex)
        {
#if 1
            u32 DoorDirection = RandomChoice(&Series, (DoorDown || DoorUp) ? 2 : 4);
#else
            u32 DoorDirection = RandomChoice(&Series, 2);
#endif
            // DoorDirection = 3;

            b32 ZDoorCreated = false;
            if (DoorDirection == 3)
            {
                ZDoorCreated = true;
                DoorDown     = true;
            } else if (DoorDirection == 2)
            {
                ZDoorCreated = true;
                DoorUp       = true;

            } else if (DoorDirection == 1)
            {
                DoorRight = true;
            } else
            {
                DoorTop = true;
            }

            AddStandardRoom(GameState,
                            (ScreenX * TilesPerWidth + TilesPerWidth / 2),
                            (ScreenY * TilesPerHeight + TilesPerHeight / 2),
                            (AbsTileZ));

            for (u32 TileY = 0; TileY < TilesPerHeight; ++TileY)
            {
                for (u32 TileX = 0; TileX < TilesPerWidth; ++TileX)
                {
                    u32 AbsTileX = ScreenX * TilesPerWidth + TileX;
                    u32 AbsTileY = ScreenY * TilesPerHeight + TileY;

                    b32 ShouldBeDoor = false;

                    if ((TileX == 0) && (!DoorLeft || (TileY != (TilesPerHeight / 2))))
                    {
                        ShouldBeDoor = true;
                    }
                    if ((TileX == (TilesPerWidth - 1)) && (!DoorRight || (TileY != (TilesPerHeight / 2))))
                    {
                        ShouldBeDoor = true;
                    }
                    if ((TileY == 0) && (!DoorBottom || (TileX != (TilesPerWidth / 2))))
                    {
                        ShouldBeDoor = true;
                    }
                    if ((TileY == (TilesPerHeight - 1)) && (!DoorTop || (TileX != (TilesPerWidth / 2))))
                    {
                        ShouldBeDoor = true;
                    }

                    if (ShouldBeDoor)
                    {
                        AddWall(GameState, AbsTileX, AbsTileY, AbsTileZ);

                    } else if (ZDoorCreated)
                    {
                        if (((AbsTileZ % 2) && (TileX == 10) && (TileY == 5)) ||
                            (!(AbsTileZ % 2) && (TileX == 4) && (TileY == 5)))
                        {
                            AddStair(GameState, AbsTileX, AbsTileY, DoorDown ? AbsTileZ - 1 : AbsTileZ);
                        }
                    }
                }
            }

            DoorLeft   = DoorRight;
            DoorBottom = DoorTop;

            if (ZDoorCreated)
            {
                DoorUp   = !DoorUp;
                DoorDown = !DoorDown;
            } else
            {
                DoorUp   = false;
                DoorDown = false;
            }

            DoorRight = false;
            DoorTop   = false;

            if (DoorDirection == 3)
            {
                AbsTileZ -= 1;
            } else if (DoorDirection == 2)
            {
                AbsTileZ += 1;

            } else if (DoorDirection == 1)
            {
                ScreenX += 1;
            } else
            {
                ScreenY += 1;
            }
        }

        world_position NewCameraP = {};

        u32 CameraTileX = ScreenBaseX * TilesPerWidth + 17 / 2;
        u32 CameraTileY = ScreenBaseY * TilesPerHeight + 9 / 2;
        u32 CameraTileZ = ScreenBaseZ;

        NewCameraP = ChunkPosFromTilePos(GameState->World,
                                         CameraTileX,
                                         CameraTileY,
                                         CameraTileZ);

        GameState->CameraP = NewCameraP;

        AddMonster(GameState, CameraTileX - 3, CameraTileY + 2, CameraTileZ);
        AddFamiliar(GameState, CameraTileX - 2, CameraTileY + 2, CameraTileZ);

        Memory->IsInitialized = true;
    }

    // NOTE(rahul): Transient Initialization

    Assert(sizeof(transient_state) <= Memory->TransientStorageSize);
    transient_state *TranState = (transient_state *) Memory->TransientStorage;
    if (!TranState->IsInitialized)
    {
        InitializeArena(&TranState->TranArena,
                        (mem_index) Memory->TransientStorageSize - sizeof(transient_state),
                        (u8 *) Memory->TransientStorage + sizeof(transient_state));

        TranState->GroundBufferCount = 64;// 128;
        TranState->GroundBuffers     = PushArray(&TranState->TranArena,
                                             TranState->GroundBufferCount, ground_buffer);

        for (u32 GroundBufferIndex = 0;
             GroundBufferIndex < TranState->GroundBufferCount;
             ++GroundBufferIndex)
        {
            ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;

            GroundBuffer->Bitmap = MakeEmptyBitmap(&TranState->TranArena,
                                                   GroundBufferWidth,
                                                   GroundBufferHeight,
                                                   false);
            GroundBuffer->P      = NullPosition();
        }

        GameState->TestDiffuse = MakeEmptyBitmap(&TranState->TranArena, 256, 256);
        DrawRectangle(&GameState->TestDiffuse, V2(0, 0), V2i(GameState->TestDiffuse.Width, GameState->TestDiffuse.Height),
                      V4(.5f, .5f, .5f, 1.f));
        GameState->TestNormal = MakeEmptyBitmap(&TranState->TranArena, GameState->TestDiffuse.Width,
                                                GameState->TestDiffuse.Height);
        MakeSphereNormalMap(&GameState->TestNormal, 0.f);
        MakeSphereDiffuseMap(&GameState->TestDiffuse);

        TranState->EnvMapWidth  = 512;
        TranState->EnvMapHeight = 256;

        for (u32 MapIndex = 0; MapIndex < ArrayCount(TranState->EnvMaps); ++MapIndex)
        {
            u32 Width  = TranState->EnvMapWidth;
            u32 Height = TranState->EnvMapHeight;

            environment_map *Map = TranState->EnvMaps + MapIndex;

            for (u32 LODIndex = 0; LODIndex < ArrayCount(Map->LOD); ++LODIndex)
            {
                Map->LOD[LODIndex] = MakeEmptyBitmap(&TranState->TranArena, Width, Height, false);

                Width >>= 1;
                Height >>= 1;
            }
        }

        TranState->IsInitialized = true;
    }

#if 0
    if (Input->ExecutableReloaded)
    {
        for (u32 GroundBufferIndex = 0;
             GroundBufferIndex < TranState->GroundBufferCount;
             ++GroundBufferIndex)
        {
            ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;

            GroundBuffer->P = NullPosition();
        }
    }
#endif

    world *World = GameState->World;

    for (int ControllerIndex = 0;
         ControllerIndex < ArrayCount(Input->Controllers);
         ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        controlled_hero *      ConHero    = &GameState->ControlledHeroes[ControllerIndex];
        if (ConHero->EntityIndex == 0)
        {
            if (Controller->Start.EndedDown)
            {
                *ConHero             = {};
                ConHero->EntityIndex = AddPlayer(GameState).LowIndex;
            }
        } else
        {
            ConHero->dZ     = 0;
            ConHero->accel  = {};
            ConHero->dSword = {};

            if (Controller->IsAnalog)
            {
                ConHero->accel = V2(Controller->StickAverageX, Controller->StickAverageY);
            } else
            {
                if (Controller->MoveUp.EndedDown)
                {
                    ConHero->accel.y = 1.0f;
                }
                if (Controller->MoveDown.EndedDown)
                {
                    ConHero->accel.y = -1.0f;
                }
                if (Controller->MoveLeft.EndedDown)
                {
                    ConHero->accel.x = -1.0f;
                }
                if (Controller->MoveRight.EndedDown)
                {
                    ConHero->accel.x = 1.0f;
                }
            }

            if (Controller->Start.EndedDown)
            {
                ConHero->dZ = 4.0f;
            }
            ConHero->dSword = {};

            if (Controller->ActionUp.EndedDown)
            {
                ConHero->dSword = {0.0f, 1.0f};
            }
            if (Controller->ActionDown.EndedDown)
            {
                ConHero->accel *= 50.0f;
                ConHero->dSword = {0.0f, -1.0f};
            }
            if (Controller->ActionLeft.EndedDown)
            {
                ConHero->dSword = {-1.0f, 0.0f};
            }
            if (Controller->ActionRight.EndedDown)
            {
                ConHero->dSword = {1.0f, 0.0f};
            }
        }
    }

    temporary_memory RenderMemory = BeginTempMemory(&TranState->TranArena);

    loaded_bitmap  DrawBuffer_ = {};
    loaded_bitmap *DrawBuffer  = &DrawBuffer_;
    DrawBuffer->Width          = Buffer->Width;
    DrawBuffer->Height         = Buffer->Height;
    DrawBuffer->Pitch          = Buffer->Pitch;
    DrawBuffer->Memory         = Buffer->Memory;

    render_group *RenderGroup = AllocateRenderGroup(&TranState->TranArena, Megabytes(4), DrawBuffer->Width, DrawBuffer->Height);

    Clear(RenderGroup, V4(.25f, 0.25f, .25f, 0.f));

    rectangle2 ScreenBounds = GetCameraRectangleAtTarget(RenderGroup);

    rectangle3 CameraBoundsInMeters = RectMinMax(V3(ScreenBounds.Min, 0.f),
                                                 V3(ScreenBounds.Max, 0.f));
    CameraBoundsInMeters.Min.z      = -3.f * GameState->TypicalFloorHeight;
    CameraBoundsInMeters.Max.z      = 1.f * GameState->TypicalFloorHeight;

    for (u32 GroundBufferIndex = 0;
         GroundBufferIndex < TranState->GroundBufferCount;
         ++GroundBufferIndex)
    {
        ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;
        if (IsValid(GroundBuffer->P))
        {
            loaded_bitmap *Bitmap = &GroundBuffer->Bitmap;
            v3             Delta  = Subtract(GameState->World, &GroundBuffer->P, &GameState->CameraP);

            if (Delta.z >= -1.f && Delta.z <= 1.f)
            {
                render_basis *Basis       = PushStruct(&TranState->TranArena, render_basis);
                RenderGroup->DefaultBasis = Basis;

                Basis->P               = Delta;
                r32 GroundSideInMeters = World->ChunkDimInMeters.x;
                PushBitmap(RenderGroup, Bitmap, V3(0, 0, 0), GroundSideInMeters);
#if 1
                PushRectOutline(RenderGroup, V3(0, 0, 0), V2(GroundSideInMeters, GroundSideInMeters),
                                V4(1.0f, 1.0f, 0.0f, 1.0f));
#endif
            }
        }
    }

    {
        world_position MinChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMinCorner(CameraBoundsInMeters));
        world_position MaxChunkP = MapIntoChunkSpace(World, GameState->CameraP, GetMaxCorner(CameraBoundsInMeters));
        for (s32 ChunkZ = MinChunkP.ChunkZ;
             ChunkZ <= MaxChunkP.ChunkZ;
             ++ChunkZ)
        {
            for (s32 ChunkY = MinChunkP.ChunkY;
                 ChunkY <= MaxChunkP.ChunkY;
                 ++ChunkY)
            {
                for (s32 ChunkX = MinChunkP.ChunkX;
                     ChunkX <= MaxChunkP.ChunkX;
                     ++ChunkX)
                {
                    //                    world_chunk *Chunk = GetWorldChunk(World, ChunkX, ChunkY, ChunkZ);
                    //                    if (Chunk)
                    {
                        world_position ChunkCenterP           = CenteredChunkPoint(ChunkX, ChunkY, ChunkZ);
                        r32            FurthestBufferLengthSq = 0.0f;
                        ground_buffer *FurthestBuffer         = 0;

                        for (u32 GroundBufferIndex = 0;
                             GroundBufferIndex < TranState->GroundBufferCount;
                             ++GroundBufferIndex)
                        {
                            ground_buffer *GroundBuffer = TranState->GroundBuffers + GroundBufferIndex;

                            if (AreOnSameChunk(World, &GroundBuffer->P, &ChunkCenterP))
                            {
                                FurthestBuffer = 0;
                                break;
                            } else if (IsValid(GroundBuffer->P))
                            {
                                v3  BufferRelP     = Subtract(World, &GroundBuffer->P, &GameState->CameraP);
                                r32 BufferLengthSq = LengthSq(BufferRelP.xy);
                                if (FurthestBufferLengthSq < BufferLengthSq)
                                {
                                    FurthestBufferLengthSq = BufferLengthSq;
                                    FurthestBuffer         = GroundBuffer;
                                }
                            } else
                            {
                                FurthestBufferLengthSq = Real32Maximum;
                                FurthestBuffer         = GroundBuffer;
                            }
                        }
                        if (FurthestBuffer)
                        {
                            FillGroundChunk(TranState, GameState, FurthestBuffer, &ChunkCenterP);
                        }

                        v3 RelP = Subtract(World, &ChunkCenterP, &GameState->CameraP);
                    }
                }
            }
        }
    }

    v3         SimBoundsExpansion = {15.0f, 15.0f, 0.0f};
    rectangle3 SimBounds          = AddRadiusTo(CameraBoundsInMeters, SimBoundsExpansion);

    temporary_memory SimMemory  = BeginTempMemory(&TranState->TranArena);
    sim_region *     SimRegion  = BeginSim(GameState, &TranState->TranArena,
                                     GameState->World, GameState->CameraP, SimBounds, Input->deltatForFrame);
    world_position   SimCenterP = GameState->CameraP;

    v3 CameraP = Subtract(World, &GameState->CameraP, &SimCenterP);

    render_basis *Basis       = PushStruct(&TranState->TranArena, render_basis);
    Basis->P                  = V3(0, 0, 0);
    RenderGroup->DefaultBasis = Basis;

    PushRectOutline(RenderGroup, V3(0, 0, 0), GetDim(ScreenBounds), V4(1, 1, 0, 1));
    PushRectOutline(RenderGroup, V3(0, 0, 0), GetDim(CameraBoundsInMeters).xy, V4(1, 0, 1, 1));
    PushRectOutline(RenderGroup, V3(0, 0, 0), GetDim(SimBounds).xy, V4(0, 1, 1, 1));
    PushRectOutline(RenderGroup, V3(0, 0, 0), GetDim(SimRegion->UpdatableBounds).xy, V4(1, 1, 1, 1));

    for (u32 EntityIndex = 0;
         EntityIndex < SimRegion->EntityCount;
         ++EntityIndex)
    {
        sim_entity *Entity = SimRegion->Entities + EntityIndex;

        if (Entity->Updatable)
        {

            r32 deltat      = Input->deltatForFrame;
            r32 ShadowAlpha = 1.0f - 0.5f * Entity->P.z;
            if (ShadowAlpha < 0)
            {
                ShadowAlpha = 0.0f;
            }

            move_spec MoveSpec      = DefaultMoveSpec();
            v3        accelOfEntity = {};

            render_basis *PBasis = PushStruct(&TranState->TranArena, render_basis);
            RenderGroup->DefaultBasis = PBasis;

            v3 CameraRelativeGroundP = GetEntityGroundPoint(Entity) - CameraP;

            r32 FadeTopEndZ      = .75f * GameState->TypicalFloorHeight;
            r32 FadeTopStartZ    = .5f * GameState->TypicalFloorHeight;
            r32 FadeBottomStartZ = -2.f * GameState->TypicalFloorHeight;
            r32 FadeBottomEndZ   = -2.25f * GameState->TypicalFloorHeight;

            RenderGroup->GlobalAlpha = 1.f;

            if (CameraRelativeGroundP.z > FadeTopStartZ)
            {
                RenderGroup->GlobalAlpha = Clamp01MapToRange(FadeTopEndZ, CameraRelativeGroundP.z, FadeTopStartZ);
            } else if (CameraRelativeGroundP.z < FadeBottomStartZ)
            {
                RenderGroup->GlobalAlpha = Clamp01MapToRange(FadeBottomEndZ, CameraRelativeGroundP.z, FadeBottomStartZ);
            }

            hero_bitmaps *HeroBitmaps = &GameState->HeroBitmaps[Entity->FacingDirection];
            switch (Entity->Type)
            {
                case EntityType_Hero: {
                    for (u32 ControlIndex = 0;
                         ControlIndex < ArrayCount(GameState->ControlledHeroes);
                         ++ControlIndex)
                    {
                        controlled_hero *ConHero = GameState->ControlledHeroes + ControlIndex;

                        if (Entity->StorageIndex == ConHero->EntityIndex)
                        {
                            if (ConHero->dZ != 0.0f)
                            {
                                Entity->deltaP.z = ConHero->dZ;
                            }

                            MoveSpec.UnitMaxAccelVector = true;
                            MoveSpec.Speed              = 70.0f;
                            MoveSpec.Drag               = 7.0f;
                            accelOfEntity               = V3(ConHero->accel, 0);

                            if ((ConHero->dSword.x != 0.0f) || (ConHero->dSword.y != 0.0f))
                            {
                                sim_entity *Sword = Entity->Sword.Ptr;
                                if (Sword && IsSet(Sword, EntityFlag_NonSpatial))
                                {
                                    Sword->DistanceLimit = 5.0f;
                                    MakeEntitySpatial(Sword,
                                                      Entity->P,
                                                      Entity->deltaP + 5.0f * V3(ConHero->dSword, 0));
                                    AddCollisionRule(GameState,
                                                     Sword->StorageIndex,
                                                     Entity->StorageIndex,
                                                     false);
                                }
                            }
                        }
                    }
                    r32 HeroSizeC = 2.f;
                    PushBitmap(RenderGroup, &HeroBitmaps->HeroTorso, V3(0, 0, 0), HeroSizeC * 1.2f);
                    PushBitmap(RenderGroup, &HeroBitmaps->HeroCape, V3(0, 0, 0), HeroSizeC * 1.2f);
                    PushBitmap(RenderGroup, &HeroBitmaps->HeroHead, V3(0, 0, 0), HeroSizeC * 1.2f);
                    PushBitmap(RenderGroup, &GameState->HeroShadow, V3(0, 0, 0), HeroSizeC * 1.f, V4(1, 1, 1, ShadowAlpha));

                    DrawHitPoints(Entity, RenderGroup);
                    break;
                }
                case EntityType_Wall: {
                    PushBitmap(RenderGroup, &GameState->Tree, V3(0, 0, 0), 2.5f);
                    break;
                }

                case EntityType_Stairwell: {
                    PushBitmap(RenderGroup, &GameState->Stair, V3(0, 0, 0), 4.f);
                    // PushRect(RenderGroup, V3(0, 0, 0), Entity->WalkableDim, V4(1, 0.5f, 0, 1));
                    // PushRect(RenderGroup, V3(0, 0, Entity->WalkableHeight), Entity->WalkableDim, V4(1, 1, 0, 1));
                    break;
                }

                case EntityType_Space: {
#if 0
                    for (u32 VolumeIndex = 0;
                         VolumeIndex < Entity->Collision->VolumeCount;
                         ++VolumeIndex)
                    {
                        sim_entity_collision_volume *Volume = Entity->Collision->Volumes + VolumeIndex;
                        PushRectOutline(RenderGroup, Volume->OffsetP - V3(0, 0, .5f * Volume->Dim.z),
                                        Volume->Dim.xy, V4(0, 0.5f, 1.0f, 1));
                    }
#endif
                    break;
                }

                case EntityType_Sword: {
                    MoveSpec.Speed = 0.0f;
                    MoveSpec.Drag  = 0.0f;

                    if (Entity->DistanceLimit == 0.0f)
                    {
                        ClearCollisionRuleFor(GameState, Entity->StorageIndex);
                        MakeEntityNonSpatial(Entity);
                    }
                    PushBitmap(RenderGroup, &GameState->Sword, V3(0, 0, 0), 1.f);
                    PushBitmap(RenderGroup, &GameState->HeroShadow, V3(0, 0, 0), 1.f, V4(1, 1, 1, ShadowAlpha));
                    break;
                }

                case EntityType_Familiar: {
                    sim_entity *ClosestHero    = 0;
                    r32         ClosestHeroDSq = Square(10.0f);

                    sim_entity *TestEntity = SimRegion->Entities;

                    for (u32 HighEntityIndex = 1;
                         HighEntityIndex < SimRegion->EntityCount;
                         ++HighEntityIndex, ++TestEntity)
                    {
                        if (TestEntity->Type == EntityType_Hero)
                        {
                            r32 TestDSq = LengthSq(TestEntity->P - Entity->P);
                            if (TestDSq < ClosestHeroDSq)
                            {
                                ClosestHero    = TestEntity;
                                ClosestHeroDSq = TestDSq;
                            }
                        }
                    }
                    if (ClosestHero && ClosestHeroDSq > Square(3.0f))
                    {
                        r32 accel         = 0.5f;
                        r32 OneOverLength = accel / SquareRoot(ClosestHeroDSq);
                        accelOfEntity     = OneOverLength * (ClosestHero->P - Entity->P);
                    }
                    MoveSpec.UnitMaxAccelVector = true;
                    MoveSpec.Speed              = 70.0f;
                    MoveSpec.Drag               = 7.0f;

                    Entity->tBob += deltat;

                    if (Entity->tBob > (2 * Pi32))
                    {
                        Entity->tBob -= (2 * Pi32);
                    }

                    r32 SinBob = Sin(4.0f * Entity->tBob);
                    PushBitmap(RenderGroup, &GameState->Familiar, V3(0, 0, 0.25f * SinBob), 2.f);

                    PushBitmap(RenderGroup, &GameState->HeroShadow, V3(0, 0, 0), 2.f,
                               V4(1, 1, 1, (0.5f * ShadowAlpha) + 0.2f * SinBob));
                    break;
                }

                case EntityType_Monster: {

                    if (Entity->HitPointMax < 1)
                    {
                        PushBitmap(RenderGroup, &GameState->MonsterDead, V3(0, 0, 0), 5.f);
                    } else
                    {
                        PushBitmap(RenderGroup, &GameState->Monster, V3(0, 0, 0), 5.f);
                    }

                    PushBitmap(RenderGroup, &GameState->HeroShadow, V3(0, 0, 0), 1.f, V4(1, 1, 1, ShadowAlpha));

                    DrawHitPoints(Entity, RenderGroup);
                    break;
                }

                default: {
                    InvalidCodePath break;
                }
            }

            if (!IsSet(Entity, EntityFlag_NonSpatial) &&
                IsSet(Entity, EntityFlag_Movable))
            {
                MoveEntity(GameState, SimRegion, Entity, accelOfEntity, &MoveSpec, Input->deltatForFrame);
            }

            PBasis->P = GetEntityGroundPoint(Entity);
        }
    }
    RenderGroup->GlobalAlpha = 1.f;

    v3 MapColor[] =
    {
    {1, 0, 0},
    {0, 1, 0},
    {0, 0, 1},
    };

    for (u32 MapIndex = 0; MapIndex < ArrayCount(TranState->EnvMaps); ++MapIndex)
    {
        environment_map *Map = TranState->EnvMaps + MapIndex;
        loaded_bitmap *  LOD = Map->LOD + 0;

        s32 CheckerdWidth  = 16;
        s32 CheckerdHeight = 16;

        b32 Checkerd = false;
        for (s32 Y = 0; Y < LOD->Height; Y += CheckerdHeight)
        {
            Checkerd = !Checkerd;
            for (s32 X = 0; X < LOD->Width; X += CheckerdWidth)
            {
                Checkerd = !Checkerd;

                v4 Color = Checkerd ? V4(MapColor[MapIndex], 1.f) : V4(0, 0, 0, 1);

                v2 MinP = V2i(X, Y);
                v2 MaxP = MinP + V2i(CheckerdWidth, CheckerdHeight);

                DrawRectangle(LOD, MinP, MaxP, Color);
            }
        }
    }
    TranState->EnvMaps[0].Pz = -8.f;
    TranState->EnvMaps[1].Pz = 0.f;
    TranState->EnvMaps[2].Pz = 8.f;

    DrawBitmap(&TranState->EnvMaps[0].LOD[0], &TranState->GroundBuffers[TranState->GroundBufferCount - 1].Bitmap, 130.f, 100.f);

#if 0
    GameState->Time += Input->deltatForFrame;

    r32 Angle = .1f * GameState->Time;
    v2  Disp  = {
    100.f * Cos(5.f * Angle),
    100.f * Sin(3.f * Angle),
    };
    v2 Origin = ScreenCenter;
    // Angle      = 0.f;
    v2 XAxis = 100.f * V2(Cos(10.f * Angle), Sin(10.f * Angle));

    // v2  YAxis  = (50.f + 50.f * Cos(Angle)) * V2(Cos(Angle + 1.f), Sin(Angle + 1.f));
    v2 YAxis = Perp(XAxis);
#if 0
        r32 CAngle = Angle*5.f;
        v4 Color = V4(.5f + 0.5f * Cos(CAngle + 2.7f),
                      .5f + 0.5f * Cos(CAngle + 2.5f),
                      .5f + 0.5f * Sin(CAngle + 9.2f),  
                      .5f + 0.5f * Sin(CAngle));
#else
    v4 Color = V4(1, 1, 1, 1);
#endif
    render_entry_coordinate_system *C = GetCoordinateSystem(RenderGroup, Disp + Origin - .5f * XAxis - .5f * YAxis,
                                                            XAxis, YAxis, Color, &GameState->TestDiffuse, &GameState->TestNormal,
                                                            TranState->EnvMaps + 2,
                                                            TranState->EnvMaps + 1,
                                                            TranState->EnvMaps + 0);

    v2 MapP = {0.f, 0.f};
    for (u32 MapIndex = 0; MapIndex < ArrayCount(TranState->EnvMaps); ++MapIndex)
    {
        environment_map *Map = TranState->EnvMaps + MapIndex;
        loaded_bitmap *  LOD = &Map->LOD[0];

        XAxis = .5f * V2((r32) LOD->Width, 0);
        YAxis = .5f * V2(0, (r32) LOD->Height);

        render_entry_coordinate_system *C = GetCoordinateSystem(RenderGroup, MapP, 
            XAxis, YAxis, V4(1, 1, 1, 1), LOD, 0, 0, 0, 0);

        MapP += YAxis + V2(0.f, 6.f);
    }
    int PIndex = 0;

#if 0
        for (r32 Y = 0.f; Y < 1.f; Y += 0.25f)
        {
            for (r32 X = 0.f; X < 1.f; X += 0.25f)
            {
                C->Points[PIndex++] = {X, Y};
            }
        }
#endif
#endif
    RenderGroupToOutput(RenderGroup, DrawBuffer);

    EndSim(GameState, SimRegion);
    EndTempMemory(SimMemory);
    EndTempMemory(RenderMemory);

    CheckArena(&GameState->WorldArena);
    CheckArena(&TranState->TranArena);

    END_TIMED_BLOCK(GameUpdateAndRender);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(SoundBuffer, GameState);
}