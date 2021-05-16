//
// Created by AgentOfChaos on 4/10/2021.
//

#if HANDMADE_WIN32
#define RED_PLACE 16
#define GREEN_PLACE 8
#define BLUE_PLACE 0

static v2
GetRenderEntityBasisP(const render_group *RenderGroup, const v2 &ScreenCenter, const render_entry_rectangle *Entry);

#else
#define RED_PLACE 0
#define GREEN_PLACE 8
#define BLUE_PLACE 16
#endif

internal void
DrawRectangle(loaded_bitmap *DrawBuffer, v2 vMin, v2 vMax,
              real32 R, real32 G, real32 B)
{
    int32 MinX = RoundReal32ToInt32(vMin.X);
    int32 MinY = RoundReal32ToInt32(vMin.Y);
    int32 MaxX = RoundReal32ToInt32(vMax.X);
    int32 MaxY = RoundReal32ToInt32(vMax.Y);
    if (MinX < 0)
    {
        MinX = 0;
    }
    if (MinY < 0)
    {
        MinY = 0;
    }
    if (MaxX > DrawBuffer->Width)
    {
        MaxX = DrawBuffer->Width;
    }
    if (MaxY > DrawBuffer->Height)
    {
        MaxY = DrawBuffer->Height;
    }
    int32    BytesPerPixel = BYTES_PER_PIXEL;
    uint32   Color         = ((RoundReal32ToUInt32(R * 255.0f) << RED_PLACE) |
                              (RoundReal32ToUInt32(G * 255.0f) << GREEN_PLACE) |
                              (RoundReal32ToUInt32(B * 255.0f) << BLUE_PLACE) |
                              (RoundReal32ToUInt32(255.0f) << 24));
    uint8    *Row          = ((uint8 *) DrawBuffer->Memory +
                              MinX * BytesPerPixel +
                              MinY * DrawBuffer->Pitch);
    for (int Y             = MinY; Y < MaxY; ++Y)
    {
        uint32   *Pixel = (uint32 *) Row;
        for (int X      = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }
        Row += DrawBuffer->Pitch;
    }
}

inline void
DrawRectangleOutline(loaded_bitmap *DrawBuffer, v2 vMin, v2 vMax, v3 Color, real32 R = 2.0f)
{

    DrawRectangle(DrawBuffer, V2(vMin.X - R, vMin.Y - R), V2(vMax.X + R, vMin.Y + R),
                  Color.R, Color.G, Color.B);
    DrawRectangle(DrawBuffer, V2(vMin.X - R, vMax.Y - R), V2(vMax.X + R, vMax.Y + R),
                  Color.R, Color.G, Color.B);

    DrawRectangle(DrawBuffer, V2(vMin.X - R, vMin.Y - R), V2(vMin.X + R, vMax.Y + R),
                  Color.R, Color.G, Color.B);
    DrawRectangle(DrawBuffer, V2(vMax.X - R, vMin.Y - R), V2(vMax.X + R, vMax.Y + R),
                  Color.R, Color.G, Color.B);
}

