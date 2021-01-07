#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wnull-dereference"

#pragma ide diagnostic ignored "modernize-loop-convert"
#pragma clang diagnostic ignored "-Wwritable-strings"
#pragma ide diagnostic ignored "bugprone-suspicious-include"

#pragma ide diagnostic ignored "UnusedValue"
#pragma ide diagnostic ignored "UnusedLocalVariable"
#pragma ide diagnostic ignored "OCUnusedGlobalDeclarationInspection"
#pragma ide diagnostic ignored "OCUnusedMacroInspection"

//#pragma ide diagnostic ignored "cppcoreguidelines-pro-type-member-init"
#pragma ide diagnostic ignored "modernize-deprecated-headers"
#pragma ide diagnostic ignored "modernize-use-auto"
#pragma ide diagnostic ignored "modernize-use-nullptr"
#pragma ide diagnostic ignored "hicpp-signed-bitwise"

#pragma clang diagnostic ignored "-Wunknown-pragmas"

#include "handmade_platform.h"

#include <windows.h>
#include <xinput.h>
#include <dsound.h>
#include <stdio.h>

#include "win32_handmade.h"

global_variable bool32 GlobalPause;
global_variable bool GlobalRunning;
global_variable win32_offscreen_buffer GlobalBackBuffer;
global_variable LPDIRECTSOUNDBUFFER GlobalSecondaryBuffer;
global_variable int64 GlobalQPerfFrequency;
global_variable bool32 DEBUGGlobalShowCursor;
global_variable WINDOWPLACEMENT GlobalWindowPlacement = {sizeof(GlobalWindowPlacement)};

//NOTE:(rahul): XInputSET GET STATE
#define X_INPUT_GET_STATE(FunctionName) DWORD WINAPI FunctionName(DWORD dwUserIndex, XINPUT_STATE *pState)
#define X_INPUT_SET_STATE(FunctionName) DWORD WINAPI FunctionName(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)

typedef X_INPUT_GET_STATE(x_input_get_state);

typedef X_INPUT_SET_STATE(x_input_set_state);

X_INPUT_GET_STATE(XInputGetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}

X_INPUT_SET_STATE(XInputSetStateStub)
{
    return (ERROR_DEVICE_NOT_CONNECTED);
}

global_variable x_input_get_state *XInputGetState_ = XInputGetStateStub;
global_variable x_input_set_state *XInputSetState_ = XInputSetStateStub;

real32 Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold);

#define XInputGetState XInputGetState_
#define XInputSetState XInputSetState_

internal void
CatStrings(size_t SourceACount, char *SourceA,
           size_t SourceBCount, char *SourceB,
           size_t DestCount, char *Dest)
{
    for (size_t Index = 0; Index < SourceACount; ++Index)
    {
        *Dest++ = *SourceA++;
    }
    for (size_t Index = 0; Index < SourceBCount; ++Index)
    {
        *Dest++ = *SourceB++;
    }

    *Dest++ = 0;
}

internal int
StringLength(char *String)
{
    int Count = 0;
    while (*String++)
    {
        ++Count;
    }
    return (Count);
}

internal void
Win32GetEXEFilename(win32_state *Win32State)
{
    DWORD SizeOfFilename = GetModuleFileNameA(0, Win32State->EXEFilename, sizeof(Win32State->EXEFilename));
    for (char *Scan = Win32State->EXEFilename; *Scan; ++Scan)
    {
        if (*Scan == '\\')
        {
            Win32State->OnePastLastSlash = Scan + 1;
        }
    }
}

internal void
Win32BuildFullFilenamePath(win32_state *Win32State, char *Filename,
                           int DestCount, char *Dest)
{
    CatStrings(Win32State->OnePastLastSlash - Win32State->EXEFilename, Win32State->EXEFilename,
               StringLength(Filename), Filename,
               DestCount, Dest);
}

inline FILETIME Win32GetLastWriteTime(char *FileName)
{
    FILETIME LastWriteTime = {};

    WIN32_FILE_ATTRIBUTE_DATA Data;
    if (GetFileAttributesExA(FileName, GetFileExInfoStandard, &Data))
    {
        LastWriteTime = Data.ftLastWriteTime;
    }
    return (LastWriteTime);
}

internal win32_game_code
Win32LoadGameCode(char *SourceGameCodeDLLFullPath, char *TempGameCodeDLLFullPath, char *LockFilename)
{
    win32_game_code Result = {};

    WIN32_FILE_ATTRIBUTE_DATA Ignored;
    if (!GetFileAttributesExA(LockFilename, GetFileExInfoStandard, &Ignored))
    {
        Result.DLLLastWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
        CopyFile(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath, FALSE);
        Result.GameCodeDLL = LoadLibraryA(TempGameCodeDLLFullPath);
        if (Result.GameCodeDLL)
        {
            Result.UpdateAndRender =
                    (game_update_and_render *) GetProcAddress(Result.GameCodeDLL, "GameUpdateAndRender");
            Result.GetSoundSamples =
                    (game_get_sound_samples *) GetProcAddress(Result.GameCodeDLL, "GameGetSoundSamples");
            Result.IsValid = (Result.UpdateAndRender && Result.GetSoundSamples);
        }
    }

    if (!Result.IsValid)
    {
        Result.UpdateAndRender = 0;
        Result.GetSoundSamples = 0;
    }
    return Result;
}

internal void
Win32UnloadGameCode(win32_game_code *GameCode)
{
    if (GameCode->GameCodeDLL)
    {
        FreeLibrary(GameCode->GameCodeDLL);
        GameCode->GameCodeDLL = 0;
    }
    GameCode->IsValid = false;
    GameCode->UpdateAndRender = 0;
    GameCode->GetSoundSamples = 0;
}

internal void
Win32LoadXInput()
{
    HMODULE XInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!XInputLibrary)
    {
        XInputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    if (XInputLibrary)
    {
        XInputGetState = (x_input_get_state *) GetProcAddress(XInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *) GetProcAddress(XInputLibrary, "XInputSetState");
    }
}
// XInput SET GET STATE END

#define DIRECT_SOUND_CREATE(FunctionName) HRESULT WINAPI FunctionName(LPCGUID pcGuidDevice, LPDIRECTSOUND *ppDS, LPUNKNOWN pUnkOuter)

typedef DIRECT_SOUND_CREATE(direct_sound_create);

#pragma ide diagnostic ignored "modernize-use-auto"

