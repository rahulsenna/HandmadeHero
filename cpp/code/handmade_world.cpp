#pragma ide diagnostic ignored "modernize-loop-convert"
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by AgentOfChaos on 12/7/2020.
//


#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX/64)

#define TILE_CHUNK_UNINITIALIZED INT32_MAX

inline world_chunk *
GetTileChunk(world *TileMap, int32 TileChunkX, int32 TileChunkY, int32 TileChunkZ,
             memory_arena *Arena = 0)
{
    Assert(TileChunkX > -TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkY > -TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkZ > -TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkX < TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkY < TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkZ < TILE_CHUNK_SAFE_MARGIN)

    int32 HashValue = 19 * TileChunkX + 7 * TileChunkY + 3 * TileChunkZ;
    int32 HashSlot = HashValue & (ArrayCount(TileMap->ChunkHash) - 1);
    Assert(HashSlot < ArrayCount(TileMap->ChunkHash))

    world_chunk *Chunk = TileMap->ChunkHash + HashSlot;

    do
    {
        if ((TileChunkX == Chunk->ChunkX) &&
            (TileChunkY == Chunk->ChunkY) &&
            (TileChunkZ == Chunk->ChunkZ))
        {
            break;
        }

        if (Arena && (Chunk->ChunkX != TILE_CHUNK_UNINITIALIZED) && (!Chunk->NextInHash))
        {
            Chunk->NextInHash = PushStruct(Arena, world_chunk);
            Chunk = Chunk->NextInHash;
            Chunk->ChunkX = TILE_CHUNK_UNINITIALIZED;
        }

        if (Arena && Chunk->ChunkX == TILE_CHUNK_UNINITIALIZED)
        {
            int32 TileCount = TileMap->ChunkDim * TileMap->ChunkDim;

            Chunk->ChunkX = TileChunkX;
            Chunk->ChunkY = TileChunkY;
            Chunk->ChunkZ = TileChunkZ;

            Chunk->NextInHash = 0;
            break;
        }
        Chunk = Chunk->NextInHash;
    } while (Chunk);

    return (Chunk);
}

inline tile_chunk_position
GetChunkPosition(world *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    tile_chunk_position Result = {};
    Result.TileRelX = AbsTileX & TileMap->ChunkMask;
    Result.TileRelY = AbsTileY & TileMap->ChunkMask;
    Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
    Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
    Result.TileChunkZ = AbsTileZ;
    return (Result);
}

internal void
InitializeWorld(world *TileMap, real32 TileSideInMeters)
{
    TileMap->TileSideInMeters = TileSideInMeters;
    TileMap->ChunkShift = 4;
    TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
    TileMap->ChunkDim = (1 << TileMap->ChunkShift);

    for (uint32 TileChunkIndex = 0;
         TileChunkIndex < ArrayCount(TileMap->ChunkHash);
         ++TileChunkIndex)
    {
        TileMap->ChunkHash[TileChunkIndex].ChunkX = TILE_CHUNK_UNINITIALIZED;
    }
}

inline void
RecanonicalizeCoord(world *World, int32 *Tile, real32 *TileRel)
{

    int32 TileMovedAmt = RoundReal32ToInt32((*TileRel / World->TileSideInMeters));
    *Tile += TileMovedAmt;
    *TileRel -= TileMovedAmt * World->TileSideInMeters;

    Assert(*TileRel >= -0.5f * World->TileSideInMeters)
    Assert(*TileRel < 0.5f * World->TileSideInMeters)
}


inline world_position
MapIntoTileSpace(world *World, world_position BasePos, v2 Offset)
{
    world_position Result = BasePos;

    Result.Offset_ += Offset;
    RecanonicalizeCoord(World, &Result.AbsTileX, &Result.Offset_.X);
    RecanonicalizeCoord(World, &Result.AbsTileY, &Result.Offset_.Y);

    return (Result);
}