internal void
DrawBitmap(loaded_bitmap *DrawBuffer, loaded_bitmap *Bitmap, real32 RealX, real32 RealY,
           real32 CAlpha = 1.0f)
{
    int32 MinX = RoundReal32ToInt32(RealX);
    int32 MinY = RoundReal32ToInt32(RealY);
    int32 MaxX = MinX + Bitmap->Width;
    int32 MaxY = MinY + Bitmap->Height;

    int32 SourceOffsetX = 0;
    if (MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX          = 0;
    }
    int32 SourceOffsetY = 0;
    if (MinY < 0)
    {
        SourceOffsetY = -MinY;
        MinY          = 0;
    }
    if (MaxX > DrawBuffer->Width)
    {
        MaxX = DrawBuffer->Width;
    }
    if (MaxY > DrawBuffer->Height)
    {
        MaxY = DrawBuffer->Height;
    }

    uint8 *SourceRow = (uint8 *) Bitmap->Memory +
                       BYTES_PER_PIXEL * SourceOffsetX +
                       Bitmap->Pitch * SourceOffsetY;
    uint8 *DestRow   = ((uint8 *) DrawBuffer->Memory +
                        MinX * BYTES_PER_PIXEL +
                        MinY * DrawBuffer->Pitch);

    for (int Y = MinY; Y < MaxY; ++Y)
    {
        uint32   *Dest   = (uint32 *) DestRow;
        uint32   *Source = (uint32 *) SourceRow;
        for (int X       = MinX; X < MaxX; ++X)
        {
            real32 SA  = (real32) ((*Source >> 24) & 0xFF);
            real32 SR  = (real32) ((*Source >> RED_PLACE) & 0xFF) * CAlpha;
            real32 SG  = (real32) ((*Source >> GREEN_PLACE) & 0xFF) * CAlpha;
            real32 SB  = (real32) ((*Source >> BLUE_PLACE) & 0xFF) * CAlpha;
            real32 RSA = (SA / 255.0f) * CAlpha;

            real32 DA  = (real32) ((*Dest >> 24) & 0xFF);
            real32 DR  = (real32) ((*Dest >> RED_PLACE) & 0xFF);
            real32 DG  = (real32) ((*Dest >> GREEN_PLACE) & 0xFF);
            real32 DB  = (real32) ((*Dest >> BLUE_PLACE) & 0xFF);
            real32 RDA = (DA / 255.0f);

            real32 InvRSA = (1.0f - RSA);
            real32 A      = 255.0f * (RSA + RDA - RSA * RDA);
            real32 R      = InvRSA * DR + SR;
            real32 G      = InvRSA * DG + SG;
            real32 B      = InvRSA * DB + SB;

            *Dest = (((uint32) (A + 0.5f) << 24) |
                     ((uint32) (R + 0.5f) << RED_PLACE) |
                     ((uint32) (G + 0.5f) << GREEN_PLACE) |
                     ((uint32) (B + 0.5f) << BLUE_PLACE));
            Dest++;
            Source++;

/*

Photoshop-style blend equations with destination alpha

The photoshop blend formulas are available in many places on the web, but
they all assume an opaque output target. Sometimes the output target is not
opaque (for example, with layer folders in Photoshop and the blend mode
not set to "normal").

The following formulas appear to be the ones used by Adobe Flash Player;
they might differ from Photoshop blend modes, and they do not include all of
the Photoshop blend modes (these formulas were developed for and verified in
Iggy, software I wrote for RAD Game Tools which "plays" Adobe Flash files).

                   out color                                                    out alpha
                   --------------                                               -----------
layer/over:    (   sc+(1-sa)*dc                                               , sa+da-sa*da   )
multiply:      (   sc*dc                                                      , sa+da-sa*da   )
screen:        (   sa*da - (da-dc)*(sa-sc)                                    , sa+da-sa*da   )
lighten:       (   max(sa*dc,sc*da)                                           , sa+da-sa*da   )
darken:        (   min(sa*dc,sc*da)                                           , sa+da-sa*da   )
add:           (   min(dc+sc,1)                                               , min(sa+da,1)  )
subtract:      (   max(dc-sc,0)                                               , min(sa+da,1)  )
difference:    (   abs(sa*dc-sc*da)                                           , sa+da-sa*da   )
invert:        (   sa*(da-dc)                                                 , sa+da-sa*da   )
overlay:       (   dc < da/2.0 ? (2.0*sc*dc) : (sa*da - 2.0*(da-dc)*(sa-sc))  , sa+da-sa*da   )
hardlight:     (   sc < sa/2.0 ? (2.0*sc*dc) : (sa*da - 2.0*(da-dc)*(sa-sc))  , sa+da-sa*da   )

  sc = source color, sa = source alpha, dc = dest color, da = dest alpha

The inputs in the above equations must be premultiplied. If inputs are
non-premultiplied, replace "sc" with "sc*sa". (Outputs are always
premultiplied, hence destination is always premultiplied.)

-- Sean Barrett, 2012/09/19


*/
        }
        DestRow += DrawBuffer->Pitch;
        SourceRow += Bitmap->Pitch;
    }
}

inline v2
GetRenderEntityBasisP(render_group *RenderGroup, v2 ScreenCenter, render_entity_basis *EntityBasis)
{
    v3     EntityBaseP = EntityBasis->Basis->P;
    real32 ZFudge      = (1.0f + 0.1f * (EntityBaseP.Z + EntityBasis->OffsetZ));

    real32 EntityGroundPointX = ScreenCenter.X + EntityBaseP.X * ZFudge * RenderGroup->MetersToPixel;
    real32 EntityGroundPointY = ScreenCenter.Y - EntityBaseP.Y * ZFudge * RenderGroup->MetersToPixel;
    real32 EntityZ            = -RenderGroup->MetersToPixel * EntityBaseP.Z;

    v2 Center = {EntityGroundPointX + EntityBasis->Offset.X,
                 EntityGroundPointY + EntityBasis->Offset.Y + EntityZ * EntityBasis->EntityZC};
    return Center;
}