internal void
Win32InitDSound(HWND Window, int32 SamplesPerSecond, int32 BufferSize)
{
    HMODULE DSoundLibrary = LoadLibraryA("dsound.dll");

    if (DSoundLibrary)
    {
        direct_sound_create *DirectSoundCreate = (direct_sound_create *)
                GetProcAddress(DSoundLibrary, "DirectSoundCreate");

        WAVEFORMATEX WaveFormat = {};
        WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
        WaveFormat.nChannels = 2;
        WaveFormat.nSamplesPerSec = SamplesPerSecond;
        WaveFormat.wBitsPerSample = 16;
        WaveFormat.nBlockAlign = (WaveFormat.nChannels * WaveFormat.wBitsPerSample) / 8;
        WaveFormat.nAvgBytesPerSec = WaveFormat.nBlockAlign * WaveFormat.nSamplesPerSec;
        WaveFormat.cbSize = 0;

        LPDIRECTSOUND DirectSound;
        if (DirectSoundCreate && SUCCEEDED(DirectSoundCreate(0, &DirectSound, 0)))
        {

            if (SUCCEEDED(DirectSound->SetCooperativeLevel(Window, DSSCL_PRIORITY)))
            {
                DSBUFFERDESC BufferDescription = {};
                BufferDescription.dwSize = sizeof(BufferDescription);
                BufferDescription.dwFlags = DSBCAPS_PRIMARYBUFFER;
                LPDIRECTSOUNDBUFFER PrimaryBuffer;

                if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, &PrimaryBuffer, 0)))
                {
                    if (SUCCEEDED(PrimaryBuffer->SetFormat(&WaveFormat)))
                    {
                        OutputDebugStringA("PrimaryBuffer READY\n");
                    } else
                    {
                        //TODO(rahul): Diagnostic
                    }
                } else
                {
                    //TODO(rahul): Diagnostic
                }
            } else
            {
                //TODO(rahul): Diagnostic
            }

            DSBUFFERDESC BufferDescription = {};
            BufferDescription.dwSize = sizeof(BufferDescription);
            BufferDescription.dwFlags = 0;
            BufferDescription.dwBufferBytes = BufferSize;
            BufferDescription.lpwfxFormat = &WaveFormat;

            if (SUCCEEDED(DirectSound->CreateSoundBuffer(&BufferDescription, // NOLINT(bugprone-branch-clone)
                                                         &GlobalSecondaryBuffer,
                                                         0)))
            {
            } else
            {
                //TODO(rahul): Diagnostic
            }
        } else
        {
            //TODO(rahul): Diagnostic
        }
    } else
    {
        //TODO(rahul): Diagnostic
    }
}

internal win32_window_dimension
Win32GetWindowDimension(HWND Window)
{

    win32_window_dimension Result = {};
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}

internal void
Win32ResizeDIBSection(win32_offscreen_buffer *Buffer, int Width, int Height)
{

    if (Buffer->Memory)
    {
        VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
    }

    Buffer->Width = Width;
    Buffer->Height = Height;
    Buffer->BytesPerPixel = 4;
    Buffer->Pitch = Buffer->BytesPerPixel * Width;

    Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
    Buffer->Info.bmiHeader.biWidth = Buffer->Width;
    Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
    Buffer->Info.bmiHeader.biPlanes = 1;
    Buffer->Info.bmiHeader.biBitCount = 32;
    Buffer->Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (Width * Height) * Buffer->BytesPerPixel;
    Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
}

internal void
Win32DisplayBufferInWindow(win32_offscreen_buffer *Buffer, HDC DeviceContext, int WindowWidth, int WindowHeight)
{
    if ((WindowWidth >= Buffer->Width * 2) && WindowHeight >= Buffer->Height * 2)
    {
        StretchDIBits(DeviceContext,
                      0, 0, WindowWidth, WindowHeight,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    } else
    {
        int OffsetX = 10;
        int OffsetY = 10;

        PatBlt(DeviceContext, 0, 0, OffsetX, WindowHeight, BLACKNESS);
        PatBlt(DeviceContext, 0, 0, WindowWidth, OffsetY, BLACKNESS);
        PatBlt(DeviceContext, 0, Buffer->Height + OffsetY, WindowWidth, WindowHeight - Buffer->Height, BLACKNESS);
        PatBlt(DeviceContext, Buffer->Width + OffsetX, 0, WindowWidth - Buffer->Width, WindowHeight, BLACKNESS);

        StretchDIBits(DeviceContext,
                      OffsetX, OffsetY, Buffer->Width, Buffer->Height,
                      0, 0, Buffer->Width, Buffer->Height,
                      Buffer->Memory,
                      &Buffer->Info,
                      DIB_RGB_COLORS, SRCCOPY);
    }
}

internal LRESULT CALLBACK
Win32MainWindowCallback(
        HWND Window,
        UINT Message,
        WPARAM wParam,
        LPARAM lParam
)
{

    LRESULT Result = 0;
    switch (Message)
    {
        case WM_ACTIVATE:
        {

#if 0
            if (wParam == TRUE)
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 255, LWA_ALPHA);
            } else
            {
                SetLayeredWindowAttributes(Window, RGB(0, 0, 0), 128, LWA_ALPHA);
            }
#endif
        }
            break;
        case WM_SETCURSOR:
        {
            if (DEBUGGlobalShowCursor)
            {
                Result = DefWindowProcA(Window, Message, wParam, lParam);
            } else
            {
                SetCursor(0);
            }
        }
            break;

        case WM_SIZE:
        {
            OutputDebugStringA("WM_SIZE\n");
        }
            break;

        case WM_DESTROY:
        {
            GlobalRunning = false;
            OutputDebugStringA("WM_DESTROY\n");
        }
            break;

        case WM_CLOSE:
        {
            GlobalRunning = false;
            OutputDebugStringA("WM_CLOSE\n");
        }
            break;

        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(Window, &Paint);

            win32_window_dimension Dimension = Win32GetWindowDimension(Window);

            Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
            EndPaint(Window, &Paint);
        }
            break;

        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
        case WM_KEYUP:
        case WM_KEYDOWN:
        {
            Assert("Keyboard Message passed through WindowCallbackFunction!!!")
        }
            break;
        default:
        {
            Result = DefWindowProcA(Window, Message, wParam, lParam);
        }
            break;
    }

    return (Result);
}

