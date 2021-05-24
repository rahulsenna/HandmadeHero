//
// Created by AgentOfChaos on 11/25/2020.
//

#ifndef HANDMADEHERO_WIN32_HANDMADE_H

struct win32_offscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int BytesPerPixel;
    int Pitch;
};

struct win32_window_dimension
{
    int Width;
    int Height;
};

struct win32_sound_output
{
    int SamplesPerSecond = 48000;
    int   BytesPerSample      = sizeof(s16) * 2;
    u32   RunningSampleIndex  = 0;
    DWORD SecondaryBufferSize = (DWORD) (BytesPerSample * SamplesPerSecond);
    DWORD SafetyBytes = 0;
};

struct win32_debug_time_marker
{
    DWORD OutputPlayCursor;
    DWORD OutputWriteCursor;
    DWORD OutputLocation;
    DWORD OutputBytesCount;
    DWORD ExpectedFlipPlayCursor;

    DWORD FlipPlayCursor;
    DWORD FlipWriteCursor;
};

struct win32_game_code
{
    HMODULE GameCodeDLL;
    FILETIME DLLLastWriteTime;
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;
    b32                    IsValid;
};

struct win32_replay_buffer
{
    HANDLE FileHandle;
    HANDLE MemoryMap;
    char Filename[MAX_PATH];
    void *MemoryBlock;
};

struct win32_state
{
    u64  TotalSize;
    void *GameMemoryBlock;
    win32_replay_buffer ReplayBuffers[4];

    HANDLE RecordingHandle;
    int InputRecordingIndex;

    HANDLE PlaybackHandle;
    int InputPlayingIndex;

    char EXEFilename[MAX_PATH];
    char *OnePastLastSlash;
};

#define HANDMADEHERO_WIN32_HANDMADE_H

#endif //HANDMADEHERO_WIN32_HANDMADE_H
