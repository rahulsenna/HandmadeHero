#pragma clang diagnostic push
#pragma clang diagnostic push

//#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "UnusedLocalVariable"
//#pragma ide diagnostic ignored "bugprone-branch-clone"
//#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

#pragma ide diagnostic ignored "hicpp-signed-bitwise"
#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "modernize-loop-convert"
#pragma ide diagnostic ignored "modernize-deprecated-headers"
#pragma ide diagnostic ignored "modernize-use-nullptr"

//
// Created by AgentOfChaos on 11/20/2020.
//

#include "handmade.h"
#include "handmade_tile.cpp"

void
GameOutputSound(game_output_sound_buffer *SoundBuffer, game_state *GameState)
{
    int16 *SampleOut = SoundBuffer->Samples;
    for (int SampleIndex = 0;
         SampleIndex < SoundBuffer->SampleCount;
         ++SampleIndex)
    {
#if 0
        int16 ToneVolume = 3000;
        real32 SineValue = sinf(GameState->tSine);
        int16 SampleValue = (int16) (SineValue * (real32) ToneVolume);
#else
        int16 SampleValue = 0;
#endif
        *SampleOut++ = SampleValue;
        *SampleOut++ = SampleValue;
#if 0
        int WavePeriod = SoundBuffer->SamplesPerSecond / 400;
        GameState->tSine += 2.0f * Pi32 * 1.0f / (real32) WavePeriod;
        if (GameState->tSine > 2.0f * Pi32)
        {
            GameState->tSine -= 2.0f * Pi32;
        }
#endif
    }
}

/*
void
RenderWeirdGradient(game_offscreen_buffer *Buffer, int XOffset, int YOffset)
{
    uint8 *Row = (uint8 *) Buffer->Memory;
    for (int y = 0; y < Buffer->Height; ++y)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int x = 0; x < Buffer->Width; ++x)
        {
            uint8 Blue = (uint8) (x + XOffset);
            uint8 Green = (uint8) (y + YOffset);
            *Pixel++ = ((Green << 16) | Blue);
        }
        Row += Buffer->Pitch;
    }
}
*/

internal void
DrawRectangle(game_offscreen_buffer *Buffer,
              real32 RealMinX, real32 RealMinY, real32 RealMaxX, real32 RealMaxY,
              real32 R, real32 G, real32 B)
{
    int32 MinX = RoundReal32ToInt32(RealMinX);
    int32 MinY = RoundReal32ToInt32(RealMinY);
    int32 MaxX = RoundReal32ToInt32(RealMaxX);
    int32 MaxY = RoundReal32ToInt32(RealMaxY);
    if (MinX < 0)
    {
        MinX = 0;
    }
    if (MinY < 0)
    {
        MinY = 0;
    }
    if (MaxX > Buffer->Width)
    {
        MaxX = Buffer->Width;
    }
    if (MaxY > Buffer->Height)
    {
        MaxY = Buffer->Height;
    }
    uint32 Color = ((RoundReal32ToUInt32(R * 255.0f) << 16) |
                    (RoundReal32ToUInt32(G * 255.0f) << 8) |
                    (RoundReal32ToUInt32(B * 255.0f) << 0));
    uint8 *Row = ((uint8 *) Buffer->Memory +
                  MinX * Buffer->BytesPerPixel +
                  MinY * Buffer->Pitch);
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Pixel = (uint32 *) Row;
        for (int X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }
        Row += Buffer->Pitch;
    }
}

