//
// Created by AgentOfChaos on 4/10/2021.
//


#if HANDMADE_WIN32
#define RED_PLACE   16
#define GREEN_PLACE 8
#define BLUE_PLACE  0

static v2
GetRenderEntityBasisP(const render_group *RenderGroup, const v2 &ScreenCenter, const render_entry_rectangle *Entry);

#else
#define RED_PLACE   0
#define GREEN_PLACE 8
#define BLUE_PLACE  16
#endif

internal v4
SRGB255ToLinear1(v4 C)
{
    r32 Inv255 = 1.0f / 255.f;

    v4 Result;
    Result.r = Square(Inv255 * C.r);
    Result.g = Square(Inv255 * C.g);
    Result.b = Square(Inv255 * C.b);
    Result.a =        Inv255 * C.a;
    
    return(Result);
}

internal v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;;
    Result.r = 255.f *  SquareRoot(C.r);
    Result.g = 255.f *  SquareRoot(C.g);
    Result.b = 255.f *  SquareRoot(C.b);
    Result.a = 255.f *             C.a;
    return(Result);
}

internal void
DrawRectangleSlowly(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color, loaded_bitmap *Texture)
{
    Color.rgb *= Color.a;

    r32 InvXAxisLengthSq = (1.f / LengthSq(XAxis));
    r32 InvYAxisLengthSq = (1.f / LengthSq(YAxis));

    u32 Color32 = ((RoundReal32ToUInt32(Color.a * 255.0f) << 24) |
                   (RoundReal32ToUInt32(Color.r * 255.0f) << RED_PLACE) |
                   (RoundReal32ToUInt32(Color.g * 255.0f) << GREEN_PLACE) |
                   (RoundReal32ToUInt32(Color.b * 255.0f) << BLUE_PLACE));

    s32 MinX = Buffer->Width - 1;
    s32 MaxX = 0;
    s32 MinY = Buffer->Height - 1;
    s32 MaxY = 0;

    v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};

    for (u32 I = 0; I < ArrayCount(P); ++I)
    {
        v2 TestP = P[I];

        s32 FloorX = FloorReal32ToInt32(TestP.x);
        s32 CeilX  = CeilReal32ToInt32(TestP.x);
        s32 FloorY = FloorReal32ToInt32(TestP.y);
        s32 CeilY  = CeilReal32ToInt32(TestP.y);

        if (MinX > FloorX)
        { MinX = FloorX; };
        if (MinY > FloorY)
        { MinY = FloorY; };
        if (MaxX < CeilX)
        { MaxX = CeilX; };
        if (MaxY < CeilY)
        { MaxY = CeilY; };
    }
    if (MinX < 0)
    { MinX = 0; };
    if (MinY < 0)
    { MinY = 0; };
    if (MaxX > Buffer->Width - 1)
    { MaxX = Buffer->Width - 1; };
    if (MaxY > Buffer->Height - 1)
    { MaxY = Buffer->Height - 1; };

    u8 *Row = ((u8 *) Buffer->Memory +
               MinX * BYTES_PER_PIXEL +
               MinY * Buffer->Pitch);

    for (int Y = MinY; Y < MaxY; ++Y)
    {
        u32 *Pixel = (u32 *) Row;

        for (int X = MinX; X < MaxX; ++X)
        {
            v2 PixelP = V2i(X, Y);
            v2 d      = PixelP - Origin;

            r32 EdgeTest0 = DotProduct(d, -Perp(XAxis));
            r32 EdgeTest1 = DotProduct(d - XAxis, -Perp(YAxis));
            r32 EdgeTest2 = DotProduct(d - XAxis - YAxis, Perp(XAxis));
            r32 EdgeTest3 = DotProduct(d - YAxis, Perp(YAxis));

            if ((EdgeTest0 < 0) &&
                (EdgeTest1 < 0) &&
                (EdgeTest2 < 0) &&
                (EdgeTest3 < 0))
            {
                r32 U = InvXAxisLengthSq * DotProduct(d, XAxis);
                r32 V = InvYAxisLengthSq * DotProduct(d, YAxis);

                // Assert((U >= 0.f) && (U <= 1.0f));
                // Assert((V >= 0.f) && (V <= 1.0f));

                r32 tX = 1.f + (U * ((r32) (Texture->Width - 3)) + .5f);
                r32 tY = 1.f + (V * ((r32) (Texture->Height - 3)) + .5f);

                s32 X = (s32) tX;
                s32 Y = (s32) tY;

                r32 fX = tX - (r32) X;
                r32 fY = tY - (r32) Y;

                Assert((X >= 0.f) && (X < Buffer->Width));
                Assert((Y >= 0.f) && (Y < Buffer->Height));

                u8 *TextPtr = ((u8 *) Texture->Memory) + (Y * Texture->Pitch) + X * sizeof(u32);

                u32 TexelPtrA = *(u32 *)  TextPtr;
                u32 TexelPtrB = *(u32 *) (TextPtr + sizeof(u32));
                u32 TexelPtrC = *(u32 *) (TextPtr + Texture->Pitch);
                u32 TexelPtrD = *(u32 *) (TextPtr + Texture->Pitch + sizeof(u32));

                v4 TexelA = V4((r32) ((TexelPtrA >> RED_PLACE) & 0xFF),
                               (r32) ((TexelPtrA >> GREEN_PLACE) & 0xFF),
                               (r32) ((TexelPtrA >> BLUE_PLACE) & 0xFF),
                               (r32) ((TexelPtrA >> 24) & 0xFF));
                v4 TexelB = V4((r32) ((TexelPtrB >> RED_PLACE) & 0xFF),
                               (r32) ((TexelPtrB >> GREEN_PLACE) & 0xFF),
                               (r32) ((TexelPtrB >> BLUE_PLACE) & 0xFF),
                               (r32) ((TexelPtrB >> 24) & 0xFF));
                v4 TexelC = V4((r32) ((TexelPtrC >> RED_PLACE) & 0xFF),
                               (r32) ((TexelPtrC >> GREEN_PLACE) & 0xFF),
                               (r32) ((TexelPtrC >> BLUE_PLACE) & 0xFF),
                               (r32) ((TexelPtrC >> 24) & 0xFF));
                v4 TexelD = V4((r32) ((TexelPtrD >> RED_PLACE) & 0xFF),
                               (r32) ((TexelPtrD >> GREEN_PLACE) & 0xFF),
                               (r32) ((TexelPtrD >> BLUE_PLACE) & 0xFF),
                               (r32) ((TexelPtrD >> 24) & 0xFF));

                TexelA = SRGB255ToLinear1(TexelA);
                TexelB = SRGB255ToLinear1(TexelB);
                TexelC = SRGB255ToLinear1(TexelC);
                TexelD = SRGB255ToLinear1(TexelD);

                v4 Texel = Lerp(Lerp(TexelA, fX, TexelB),
                                fY,
                                Lerp(TexelC, fX, TexelD));

                Texel = Hadamard(Texel, Color);

                v4 Dest  = {(r32) ((*Pixel >> RED_PLACE)   & 0xFF),
                            (r32) ((*Pixel >> GREEN_PLACE) & 0xFF),
                            (r32) ((*Pixel >> BLUE_PLACE)  & 0xFF),
                            (r32) ((*Pixel >> 24)          & 0xFF)};

                Dest = SRGB255ToLinear1(Dest);

                v4 Blended  = (1.0f - Texel.a) * Dest + Texel;
                v4 Blended255 = Linear1ToSRGB255(Blended);

                *Pixel = (((u32) (Blended255.r + 0.5f) << RED_PLACE) |
                          ((u32) (Blended255.g + 0.5f) << GREEN_PLACE) |
                          ((u32) (Blended255.b + 0.5f) << BLUE_PLACE)) |
                          ((u32) (Blended255.a + 0.5f) << 24);
            }
            ++Pixel;
        }
        Row += Buffer->Pitch;
    }
}

