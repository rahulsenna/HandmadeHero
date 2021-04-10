//
// Created by AgentOfChaos on 4/10/2021.
//

internal render_group *
AllocateRenderGroup(memory_arena *Arena, uint32 MaxPushBufferSize, real32 MetersToPixel)
{
    render_group *Result = PushStruct(Arena, render_group);
    Result->PushBufferBase = (uint8 *) PushSize(Arena, MaxPushBufferSize);

    Result->DefaultBasis = PushStruct(Arena, render_basis);
    Result->DefaultBasis->P = V3(0, 0, 0);
    Result->MetersToPixel = MetersToPixel;
    Result->Count = 0;

    Result->MaxPushBufferSize = MaxPushBufferSize;
    Result->PushBufferSize = 0;

    return(Result);
}