internal void
Win32FillSoundBuffer(win32_sound_output *SoundOutput, DWORD BytesToLock, DWORD BytesToWrite,
                     game_sound_output_buffer *SourceBuffer)
{
    VOID *Region1;
    VOID *Region2;
    DWORD Region1Size;
    DWORD Region2Size;

    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(BytesToLock, BytesToWrite,
                                              &Region1, &Region1Size,
                                              &Region2, &Region2Size,
                                              0)))
    {
        DWORD Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
        int16 *DestSample = (int16 *) Region1;
        int16 *SourceSample = SourceBuffer->Samples;
        for (DWORD SampleIndex = 0; SampleIndex < Region1SampleCount; ++SampleIndex)
        {
            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;

            SoundOutput->RunningSampleIndex++;
        }

        DWORD Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
        DestSample = (int16 *) Region2;
        for (DWORD SampleIndex = 0; SampleIndex < Region2SampleCount; ++SampleIndex)
        {

            *DestSample++ = *SourceSample++;
            *DestSample++ = *SourceSample++;

            SoundOutput->RunningSampleIndex++;
        }

        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32ClearSoundBuffer(win32_sound_output *SoundOutput)
{
    VOID *Region1;
    VOID *Region2;
    DWORD Region1Size;
    DWORD Region2Size;

    if (SUCCEEDED(GlobalSecondaryBuffer->Lock(0, SoundOutput->SecondaryBufferSize,
                                              &Region1, &Region1Size,
                                              &Region2, &Region2Size,
                                              0)))
    {
        uint8 *SampleOut = (uint8 *) Region1;
        for (DWORD ByteIndex = 0; ByteIndex < Region1Size; ++ByteIndex)
        {
            *SampleOut++ = 0;
        }

        SampleOut = (uint8 *) Region2;
        for (DWORD ByteIndex = 0; ByteIndex < Region2Size; ++ByteIndex)
        {
            *SampleOut++ = 0;
        }
        GlobalSecondaryBuffer->Unlock(Region1, Region1Size, Region2, Region2Size);
    }
}

internal void
Win32ProcessXInputDigitalButton(DWORD XInputButtonState,
                                game_button_state *OldState, DWORD ButtonBit,
                                game_button_state *NewState)
{
    NewState->EndedDown = ((XInputButtonState & ButtonBit) == ButtonBit);
    NewState->HalfTransitionCount = (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

real32 Win32ProcessXInputStickValue(SHORT Value, SHORT DeadZoneThreshold)
{
    real32 Result = 0;
    if (Value < -DeadZoneThreshold)
    {
        Result = ((real32) (Value + DeadZoneThreshold) / (32768.0f - (real32) DeadZoneThreshold));
    } else if (Value > DeadZoneThreshold)
    {
        Result = ((real32) (Value - DeadZoneThreshold) / (32767.0f - (real32) DeadZoneThreshold));
    }
    return Result;
}

internal void
Win32ProcessKeyboardMessage(game_button_state *NewState, bool32 IsDown)
{
    if (NewState->EndedDown != IsDown)
    {
        NewState->EndedDown = IsDown;
        ++NewState->HalfTransitionCount;
    }
}

DEBUG_PLATFORM_FREE_FILE_MEMORY(DEBUGPlatformFreeFileMemory)
{
    if (Memory)
    {
        VirtualFree(Memory, 0, MEM_RELEASE);
    }
}

DEBUG_PLATFORM_READ_ENTIRE_FILE(DEBUGPlatformReadEntireFile)
{
    debug_read_file_result Result = {};

    HANDLE FileHandle = CreateFileA(Filename, GENERIC_READ, FILE_SHARE_READ,
                                    0, OPEN_EXISTING, 0, 0);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        LARGE_INTEGER FileSize;
        if (GetFileSizeEx(FileHandle, &FileSize))
        {
            uint32 FileSize32 = SafeTruncateUInt64(FileSize.QuadPart);
            Result.Contents = VirtualAlloc(0, FileSize32,
                                           MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            if (Result.Contents)
            {
                DWORD BytesRead;
                if (ReadFile(FileHandle, Result.Contents, FileSize32, &BytesRead, 0)
                    && (FileSize32 == BytesRead))
                {
                    Result.ContentsSize = FileSize32;
                } else
                {
                    DEBUGPlatformFreeFileMemory(Thread, Result.Contents);
                    Result.Contents = 0;
                }
            } else
            {
                //TODO(rahul): Logging
            }
        } else
        {
            //TODO(rahul): Logging
        }

        CloseHandle(FileHandle);
    } else
    {
        //TODO(rahul): Logging
    }
    return (Result);
}

DEBUG_PLATFORM_WRITE_ENTIRE_FILE(DEBUGPlatformWriteEntireFile)
{
    bool32 Result = false;
    HANDLE FileHandle = CreateFileA(Filename, GENERIC_WRITE,
                                    0, 0,
                                    CREATE_ALWAYS, 0, 0);

    if (FileHandle != INVALID_HANDLE_VALUE)
    {
        DWORD BytesWritten;
        if (WriteFile(FileHandle, Memory, FileSize, &BytesWritten, 0))
        {
            Result = (BytesWritten == FileSize);
        } else
        {
            //TODO(rahul): Logging
        }

        CloseHandle(FileHandle);
    } else
    {
        //TODO(rahul): Logging
    }
    return (Result);
}

inline LARGE_INTEGER
Win32GetWallClock()
{
    LARGE_INTEGER Result;
    QueryPerformanceCounter(&Result);
    return Result;
}

inline real32 Win32GetSecondsElapsed(LARGE_INTEGER Start, LARGE_INTEGER End)
{
    int64 CounterElapsed = End.QuadPart - Start.QuadPart;
    real32 Result = ((real32) CounterElapsed / (real32) GlobalQPerfFrequency);
    return Result;
}

#if 0
internal void
Win32DebugDrawVertical(win32_offscreen_buffer *BackBuffer, int X, int Top, int Bottom, uint32 Color)
{
    if (Top <= 0)
    {
        Top = 0;
    }
    if (Bottom > BackBuffer->Height)
    {
        Bottom = BackBuffer->Height;
    }

    if ((X >= 0) && (X < BackBuffer->Width))
    {
        uint8 *Pixel = (uint8 *) BackBuffer->Memory +
                       X * BackBuffer->BytesPerPixel +
                       Top * BackBuffer->Pitch;

        for (int Y = Top; Y < Bottom; ++Y)
        {
            *(uint32 *) Pixel = Color;
            Pixel += BackBuffer->Pitch;
        }
    }
}

#pragma ide diagnostic ignored "readability-non-const-parameter"
internal void
Win32DrawSoundBufferMarker(win32_offscreen_buffer *BackBuffer, win32_sound_output *SoundOutput,
                           int PadX, int Top, int Bottom, real32 C, DWORD Value, uint32 Color)
{
    int X = PadX + (int) (C * (real32) Value);
    Win32DebugDrawVertical(BackBuffer, X, Top, Bottom, Color);
}

internal void
Win32DebugSyncDisplay(win32_offscreen_buffer *BackBuffer, int MarkerCount, int CurrentMarkerIndex,
                      win32_debug_time_marker *Markers,
                      win32_sound_output *SoundOutput, real32 TargetSecondsPerFrame)
{
    int PadX = 16;
    int PadY = 16;

    int LineHeight = 64;

    real32 C = (real32) (BackBuffer->Width - 2 * PadX) / ((real32) SoundOutput->SecondaryBufferSize);
    for (int MarkerIndex = 0; MarkerIndex < MarkerCount; ++MarkerIndex)
    {
        win32_debug_time_marker ThisMarker = Markers[MarkerIndex];

        Assert(ThisMarker.OutputLocation < SoundOutput->SecondaryBufferSize)
        Assert(ThisMarker.OutputBytesCount < SoundOutput->SecondaryBufferSize)
        Assert(ThisMarker.OutputPlayCursor < SoundOutput->SecondaryBufferSize)
        Assert(ThisMarker.OutputWriteCursor < SoundOutput->SecondaryBufferSize)
        Assert(ThisMarker.FlipPlayCursor < SoundOutput->SecondaryBufferSize)
        Assert(ThisMarker.FlipWriteCursor < SoundOutput->SecondaryBufferSize)

        DWORD PlayColor = 0xFFFFFFFF;
        DWORD WriteColor = 0xFFFF0000;
        DWORD ExpectedFlipColor = 0xFFFFFF00;
        DWORD PlayWindowColor = 0xFFFF00FF;

        int Top = PadY;
        int Bottom = PadY + LineHeight;
        if (MarkerIndex == CurrentMarkerIndex)
        {
            Top += PadY + LineHeight;
            Bottom += PadY + LineHeight;

            int FirstTop = Top;

            Win32DrawSoundBufferMarker(
                    BackBuffer, SoundOutput, PadX, Top, Bottom, C, ThisMarker.OutputPlayCursor, PlayColor);
            Win32DrawSoundBufferMarker(
                    BackBuffer, SoundOutput, PadX, Top, Bottom, C, ThisMarker.OutputWriteCursor, WriteColor);

            Top += PadY + LineHeight;
            Bottom += PadY + LineHeight;
            Win32DrawSoundBufferMarker(
                    BackBuffer, SoundOutput, PadX, Top, Bottom, C, ThisMarker.OutputLocation, PlayColor);
            Win32DrawSoundBufferMarker(
                    BackBuffer, SoundOutput, PadX, Top, Bottom, C,
                    (ThisMarker.OutputLocation + ThisMarker.OutputBytesCount), WriteColor);

            Top += PadY + LineHeight;
            Bottom += PadY + LineHeight;

            Win32DrawSoundBufferMarker(
                    BackBuffer, SoundOutput, PadX, FirstTop, Bottom, C, ThisMarker.ExpectedFlipPlayCursor,
                    ExpectedFlipColor);
        }
        Win32DrawSoundBufferMarker(
                BackBuffer, SoundOutput, PadX, Top, Bottom, C, ThisMarker.FlipPlayCursor, PlayColor);
        Win32DrawSoundBufferMarker(
                BackBuffer, SoundOutput, PadX, Top, Bottom, C,
                ThisMarker.FlipPlayCursor + 480 * SoundOutput->BytesPerSample, PlayWindowColor);
        Win32DrawSoundBufferMarker(
                BackBuffer, SoundOutput, PadX, Top, Bottom, C, ThisMarker.FlipWriteCursor, WriteColor);
    }
}
#endif

internal void
Win32GetInputFileLocation(win32_state *Win32State, bool32 InputStream, int SlotIndex, int DestCount, char *Dest)
{
    char Temp[64];
    wsprintf(Temp, "loop_edit_%d_%s.hmi", SlotIndex, InputStream ? "input" : "state");
    Win32BuildFullFilenamePath(Win32State, Temp, DestCount, Dest);
}

internal win32_replay_buffer *
Win32GetReplayBuffer(win32_state *Win32State, int unsigned Index)
{
    Assert(Index < ArrayCount(Win32State->ReplayBuffers))
    win32_replay_buffer *Result = &Win32State->ReplayBuffers[Index];
    return (Result);
}

internal void
Win32BeginRecordingInput(win32_state *Win32State, int InputRecordingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(Win32State, InputRecordingIndex);
    if (ReplayBuffer->MemoryBlock)
    {
        Win32State->InputRecordingIndex = InputRecordingIndex;
        Win32State->RecordingHandle = ReplayBuffer->FileHandle;

        char Filename[MAX_PATH];
        Win32GetInputFileLocation(Win32State, true, InputRecordingIndex, sizeof(Filename), Filename);
        Win32State->RecordingHandle = CreateFileA(Filename, GENERIC_WRITE,
                                                  0, 0,
                                                  CREATE_ALWAYS, 0, 0);
#if 0
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = Win32State->TotalSize;
        SetFilePointerEx(Win32State->RecordingHandle, FilePosition, 0, FILE_BEGIN);
#endif
        CopyMemory(ReplayBuffer->MemoryBlock, Win32State->GameMemoryBlock, (size_t) Win32State->TotalSize);
    }
}

internal void
Win32EndRecordingInput(win32_state *Win32State)
{
    CloseHandle(Win32State->RecordingHandle);
    Win32State->InputRecordingIndex = 0;
}

internal void
Win32RecordInput(win32_state *Win32State, game_input *Input)
{
    DWORD BytesWritten;
    WriteFile(Win32State->RecordingHandle, Input, sizeof(*Input), &BytesWritten, 0);
}

internal void
Win32BeginPlaybackInput(win32_state *Win32State, int InputPlayingIndex)
{
    win32_replay_buffer *ReplayBuffer = Win32GetReplayBuffer(Win32State, InputPlayingIndex);
    if (ReplayBuffer->MemoryBlock)
    {
        Win32State->InputPlayingIndex = InputPlayingIndex;
        Win32State->RecordingHandle = ReplayBuffer->FileHandle;
        char Filename[MAX_PATH];
        Win32GetInputFileLocation(Win32State, true, InputPlayingIndex, sizeof(Filename), Filename);
        Win32State->PlaybackHandle = CreateFileA(Filename, GENERIC_READ,
                                                 0, 0,
                                                 OPEN_EXISTING, 0, 0);
#if 0
        LARGE_INTEGER FilePosition;
        FilePosition.QuadPart = Win32State->TotalSize;
        SetFilePointerEx(Win32State->PlaybackHandle, FilePosition, 0, FILE_BEGIN);
#endif
        CopyMemory(Win32State->GameMemoryBlock, ReplayBuffer->MemoryBlock, (size_t) Win32State->TotalSize);
    }
}

internal void
Win32EndPlaybackInput(win32_state *Win32State)
{
    CloseHandle(Win32State->PlaybackHandle);
    Win32State->InputPlayingIndex = 0;
}

internal void
Win32PlayBackInput(win32_state *Win32State, game_input *Input)
{
    DWORD BytesRead;
    if (ReadFile(Win32State->PlaybackHandle, Input, sizeof(*Input), &BytesRead, 0))
    {
        if (BytesRead == 0)
        {
            int PlayingIndex = Win32State->InputPlayingIndex;
            Win32EndPlaybackInput(Win32State);
            Win32BeginPlaybackInput(Win32State, PlayingIndex);
            ReadFile(Win32State->PlaybackHandle, Input, sizeof(*Input), &BytesRead, 0);
        }
    }
}

internal void
ToggleFullScreen(HWND Window)
{
    DWORD Style = GetWindowLong(Window, GWL_STYLE);
    if (Style & WS_OVERLAPPEDWINDOW)
    {
        MONITORINFO MonitorInfo = {sizeof(MonitorInfo)};
        if (GetWindowPlacement(Window, &GlobalWindowPlacement) &&
            GetMonitorInfo(MonitorFromWindow(Window, MONITOR_DEFAULTTOPRIMARY), &MonitorInfo))
        {
            SetWindowLong(Window, GWL_STYLE, Style & ~WS_OVERLAPPEDWINDOW);
            SetWindowPos(Window, HWND_TOP,
                         MonitorInfo.rcMonitor.left, MonitorInfo.rcMonitor.top,
                         MonitorInfo.rcMonitor.right - MonitorInfo.rcMonitor.left,
                         MonitorInfo.rcMonitor.bottom - MonitorInfo.rcMonitor.top,
                         SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
        }
    } else
    {
        SetWindowLong(Window, GWL_STYLE,
                      Style | WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(Window, &GlobalWindowPlacement);
        SetWindowPos(Window, NULL, 0, 0, 0, 0,
                     SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER |
                     SWP_NOOWNERZORDER | SWP_FRAMECHANGED);
    }
}

internal void
Win32ProcessPendingMessages(win32_state *Win32State, game_controller_input *KeyboardController)
{
    MSG Message;
    while (PeekMessage(&Message, 0, 0, 0, PM_REMOVE))
    {
        if (Message.message == WM_QUIT)
        {
            GlobalRunning = false;
        }

        switch (Message.message)
        {
            case WM_SYSKEYDOWN:
            case WM_SYSKEYUP:
            case WM_KEYUP:
            case WM_KEYDOWN:
            {
                bool WasDown = ((Message.lParam & (1 << 30)) != 0);
                bool IsDown = ((Message.lParam & (1 << 31)) == 0);

                uint32 VKCode = (uint32) Message.wParam;

                if (WasDown != IsDown)
                {

                    if (VKCode == 'W')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveUp, IsDown);
                    } else if (VKCode == 'A')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveLeft, IsDown);
                    } else if (VKCode == 'S')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveDown, IsDown);
                    } else if (VKCode == 'D')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->MoveRight, IsDown);
                    } else if (VKCode == 'Q')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->LeftShoulder, IsDown);
                    } else if (VKCode == 'E')
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->RightShoulder, IsDown);
                    } else if (VKCode == VK_UP)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->YButton, IsDown);
                    } else if (VKCode == VK_DOWN)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->AButton, IsDown);
                    } else if (VKCode == VK_LEFT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->XButton, IsDown);
                    } else if (VKCode == VK_RIGHT)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->BButton, IsDown);
                    } else if (VKCode == VK_ESCAPE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Back, IsDown);
                    } else if (VKCode == VK_SPACE)
                    {
                        Win32ProcessKeyboardMessage(&KeyboardController->Start, IsDown);
                    }
