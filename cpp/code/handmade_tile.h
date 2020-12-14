//
// Created by AgentOfChaos on 12/7/2020.
//

#ifndef HANDMADEHERO_HANDMADE_TILE_H
struct tile_chunk_position
{
    uint32 TileRelX;
    uint32 TileRelY;

    uint32 TileChunkX;
    uint32 TileChunkY;
    uint32 TileChunkZ;
};

struct tile_map_difference
{
    v2 dXY;
    real32 dZ;
};
struct tile_map_position
{
    uint32 AbsTileX;
    uint32 AbsTileY;
    uint32 AbsTileZ;

    v2 Offset;
};

struct tile_chunk
{
    uint32 *Tiles;
};

struct tile_map
{
    uint32 ChunkShift;
    uint32 ChunkMask;

    real32 TileSideInMeters;

    uint32 ChunkDim;

    uint32 TileChunkCountX;
    uint32 TileChunkCountY;
    uint32 TileChunkCountZ;

    tile_chunk *TileChunks;
};
#define HANDMADEHERO_HANDMADE_TILE_H
#endif //HANDMADEHERO_HANDMADE_TILE_H