inline world_difference
Subtract(world *World, world_position *A, world_position *B)
{
    world_difference Result = {};

    v2 deltaTileXY = {(real32) A->AbsTileX - (real32) B->AbsTileX,
                      (real32) A->AbsTileY - (real32) B->AbsTileY};
    real32 deltaTileZ = (real32) A->AbsTileZ - (real32) B->AbsTileZ;

    Result.deltaXY = World->TileSideInMeters * deltaTileXY + (A->Offset_ - B->Offset_);

    Result.deltaZ = deltaTileZ * World->TileSideInMeters + (0.0f);
    return (Result);
}

inline bool32
AreOnSameTile(world_position *NewPlayerP, world_position *OldPlayerP)
{
    bool32 Result = ((NewPlayerP->AbsTileX == OldPlayerP->AbsTileX) &&
                     (NewPlayerP->AbsTileY == OldPlayerP->AbsTileY) &&
                     (NewPlayerP->AbsTileZ == OldPlayerP->AbsTileZ));
    return (Result);
}

inline world_position
CenteredTilePoint(int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position Result = {};
    Result.AbsTileX = AbsTileX;
    Result.AbsTileY = AbsTileY;
    Result.AbsTileZ = AbsTileZ;
    return (Result);
}

#if 0

inline uint32
GetTileChunkValueUnchecked(world *TileMap, world_chunk *TileChunk, int32 TileX, int32 TileY)
{
    Assert(TileChunk)
    Assert((TileX < TileMap->ChunkDim) && (TileY < TileMap->ChunkDim))
    uint32 TileChunkValue = TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX];
    return (TileChunkValue);
}

inline void
SetTileChunkValueUnchecked(world *TileMap, world_chunk *TileChunk, int32 TileX, int32 TileY, int32 TileValue)
{
    Assert(TileChunk)
    Assert((TileX < TileMap->ChunkDim) && (TileY < TileMap->ChunkDim))
    TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX] = TileValue;
}

internal uint32
GetTileValue(world *TileMap, world_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
    uint32 TileChunkValue = 0;

    if (TileChunk && TileChunk->Tiles)
    {
        TileChunkValue = GetTileChunkValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
    }
    return (TileChunkValue);
}

internal void
SetTileValue(world *TileMap, world_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY, uint32 TileValue)
{
    if (TileChunk && TileChunk->Tiles)
    {
        SetTileChunkValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
    }
}

internal uint32
GetTileValue(world *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    uint32 Result;
    tile_chunk_position ChunkPos = GetChunkPosition(TileMap, AbsTileX, AbsTileY, AbsTileZ);
    world_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
    Result = GetTileValue(TileMap, TileChunk, ChunkPos.TileRelX, ChunkPos.TileRelY);

    return (Result);
}

internal uint32
GetTileValue(world *TileMap, world_position Player)
{
    uint32 Result = GetTileValue(TileMap, Player.AbsTileX, Player.AbsTileY, Player.AbsTileZ);
    return (Result);
}

internal bool32
IsTileValueEmpty(uint32 TileChunkValue)
{
    bool32 Empty = (TileChunkValue == 1) ||
                   (TileChunkValue == 3) ||
                   (TileChunkValue == 4);
    return Empty;
}

internal bool32
IsTileMapPointEmpty(world *TileMap, world_position TestPos)
{
    uint32 TileChunkValue = GetTileValue(TileMap, TestPos);
    bool32 Empty = IsTileValueEmpty(TileChunkValue);
    return (Empty);
}

internal void
SetTileValue(memory_arena *Arena, world *TileMap,
             uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ, uint32 TileValue)
{
    tile_chunk_position ChunkPos = GetChunkPosition(TileMap, AbsTileX, AbsTileY, AbsTileZ);
    world_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ, Arena);

    SetTileValue(TileMap, TileChunk, ChunkPos.TileRelX, ChunkPos.TileRelY, TileValue);
}

#endif