internal void
DrawRectangle(loaded_bitmap *DrawBuffer, v2 vMin, v2 vMax,
              r32 R, r32 G, r32 B, r32 A = 1.0f)
{
    s32 MinX = RoundReal32ToInt32(vMin.x);
    s32 MinY = RoundReal32ToInt32(vMin.y);
    s32 MaxX = RoundReal32ToInt32(vMax.x);
    s32 MaxY = RoundReal32ToInt32(vMax.y);
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
    s32      BytesPerPixel = BYTES_PER_PIXEL;
    u32      Color         = ((RoundReal32ToUInt32(A * 255.0f) << 24) |
                              (RoundReal32ToUInt32(R * 255.0f) << RED_PLACE) |
                              (RoundReal32ToUInt32(G * 255.0f) << GREEN_PLACE) |
                              (RoundReal32ToUInt32(B * 255.0f) << BLUE_PLACE));
    u8       *Row          = ((u8 *) DrawBuffer->Memory +
                              MinX * BytesPerPixel +
                              MinY * DrawBuffer->Pitch);
    for (int Y             = MinY; Y < MaxY; ++Y)
    {
        u32      *Pixel = (u32 *) Row;
        for (int X      = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color;
        }
        Row += DrawBuffer->Pitch;
    }
}

inline void
DrawRectangleOutline(loaded_bitmap *DrawBuffer, v2 vMin, v2 vMax, v3 Color, r32 R = 2.0f)
{

    DrawRectangle(DrawBuffer, V2(vMin.x - R, vMin.y - R), V2(vMax.x + R, vMin.y + R),
                  Color.r, Color.g, Color.b);
    DrawRectangle(DrawBuffer, V2(vMin.x - R, vMax.y - R), V2(vMax.x + R, vMax.y + R),
                  Color.r, Color.g, Color.b);

    DrawRectangle(DrawBuffer, V2(vMin.x - R, vMin.y - R), V2(vMin.x + R, vMax.y + R),
                  Color.r, Color.g, Color.b);
    DrawRectangle(DrawBuffer, V2(vMax.x - R, vMin.y - R), V2(vMax.x + R, vMax.y + R),
                  Color.r, Color.g, Color.b);
}