internal void
InitializeArena(memory_arena *Arena, mem_index Size, uint8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#define PushStruct(Arena, type) (type *)PushSize_(Arena, sizeof(type))
#define PushArray(Arena, Count, type) (type *)PushSize_(Arena, (Count * sizeof(type)))

void *
PushSize_(memory_arena *Arena, mem_index Size)
{
    Assert(Arena->Used + Size <= Arena->Size)
    void *Result = Arena->Base + Arena->Used;
    Arena->Used += Size;
    return (Result);
}

//#pragma clang diagnostic ignored "-Wnull-dereference"
//#pragma clang diagnostic ignored "-Wwritable-strings"
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)

    real32 PlayerHeight = 1.4f;
    real32 PlayerWidth = PlayerHeight * 0.75f;

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        Memory->IsInitialized = true;

        GameState->PlayerP.AbsTileX = 3;
        GameState->PlayerP.AbsTileY = 3;
        GameState->PlayerP.TileRelX = 6.0f;
        GameState->PlayerP.TileRelY = 6.0f;

        InitializeArena(&GameState->WorldArena, (mem_index) Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *) Memory->PermanentStorage + sizeof(game_state));
        GameState->World = PushStruct(&GameState->WorldArena, world);

        world *World = GameState->World;
        World->TileMap = PushStruct(&GameState->WorldArena, tile_map);

        tile_map *TileMap = World->TileMap;

        TileMap->TileSideInMeters = 1.4f;
        TileMap->TileSideInPixels = 60;
        TileMap->MetersToPixel = (real32) TileMap->TileSideInPixels / TileMap->TileSideInMeters;

        TileMap->ChunkShift = 8;
        TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;

        TileMap->ChunkDim = (1 << TileMap->ChunkShift);

        TileMap->TileChunkCountX = 4;
        TileMap->TileChunkCountY = 4;

        TileMap->TileChunks = PushArray(&GameState->WorldArena,
                                        TileMap->TileChunkCountX * TileMap->TileChunkCountY,
                                        tile_chunk);
        for (uint32 Y = 0; Y < TileMap->TileChunkCountY; ++Y)
        {
            for (uint32 X = 0; X < TileMap->TileChunkCountX; ++X)
            {
                TileMap->TileChunks[TileMap->TileChunkCountX * Y + X].Tiles =
                        PushArray(&GameState->WorldArena, TileMap->ChunkDim * TileMap->ChunkDim, uint32);
            }
        }

        uint32 TilesPerHeight = 9;
        uint32 TilesPerWidth = 17;

        for (int ScreenY = 0; ScreenY < 32; ++ScreenY)
        {
            for (int ScreenX = 0; ScreenX < 32; ++ScreenX)
            {
                for (uint32 TileY = 0; TileY < TilesPerHeight; ++TileY)
                {
                    for (uint32 TileX = 0; TileX < TilesPerWidth; ++TileX)
                    {
                        uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
                        uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;
                        SetTileValue(&GameState->WorldArena, World->TileMap,
                                     AbsTileX, AbsTileY,
                                     (TileY == TileX) & (TileY % 2) ? 1 : 0);
                    }
                }
            }
        }
    }

    world *World = GameState->World;
    tile_map *TileMap = World->TileMap;

    for (int ControllerIndex = 0; ControllerIndex < ArrayCount(Input->Controllers); ++ControllerIndex)
    {
        game_controller_input *Controller = GetController(Input, ControllerIndex);
        if (Controller->IsAnalog)
        {
        } else
        {
            real32 PixelPerFrameX = 0;
            real32 PixelPerFrameY = 0;
            if (Controller->MoveUp.EndedDown)
            {
                PixelPerFrameY = 1.0f;
            }
            if (Controller->MoveDown.EndedDown)
            {
                PixelPerFrameY = -1.0f;
            }
            if (Controller->MoveLeft.EndedDown)
            {
                PixelPerFrameX = -1.0f;
            }
            if (Controller->MoveRight.EndedDown)
            {
                PixelPerFrameX = 1.0f;
            }
            if (Controller->AButton.EndedDown)
            {
                PixelPerFrameX *= 10.0f;
                PixelPerFrameY *= 10.0f;
            }

            PixelPerFrameX *= 10.0f;
            PixelPerFrameY *= 10.0f;

            tile_map_position NewPlayerP = GameState->PlayerP;
            NewPlayerP.TileRelX += Input->SecondsToAdvancePerFrame * PixelPerFrameX;
            NewPlayerP.TileRelY += Input->SecondsToAdvancePerFrame * PixelPerFrameY;
            NewPlayerP = ReCanonicalizePosition(TileMap, NewPlayerP);

            tile_map_position PlayerLeft = NewPlayerP;
            PlayerLeft.TileRelX -= PlayerWidth * 0.5f;
            PlayerLeft = ReCanonicalizePosition(TileMap, PlayerLeft);

            tile_map_position PlayerRight = NewPlayerP;
            PlayerRight.TileRelX += PlayerWidth * 0.5f;
            PlayerRight = ReCanonicalizePosition(TileMap, PlayerRight);

            if (IsTileMapPointEmpty(TileMap, NewPlayerP) &&
                IsTileMapPointEmpty(TileMap, PlayerLeft) &&
                IsTileMapPointEmpty(TileMap, PlayerRight))
            {
                GameState->PlayerP = NewPlayerP;
            }
        }
    }
    DrawRectangle(Buffer, 0.0f, 0.0f, (real32) Buffer->Width, (real32) Buffer->Height,
                  1.0f, 0.0f, 0.0f);

    real32 ScreenCenterX = (real32) Buffer->Width * 0.5f;
    real32 ScreenCenterY = (real32) Buffer->Height * 0.5f;

    for (int32 RelRow = -6; RelRow < 6; ++RelRow)
    {
        for (int32 RelColumn = -9; RelColumn < 9; ++RelColumn)
        {
            uint32 Column = (uint32) RelColumn + GameState->PlayerP.AbsTileX;
            uint32 Row = (uint32) RelRow + GameState->PlayerP.AbsTileY;

            uint32 TileId = GetTileValue(TileMap, Column, Row);
            real32 Gray = 0.2f;
            if (TileId == 1)
            {
                Gray = 1.0f;
            }
            if ((Row == GameState->PlayerP.AbsTileY) && (Column == GameState->PlayerP.AbsTileX))
            {
                Gray = 0.0;
            }

/*
 //         Tile Chunk Look Up Visualized
            real32 MinX = ((real32) Column) * 60;
            real32 MinY = ((real32) Row) * 60;
            real32 MaxX = MinX + 60;
            real32 MaxY = MinY + 60;
            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);*/

            real32 CenterX = (ScreenCenterX - (TileMap->MetersToPixel * GameState->PlayerP.TileRelX)) +
                             ((real32) RelColumn) * TileMap->TileSideInPixels;
            real32 CenterY = (ScreenCenterY + (TileMap->MetersToPixel * GameState->PlayerP.TileRelY)) -
                             ((real32) RelRow) * TileMap->TileSideInPixels;
            real32 MinX = CenterX - 0.5f * TileMap->TileSideInPixels;
            real32 MinY = CenterY - 0.5f * TileMap->TileSideInPixels;

            real32 MaxX = CenterX + 0.5f * TileMap->TileSideInPixels;
            real32 MaxY = CenterY + 0.5f * TileMap->TileSideInPixels;
            DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
        }
    }

    real32 PlayerLeft = ScreenCenterX - (0.5f * PlayerWidth * TileMap->MetersToPixel);
    real32 PlayerTop = ScreenCenterY - (PlayerHeight * TileMap->MetersToPixel);

    DrawRectangle(Buffer, PlayerLeft, PlayerTop,
                  PlayerLeft + PlayerWidth * TileMap->MetersToPixel,
                  PlayerTop + PlayerHeight * TileMap->MetersToPixel,
                  1, 1, 0);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(SoundBuffer, GameState);
}