void
RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget)
{
    v2 ScreenCenter = {0.5f * (real32) OutputTarget->Width,
                       0.5f * (real32) OutputTarget->Height};

    for (uint32 BaseAddress = 0;
         BaseAddress < RenderGroup->PushBufferSize;)
    {
        render_group_entry_header *Header = (render_group_entry_header *)
        (RenderGroup->PushBufferBase + BaseAddress);

        switch (Header->Type)
        {
            case RenderGroupEntryType_render_entry_clear:
            {
                render_entry_clear *Entry = (render_entry_clear *) Header;

                BaseAddress += sizeof(*Entry);
                break;
            }
            case RenderGroupEntryType_render_entry_rectangle:
            {
                render_entry_rectangle *Entry = (render_entry_rectangle *) Header;
                BaseAddress += sizeof(*Entry);

                v2 P = GetRenderEntityBasisP(RenderGroup, ScreenCenter, &Entry->EntityBasis);

                DrawRectangle(OutputTarget, P, P + Entry->Dim,
                              Entry->R, Entry->G, Entry->B);

                break;
            }

            case RenderGroupEntryType_render_entry_bitmap:
            {
                render_entry_bitmap *Entry = (render_entry_bitmap *) Header;
                BaseAddress += sizeof(*Entry);

                v2 P = GetRenderEntityBasisP(RenderGroup, ScreenCenter, &Entry->EntityBasis);

                Assert(Entry->Bitmap)
                DrawBitmap(OutputTarget, Entry->Bitmap, P.X, P.Y, Entry->A);

                break;
            }

            InvalidDefaultCase;
        }
    }
}

internal render_group *
AllocateRenderGroup(memory_arena *Arena, uint32 MaxPushBufferSize, real32 MetersToPixel)
{
    render_group *Result = PushStruct(Arena, render_group);
    Result->PushBufferBase = (uint8 *) PushSize(Arena, MaxPushBufferSize);

    Result->DefaultBasis    = PushStruct(Arena, render_basis);
    Result->DefaultBasis->P = V3(0, 0, 0);
    Result->MetersToPixel   = MetersToPixel;

    Result->MaxPushBufferSize = MaxPushBufferSize;
    Result->PushBufferSize    = 0;

    return (Result);
}

inline void
PushPiece(render_group *Group, loaded_bitmap *Bitmap, v2 Offset, real32 OffsetZ,
          v2 Dim, v2 Align, v4 Color, real32 EntityZC = 1.0f)
{
    render_entry_bitmap *Piece = PushRenderElement(Group, render_entry_bitmap);
    if (Piece)
    {
        Piece->EntityBasis.Basis    = Group->DefaultBasis;
        Piece->Bitmap               = Bitmap;
        Piece->EntityBasis.Offset   = Group->MetersToPixel * V2(Offset.X, -Offset.Y) - Align;
        Piece->EntityBasis.OffsetZ  = OffsetZ;
        Piece->EntityBasis.EntityZC = EntityZC;
        Piece->R                    = Color.R;
        Piece->G                    = Color.G;
        Piece->B                    = Color.B;
        Piece->A                    = Color.A;
    }
}

inline void
PushRect(render_group *Group, v2 Offset, real32 OffsetZ, v2 Dim,
         v4 Color, real32 EntityZC = 1.0f)
{
    render_entry_rectangle *Piece = PushRenderElement(Group, render_entry_rectangle);
    if (Piece)
    {
        v2 HalfDim = 0.5f * Group->MetersToPixel * Dim;

        Piece->EntityBasis.Basis    = Group->DefaultBasis;
        Piece->EntityBasis.Offset   = Group->MetersToPixel * V2(Offset.X, -Offset.Y) - HalfDim;
        Piece->EntityBasis.OffsetZ  = OffsetZ;
        Piece->EntityBasis.EntityZC = EntityZC;
        Piece->R                    = Color.R;
        Piece->G                    = Color.G;
        Piece->B                    = Color.B;
        Piece->A                    = Color.A;
        Piece->Dim                  = Dim;
    }
}

inline void
PushRectOutline(render_group *Group, v2 Offset, real32 OffsetZ, v2 Dim,
                v4 Color, real32 EntityZC = 1.0f)
{
    real32 Thickness = 0.1f;
    PushPiece(Group, 0, Offset + V2(0, 0.5f * Dim.Y), OffsetZ, V2(Dim.X, Thickness),
              V2(0, 0), Color, EntityZC);
    PushPiece(Group, 0, Offset - V2(0, 0.5f * Dim.Y), OffsetZ, V2(Dim.X, Thickness),
              V2(0, 0), Color, EntityZC);

    PushPiece(Group, 0, Offset + V2(0.5f * Dim.X, 0), OffsetZ, V2(Thickness, Dim.Y),
              V2(0, 0), Color, EntityZC);
    PushPiece(Group, 0, Offset - V2(0.5f * Dim.X, 0), OffsetZ, V2(Thickness, Dim.Y),
              V2(0, 0), Color, EntityZC);
}

inline void
PushBitmap(render_group *Group, loaded_bitmap *Bitmap,
           v2 Offset, real32 OffsetZ, v2 Align, real32 Alpha = 1.0f, real32 EntityZC = 1.0f)
{
    PushPiece(Group, Bitmap, Offset, OffsetZ, V2(0, 0),
              Align, V4(0, 0, 0, Alpha), EntityZC);
}