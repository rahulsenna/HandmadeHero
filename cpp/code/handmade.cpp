#pragma clang diagnostic push
#pragma clang diagnostic push

#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "UnusedLocalVariable"
//#pragma ide diagnostic ignored "bugprone-branch-clone"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"

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
#include "handmade_random.h"

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
DrawBitmap(const game_offscreen_buffer *Buffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY)
{
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = RoundReal32ToInt32(RealX + (real32) Bitmap->Width);
    int32 MaxY = RoundReal32ToInt32(RealY + (real32) Bitmap->Height);
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

    uint32 *SourceRow = Bitmap->Pixels + Bitmap->Width * (Bitmap->Height - 1);
    uint8 *DestRow = ((uint8 *) Buffer->Memory +
                      MinX * Buffer->BytesPerPixel +
                      MinY * Buffer->Pitch);
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32 *Dest = (uint32 *) DestRow;
        uint32 *Source = (uint32 *) SourceRow;
        for (int X = MinX; X < MaxX; ++X)
        {
            *Dest++ = *Source++;
        }
        DestRow += Buffer->Pitch;
        SourceRow -= Bitmap->Width;
    }
}

internal void
InitializeArena(memory_arena *Arena, mem_index Size, uint8 *Base)
{
    Arena->Size = Size;
    Arena->Base = Base;
    Arena->Used = 0;
}

#pragma pack(push, 1)
struct bitmap_header
{
    uint16 FileType;
    uint32 FileSize;
    uint16 FileReserved1;
    uint16 FileReserved2;
    uint32 FileOffBits;

    uint32 Size;
    uint32 Width;
    uint32 Height;
    uint16 Planes;
    uint16 BitCount;

    uint32 Compression;
    uint32 SizeImage;
    int32 XPelsPerMeter;
    int32 YPelsPerMeter;
    uint32 ClrUsed;
    uint32 ClrImportant;
    uint32 RedMask;
    uint32 GreenMask;
    uint32 BlueMask;
    uint32 AlphaMask;
};
#pragma pack(pop)

internal loaded_bitmap
DEBUGLoadBMP(thread_context *Thread, debug_platform_read_entire_file *ReadEntireFile, char *Filename)
{
    debug_read_file_result ReadResult = ReadEntireFile(Thread, Filename);

    loaded_bitmap Result = {};
    if (ReadResult.ContentsSize != 0)
    {
        bitmap_header *Header = (bitmap_header *) ReadResult.Contents;
        uint32 *Pixels = (uint32 *) ((uint8 *) ReadResult.Contents + Header->FileOffBits);
        Result.Width = Header->Width;
        Result.Height = Header->Height;
        Result.Pixels = Pixels;

        uint32 *Source = Pixels;
        for (uint32 Y = 0; Y < Header->Height; ++Y)
        {
            for (uint32 X = 0; X < Header->Width; ++X)
            {
                *Source = (*Source >> 8) | (*Source << 24);
                ++Source;
            }
        }
    }
    return (Result);
}

