#pragma ide diagnostic ignored "modernize-loop-convert"
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by AgentOfChaos on 12/7/2020.
//


inline void
RecanonicalizeCoord(tile_map *TileMap, int32 *Tile, real32 *TileRel)
{

    int32 TileMovedAmt = RoundReal32ToInt32((*TileRel / TileMap->TileSideInMeters));
    *Tile += TileMovedAmt;
    *TileRel -= TileMovedAmt * TileMap->TileSideInMeters;

    Assert(*TileRel >= -0.5f * TileMap->TileSideInMeters)
    Assert(*TileRel < 0.5f * TileMap->TileSideInMeters)
}

inline tile_map_position
MapIntoTileSpace(tile_map *TileMap, tile_map_position BasePos, v2 Offset)
{
    tile_map_position Result = BasePos;

    Result.Offset_ += Offset;
    RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.Offset_.X);
    RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.Offset_.Y);

    return (Result);
}

inline tile_chunk_position
GetChunkPosition(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    tile_chunk_position Result = {};
    Result.TileRelX = AbsTileX & TileMap->ChunkMask;
    Result.TileRelY = AbsTileY & TileMap->ChunkMask;
    Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
    Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
    Result.TileChunkZ = AbsTileZ;
    return (Result);
}

#define TILE_CHUNK_SAFE_MARGIN (INT32_MAX/64)

#define TILE_CHUNK_UNINITIALIZED INT32_MAX

inline tile_chunk *
GetTileChunk(tile_map *TileMap, int32 TileChunkX, int32 TileChunkY, int32 TileChunkZ,
             memory_arena *Arena = 0)
{
    Assert(TileChunkX > -TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkY > -TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkZ > -TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkX < TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkY < TILE_CHUNK_SAFE_MARGIN)
    Assert(TileChunkZ < TILE_CHUNK_SAFE_MARGIN)

    int32 HashValue = 19 * TileChunkX + 7 * TileChunkY + 3 * TileChunkZ;
    int32 HashSlot = HashValue & (ArrayCount(TileMap->TileChunkHash) - 1);
    Assert(HashSlot < ArrayCount(TileMap->TileChunkHash))

    tile_chunk *Chunk = TileMap->TileChunkHash + HashSlot;

    do
    {
        if ((TileChunkX == Chunk->TileChunkX) &&
            (TileChunkY == Chunk->TileChunkY) &&
            (TileChunkZ == Chunk->TileChunkZ))
        {
            break;
        }

        if (Arena && (Chunk->TileChunkX != TILE_CHUNK_UNINITIALIZED) && (!Chunk->NextInHash))
        {
            Chunk->NextInHash = PushStruct(Arena, tile_chunk);
            Chunk = Chunk->NextInHash;
            Chunk->TileChunkX = TILE_CHUNK_UNINITIALIZED;
        }

        if (Arena && Chunk->TileChunkX == TILE_CHUNK_UNINITIALIZED)
        {
            int32 TileCount = TileMap->ChunkDim * TileMap->ChunkDim;

            Chunk->TileChunkX = TileChunkX;
            Chunk->TileChunkY = TileChunkY;
            Chunk->TileChunkZ = TileChunkZ;

            Chunk->Tiles = PushArray(Arena, TileCount, int32);
            for (int32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
            {
                Chunk->Tiles[TileIndex] = 1;
            }
            Chunk->NextInHash = 0;
            break;
        }
        Chunk = Chunk->NextInHash;
    } while (Chunk);

    return (Chunk);
}

internal void
InitializeTileMap(tile_map *TileMap, real32 TileSideInMeters)
{
    TileMap->TileSideInMeters = TileSideInMeters;
    TileMap->ChunkShift = 4;
    TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;
    TileMap->ChunkDim = (1 << TileMap->ChunkShift);

    for (uint32 TileChunkIndex = 0;
         TileChunkIndex < ArrayCount(TileMap->TileChunkHash);
         ++TileChunkIndex)
    {
        TileMap->TileChunkHash[TileChunkIndex].TileChunkX = TILE_CHUNK_UNINITIALIZED;
    }
}

inline uint32
GetTileChunkValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, int32 TileX, int32 TileY)
{
    Assert(TileChunk)
    Assert((TileX < TileMap->ChunkDim) && (TileY < TileMap->ChunkDim))
    uint32 TileChunkValue = TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX];
    return (TileChunkValue);
}

inline void
SetTileChunkValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, int32 TileX, int32 TileY, int32 TileValue)
{
    Assert(TileChunk)
    Assert((TileX < TileMap->ChunkDim) && (TileY < TileMap->ChunkDim))
    TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX] = TileValue;
}

internal uint32
GetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
    uint32 TileChunkValue = 0;

    if (TileChunk && TileChunk->Tiles)
    {
        TileChunkValue = GetTileChunkValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
    }
    return (TileChunkValue);
}

internal void
SetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY, uint32 TileValue)
{
    if (TileChunk && TileChunk->Tiles)
    {
        SetTileChunkValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
    }
}

internal uint32
GetTileValue(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ)
{
    uint32 Result;
    tile_chunk_position ChunkPos = GetChunkPosition(TileMap, AbsTileX, AbsTileY, AbsTileZ);
    tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);
    Result = GetTileValue(TileMap, TileChunk, ChunkPos.TileRelX, ChunkPos.TileRelY);

    return (Result);
}

internal uint32
GetTileValue(tile_map *TileMap, tile_map_position Player)
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
IsTileMapPointEmpty(tile_map *TileMap, tile_map_position TestPos)
{
    uint32 TileChunkValue = GetTileValue(TileMap, TestPos);
    bool32 Empty = IsTileValueEmpty(TileChunkValue);
    return (Empty);
}

internal void
SetTileValue(memory_arena *Arena, tile_map *TileMap,
             uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ, uint32 TileValue)
{
    tile_chunk_position ChunkPos = GetChunkPosition(TileMap, AbsTileX, AbsTileY, AbsTileZ);
    tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ, Arena);

    SetTileValue(TileMap, TileChunk, ChunkPos.TileRelX, ChunkPos.TileRelY, TileValue);
}

inline bool32
AreOnSameTile(tile_map_position *NewPlayerP, tile_map_position *OldPlayerP)
{
    bool32 Result = ((NewPlayerP->AbsTileX == OldPlayerP->AbsTileX) &&
                     (NewPlayerP->AbsTileY == OldPlayerP->AbsTileY) &&
                     (NewPlayerP->AbsTileZ == OldPlayerP->AbsTileZ));
    return (Result);
}

inline tile_map_difference
Subtract(tile_map *TileMap, tile_map_position *A, tile_map_position *B)
{
    tile_map_difference Result = {};

    v2 dTileXY = {(real32) A->AbsTileX - (real32) B->AbsTileX,
                  (real32) A->AbsTileY - (real32) B->AbsTileY};
    real32 dTileZ = (real32) A->AbsTileZ - (real32) B->AbsTileZ;

    Result.deltaXY = TileMap->TileSideInMeters * dTileXY + (A->Offset_ - B->Offset_);

    Result.dZ = dTileZ * TileMap->TileSideInMeters + (0.0f);
    return (Result);
}

inline tile_map_position
CenteredTilePoint(int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    tile_map_position Result = {};
    Result.AbsTileX = AbsTileX;
    Result.AbsTileY = AbsTileY;
    Result.AbsTileZ = AbsTileZ;
    return (Result);
}
