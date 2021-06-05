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
    RenderGroupEntryType_render_entry_coordinate_system
};

struct render_group_entry_header
{
    render_group_entry_type Type;
};

struct render_entity_basis
{
    render_basis *Basis;
    v2           Offset;
    r32          OffsetZ;
    r32          EntityZC;
};

struct render_entry_bitmap
{
    loaded_bitmap             *Bitmap;
    render_entity_basis       EntityBasis;
    r32                       R, G, B, A;
};

struct render_entry_rectangle
{
    render_entity_basis       EntityBasis;
    r32                       R, G, B, A;
    v2                        Dim;
};

struct render_entry_clear
{
    v4                        Color;
};

struct render_entry_coordinate_system
{
    v2 Origin;
    v2 XAxis;
    v2 YAxis;
    v4 Color;

    v2 Points[16];

    loaded_bitmap *Texture;
};

struct render_group
{
    render_basis *DefaultBasis;

    r32 MetersToPixel;

    game_state *GameState;

    u32 MaxPushBufferSize;
    u32 PushBufferSize;
    u8  *PushBufferBase;
};

#define PushRenderElement(Group, type) (type *) PushRenderElement_(Group, sizeof(type), RenderGroupEntryType_##type)

inline void *
PushRenderElement_(render_group *Group, u32 Size, render_group_entry_type Type)
{
    void *Result = 0;

    Size += sizeof(render_group_entry_header);

    if (((Group->PushBufferSize + Size) < Group->MaxPushBufferSize))
    {
        render_group_entry_header *Header = (render_group_entry_header *) (Group->PushBufferBase + Group->PushBufferSize);
        Header->Type = Type;
        Result = ((u8*)Header + sizeof(*Header));
        Group->PushBufferSize += Size;
    } else
    {
        InvalidCodePath
    }

    return (Result);
}

#define HANDMADEHERO_HANDMADE_RENDER_GROUP_H
#endif //HANDMADEHERO_HANDMADE_RENDER_GROUP_H