//#pragma clang diagnostic ignored "-Wnull-dereference"
#pragma clang diagnostic ignored "-Wwritable-strings"
extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
    Assert(sizeof(game_state) <= Memory->PermanentStorageSize)

    real32 PlayerHeight = 1.4f;
    real32 PlayerWidth = PlayerHeight * 0.75f;

    game_state *GameState = (game_state *) Memory->PermanentStorage;
    if (!Memory->IsInitialized)
    {
        GameState->Backdrop = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "test/test_background.bmp");

        GameState->HeroHead = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "test/test_hero_front_head.bmp");
        GameState->HeroTorso = DEBUGLoadBMP(Thread,
                                            Memory->DEBUGPlatformReadEntireFile,
                                            "test/test_hero_front_torso.bmp");
        GameState->HeroCape = DEBUGLoadBMP(Thread,
                                           Memory->DEBUGPlatformReadEntireFile,
                                           "test/test_hero_front_cape.bmp");
        Memory->IsInitialized = true;

        GameState->PlayerP.AbsTileX = 3;
        GameState->PlayerP.AbsTileY = 3;
        GameState->PlayerP.OffsetX = 6.0f;
        GameState->PlayerP.OffsetY = 6.0f;

        InitializeArena(&GameState->WorldArena, (mem_index) Memory->PermanentStorageSize - sizeof(game_state),
                        (uint8 *) Memory->PermanentStorage + sizeof(game_state));
        GameState->World = PushStruct(&GameState->WorldArena, world);

        world *World = GameState->World;
        World->TileMap = PushStruct(&GameState->WorldArena, tile_map);

        tile_map *TileMap = World->TileMap;

        TileMap->TileSideInMeters = 1.4f;

        TileMap->ChunkShift = 4;
        TileMap->ChunkMask = (1 << TileMap->ChunkShift) - 1;

        TileMap->ChunkDim = (1 << TileMap->ChunkShift);

        TileMap->TileChunkCountX = 128;
        TileMap->TileChunkCountY = 128;
        TileMap->TileChunkCountZ = 2;

        TileMap->TileChunks = PushArray(&GameState->WorldArena,
                                        TileMap->TileChunkCountX *
                                        TileMap->TileChunkCountY *
                                        TileMap->TileChunkCountZ,
                                        tile_chunk);

        uint32 TilesPerHeight = 9;
        uint32 TilesPerWidth = 17;

        int ScreenX = 0;
        int ScreenY = 0;
        uint32 RandomNumberIndex = 0;
        uint32 AbsTileZ = 0;

        bool32 DoorLeft = false;
        bool32 DoorRight = false;
        bool32 DoorTop = false;
        bool32 DoorBottom = false;

        bool32 DoorUp = false;
        bool32 DoorDown = false;

        for (int ScreenIndex = 0; ScreenIndex < 100; ++ScreenIndex)
        {
            Assert(RandomNumberIndex < ArrayCount(RandomNumberTable))
            uint32 RandomChoice;

            if (DoorDown || DoorUp)
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 2;
            } else
            {
                RandomChoice = RandomNumberTable[RandomNumberIndex++] % 3;
            }

            bool32 ZDoorCreated = false;
            if (RandomChoice == 2)
            {
                ZDoorCreated = true;
                if (AbsTileZ == 0)
                {
                    DoorUp = true;
                } else
                {
                    DoorDown = true;
                }
            } else if (RandomChoice == 1)
            {
                DoorRight = true;
            } else
            {
                DoorTop = true;
            }
            for (uint32 TileY = 0; TileY < TilesPerHeight; ++TileY)
            {
                for (uint32 TileX = 0; TileX < TilesPerWidth; ++TileX)
                {
                    uint32 AbsTileX = ScreenX * TilesPerWidth + TileX;
                    uint32 AbsTileY = ScreenY * TilesPerHeight + TileY;

                    uint32 TileValue = 1;

                    if ((TileX == 0) && (!DoorLeft || (TileY != (TilesPerHeight / 2))))
                    {
                        TileValue = 2;
                    }
                    if ((TileX == (TilesPerWidth - 1)) && (!DoorRight || (TileY != (TilesPerHeight / 2))))
                    {
                        TileValue = 2;
                    }
                    if ((TileY == 0) && (!DoorBottom || (TileX != (TilesPerWidth / 2))))
                    {
                        TileValue = 2;
                    }
                    if ((TileY == (TilesPerHeight - 1)) && (!DoorTop || (TileX != (TilesPerWidth / 2))))
                    {
                        TileValue = 2;
                    }

                    if ((TileX == 10) && (TileY == 6))
                    {
                        if (DoorUp)
                        {
                            TileValue = 3;
                        }
                        if (DoorDown)
                        {
                            TileValue = 4;
                        }
                    }

                    SetTileValue(&GameState->WorldArena, World->TileMap,
                                 AbsTileX, AbsTileY, AbsTileZ, TileValue);
                }
            }

            DoorLeft = DoorRight;
            DoorBottom = DoorTop;
            DoorRight = false;
            DoorTop = false;

            if (ZDoorCreated)
            {
                DoorUp = !DoorUp;
                DoorDown = !DoorDown;
            } else
            {
                DoorUp = false;
                DoorDown = false;
            }

            if (RandomChoice == 2)
            {
                if (AbsTileZ == 0)
                {
                    AbsTileZ = 1;
                } else
                {
                    AbsTileZ = 0;
                }
            } else if (RandomChoice == 1)
            {
                ScreenX += 1;
            } else
            {
                ScreenY += 1;
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
            NewPlayerP.OffsetX += Input->SecondsToAdvancePerFrame * PixelPerFrameX;
            NewPlayerP.OffsetY += Input->SecondsToAdvancePerFrame * PixelPerFrameY;
            NewPlayerP = ReCanonicalizePosition(TileMap, NewPlayerP);

            tile_map_position PlayerLeft = NewPlayerP;
            PlayerLeft.OffsetX -= PlayerWidth * 0.5f;
            PlayerLeft = ReCanonicalizePosition(TileMap, PlayerLeft);

            tile_map_position PlayerRight = NewPlayerP;
            PlayerRight.OffsetX += PlayerWidth * 0.5f;
            PlayerRight = ReCanonicalizePosition(TileMap, PlayerRight);

            if (IsTileMapPointEmpty(TileMap, NewPlayerP) &&
                IsTileMapPointEmpty(TileMap, PlayerLeft) &&
                IsTileMapPointEmpty(TileMap, PlayerRight))
            {
                if (!AreOnSameTile(&NewPlayerP, &GameState->PlayerP))
                {
                    uint32 NewTileValue = GetTileValue(TileMap, NewPlayerP);
                    if (NewTileValue == 3)
                    {
                        ++NewPlayerP.AbsTileZ;
                    } else if (NewTileValue == 4)
                    {
                        --NewPlayerP.AbsTileZ;
                    }
                }
                GameState->PlayerP = NewPlayerP;
            }
        }
    }

    DrawBitmap(Buffer, &GameState->Backdrop, 0, 0);

    real32 ScreenCenterX = (real32) Buffer->Width * 0.5f;
    real32 ScreenCenterY = (real32) Buffer->Height * 0.5f;

    real32 TileSideInPixels = 60;
    real32 MetersToPixel = (real32) TileSideInPixels / TileMap->TileSideInMeters;

    for (int32 RelRow = -10; RelRow < 10; ++RelRow)
    {
        for (int32 RelColumn = -20; RelColumn < 20; ++RelColumn)
        {
            uint32 Column = (uint32) RelColumn + GameState->PlayerP.AbsTileX;
            uint32 Row = (uint32) RelRow + GameState->PlayerP.AbsTileY;

            uint32 TileID = GetTileValue(TileMap, Column, Row, GameState->PlayerP.AbsTileZ);

            if (TileID > 1)
            {
                real32 Gray = 0.2f;
                if (TileID == 2)
                {
                    Gray = 1.0f;
                }
                if (TileID > 2)
                {
                    Gray = 0.5f;
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

                real32 CenterX = (ScreenCenterX - (MetersToPixel * GameState->PlayerP.OffsetX)) +
                                 ((real32) RelColumn) * TileSideInPixels;
                real32 CenterY = (ScreenCenterY + (MetersToPixel * GameState->PlayerP.OffsetY)) -
                                 ((real32) RelRow) * TileSideInPixels;
                real32 MinX = CenterX - 0.5f * TileSideInPixels;
                real32 MinY = CenterY - 0.5f * TileSideInPixels;

                real32 MaxX = CenterX + 0.5f * TileSideInPixels;
                real32 MaxY = CenterY + 0.5f * TileSideInPixels;
                DrawRectangle(Buffer, MinX, MinY, MaxX, MaxY, Gray, Gray, Gray);
            }
        }
    }

    real32 PlayerLeft = ScreenCenterX - (0.5f * PlayerWidth * MetersToPixel);
    real32 PlayerTop = ScreenCenterY - (PlayerHeight * MetersToPixel);

//    DrawRectangle(Buffer, PlayerLeft, PlayerTop,
//                  PlayerLeft + PlayerWidth * MetersToPixel,
//                  PlayerTop + PlayerHeight * MetersToPixel,
//                  1, 1, 0);

    DrawBitmap(Buffer, &GameState->HeroHead, PlayerLeft, PlayerTop);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
    game_state *GameState = (game_state *) Memory->PermanentStorage;
    GameOutputSound(SoundBuffer, GameState);
}
