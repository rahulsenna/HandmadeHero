
#pragma ide diagnostic ignored "NullDereferences"
#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "modernize-loop-convert"
#pragma ide diagnostic ignored "modernize-use-nullptr"
//
// Created by AgentOfChaos on 12/7/2020.
//


#define WORLD_CHUNK_SAFE_MARGIN (INT32_MAX/64)
#define WORLD_CHUNK_UNINITIALIZED INT32_MAX
#define TILES_PER_CHUNK 16

inline bool32
IsCanonical(world *World, real32 TileRel)
{
    bool32 Result = ((TileRel >= -0.5f * World->ChunkSideInMeters) &&
                     (TileRel <= 0.5f * World->ChunkSideInMeters));
    return (Result);
}

inline bool32
IsCanonical(world *World, v2 Offset)
{
    bool32 Result = (IsCanonical(World, Offset.X) && IsCanonical(World, Offset.Y));
    return (Result);
}

inline world_chunk *
GetWorldChunk(world *World, int32 ChunkX, int32 ChunkY, int32 ChunkZ,
              memory_arena *Arena = 0)
{
    Assert(ChunkX > -WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkY > -WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkZ > -WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkX < WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkY < WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkZ < WORLD_CHUNK_SAFE_MARGIN)

    int32 HashValue = 19 * ChunkX + 7 * ChunkY + 3 * ChunkZ;
    int32 HashSlot = HashValue & (ArrayCount(World->ChunkHash) - 1);
    Assert(HashSlot < ArrayCount(World->ChunkHash))

    world_chunk *Chunk = World->ChunkHash + HashSlot;

    do
    {
        if ((ChunkX == Chunk->ChunkX) &&
            (ChunkY == Chunk->ChunkY) &&
            (ChunkZ == Chunk->ChunkZ))
        {
            break;
        }

        if (Arena && (Chunk->ChunkX != WORLD_CHUNK_UNINITIALIZED) && (!Chunk->NextInHash))
        {
            Chunk->NextInHash = PushStruct(Arena, world_chunk);
            Chunk = Chunk->NextInHash;
            Chunk->ChunkX = WORLD_CHUNK_UNINITIALIZED;
        }

        if (Arena && Chunk->ChunkX == WORLD_CHUNK_UNINITIALIZED)
        {
            Chunk->ChunkX = ChunkX;
            Chunk->ChunkY = ChunkY;
            Chunk->ChunkZ = ChunkZ;

            Chunk->NextInHash = 0;
            break;
        }
        Chunk = Chunk->NextInHash;
    } while (Chunk);

    return (Chunk);
}

internal void
InitializeWorld(world *World, real32 TileSideInMeters)
{
    World->TileSideInMeters = TileSideInMeters;
    World->ChunkSideInMeters = (real32) TILES_PER_CHUNK * TileSideInMeters;
    World->FirstFree = 0;

    for (uint32 ChunkIndex = 0;
         ChunkIndex < ArrayCount(World->ChunkHash);
         ++ChunkIndex)
    {
        World->ChunkHash[ChunkIndex].ChunkX = WORLD_CHUNK_UNINITIALIZED;
        World->ChunkHash[ChunkIndex].FirstBlock.EntityCount = 0;
    }
}

inline void
RecanonicalizeCoord(world *World, int32 *Tile, real32 *TileRel)
{

    int32 TileMovedAmt = RoundReal32ToInt32((*TileRel / World->ChunkSideInMeters));
    *Tile += TileMovedAmt;
    *TileRel -= TileMovedAmt * World->ChunkSideInMeters;

    Assert(IsCanonical(World, *TileRel))
}

inline world_position
MapIntoChunkSpace(world *World, world_position BasePos, v2 Offset)
{
    world_position Result = BasePos;

    Result.Offset_ += Offset;
    RecanonicalizeCoord(World, &Result.ChunkX, &Result.Offset_.X);
    RecanonicalizeCoord(World, &Result.ChunkY, &Result.Offset_.Y);

    return (Result);
}

