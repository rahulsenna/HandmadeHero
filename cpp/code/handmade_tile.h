//
// Created by AgentOfChaos on 12/7/2020.
//

#ifndef HANDMADEHERO_HANDMADE_TILE_H

struct tile_map_difference
{
    v2 deltaXY;
    real32 dZ;
};

struct tile_chunk_position
{
    int32 TileRelX;
    int32 TileRelY;

    int32 TileChunkX;
    int32 TileChunkY;
    int32 TileChunkZ;
};
struct tile_map_position
{
    int32 AbsTileX;
    int32 AbsTileY;
    int32 AbsTileZ;

    v2 Offset_;
};

struct tile_chunk
{
    int32 TileChunkX;
    int32 TileChunkY;
    int32 TileChunkZ;

    int32 *Tiles;

    tile_chunk *NextInHash;
};

struct tile_map
{
    int32 ChunkShift;
    int32 ChunkMask;
    int32 ChunkDim;

    real32 TileSideInMeters;

    tile_chunk TileChunkHash[4096];
};

#define HANDMADEHERO_HANDMADE_TILE_H
#endif //HANDMADEHERO_HANDMADE_TILE_H
