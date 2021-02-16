//
// Created by AgentOfChaos on 12/7/2020.
//

#ifndef HANDMADEHERO_HANDMADE_TILE_H

struct world_position
{
    int32 ChunkX;
    int32 ChunkY;
    int32 ChunkZ;

    v3 Offset_;
};

struct world_entity_block
{
    uint32 EntityCount;
    uint32 LowEntityIndex[16];
    world_entity_block *Next;
};

struct world_chunk
{
    int32 ChunkX;
    int32 ChunkY;
    int32 ChunkZ;

    world_entity_block FirstBlock;
    world_chunk *NextInHash;
};

struct world
{
    real32 TileSideInMeters;
    real32 TileDepthInMeters;
    v3 ChunkDimInMeters;

    world_entity_block *FirstFree;

    world_chunk ChunkHash[4096];
};

#define HANDMADEHERO_HANDMADE_TILE_H
#endif //HANDMADEHERO_HANDMADE_TILE_H
