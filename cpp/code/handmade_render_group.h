//
// Created by AgentOfChaos on 4/10/2021.
//

#ifndef HANDMADEHERO_HANDMADE_RENDER_GROUP_H

struct render_basis
{

    v3 P;
};

enum render_group_entry_type
{
    RenderGroupEntryType_render_entry_clear,
    RenderGroupEntryType_render_entry_bitmap,
    RenderGroupEntryType_render_entry_rectangle,
};

struct render_group_entry_header
{
    render_group_entry_type Type;
};

struct render_entity_basis
{
    render_basis *Basis;
    v2           Offset;
    real32       OffsetZ;
    real32       EntityZC;
};

struct render_entry_bitmap
{
    render_group_entry_header Header;
    loaded_bitmap             *Bitmap;
    render_entity_basis       EntityBasis;
    real32                    R, G, B, A;
};

struct render_entry_rectangle
{
    render_group_entry_header Header;
    render_entity_basis       EntityBasis;
    real32                    R, G, B, A;
    v2                        Dim;
};

struct render_entry_clear
{
    render_group_entry_header Header;
    real32                    R, G, B, A;
};

struct render_group
{
    render_basis *DefaultBasis;

    real32 MetersToPixel;

    game_state *GameState;

    uint32 MaxPushBufferSize;
    uint32 PushBufferSize;
    uint8  *PushBufferBase;
};

#define PushRenderElement(Group, type) (type *) PushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)

inline void *
PushRenderElement_(render_group *Group, uint32 Size, render_group_entry_type Type)
{
    render_group_entry_header *Result = 0;

    if (((Group->PushBufferSize + Size) < Group->MaxPushBufferSize))
    {
        Result = (render_group_entry_header *) (Group->PushBufferBase + Group->PushBufferSize);
        Result->Type = Type;
        Group->PushBufferSize += Size;
    } else
    {
        InvalidCodePath
    }

    return (Result);
}

#define HANDMADEHERO_HANDMADE_RENDER_GROUP_H
#endif //HANDMADEHERO_HANDMADE_RENDER_GROUP_H