#if HANDMADE_INTERNAL
                    else if (VKCode == 'G')
                    {
                        if (IsDown)
                        {
                            GlobalPause = !GlobalPause;
                        }
                    } else if (VKCode == 'L')
                    {
                        if (IsDown)
                        {
                            if (Win32State->InputPlayingIndex == 0)
                            {
                                if (Win32State->InputRecordingIndex == 0)
                                {
                                    Win32BeginRecordingInput(Win32State, 1);
                                } else
                                {
                                    Win32EndRecordingInput(Win32State);
                                    Win32BeginPlaybackInput(Win32State, 1);
                                }
                            } else
                            {
                                Win32EndPlaybackInput(Win32State);
                            }
                        }
                    }
#endif
                    bool32 AltKeyWasDown = (Message.lParam & (1 << 29));

                    if ((VKCode == VK_F4) && AltKeyWasDown)
                    {
                        GlobalRunning = false;
                    }
                    if (IsDown)
                    {
                        if ((VKCode == VK_RETURN) && AltKeyWasDown)
                        {
                            if (Message.hwnd)
                            {
                                ToggleFullScreen(Message.hwnd);
                            }
                        }
                    }
                }
            }
                break;
        }
        TranslateMessage(&Message);
        DispatchMessageA(&Message);
    }
}

