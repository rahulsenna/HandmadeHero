//
// Created by AgentOfChaos on 12/7/2020.
//

inline void
RecanonicalizeCoord(tile_map *TileMap, uint32 *Tile, real32 *TileRel)
{

    int32 TileMovedAmt = RoundReal32ToInt32((*TileRel / TileMap->TileSideInMeters));
    *Tile += TileMovedAmt;
    *TileRel -= TileMovedAmt * TileMap->TileSideInMeters;

    Assert(*TileRel >= -0.5f * TileMap->TileSideInPixels)
    Assert(*TileRel < 0.5f * TileMap->TileSideInMeters)
}

inline tile_map_position
ReCanonicalizePosition(tile_map *TileMap, tile_map_position Pos)
{
    tile_map_position Result = Pos;

    RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.TileRelX);
    RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.TileRelY);

    return (Result);
}

inline tile_chunk_position
GetChunkPosition(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY)
{
    tile_chunk_position Result = {};
    Result.TileRelX = AbsTileX & TileMap->ChunkMask;
    Result.TileRelY = AbsTileY & TileMap->ChunkMask;
    Result.TileChunkX = AbsTileX >> TileMap->ChunkShift;
    Result.TileChunkY = AbsTileY >> TileMap->ChunkShift;
    return (Result);
}

inline tile_chunk *
GetTileChunk(tile_map *TileMap, uint32 TileChunkX, uint32 TileChunkY)
{
    tile_chunk *TileChunkValue = 0;
    if ((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
        (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY))
    {
        TileChunkValue = &TileMap->TileChunks[TileChunkY * TileMap->TileChunkCountX + TileChunkX];
    }
    return (TileChunkValue);
}

inline uint32
GetTileChunkValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, uint32 TileX, uint32 TileY)
{
    Assert(TileChunk)
    Assert((TileX < TileMap->ChunkDim) && (TileY < TileMap->ChunkDim))
    uint32 TileChunkValue = TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX];
    return (TileChunkValue);
}

inline void
SetTileChunkValueUnchecked(tile_map *TileMap, tile_chunk *TileChunk, uint32 TileX, uint32 TileY, uint32 TileValue)
{
    Assert(TileChunk)
    Assert((TileX < TileMap->ChunkDim) && (TileY < TileMap->ChunkDim))
    TileChunk->Tiles[TileY * TileMap->ChunkDim + TileX] = TileValue;
}

internal uint32
GetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY)
{
    uint32 TileChunkValue = 0;

    if (TileChunk)
    {
        TileChunkValue = GetTileChunkValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY);
    }
    return (TileChunkValue);
}

internal void
SetTileValue(tile_map *TileMap, tile_chunk *TileChunk, uint32 TestTileX, uint32 TestTileY, uint32 TileValue)
{
    if (TileChunk)
    {
        SetTileChunkValueUnchecked(TileMap, TileChunk, TestTileX, TestTileY, TileValue);
    }
}

internal uint32
GetTileValue(tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY)
{
    uint32 Result;
    tile_chunk_position ChunkPos = GetChunkPosition(TileMap, AbsTileX, AbsTileY);
    tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY);
    Result = GetTileValue(TileMap, TileChunk, ChunkPos.TileRelX, ChunkPos.TileRelY);

    return (Result);
}

internal bool32
IsTileMapPointEmpty(tile_map *TileMap, tile_map_position TestPos)
{
    uint32 TileChunkValue = GetTileValue(TileMap, TestPos.AbsTileX, TestPos.AbsTileY);
    bool32 Empty = (TileChunkValue == 0);

    return (Empty);
}

internal void
SetTileValue(memory_arena *Arena, tile_map *TileMap, uint32 AbsTileX, uint32 AbsTileY, uint32 TileValue)
{
    tile_chunk_position ChunkPos = GetChunkPosition(TileMap, AbsTileX, AbsTileY);
    tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY);

    if (TileChunk)
    {
    SetTileValue(TileMap, TileChunk, ChunkPos.TileRelX, ChunkPos.TileRelY, TileValue);
    }
}
