
//
// Created by AgentOfChaos on 12/7/2020.
//


#define WORLD_CHUNK_SAFE_MARGIN (INT32_MAX/64)
#define WORLD_CHUNK_UNINITIALIZED INT32_MAX
#define TILES_PER_CHUNK 8

inline world_position
NullPosition()
{
    world_position Result = {};
    Result.ChunkX = WORLD_CHUNK_UNINITIALIZED;
    return (Result);
}

inline b32
IsValid(world_position P)
{
    b32 Result = P.ChunkX != WORLD_CHUNK_UNINITIALIZED;
    return (Result);
}

inline b32
IsCanonical(r32 ChunkDim, r32 TileRel)
{
    r32 Epsilon = 0.01f;
    b32 Result  = ((TileRel >= -(0.5f * ChunkDim + Epsilon)) &&
                      (TileRel <= (0.5f * ChunkDim + Epsilon)));
    return (Result);
}

inline b32
IsCanonical(world *World, v3 Offset)
{

    b32 Result = (IsCanonical(World->ChunkDimInMeters.x, Offset.x) &&
                  IsCanonical(World->ChunkDimInMeters.y, Offset.y) &&
                  IsCanonical(World->ChunkDimInMeters.z, Offset.z));
    return (Result);
}

inline world_chunk *
GetWorldChunk(world *World, s32 ChunkX, s32 ChunkY, s32 ChunkZ,
              memory_arena *Arena = 0)
{
    Assert(ChunkX > -WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkY > -WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkZ > -WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkX < WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkY < WORLD_CHUNK_SAFE_MARGIN)
    Assert(ChunkZ < WORLD_CHUNK_SAFE_MARGIN)

    s32 HashValue = 19 * ChunkX + 7 * ChunkY + 3 * ChunkZ;
    s32 HashSlot  = HashValue & (ArrayCount(World->ChunkHash) - 1);
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
InitializeWorld(world *World,v3 ChunkDimInMeters)
{

    World->ChunkDimInMeters = ChunkDimInMeters;
    World->FirstFree = 0;

    for (u32 ChunkIndex = 0;
         ChunkIndex < ArrayCount(World->ChunkHash);
         ++ChunkIndex)
    {
        World->ChunkHash[ChunkIndex].ChunkX = WORLD_CHUNK_UNINITIALIZED;
        World->ChunkHash[ChunkIndex].FirstBlock.EntityCount = 0;
    }
}

inline void
RecanonicalizeCoord(r32 ChunkDim, s32 *Tile, r32 *TileRel)
{

    s32 TileMovedAmt = RoundReal32ToInt32((*TileRel / ChunkDim));
    *Tile += TileMovedAmt;
    *TileRel -= (r32) TileMovedAmt * ChunkDim;

    Assert(IsCanonical(ChunkDim, *TileRel))
}

inline world_position
MapIntoChunkSpace(world *World, world_position BasePos, v3 Offset)
{
    world_position Result = BasePos;

    Result.Offset_ += Offset;
    RecanonicalizeCoord(World->ChunkDimInMeters.x, &Result.ChunkX, &Result.Offset_.x);
    RecanonicalizeCoord(World->ChunkDimInMeters.y, &Result.ChunkY, &Result.Offset_.y);
    RecanonicalizeCoord(World->ChunkDimInMeters.z, &Result.ChunkZ, &Result.Offset_.z);

    return (Result);
}



inline v3
Subtract(world *World, world_position *A, world_position *B)
{
    v3 Result = {};

    v3 deltaTile = {(r32) A->ChunkX - (r32) B->ChunkX,
                    (r32) A->ChunkY - (r32) B->ChunkY,
                    (r32) A->ChunkZ - (r32) B->ChunkZ};

    Result = Hadamard(World->ChunkDimInMeters, deltaTile) + (A->Offset_ - B->Offset_);
    return (Result);
}

inline b32
AreOnSameChunk(world *World, world_position *A, world_position *B)
{
    Assert(IsCanonical(World, A->Offset_))
    Assert(IsCanonical(World, B->Offset_))

    b32 Result = ((A->ChunkX == B->ChunkX) &&
                  (A->ChunkY == B->ChunkY) &&
                  (A->ChunkZ == B->ChunkZ));
    return (Result);
}

inline world_position
CenteredChunkPoint(s32 ChunkX, s32 ChunkY, s32 ChunkZ)
{
    world_position Result = {};
    Result.ChunkX = ChunkX;
    Result.ChunkY = ChunkY;
    Result.ChunkZ = ChunkZ;
    return (Result);
}

inline world_position
CenteredChunkPoint(world_chunk *Chunk)
{
    world_position Result = CenteredChunkPoint(Chunk->ChunkX, Chunk->ChunkY, Chunk->ChunkZ);

    return(Result);
}

inline void
ChangeEntityLocationRaw(memory_arena *Arena, world *World, u32 LowEntityIndex,
                        world_position *OldP, world_position *NewP)
{

    Assert(!OldP || IsValid(*OldP))
    Assert(!NewP || IsValid(*NewP))

    if (OldP && NewP && AreOnSameChunk(World, OldP, NewP))
    {
        // NOTE(rahul): Leave sim_entity where it is
    } else
    {
        if (OldP)
        {
            // NOTE(rahul): Pull the sim_entity out of it's old sim_entity block
            world_chunk *Chunk = GetWorldChunk(World, OldP->ChunkX, OldP->ChunkY, OldP->ChunkZ);
            Assert(Chunk)
            if (Chunk)
            {
                b32                NotFound    = true;
                world_entity_block *FirstBlock = &Chunk->FirstBlock;
                for (world_entity_block *Block = FirstBlock;
                     Block && NotFound;
                     Block = Block->Next)
                {
                    for (u32 Index = 0; (Index < Block->EntityCount) && NotFound; ++Index)
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

        if (NewP)
        {
            // NOTE(rahul): Insert the sim_entity into it's new sim_entity block
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
}

internal void
ChangeEntityLocation(memory_arena *Arena, world *World, u32 LowEntityIndex,
                     low_entity *LowEntity, world_position NewPInit)
{
    world_position *OldP = 0;
    world_position *NewP = 0;

    if (!IsSet(&LowEntity->Sim, EntityFlag_NonSpatial) && IsValid(LowEntity->P))
    {
        OldP = &LowEntity->P;
    }

    if (IsValid(NewPInit))
    {
        NewP = &NewPInit;
    }

    ChangeEntityLocationRaw(Arena, World, LowEntityIndex, OldP, NewP);

    if (NewP)
    {
        LowEntity->P = *NewP;
        ClearFlags(&LowEntity->Sim, EntityFlag_NonSpatial);
    } else
    {
        LowEntity->P = NullPosition();
        AddFlags(&LowEntity->Sim, EntityFlag_NonSpatial);
    }
}

