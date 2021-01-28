// OSX Main.h
// Handmade Hero Mac Port
// by Ted Bendixson
//

#include "../cpp/code/handmade_platform.h"

#define MAC_MAX_FILENAME_SIZE 4096

struct mac_game_controller
{
    // Usage IDs

    // TODO: (ted) Maybe make this a union struct?
    int32 LeftThumbXUsageID;
    int32 LeftThumbYUsageID;
    int32 ButtonAUsageID;
    int32 ButtonBUsageID;
    int32 ButtonXUsageID;
    int32 ButtonYUsageID;
    int32 ButtonLeftShoulderUsageID;
    int32 ButtonRightShoulderUsageID;

    // Values
    real32 LeftThumbstickX;
    real32 LeftThumbstickY;
    bool32 UsesHatSwitch;

    int32 DPadX;
    int32 DPadY;

    bool32 ActionDownState;
    bool32 ActionRightState;
    bool32 ActionLeftState;
    bool32 ActionUpState;
    bool32 ButtonStart;

    bool32 ButtonLeft;
    bool32 ButtonRight;
    bool32 ButtonUp;
    bool32 ButtonDown;

    bool32 ButtonLeftShoulderState;
    bool32 ButtonRightShoulderState;
};

struct mac_sound_output
{
    uint32 SamplesPerSecond;
    uint32 BytesPerSample;
    uint32 RunningSampleIndex;
    uint32 BufferSize;
    uint32 SafetyBytes;
    uint32 WriteCursor;
    uint32 PlayCursor;
    void *Data;
};

struct mac_debug_time_marker
{
    uint32 OutputPlayCursor;
    uint32 OutputWriteCursor;
    uint32 OutputLocation;
    uint32 OutputByteCount;
    uint32 ExpectedFlipPlayCursor;
    uint32 FlipWriteCursor;
    uint32 FlipPlayCursor;
};

struct mac_replay_buffer
{
    FILE *FileHandle;
    char ReplayFileName[MAC_MAX_FILENAME_SIZE];
    void *MemoryBlock;
};

struct mac_app_path
{
    char Filename[MAC_MAX_FILENAME_SIZE];
    char *OnePastLastAppFileNameSlash;
};

struct mac_state
{
    void *GameMemoryBlock;
    uint64 PermanentStorageSize;

    mac_replay_buffer ReplayBuffers[4];

    mac_app_path *Path;

    FILE *RecordingHandle;
    int InputRecordingIndex;

    FILE *PlaybackHandle;
    int InputPlayingIndex;

    char ResourcesDirectory[MAC_MAX_FILENAME_SIZE];
    int ResourcesDirectorySize;
};

struct mac_recorded_iput
{
    int InputCount;
    game_input *InputStream;
};

struct mac_game_code
{
    void *GameCodeDLL;
    time_t DLLLastWriteTime;

    // IMPORTANT:   Either of these can be null. Check before using.
    game_update_and_render *UpdateAndRender;
    game_get_sound_samples *GetSoundSamples;

    bool32 IsValid;
};