int CALLBACK
WinMain(
        HINSTANCE hInstance,
        HINSTANCE hPrevInstance,
        LPSTR lpCmdLine,
        int nShowCmd
)
{
    win32_state Win32State = {};

    Win32GetEXEFilename(&Win32State);

    char SourceGameCodeDLLFullPath[MAX_PATH];
    Win32BuildFullFilenamePath(&Win32State, "handmade.dll",
                               sizeof(SourceGameCodeDLLFullPath) - 1, SourceGameCodeDLLFullPath);

    char TempGameCodeDLLFullPath[MAX_PATH];
    Win32BuildFullFilenamePath(&Win32State, "handmade_temp.dll",
                               sizeof(TempGameCodeDLLFullPath) - 1, TempGameCodeDLLFullPath);

    char GameCodeLockFullPath[MAX_PATH];
    Win32BuildFullFilenamePath(&Win32State, "lock.tmp",
                               sizeof(GameCodeLockFullPath) - 1, GameCodeLockFullPath);
    LARGE_INTEGER QPerfFrequencyResult;
    QueryPerformanceFrequency(&QPerfFrequencyResult);
    GlobalQPerfFrequency = QPerfFrequencyResult.QuadPart;
    bool32 IsSleepGranular = (timeBeginPeriod(1) == TIMERR_NOERROR);

    Win32LoadXInput();

#if HANDMADE_INTERNAL
    DEBUGGlobalShowCursor = true;
#endif

    WNDCLASS WindowClass = {};
    Win32ResizeDIBSection(&GlobalBackBuffer, 960, 540);

    WindowClass.style = CS_VREDRAW | CS_HREDRAW;
    WindowClass.lpfnWndProc = Win32MainWindowCallback;
    WindowClass.hInstance = hInstance;
    WindowClass.hCursor = LoadCursorA(0, IDC_ARROW);
//        HICON     hIcon;
    WindowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClass(&WindowClass))
    {
        HWND Window = CreateWindowEx(
                0, //WS_EX_LAYERED | WS_EX_TOPMOST,
                WindowClass.lpszClassName,
                "HandmadeHero",
                WS_OVERLAPPEDWINDOW | WS_VISIBLE,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                hInstance,
                0);

        if (Window)
        {
            HDC DeviceContext = GetDC(Window);

            int MonitorRefreshHz = 60;
            int Win32RefreshRate = GetDeviceCaps(DeviceContext, VREFRESH);
            if (Win32RefreshRate > 1)
            {
                MonitorRefreshHz = Win32RefreshRate;
            }
            real32 GameUpdateHz = ((real32) MonitorRefreshHz / 2.0f);
            real32 TargetSecondsPerFrame = (1.0f / ((real32) GameUpdateHz));

            win32_sound_output SoundOutput = {};
            SoundOutput.SafetyBytes = (int) ((real32) SoundOutput.SecondaryBufferSize / GameUpdateHz) / 3;
            Win32InitDSound(Window, SoundOutput.SamplesPerSecond, SoundOutput.SecondaryBufferSize);
            Win32ClearSoundBuffer(&SoundOutput);
            GlobalSecondaryBuffer->Play(0, 0, DSBPLAY_LOOPING);

            GlobalRunning = true;

#if 0
            while (GlobalRunning)
            {
                DWORD FlipPlayCursor;
                DWORD FlipWriteCursor;
                GlobalSecondaryBuffer->GetCurrentPosition(&FlipPlayCursor, &FlipWriteCursor);

                char TextBuffer[256];
                sprintf_s(TextBuffer, sizeof(TextBuffer),
                          "PC:%u WC:%u\n", FlipPlayCursor, FlipWriteCursor);
                OutputDebugStringA(TextBuffer);
            }
#endif

            int16 *Samples = (int16 *) VirtualAlloc(0, SoundOutput.SecondaryBufferSize,
                                                    MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);

#if HANDMADE_INTERNAL
            LPVOID BaseAddress = (LPVOID) Terabytes((uint64) 2);
#else
            LPVOID BaseAddress = 0;
#endif

            game_memory GameMemory = {};
            GameMemory.PermanentStorageSize = Megabytes((uint64) 64);
            GameMemory.TransientStorageSize = Megabytes((uint64) 128);
            GameMemory.DEBUGPlatformReadEntireFile = DEBUGPlatformReadEntireFile;
            GameMemory.DEBUGPlatformWriteEntireFile = DEBUGPlatformWriteEntireFile;
            GameMemory.DEBUGPlatformFreeFileMemory = DEBUGPlatformFreeFileMemory;

            Win32State.TotalSize = GameMemory.PermanentStorageSize + GameMemory.TransientStorageSize;
            Win32State.GameMemoryBlock = VirtualAlloc(BaseAddress, (size_t) Win32State.TotalSize,
                                                      MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
            GameMemory.PermanentStorage = Win32State.GameMemoryBlock;
            GameMemory.TransientStorage = ((uint8 *) GameMemory.PermanentStorage + GameMemory.PermanentStorageSize);

            for (int ReplayIndex = 0;
                 ReplayIndex < ArrayCount(Win32State.ReplayBuffers);
                 ++ReplayIndex)
            {
                win32_replay_buffer *ReplayBuffer = &Win32State.ReplayBuffers[ReplayIndex];

                Win32GetInputFileLocation(&Win32State, false, ReplayIndex,
                                          sizeof(ReplayBuffer->Filename), ReplayBuffer->Filename);

                ReplayBuffer->FileHandle = CreateFileA(ReplayBuffer->Filename,
                                                       GENERIC_WRITE | GENERIC_READ,
                                                       0, 0,
                                                       CREATE_ALWAYS, 0, 0);

                DWORD MaxSizeHigh = Win32State.TotalSize >> 32;
                DWORD MaxSizeLow = Win32State.TotalSize & 0xFFFFFFFF;
                ReplayBuffer->MemoryMap = CreateFileMapping(ReplayBuffer->FileHandle,
                                                            0, PAGE_READWRITE,
                                                            MaxSizeHigh, MaxSizeLow,
                                                            0);

                ReplayBuffer->MemoryBlock = MapViewOfFile(ReplayBuffer->MemoryMap,
                                                          FILE_MAP_ALL_ACCESS,
                                                          0, 0,
                                                          (size_t) Win32State.TotalSize);
                if (!ReplayBuffer->MemoryBlock) // NOLINT(bugprone-branch-clone)
                {
                } else
                {
                    //TODO(rahul): Diagnostic
                }
            }

            if (Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
            {
                game_input Input[2] = {};
                game_input *NewInput = &Input[0];
                game_input *OldInput = &Input[1];

                LARGE_INTEGER LastCounter = Win32GetWallClock();
                LARGE_INTEGER FlipWallClock = Win32GetWallClock();

                int DebugTimeMarkerIndex = 0;
                win32_debug_time_marker DebugTimeMarker[30] = {0};

                bool32 SoundIsValid = false;
                DWORD AudioLatencyBytes = 0;
                real32 AudioLatencySeconds = 0;

                win32_game_code Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath,
                                                         GameCodeLockFullPath);

                uint64 LastCycleCount = __rdtsc();
                while (GlobalRunning)
                {
                    NewInput->dtForFrame = TargetSecondsPerFrame;

                    FILETIME NewDLLWriteTime = Win32GetLastWriteTime(SourceGameCodeDLLFullPath);
                    if (CompareFileTime(&NewDLLWriteTime, &Game.DLLLastWriteTime) != 0)
                    {
                        Win32UnloadGameCode(&Game);
                        Game = Win32LoadGameCode(SourceGameCodeDLLFullPath, TempGameCodeDLLFullPath,
                                                 GameCodeLockFullPath);
                    }

                    game_controller_input *NewKeyboardController = GetController(NewInput, 0);
                    game_controller_input *OldKeyboardController = GetController(OldInput, 0);

                    game_controller_input ZeroController = {};
                    *NewKeyboardController = ZeroController;

                    NewKeyboardController->IsConnected = true;
                    for (DWORD ButtonIndex = 0; ButtonIndex < ArrayCount(NewKeyboardController->Buttons); ++ButtonIndex)
                    {
                        NewKeyboardController->Buttons[ButtonIndex].EndedDown =
                                OldKeyboardController->Buttons[ButtonIndex].EndedDown;
                    }

                    Win32ProcessPendingMessages(&Win32State, NewKeyboardController);

                    if (!GlobalPause)
                    {
                        POINT MouseP;
                        GetCursorPos(&MouseP);
                        ScreenToClient(Window, &MouseP);
                        NewInput->MouseX = MouseP.x;
                        NewInput->MouseY = MouseP.y;
                        NewInput->MouseZ = 0;
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[0],
                                                    GetKeyState(VK_LBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[1],
                                                    GetKeyState(VK_MBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[2],
                                                    GetKeyState(VK_RBUTTON) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[3],
                                                    GetKeyState(VK_XBUTTON1) & (1 << 15));
                        Win32ProcessKeyboardMessage(&NewInput->MouseButtons[4],
                                                    GetKeyState(VK_XBUTTON2) & (1 << 15));

                        DWORD MaxControllerCount = XUSER_MAX_COUNT;
                        if (MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
                        {
                            MaxControllerCount = (ArrayCount(NewInput->Controllers) - 1);
                        }
                        for (DWORD ControllerIndex = 0;
                             ControllerIndex < MaxControllerCount;
                             ++ControllerIndex)
                        {
                            DWORD OurControllerIndex = ControllerIndex + 1;
                            game_controller_input *NewController = GetController(NewInput, OurControllerIndex);
                            game_controller_input *OldController = GetController(OldInput, OurControllerIndex);

                            XINPUT_STATE ControllerState;
                            if (XInputGetState(ControllerIndex, &ControllerState) == ERROR_SUCCESS)
                            {
                                NewController->IsConnected = true;
                                NewController->IsAnalog = OldController->IsAnalog;

                                XINPUT_GAMEPAD *Pad = &ControllerState.Gamepad;

                                NewController->StickAverageX =
                                        Win32ProcessXInputStickValue(Pad->sThumbLX,
                                                                     XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);
                                NewController->StickAverageY =
                                        Win32ProcessXInputStickValue(Pad->sThumbLY,
                                                                     XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE);

                                if (NewController->StickAverageX != 0.0f || NewController->StickAverageY != 0.0f)
                                {
                                    NewController->IsAnalog = true;
                                }
                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_UP)
                                {
                                    NewController->StickAverageY = 1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
                                {
                                    NewController->StickAverageY = -1.0f;
                                    NewController->IsAnalog = false;
                                }

                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
                                {
                                    NewController->StickAverageX = -1.0f;
                                    NewController->IsAnalog = false;
                                }
                                if (Pad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
                                {
                                    NewController->StickAverageX = 1.0f;
                                    NewController->IsAnalog = false;
                                }

                                real32 Threshold = 0.5f;

                                Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageX < -Threshold) ? 1 : 0,
                                        &OldController->MoveLeft, 1, &NewController->MoveLeft);
                                Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageX > Threshold) ? 1 : 0,
                                        &OldController->MoveRight, 1, &NewController->MoveRight);

                                Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageY < -Threshold) ? 1 : 0,
                                        &OldController->MoveLeft, 1, &NewController->MoveLeft);
                                Win32ProcessXInputDigitalButton(
                                        (NewController->StickAverageY > Threshold) ? 1 : 0,
                                        &OldController->MoveRight, 1, &NewController->MoveRight);

                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->AButton,
                                                                XINPUT_GAMEPAD_A, &NewController->AButton);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->BButton,
                                                                XINPUT_GAMEPAD_B, &NewController->BButton);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->XButton,
                                                                XINPUT_GAMEPAD_X, &NewController->XButton);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->YButton,
                                                                XINPUT_GAMEPAD_Y, &NewController->YButton);

                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->LeftShoulder,
                                                                XINPUT_GAMEPAD_LEFT_SHOULDER,
                                                                &NewController->LeftShoulder);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->RightShoulder,
                                                                XINPUT_GAMEPAD_RIGHT_SHOULDER,
                                                                &NewController->RightShoulder);

                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Start,
                                                                XINPUT_GAMEPAD_START, &NewController->Start);
                                Win32ProcessXInputDigitalButton(Pad->wButtons, &OldController->Back,
                                                                XINPUT_GAMEPAD_BACK, &NewController->Back);
                            } else
                            {
                                NewController->IsConnected = false;
                            }
                        }

                        thread_context Thread = {};

                        game_offscreen_buffer Buffer = {};
                        Buffer.Height = GlobalBackBuffer.Height;
                        Buffer.Width = GlobalBackBuffer.Width;
                        Buffer.Pitch = GlobalBackBuffer.Pitch;
                        Buffer.Memory = GlobalBackBuffer.Memory;
                        Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;

                        if (Win32State.InputRecordingIndex)
                        {
                            Win32RecordInput(&Win32State, NewInput);
                        }
                        if (Win32State.InputPlayingIndex)
                        {
                            Win32PlayBackInput(&Win32State, NewInput);
                        }
                        if (Game.UpdateAndRender)
                        {
                            Game.UpdateAndRender(&Thread, &GameMemory, NewInput, &Buffer);
                        }

                        LARGE_INTEGER AudioWallClock = Win32GetWallClock();
                        real32 FromBeginToAudioSeconds = Win32GetSecondsElapsed(FlipWallClock, AudioWallClock);

                        DWORD PlayCursor;
                        DWORD WriteCursor;
                        if (GlobalSecondaryBuffer->GetCurrentPosition(&PlayCursor, &WriteCursor) == DS_OK)
                        {

                            if (!SoundIsValid)
                            {
                                SoundOutput.RunningSampleIndex = WriteCursor / SoundOutput.BytesPerSample;
                                SoundIsValid = true;
                            }

                            DWORD BytesToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) %
                                                SoundOutput.SecondaryBufferSize;

                            DWORD ExpectedSoundBytesPerFrame = (int) ((real32) SoundOutput.SecondaryBufferSize /
                                                                      GameUpdateHz);

                            real32 SecondsLeftUntilFlip = TargetSecondsPerFrame - FromBeginToAudioSeconds;
                            DWORD ExpectedBytesUntilFlip =
                                    (DWORD) ((SecondsLeftUntilFlip / TargetSecondsPerFrame) *
                                             (real32) ExpectedSoundBytesPerFrame);

                            DWORD ExpectedFrameBoundaryByte = PlayCursor + ExpectedBytesUntilFlip;

                            DWORD SafeWriteCursor = WriteCursor;
                            if (WriteCursor < PlayCursor)
                            {
                                SafeWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            Assert(SafeWriteCursor >= PlayCursor)
                            SafeWriteCursor += SoundOutput.SafetyBytes;

                            bool32 AudioCardIsLowLatency = (SafeWriteCursor < ExpectedFrameBoundaryByte);
                            DWORD TargetCursor = 0;
                            if (AudioCardIsLowLatency)
                            {
                                TargetCursor = (ExpectedFrameBoundaryByte + ExpectedSoundBytesPerFrame);
                            } else
                            {
                                TargetCursor = (WriteCursor + ExpectedSoundBytesPerFrame + SoundOutput.SafetyBytes);
                            }
                            TargetCursor = (TargetCursor % SoundOutput.SecondaryBufferSize);

                            DWORD BytesToWrite = 0;
                            if (BytesToLock > TargetCursor)
                            {
                                BytesToWrite = SoundOutput.SecondaryBufferSize - BytesToLock;
                                BytesToWrite += TargetCursor;
                            } else
                            {
                                BytesToWrite = TargetCursor - BytesToLock;
                            }

                            game_sound_output_buffer SoundBuffer = {};
                            SoundBuffer.SamplesPerSecond = SoundOutput.SamplesPerSecond;
                            SoundBuffer.SampleCount = (int) BytesToWrite / SoundOutput.BytesPerSample;
                            SoundBuffer.Samples = Samples;
                            if (Game.GetSoundSamples)
                            {
                                Game.GetSoundSamples(&Thread, &GameMemory, &SoundBuffer);
                            }

#if HANDMADE_INTERNAL
                            win32_debug_time_marker *Marker = &DebugTimeMarker[DebugTimeMarkerIndex];
                            Marker->OutputLocation = BytesToLock;
                            Marker->OutputBytesCount = BytesToWrite;
                            Marker->OutputPlayCursor = PlayCursor;
                            Marker->OutputWriteCursor = WriteCursor;
                            Marker->ExpectedFlipPlayCursor = ExpectedFrameBoundaryByte;

                            DWORD UnwrappedWriteCursor = WriteCursor;
                            if (WriteCursor < PlayCursor)
                            {
                                UnwrappedWriteCursor += SoundOutput.SecondaryBufferSize;
                            }
                            AudioLatencyBytes = UnwrappedWriteCursor - PlayCursor;

                            AudioLatencySeconds =
                                    (((real32) AudioLatencyBytes / (real32) SoundOutput.BytesPerSample) /
                                     (real32) SoundOutput.SamplesPerSecond);
#if 0
                            char TextBuffer[256];
                            sprintf_s(TextBuffer, sizeof(TextBuffer),
                                      "  PC:%u WC:%u TC:%u RunningSample:%u BTL:%u BTW:%u SBS:%u  Latency:%uBytes %fSec\n",
                                      PlayCursor, WriteCursor, TargetCursor,
                                      SoundOutput.RunningSampleIndex, BytesToLock, BytesToWrite,
                                      SoundOutput.SecondaryBufferSize, AudioLatencyBytes, AudioLatencySeconds);
                            OutputDebugStringA(TextBuffer);
#endif
#endif

                            Win32FillSoundBuffer(&SoundOutput, BytesToLock, BytesToWrite, &SoundBuffer);
                        } else
                        {
                            SoundIsValid = false;
                        }

                        LARGE_INTEGER WorkCounter = Win32GetWallClock();
                        real32 WorkSecondsElapsed = Win32GetSecondsElapsed(LastCounter, WorkCounter);

                        real32 SecondsElapsedPerFrame = WorkSecondsElapsed;
                        if (SecondsElapsedPerFrame < TargetSecondsPerFrame)
                        {
                            if (IsSleepGranular)
                            {
                                DWORD SleepMS = (DWORD) (1000.0f * (TargetSecondsPerFrame - SecondsElapsedPerFrame));
                                if (SleepMS > 0)
                                {
                                    Sleep(SleepMS);
                                }
                            }

                            while (SecondsElapsedPerFrame < TargetSecondsPerFrame)
                            {
                                SecondsElapsedPerFrame = Win32GetSecondsElapsed(LastCounter, Win32GetWallClock());
                            }
                        } else
                        {
                            //TODO::(rahul) Logging Missed Frame
                        }

                        LARGE_INTEGER EndCounter = Win32GetWallClock();
                        real32 MsPerFrame = (1000.0f * Win32GetSecondsElapsed(LastCounter, EndCounter));
                        LastCounter = EndCounter;

                        win32_window_dimension Dimension = Win32GetWindowDimension(Window);