inline world_difference
Subtract(world *World, world_position *A, world_position *B)
{
    world_difference Result = {};

    v2 deltaTileXY = {(real32) A->ChunkX - (real32) B->ChunkX,
                      (real32) A->ChunkY - (real32) B->ChunkY};
    real32 deltaTileZ = (real32) A->ChunkZ - (real32) B->ChunkZ;

    Result.deltaXY = World->ChunkSideInMeters * deltaTileXY + (A->Offset_ - B->Offset_);

    Result.deltaZ = deltaTileZ * World->ChunkSideInMeters + (0.0f);
    return (Result);
}

inline bool32
AreOnSameChunk(world *World, world_position *A, world_position *B)
{
    Assert(IsCanonical(World, A->Offset_))
    Assert(IsCanonical(World, B->Offset_))

    bool32 Result = ((A->ChunkX == B->ChunkX) &&
                     (A->ChunkY == B->ChunkY) &&
                     (A->ChunkZ == B->ChunkZ));
    return (Result);
}

inline world_position
CenteredChunkPoint(int32 ChunkX, int32 ChunkY, int32 ChunkZ)
{
    world_position Result = {};
    Result.ChunkX = ChunkX;
    Result.ChunkY = ChunkY;
    Result.ChunkZ = ChunkZ;
    return (Result);
}

inline world_position
ChunkPosFromTilePos(world *World, int32 AbsTileX, int32 AbsTileY, int32 AbsTileZ)
{
    world_position Result = {};
    Result.ChunkX = AbsTileX / TILES_PER_CHUNK;
    Result.ChunkY = AbsTileY / TILES_PER_CHUNK;
    Result.ChunkZ = AbsTileZ / TILES_PER_CHUNK;

    Result.Offset_.X = (real32) (AbsTileX - (Result.ChunkX * TILES_PER_CHUNK)) * World->TileSideInMeters;
    Result.Offset_.Y = (real32) (AbsTileY - (Result.ChunkY * TILES_PER_CHUNK)) * World->TileSideInMeters;
    return (Result);
}

inline void
ChangeEntityLocation(memory_arena *Arena, world *World, uint32 LowEntityIndex,
                     world_position *OldP, world_position *NewP)
{
    if (OldP && AreOnSameChunk(World, OldP, NewP))
    {
        // NOTE(rahul): Leave entity where it is
    } else
    {
        if (OldP)
        {
            // NOTE(rahul): Pull the entity out of it's old entity block
            world_chunk *Chunk = GetWorldChunk(World, OldP->ChunkX, OldP->ChunkY, OldP->ChunkZ);
            Assert(Chunk)
            if (Chunk)
            {
                bool32 NotFound = true;
                world_entity_block *FirstBlock = &Chunk->FirstBlock;
                for (world_entity_block *Block = FirstBlock;
                     Block && NotFound;
                     Block = Block->Next)
                {
                    for (uint32 Index = 0; (Index < Block->EntityCount) && NotFound; ++Index)
                    {
                        Assert(FirstBlock->EntityCount > 0)
                        if (Block->LowEntityIndex[Index] == LowEntityIndex)
                        {
                            Block->LowEntityIndex[Index] =
                                    FirstBlock->LowEntityIndex[--FirstBlock->EntityCount];

                            if (FirstBlock->EntityCount == 0)
                            {
                                if (FirstBlock->Next)
                                {
                                    world_entity_block *NextBlock = FirstBlock->Next;
                                    *FirstBlock = *NextBlock;

                                    NextBlock->Next = World->FirstFree;
                                    World->FirstFree = NextBlock;
                                }
                            }
                            NotFound = false;
                        }
                    }
                }
            }
        }

        // NOTE(rahul): Insert the entity into it's new entity block
        world_chunk *Chunk = GetWorldChunk(World, NewP->ChunkX, NewP->ChunkY, NewP->ChunkZ, Arena);
        Assert(Chunk)
        world_entity_block *Block = &Chunk->FirstBlock;
        if (Block->EntityCount == ArrayCount(Block->LowEntityIndex))
        {
            world_entity_block *OldBlock = World->FirstFree;
            if (OldBlock)
            {
                World->FirstFree = OldBlock->Next;
            } else
            {
                OldBlock = PushStruct(Arena, world_entity_block);
            }
            *OldBlock = *Block;
            Block->Next = OldBlock;
            Block->EntityCount = 0;
        }
        Assert(Block->EntityCount < ArrayCount(Block->LowEntityIndex))
        Block->LowEntityIndex[Block->EntityCount++] = LowEntityIndex;
    }
}

