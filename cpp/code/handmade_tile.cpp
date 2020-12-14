#pragma clang diagnostic push
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by AgentOfChaos on 12/7/2020.
//

inline void
RecanonicalizeCoord(tile_map *TileMap, uint32 *Tile, real32 *TileRel)
{

    int32 TileMovedAmt = RoundReal32ToInt32((*TileRel / TileMap->TileSideInMeters));
    *Tile += TileMovedAmt;
    *TileRel -= TileMovedAmt * TileMap->TileSideInMeters;

    Assert(*TileRel >= -0.5f * TileMap->TileSideInMeters)
    Assert(*TileRel < 0.5f * TileMap->TileSideInMeters)
}

inline tile_map_position
ReCanonicalizePosition(tile_map *TileMap, tile_map_position Pos)
{
    tile_map_position Result = Pos;

    RecanonicalizeCoord(TileMap, &Result.AbsTileX, &Result.Offset.X);
    RecanonicalizeCoord(TileMap, &Result.AbsTileY, &Result.Offset.Y);

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

inline tile_chunk *
GetTileChunk(tile_map *TileMap, uint32 TileChunkX, uint32 TileChunkY, uint32 TileChunkZ)
{
    tile_chunk *TileChunkValue = 0;
    if ((TileChunkX >= 0) && (TileChunkX < TileMap->TileChunkCountX) &&
        (TileChunkY >= 0) && (TileChunkY < TileMap->TileChunkCountY) &&
        (TileChunkZ >= 0) && (TileChunkZ < TileMap->TileChunkCountZ))
    {
        TileChunkValue = &TileMap->TileChunks[TileChunkZ * TileMap->TileChunkCountY * TileMap->TileChunkCountX +
                                              TileChunkY * TileMap->TileChunkCountX + TileChunkX];
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
IsTileMapPointEmpty(tile_map *TileMap, tile_map_position TestPos)
{
    uint32 TileChunkValue = GetTileValue(TileMap, TestPos.AbsTileX, TestPos.AbsTileY, TestPos.AbsTileZ);
    bool32 Empty = (TileChunkValue == 1) ||
                   (TileChunkValue == 3) ||
                   (TileChunkValue == 4);

    return (Empty);
}

internal void
SetTileValue(memory_arena *Arena, tile_map *TileMap,
             uint32 AbsTileX, uint32 AbsTileY, uint32 AbsTileZ, uint32 TileValue)
{
    tile_chunk_position ChunkPos = GetChunkPosition(TileMap, AbsTileX, AbsTileY, AbsTileZ);
    tile_chunk *TileChunk = GetTileChunk(TileMap, ChunkPos.TileChunkX, ChunkPos.TileChunkY, ChunkPos.TileChunkZ);

    if (!TileChunk->Tiles)
    {

        uint32 TileCount = TileMap->ChunkDim * TileMap->ChunkDim;

        TileChunk->Tiles = PushArray(Arena, TileCount, uint32);
        for (uint32 TileIndex = 0; TileIndex < TileCount; ++TileIndex)
        {
            TileChunk->Tiles[TileIndex] = 1;
        }
    }
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

    Result.dXY = TileMap->TileSideInMeters * dTileXY + (A->Offset - B->Offset);

    Result.dZ = dTileZ * TileMap->TileSideInMeters + (0.0f);
    return (Result);
}