#if 0
                        Win32DebugSyncDisplay(&GlobalBackBuffer, ArrayCount(DebugTimeMarker), DebugTimeMarkerIndex - 1,
                                              DebugTimeMarker,
                                              &SoundOutput, TargetSecondsPerFrame);
#endif
                        Win32DisplayBufferInWindow(&GlobalBackBuffer, DeviceContext, Dimension.Width, Dimension.Height);
                        FlipWallClock = Win32GetWallClock();

#if HANDMADE_INTERNAL
                        {
                            DWORD DebugPlayCursor;
                            DWORD DebugWriteCursor;
                            if (GlobalSecondaryBuffer->GetCurrentPosition(&DebugPlayCursor, &DebugWriteCursor)
                                == DS_OK)
                            {
                                win32_debug_time_marker *Marker = &DebugTimeMarker[DebugTimeMarkerIndex];

                                Marker->FlipPlayCursor = DebugPlayCursor;
                                Marker->FlipWriteCursor = DebugWriteCursor;
                            }
                        }
#endif

#if 0
                        uint64 EndCycleCount = __rdtsc();
                        uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
                        real32 FramePerSecond = 0.0f; //(real32) GlobalQPerfFrequency / (real32) CounterElapsed;
                        real32 MCPF = (real32) (CyclesElapsed / (1000.0f * 1000.0f));

                        char FrameBuffer[256];

                        sprintf_s(FrameBuffer, "%f f/ms , FPS: %f , MCPF: %f\n", MsPerFrame, FramePerSecond, MCPF);
                        OutputDebugStringA(FrameBuffer);
                        LastCycleCount = EndCycleCount;
#endif

#if HANDMADE_INTERNAL
                        ++DebugTimeMarkerIndex;
                        if (DebugTimeMarkerIndex >= ArrayCount(DebugTimeMarker))
                        {
                            DebugTimeMarkerIndex = 0;
                        }
#endif
                        game_input *Temp = NewInput;
                        NewInput = OldInput;
                        OldInput = Temp;
                    }
                }
            } else
            {
            }
        } else
        {
            //TODO::(rahul) Logging
        }
    } else
    {
        //TODO::(rahul) Logging
    }
    return (0);
}

#pragma clang diagnostic pop