internal void
DrawBitmap(loaded_bitmap *DrawBuffer, loaded_bitmap *Bitmap, r32 RealX, r32 RealY,
           r32 CAlpha = 1.0f)
{
    s32 MinX = RoundReal32ToInt32(RealX);
    s32 MinY = RoundReal32ToInt32(RealY);
    s32 MaxX = MinX + Bitmap->Width;
    s32 MaxY = MinY + Bitmap->Height;

    s32 SourceOffsetX = 0;
    if (MinX < 0)
    {
        SourceOffsetX = -MinX;
        MinX          = 0;
    }
    s32 SourceOffsetY = 0;
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

    u8 *SourceRow = (u8 *) Bitmap->Memory +
                    BYTES_PER_PIXEL * SourceOffsetX +
                    Bitmap->Pitch * SourceOffsetY;
    u8 *DestRow   = ((u8 *) DrawBuffer->Memory +
                     MinX * BYTES_PER_PIXEL +
                     MinY * DrawBuffer->Pitch);

    for (int Y = MinY; Y < MaxY; ++Y)
    {
        u32      *Dest   = (u32 *) DestRow;
        u32      *Source = (u32 *) SourceRow;
        for (int X       = MinX; X < MaxX; ++X)
        {
            v4 Texel  = {(r32) ((*Source >> RED_PLACE) & 0xFF) ,
                         (r32) ((*Source >> GREEN_PLACE) & 0xFF),
                         (r32) ((*Source >> BLUE_PLACE) & 0xFF),
                         (r32) ((*Source >> 24) & 0xFF)};

            Texel = SRGB255ToLinear1(Texel);
            Texel *= CAlpha;
            
            v4 D = {(r32) ((*Dest >> RED_PLACE) & 0xFF),
                    (r32) ((*Dest >> GREEN_PLACE) & 0xFF),
                    (r32) ((*Dest >> BLUE_PLACE) & 0xFF),
                    (r32) ((*Dest >> 24) & 0xFF)};

            D = SRGB255ToLinear1(D);

            v4 Result = (1.0f - Texel.a) * D + Texel;

            Result = Linear1ToSRGB255(Result);

            *Dest = (((u32) (Result.r + 0.5f) << RED_PLACE) |
                     ((u32) (Result.g + 0.5f) << GREEN_PLACE) |
                     ((u32) (Result.b+ 0.5f) << BLUE_PLACE) |
                     ((u32) (Result.a+ 0.5f) << 24));
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
    v3  EntityBaseP = EntityBasis->Basis->P;
    r32 ZFudge      = (1.0f + 0.1f * (EntityBaseP.z + EntityBasis->OffsetZ));

    r32 EntityGroundPointX = ScreenCenter.x + EntityBaseP.x * ZFudge * RenderGroup->MetersToPixel;
    r32 EntityGroundPointY = ScreenCenter.y - EntityBaseP.y * ZFudge * RenderGroup->MetersToPixel;
    r32 EntityZ            = -RenderGroup->MetersToPixel * EntityBaseP.z;

    v2 Center = {EntityGroundPointX + EntityBasis->Offset.x,
                 EntityGroundPointY + EntityBasis->Offset.y + EntityZ * EntityBasis->EntityZC};
    return Center;
}

internal void 
RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget)
{
    v2 ScreenCenter = {0.5f * (r32) OutputTarget->Width,
                       0.5f * (r32) OutputTarget->Height};

    for (u32 BaseAddress = 0;
         BaseAddress < RenderGroup->PushBufferSize;)
    {
        render_group_entry_header *Header = (render_group_entry_header *) (RenderGroup->PushBufferBase + BaseAddress);
        BaseAddress += sizeof(*Header);
        void *Data = (u8*)Header + sizeof(*Header);

        switch (Header->Type)
        {
            case RenderGroupEntryType_render_entry_clear:
            {
                render_entry_clear *Entry = (render_entry_clear *) Data;

                DrawRectangle(OutputTarget, V2(0, 0),
                              V2((r32) OutputTarget->Width, (r32) OutputTarget->Height),
                              Entry->Color.r, Entry->Color.g, Entry->Color.b, Entry->Color.b);

                BaseAddress += sizeof(*Entry);
                break;
            }
            case RenderGroupEntryType_render_entry_rectangle:
            {
                render_entry_rectangle *Entry = (render_entry_rectangle *) Data;
                BaseAddress += sizeof(*Entry);

                v2 P = GetRenderEntityBasisP(RenderGroup, ScreenCenter, &Entry->EntityBasis);

                DrawRectangle(OutputTarget, P, P + Entry->Dim, Entry->R, Entry->G, Entry->B);

                break;
            }

            case RenderGroupEntryType_render_entry_bitmap:
            {
                render_entry_bitmap *Entry = (render_entry_bitmap *) Data;
                BaseAddress += sizeof(*Entry);

                v2 P = GetRenderEntityBasisP(RenderGroup, ScreenCenter, &Entry->EntityBasis);

                Assert(Entry->Bitmap)
                DrawBitmap(OutputTarget, Entry->Bitmap, P.x, P.y, Entry->A);

                break;
            }
            case RenderGroupEntryType_render_entry_coordinate_system:
            {
                render_entry_coordinate_system *Entry = (render_entry_coordinate_system *) Data;
                BaseAddress += sizeof(*Entry);

                v4 Color = V4(1.f, 1.0f, 0, 0);
                v2 Dim   = {4, 4};
                v2 P     = Entry->Origin;
                DrawRectangle(OutputTarget, P, P + Dim, Color.r, Color.g, Color.b);

                P = Entry->Origin + Entry->XAxis;
                DrawRectangle(OutputTarget, P, P + Dim, Color.r, Color.g, Color.b);

                P = Entry->Origin + Entry->YAxis;
                DrawRectangle(OutputTarget, P, P + Dim, Color.r, Color.g, Color.b);

                P = Entry->Origin + Entry->XAxis + Entry->YAxis;
                DrawRectangle(OutputTarget, P, P + Dim, Color.r, Color.g, Color.b);

                DrawRectangleSlowly(OutputTarget, Entry->Origin, Entry->XAxis, Entry->YAxis, Entry->Color,
                                    Entry->Texture);

#if 0
                for (u32 I = 0; I < ArrayCount(Entry->Points); ++I)
                {
                    v2 Point = Entry->Points[I];
                    Point = Entry->Origin + Point.x * Entry->XAxis + Point.y * Entry->YAxis;
                    DrawRectangle(OutputTarget, Point, Point + Dim, Entry->Color.r, Entry->Color.g, Entry->Color.b);
                }
#endif
                break;

            }
            InvalidDefaultCase;
        }
    }
}

internal render_group *
AllocateRenderGroup(memory_arena *Arena, u32 MaxPushBufferSize, r32 MetersToPixel)
{
    render_group *Result = PushStruct(Arena, render_group);
    Result->PushBufferBase = (u8 *) PushSize(Arena, MaxPushBufferSize);

    Result->DefaultBasis    = PushStruct(Arena, render_basis);
    Result->DefaultBasis->P = V3(0, 0, 0);
    Result->MetersToPixel   = MetersToPixel;

    Result->MaxPushBufferSize = MaxPushBufferSize;
    Result->PushBufferSize    = 0;

    return (Result);
}

inline void
PushPiece(render_group *Group, loaded_bitmap *Bitmap, v2 Offset, r32 OffsetZ,
          v2 Dim, v2 Align, v4 Color, r32 EntityZC = 1.0f)
{
    render_entry_bitmap *Piece = PushRenderElement(Group, render_entry_bitmap);
    if (Piece)
    {
        Piece->EntityBasis.Basis    = Group->DefaultBasis;
        Piece->Bitmap               = Bitmap;
        Piece->EntityBasis.Offset   = Group->MetersToPixel * V2(Offset.x, -Offset.y) - Align;
        Piece->EntityBasis.OffsetZ  = OffsetZ;
        Piece->EntityBasis.EntityZC = EntityZC;
        Piece->R                    = Color.r;
        Piece->G                    = Color.g;
        Piece->B                    = Color.b;
        Piece->A                    = Color.a;
    }
}

inline void
PushRect(render_group *Group, v2 Offset, r32 OffsetZ, v2 Dim, v4 Color, r32 EntityZC = 1.0f)
{
    render_entry_rectangle *Piece = PushRenderElement(Group, render_entry_rectangle);
    if (Piece)
    {
        v2 HalfDim = 0.5f * Group->MetersToPixel * Dim;

        Piece->EntityBasis.Basis    = Group->DefaultBasis;
        Piece->EntityBasis.Offset   = Group->MetersToPixel * V2(Offset.x, -Offset.y) - HalfDim;
        Piece->EntityBasis.OffsetZ  = OffsetZ;
        Piece->EntityBasis.EntityZC = EntityZC;
        Piece->R                    = Color.r;
        Piece->G                    = Color.g;
        Piece->B                    = Color.b;
        Piece->A                    = Color.a;
        Piece->Dim                  = Group->MetersToPixel * Dim;
    }
}

inline void
PushRectOutline(render_group *Group, v2 Offset, r32 OffsetZ, v2 Dim, v4 Color, r32 EntityZC = 1.0f)
{
    r32 Thickness = 0.1f;
    PushRect(Group, Offset - V2(0, 0.5f * Dim.y), OffsetZ, V2(Dim.x, Thickness), Color, EntityZC);
    PushRect(Group, Offset + V2(0, 0.5f * Dim.y), OffsetZ, V2(Dim.x, Thickness), Color, EntityZC);

    PushRect(Group, Offset - V2(0.5f * Dim.x, 0), OffsetZ, V2(Thickness, Dim.y), Color, EntityZC);
    PushRect(Group, Offset + V2(0.5f * Dim.x, 0), OffsetZ, V2(Thickness, Dim.y), Color, EntityZC);
}

inline void
PushBitmap(render_group *Group, loaded_bitmap *Bitmap,
           v2 Offset, r32 OffsetZ, v2 Align, r32 Alpha = 1.0f, r32 EntityZC = 1.0f)
{
    PushPiece(Group, Bitmap, Offset, OffsetZ, V2(0, 0),
              Align, V4(0, 0, 0, Alpha), EntityZC);
}

inline void
Clear(render_group *Group, v4 Color)
{
    render_entry_clear *Piece = PushRenderElement(Group, render_entry_clear);
    if (Piece)
    {
        Piece->Color = Color;
    }
}

inline render_entry_coordinate_system *
GetCoordinateSystem(render_group *Group, v2 Origin, v2 XAxis, v2 YAxis, v4 Color, loaded_bitmap *Texture)
{
    render_entry_coordinate_system *Piece = PushRenderElement(Group, render_entry_coordinate_system);
    if (Piece)
    {
        Piece->Origin  = Origin;
        Piece->XAxis   = XAxis;
        Piece->YAxis   = YAxis;
        Piece->Color   = Color;
        Piece->Texture = Texture;
    }
    return (Piece);
}