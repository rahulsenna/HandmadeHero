//
// Created by AgentOfChaos on 4/10/2021.
//

#if HANDMADE_WIN32
#define RED_SPACE   16
#define GREEN_SPACE 8
#define BLUE_SPACE  0

static v2
GetRenderEntityBasisP(render_group *RenderGroup, v2 &ScreenCenter, render_entry_rectangle *Entry, r32 MetersToPixel);

#else
#define RED_SPACE   0
#define GREEN_SPACE 8
#define BLUE_SPACE  16
#endif

internal v4
SRGB255ToLinear1(v4 C)
{
    r32 Inv255 = 1.0f / 255.f;

    v4 Result;
    Result.r = Square(Inv255 * C.r);
    Result.g = Square(Inv255 * C.g);
    Result.b = Square(Inv255 * C.b);
    Result.a = Inv255 * C.a;

    return (Result);
}

internal v4
Linear1ToSRGB255(v4 C)
{
    v4 Result;

    Result.r = 255.f * SquareRoot(C.r);
    Result.g = 255.f * SquareRoot(C.g);
    Result.b = 255.f * SquareRoot(C.b);
    Result.a = 255.f * C.a;
    return (Result);
}

inline v4
UnPack(u32 Packed)
{
    v4 Result = V4((r32) ((Packed >> RED_SPACE) & 0xFF),
                   (r32) ((Packed >> GREEN_SPACE) & 0xFF),
                   (r32) ((Packed >> BLUE_SPACE) & 0xFF),
                   (r32) ((Packed >> 24) & 0xFF));

    return (Result);
}

inline v4
UnscaledAndBiasNormal(v4 Normal)
{
    r32 Inv255 = 1.0f / 255.f;

    v4 Result;
    Result.x = -1.f + 2.f * (Inv255 * Normal.x);
    Result.y = -1.f + 2.f * (Inv255 * Normal.y);
    Result.z = -1.f + 2.f * (Inv255 * Normal.z);
    Result.w = Inv255 * Normal.w;

    return (Result);
}

struct bilinear_sample
{
    u32 A, B, C, D;
};

inline v4
SRGBBilinearBlend(bilinear_sample TexelSample, r32 fX, r32 fY)
{
    v4 TexelA = UnPack(TexelSample.A);
    v4 TexelB = UnPack(TexelSample.B);
    v4 TexelC = UnPack(TexelSample.C);
    v4 TexelD = UnPack(TexelSample.D);

    TexelA = SRGB255ToLinear1(TexelA);
    TexelB = SRGB255ToLinear1(TexelB);
    TexelC = SRGB255ToLinear1(TexelC);
    TexelD = SRGB255ToLinear1(TexelD);

    v4 Texel = Lerp(Lerp(TexelA, fX, TexelB),
                    fY,
                    Lerp(TexelC, fX, TexelD));

    return (Texel);
}

inline bilinear_sample
BilenearSample(loaded_bitmap *Bitmap, s32 X, s32 Y)
{
    bilinear_sample Result;

    u8 *BitmapPtr = ((u8 *) Bitmap->Memory) + (Y * Bitmap->Pitch) + X * sizeof(u32);

    Result.A = *(u32 *) BitmapPtr;
    Result.B = *(u32 *) (BitmapPtr + sizeof(u32));
    Result.C = *(u32 *) (BitmapPtr + Bitmap->Pitch);
    Result.D = *(u32 *) (BitmapPtr + Bitmap->Pitch + sizeof(u32));

    return (Result);
}

inline v3
SampleEnvironmentMap(v2 ScreenSpaceUV, v3 SampleDirection, r32 Roughness, environment_map *Map, r32 DistanceFromMapInZ)
{
    u32 LODIndex = (u32) (Roughness * (r32) (ArrayCount(Map->LOD) - 1) + .5f);
    Assert(LODIndex < ArrayCount(Map->LOD));

    loaded_bitmap *LOD = &Map->LOD[LODIndex];

    r32 UVPerMeter = .01f;
    r32 C          = (UVPerMeter * DistanceFromMapInZ) / SampleDirection.y;

    v2 Offset = C * V2(SampleDirection.x, SampleDirection.z);
    v2 UV     = ScreenSpaceUV + Offset;

    UV.x = Clamp01(UV.x);
    UV.y = Clamp01(UV.y);

    r32 tX = UV.x * (r32) (LOD->Width - 2);
    r32 tY = UV.y * (r32) (LOD->Height - 2);

    s32 X = (s32) tX;
    s32 Y = (s32) tY;

    r32 fX = tX - (r32) X;
    r32 fY = tY - (r32) Y;

    Assert((X >= 0.f) && (X < LOD->Width));
    Assert((Y >= 0.f) && (Y < LOD->Height));

    bilinear_sample Sample = BilenearSample(LOD, X, Y);
    v3              Result = SRGBBilinearBlend(Sample, fX, fY).xyz;

    return (Result);
}

#if 0
#include <iacaMarks.h>
#else

#define IACA_VC64_START
#define IACA_VC64_END

#endif

internal void
DrawRectangleQuickly(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                     loaded_bitmap *  Texture,
                     loaded_bitmap *  NormalMap,
                     environment_map *Top,
                     environment_map *Middle,
                     environment_map *Bottom,
                     r32              PixelsToMeter,
                     rectangle2i      ClipRect,
                     b32              Even)
{
    BEGIN_TIMED_BLOCK(DrawRectangleHopefullyQuickly);

    Color.rgb *= Color.a;

    r32 XAxisLength = Length(XAxis);
    r32 YAxisLength = Length(YAxis);

    v2  NxAxis  = (YAxisLength / XAxisLength) * XAxis;
    v2  NyAxis  = (XAxisLength / YAxisLength) * YAxis;
    r32 NZScale = .5f * (XAxisLength + YAxisLength);

    r32 InvXAxisLengthSq = (1.f / LengthSq(XAxis));
    r32 InvYAxisLengthSq = (1.f / LengthSq(YAxis));

    s32 WidthMax  = Buffer->Width - 3;
    s32 HeightMax = Buffer->Height - 3;

    rectangle2i FillRect = InvertedInfinityRectangle();

    v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};

    for (u32 I = 0; I < ArrayCount(P); ++I)
    {
        v2 TestP = P[I];

        s32 FloorX = FloorReal32ToInt32(TestP.x);
        s32 CeilX  = CeilReal32ToInt32(TestP.x) + 1;
        s32 FloorY = FloorReal32ToInt32(TestP.y);
        s32 CeilY  = CeilReal32ToInt32(TestP.y) + 1;

        if (FillRect.MinX > FloorX) { FillRect.MinX = FloorX; }
        if (FillRect.MinY > FloorY) { FillRect.MinY = FloorY; }
        if (FillRect.MaxX < CeilX) { FillRect.MaxX = CeilX; }
        if (FillRect.MaxY < CeilY) { FillRect.MaxY = CeilY; }
    }

    FillRect = Intersect(ClipRect, FillRect);

    if ((!Even) == (FillRect.MinY & 1))
    {
        FillRect.MinY += 1;
    }

    __m256i StartupClipMask = _mm256_set1_epi8(-1);
    s32     FillWidth       = FillRect.MaxX - FillRect.MinX;
    s32     FillWidthAlign  = FillWidth & 7;
    if (FillWidthAlign > 0)
    {
        s32 Adjustment = (8 - FillWidthAlign);
        FillWidth += Adjustment;
        FillRect.MinX = FillRect.MaxX - FillWidth;

        __m128i lo = _mm_set1_epi8(-1);
        __m128i hi = _mm_set1_epi8(-1);
        if (Adjustment < 5)
        {
            switch (Adjustment)
            {
                case 1: lo = _mm_slli_si128(lo, 1 * 4); break;
                case 2: lo = _mm_slli_si128(lo, 2 * 4); break;
                case 3: lo = _mm_slli_si128(lo, 3 * 4); break;
                case 4: lo = _mm_slli_si128(lo, 4 * 4); break;
            }
        } else
        {
            lo = _mm_set1_epi8(0);
            switch (Adjustment)
            {
                case 5: hi = _mm_slli_si128(hi, 1 * 4); break;
                case 6: hi = _mm_slli_si128(hi, 2 * 4); break;
                case 7: hi = _mm_slli_si128(hi, 3 * 4); break;
            }
        }
        StartupClipMask = _mm256_set_m128i(hi, lo);
    }

#define mmSquare(a) _mm256_mul_ps(a, a)
#define M(a, i)     ((r32 *) &(a))[i]
#define Mi(a, i)    ((u32 *) &(a))[i]

    __m256 Inv255 = _mm256_set1_ps(1.0f / 255.f);
    __m256 One255 = _mm256_set1_ps(255.f);

    __m256 Zero  = _mm256_set1_ps(0.0f);
    __m256 Half  = _mm256_set1_ps(0.5f);
    __m256 One   = _mm256_set1_ps(1.0f);
    __m256 Four  = _mm256_set1_ps(4.0f);
    __m256 Eight = _mm256_set1_ps(8.0f);

    __m256 Colorr = _mm256_set1_ps(Color.r);
    __m256 Colorg = _mm256_set1_ps(Color.g);
    __m256 Colorb = _mm256_set1_ps(Color.b);
    __m256 Colora = _mm256_set1_ps(Color.a);

    __m256 Originx = _mm256_set1_ps(Origin.x);
    __m256 Originy = _mm256_set1_ps(Origin.y);

    v2 nXAxis = XAxis * InvXAxisLengthSq;
    v2 nYAxis = YAxis * InvYAxisLengthSq;

    __m256 nXAxisx = _mm256_set1_ps(nXAxis.x);
    __m256 nXAxisy = _mm256_set1_ps(nXAxis.y);
    __m256 nYAxisx = _mm256_set1_ps(nYAxis.x);
    __m256 nYAxisy = _mm256_set1_ps(nYAxis.y);

    __m256 WidthM2  = _mm256_set1_ps((r32) (Texture->Width - 2));
    __m256 HeightM2 = _mm256_set1_ps((r32) (Texture->Height - 2));

    __m256i MaskFF = _mm256_set1_epi32(0xFF);

    void *TextureMemory = Texture->Memory;
    s32   TexturePitch  = Texture->Pitch;
    s32   TextureWidth  = Texture->Width;
    s32   TextureHeight = Texture->Height;

    __m256i TexturePitch_8x = _mm256_set1_epi32(Texture->Pitch);

    u8 *Row = ((u8 *) Buffer->Memory +
               FillRect.MinX * BYTES_PER_PIXEL +
               FillRect.MinY * Buffer->Pitch);

    s32 RowAdvance = Buffer->Pitch * 2;

    BEGIN_TIMED_BLOCK(ProcessPixel);
    s32 MinX = FillRect.MinX;
    s32 MinY = FillRect.MinY;
    s32 MaxX = FillRect.MaxX;
    s32 MaxY = FillRect.MaxY;

    for (s32 Y = MinY; Y < MaxY; Y += 2)
    {
        u32 *Pixel = (u32 *) Row;

        __m256 PixelPy = _mm256_set1_ps((r32) Y);
        __m256 PixelPx = _mm256_set_ps((r32) (MinX + 7),
                                       (r32) (MinX + 6),
                                       (r32) (MinX + 5),
                                       (r32) (MinX + 4),
                                       (r32) (MinX + 3),
                                       (r32) (MinX + 2),
                                       (r32) (MinX + 1),
                                       (r32) (MinX + 0));

        PixelPx = _mm256_sub_ps(PixelPx, Originx);
        PixelPy = _mm256_sub_ps(PixelPy, Originy);

        __m256i ClipMask = StartupClipMask;

        for (s32 XI = MinX; XI < MaxX; XI += 8)
        {
            IACA_VC64_START;
            __m256 U = _mm256_add_ps(_mm256_mul_ps(nXAxisx, PixelPx), _mm256_mul_ps(nXAxisy, PixelPy));
            __m256 V = _mm256_add_ps(_mm256_mul_ps(nYAxisx, PixelPx), _mm256_mul_ps(nYAxisy, PixelPy));

            __m256i WriteMask = _mm256_castps_si256(
            _mm256_and_ps(_mm256_and_ps(_mm256_cmp_ps(U, One, _CMP_LE_OQ), _mm256_cmp_ps(U, Zero, _CMP_GE_OQ)),
                          _mm256_and_ps(_mm256_cmp_ps(V, One, _CMP_LE_OQ), _mm256_cmp_ps(V, Zero, _CMP_GE_OQ))));

            WriteMask = _mm256_and_si256(WriteMask, ClipMask);

            // __m256i WriteMask = _mm256_set1_epi32(0xFFFFFFFF);

            // if (_mm256_movemask_epi8(WriteMask))
            {
                // __m256i OriginalDest = _mm256_loadu_si256((__m256i *) Pixel);
                __m256i OriginalDest = _mm256_loadu2_m128i((__m128i *) (Pixel + 4), (__m128i *) Pixel);

                U = _mm256_min_ps(_mm256_max_ps(U, Zero), One);
                V = _mm256_min_ps(_mm256_max_ps(V, Zero), One);

                __m256 tX = _mm256_mul_ps(U, WidthM2);
                __m256 tY = _mm256_mul_ps(V, HeightM2);

                __m256i FetchX_8x = _mm256_cvttps_epi32(tX);
                __m256i FetchY_8x = _mm256_cvttps_epi32(tY);

                __m256 fX = _mm256_sub_ps(tX, _mm256_cvtepi32_ps(FetchX_8x));
                __m256 fY = _mm256_sub_ps(tY, _mm256_cvtepi32_ps(FetchY_8x));

                //NOTE(rahul): Bilenear Sample

                FetchX_8x = _mm256_slli_epi32(FetchX_8x, 2);
                FetchY_8x = _mm256_mullo_epi32(FetchY_8x, TexturePitch_8x);

                __m256i Fetch_8x = _mm256_add_epi32(FetchX_8x, FetchY_8x);

                s32 Fetch0 = Mi(Fetch_8x, 0);
                s32 Fetch1 = Mi(Fetch_8x, 1);
                s32 Fetch2 = Mi(Fetch_8x, 2);
                s32 Fetch3 = Mi(Fetch_8x, 3);
                s32 Fetch4 = Mi(Fetch_8x, 4);
                s32 Fetch5 = Mi(Fetch_8x, 5);
                s32 Fetch6 = Mi(Fetch_8x, 6);
                s32 Fetch7 = Mi(Fetch_8x, 7);

                u8 *TexelPtr0 = ((u8 *) TextureMemory) + Fetch0;
                u8 *TexelPtr1 = ((u8 *) TextureMemory) + Fetch1;
                u8 *TexelPtr2 = ((u8 *) TextureMemory) + Fetch2;
                u8 *TexelPtr3 = ((u8 *) TextureMemory) + Fetch3;
                u8 *TexelPtr4 = ((u8 *) TextureMemory) + Fetch4;
                u8 *TexelPtr5 = ((u8 *) TextureMemory) + Fetch5;
                u8 *TexelPtr6 = ((u8 *) TextureMemory) + Fetch6;
                u8 *TexelPtr7 = ((u8 *) TextureMemory) + Fetch7;

                __m256i TexelSampleA = _mm256_setr_epi32(*(u32 *) TexelPtr0,
                                                         *(u32 *) TexelPtr1,
                                                         *(u32 *) TexelPtr2,
                                                         *(u32 *) TexelPtr3,
                                                         *(u32 *) TexelPtr4,
                                                         *(u32 *) TexelPtr5,
                                                         *(u32 *) TexelPtr6,
                                                         *(u32 *) TexelPtr7);

                __m256i TexelSampleB = _mm256_setr_epi32(*(u32 *) (TexelPtr0 + sizeof(u32)),
                                                         *(u32 *) (TexelPtr1 + sizeof(u32)),
                                                         *(u32 *) (TexelPtr2 + sizeof(u32)),
                                                         *(u32 *) (TexelPtr3 + sizeof(u32)),
                                                         *(u32 *) (TexelPtr4 + sizeof(u32)),
                                                         *(u32 *) (TexelPtr5 + sizeof(u32)),
                                                         *(u32 *) (TexelPtr6 + sizeof(u32)),
                                                         *(u32 *) (TexelPtr7 + sizeof(u32)));

                __m256i TexelSampleC = _mm256_setr_epi32(*(u32 *) (TexelPtr0 + TexturePitch),
                                                         *(u32 *) (TexelPtr1 + TexturePitch),
                                                         *(u32 *) (TexelPtr2 + TexturePitch),
                                                         *(u32 *) (TexelPtr3 + TexturePitch),
                                                         *(u32 *) (TexelPtr4 + TexturePitch),
                                                         *(u32 *) (TexelPtr5 + TexturePitch),
                                                         *(u32 *) (TexelPtr6 + TexturePitch),
                                                         *(u32 *) (TexelPtr7 + TexturePitch));

                __m256i TexelSampleD = _mm256_setr_epi32(*(u32 *) (TexelPtr0 + TexturePitch + sizeof(u32)),
                                                         *(u32 *) (TexelPtr1 + TexturePitch + sizeof(u32)),
                                                         *(u32 *) (TexelPtr2 + TexturePitch + sizeof(u32)),
                                                         *(u32 *) (TexelPtr3 + TexturePitch + sizeof(u32)),
                                                         *(u32 *) (TexelPtr4 + TexturePitch + sizeof(u32)),
                                                         *(u32 *) (TexelPtr5 + TexturePitch + sizeof(u32)),
                                                         *(u32 *) (TexelPtr6 + TexturePitch + sizeof(u32)),
                                                         *(u32 *) (TexelPtr7 + TexturePitch + sizeof(u32)));

                //NOTE(rahul): Convert texture from sRGB to "linear" brightness space

                __m256 TexelAr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, RED_SPACE), MaskFF));
                __m256 TexelAg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, GREEN_SPACE), MaskFF));
                __m256 TexelAb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, BLUE_SPACE), MaskFF));
                __m256 TexelAa = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, 24), MaskFF));

                __m256 TexelBr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, RED_SPACE), MaskFF));
                __m256 TexelBg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, GREEN_SPACE), MaskFF));
                __m256 TexelBb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, BLUE_SPACE), MaskFF));
                __m256 TexelBa = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, 24), MaskFF));

                __m256 TexelCr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, RED_SPACE), MaskFF));
                __m256 TexelCg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, GREEN_SPACE), MaskFF));
                __m256 TexelCb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, BLUE_SPACE), MaskFF));
                __m256 TexelCa = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, 24), MaskFF));

                __m256 TexelDr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleD, RED_SPACE), MaskFF));
                __m256 TexelDg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleD, GREEN_SPACE), MaskFF));
                __m256 TexelDb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleD, BLUE_SPACE), MaskFF));
                __m256 TexelDa = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleD, 24), MaskFF));

                TexelAr = mmSquare(_mm256_mul_ps(Inv255, TexelAr));
                TexelAg = mmSquare(_mm256_mul_ps(Inv255, TexelAg));
                TexelAb = mmSquare(_mm256_mul_ps(Inv255, TexelAb));
                TexelAa = _mm256_mul_ps(Inv255, TexelAa);

                TexelBr = mmSquare(_mm256_mul_ps(Inv255, TexelBr));
                TexelBg = mmSquare(_mm256_mul_ps(Inv255, TexelBg));
                TexelBb = mmSquare(_mm256_mul_ps(Inv255, TexelBb));
                TexelBa = _mm256_mul_ps(Inv255, TexelBa);

                TexelCr = mmSquare(_mm256_mul_ps(Inv255, TexelCr));
                TexelCg = mmSquare(_mm256_mul_ps(Inv255, TexelCg));
                TexelCb = mmSquare(_mm256_mul_ps(Inv255, TexelCb));
                TexelCa = _mm256_mul_ps(Inv255, TexelCa);

                TexelDr = mmSquare(_mm256_mul_ps(Inv255, TexelDr));
                TexelDg = mmSquare(_mm256_mul_ps(Inv255, TexelDg));
                TexelDb = mmSquare(_mm256_mul_ps(Inv255, TexelDb));
                TexelDa = _mm256_mul_ps(Inv255, TexelDa);

                //NOTE(rahul): Bilinear texture blend
                __m256 ifX = _mm256_sub_ps(One, fX);
                __m256 ifY = _mm256_sub_ps(One, fY);
                __m256 l0  = _mm256_mul_ps(ifY, ifX);
                __m256 l1  = _mm256_mul_ps(ifY, fX);
                __m256 l2  = _mm256_mul_ps(fY, ifX);
                __m256 l3  = _mm256_mul_ps(fY, fX);

                __m256 Texelr = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, TexelAr), _mm256_mul_ps(l1, TexelBr)),
                                              _mm256_add_ps(_mm256_mul_ps(l2, TexelCr), _mm256_mul_ps(l3, TexelDr)));
                __m256 Texelg = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, TexelAg), _mm256_mul_ps(l1, TexelBg)),
                                              _mm256_add_ps(_mm256_mul_ps(l2, TexelCg), _mm256_mul_ps(l3, TexelDg)));
                __m256 Texelb = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, TexelAb), _mm256_mul_ps(l1, TexelBb)),
                                              _mm256_add_ps(_mm256_mul_ps(l2, TexelCb), _mm256_mul_ps(l3, TexelDb)));
                __m256 Texela = _mm256_add_ps(_mm256_add_ps(_mm256_mul_ps(l0, TexelAa), _mm256_mul_ps(l1, TexelBa)),
                                              _mm256_add_ps(_mm256_mul_ps(l2, TexelCa), _mm256_mul_ps(l3, TexelDa)));

                //NOTE(rahul): Modulate by incoming color
                Texelr = _mm256_mul_ps(Texelr, Colorr);
                Texelg = _mm256_mul_ps(Texelg, Colorg);
                Texelb = _mm256_mul_ps(Texelb, Colorb);
                Texela = _mm256_mul_ps(Texela, Colora);

                //NOTE(rahul): Clamp colors to valid range
                Texelr = _mm256_min_ps(_mm256_max_ps(Texelr, Zero), One);
                Texelg = _mm256_min_ps(_mm256_max_ps(Texelg, Zero), One);
                Texelb = _mm256_min_ps(_mm256_max_ps(Texelb, Zero), One);

                //NOTE(rahul): Load Destinatioln
                __m256 Destr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(OriginalDest, RED_SPACE), MaskFF));
                __m256 Destg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(OriginalDest, GREEN_SPACE), MaskFF));
                __m256 Destb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(OriginalDest, BLUE_SPACE), MaskFF));
                __m256 Desta = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(OriginalDest, 24), MaskFF));

                //NOTE(rahul): SRGB to  Linear
                Destr = mmSquare(_mm256_mul_ps(Inv255, Destr));
                Destg = mmSquare(_mm256_mul_ps(Inv255, Destg));
                Destb = mmSquare(_mm256_mul_ps(Inv255, Destb));
                Desta = _mm256_mul_ps(Inv255, Desta);

                //NOTE(rahul): Destination blend
                __m256 InvTexelA = _mm256_sub_ps(One, Texela);

                __m256 Blendedr = _mm256_add_ps(_mm256_mul_ps(InvTexelA, Destr), Texelr);
                __m256 Blendedg = _mm256_add_ps(_mm256_mul_ps(InvTexelA, Destg), Texelg);
                __m256 Blendedb = _mm256_add_ps(_mm256_mul_ps(InvTexelA, Destb), Texelb);
                __m256 Blendeda = _mm256_add_ps(_mm256_mul_ps(InvTexelA, Desta), Texela);

                //NOTE(rahul):Linear to SRGB
                Blendedr = _mm256_mul_ps(One255, _mm256_mul_ps(Blendedr, _mm256_rsqrt_ps(Blendedr)));
                Blendedg = _mm256_mul_ps(One255, _mm256_mul_ps(Blendedg, _mm256_rsqrt_ps(Blendedg)));
                Blendedb = _mm256_mul_ps(One255, _mm256_mul_ps(Blendedb, _mm256_rsqrt_ps(Blendedb)));
                Blendeda = _mm256_mul_ps(One255, Blendeda);

                __m256i Intr = _mm256_cvtps_epi32(Blendedr);
                __m256i Intg = _mm256_cvtps_epi32(Blendedg);
                __m256i Intb = _mm256_cvtps_epi32(Blendedb);
                __m256i Inta = _mm256_cvtps_epi32(Blendeda);

                __m256i Sr = _mm256_slli_epi32(Intr, 16);
                __m256i Sg = _mm256_slli_epi32(Intg, 8);
                __m256i Sb = Intb;
                __m256i Sa = _mm256_slli_epi32(Inta, 24);

                __m256i Out = _mm256_or_si256(_mm256_or_si256(Sr, Sg), _mm256_or_si256(Sb, Sa));

                __m256i MaskedOut = _mm256_or_si256(_mm256_and_si256(WriteMask, Out),
                                                    _mm256_andnot_si256(WriteMask, OriginalDest));

                // _mm256_storeu_si256((__m256i *) Pixel, MaskedOut);
                _mm256_storeu2_m128i((__m128i *) (Pixel + 4), (__m128i *) Pixel, MaskedOut);
            }
            PixelPx = _mm256_add_ps(PixelPx, Eight);
            Pixel += 8;
            ClipMask = _mm256_set1_epi8(-1);
            IACA_VC64_END;
        }
        Row += RowAdvance;
    }

    END_TIMED_BLOCK_COUNTED(ProcessPixel, GetClampedRectArea(FillRect) / 2);
    END_TIMED_BLOCK(DrawRectangleHopefullyQuickly);
}

internal void
DrawRectangleSlowly(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                    loaded_bitmap *  Texture,
                    loaded_bitmap *  NormalMap,
                    environment_map *Top,
                    environment_map *Middle,
                    environment_map *Bottom,
                    r32              PixelsToMeter)
{
    BEGIN_TIMED_BLOCK(DrawRectangleSlowly);

    Color.rgb *= Color.a;

    r32 XAxisLength = Length(XAxis);
    r32 YAxisLength = Length(YAxis);

    r32 InvXAxisLengthSq = (1.f / LengthSq(XAxis));
    r32 InvYAxisLengthSq = (1.f / LengthSq(YAxis));

    s32 WidthMax  = (Buffer->Width - 1);
    s32 HeightMax = (Buffer->Height - 1);

    r32 InvWidthMax  = 1.f / (r32) WidthMax;
    r32 InvHeightMax = 1.f / (r32) HeightMax;

    r32 OriginZ    = 0.f;
    r32 OriginY    = (Origin + .5f * XAxis + .5f * YAxis).y;
    r32 FixedCastY = InvHeightMax * OriginY;

    s32 MinX = WidthMax;
    s32 MaxX = 0;
    s32 MinY = HeightMax;
    s32 MaxY = 0;

    v2 P[4] = {Origin, Origin + XAxis, Origin + XAxis + YAxis, Origin + YAxis};

    for (u32 I = 0; I < ArrayCount(P); ++I)
    {
        v2 TestP = P[I];

        s32 FloorX = FloorReal32ToInt32(TestP.x);
        s32 CeilX  = CeilReal32ToInt32(TestP.x);
        s32 FloorY = FloorReal32ToInt32(TestP.y);
        s32 CeilY  = CeilReal32ToInt32(TestP.y);

        if (MinX > FloorX) { MinX = FloorX; }
        if (MinY > FloorY) { MinY = FloorY; }
        if (MaxX < CeilX) { MaxX = CeilX; }
        if (MaxY < CeilY) { MaxY = CeilY; }
    }
    if (MinX < 0) { MinX = 0; }
    if (MinY < 0) { MinY = 0; }
    if (MaxX > Buffer->Width - 1) { MaxX = Buffer->Width - 1; }
    if (MaxY > Buffer->Height - 1) { MaxY = Buffer->Height - 1; }

    u8 *Row = ((u8 *) Buffer->Memory +
               MinX * BYTES_PER_PIXEL +
               MinY * Buffer->Pitch);

    BEGIN_TIMED_BLOCK(ProcessPixel);
    for (s32 Y = MinY; Y < MaxY; ++Y)
    {
        u32 *Pixel = (u32 *) Row;

        for (s32 X = MinX; X < MaxX; ++X)
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
                v2 ScreenSpaceUV = V2(InvWidthMax * X, FixedCastY);

                r32 ZDiff = PixelsToMeter * ((r32) Y - OriginY);

                r32 U = InvXAxisLengthSq * DotProduct(d, XAxis);
                r32 V = InvYAxisLengthSq * DotProduct(d, YAxis);

                // Assert((U >= 0.f) && (U <= 1.0f));
                // Assert((V >= 0.f) && (V <= 1.0f));

                r32 tX = 1.f + (U * ((r32) (Texture->Width - 3)) + .5f);
                r32 tY = 1.f + (V * ((r32) (Texture->Height - 3)) + .5f);

                s32 FetchX = (s32) tX;
                s32 FetchY = (s32) tY;

                r32 fX = tX - (r32) FetchX;
                r32 fY = tY - (r32) FetchY;

                Assert((FetchX >= 0) && (FetchX < Texture->Width));
                Assert((FetchY >= 0) && (FetchY < Texture->Height));

                bilinear_sample TexelSample = BilenearSample(Texture, FetchX, FetchY);
                v4              Texel       = SRGBBilinearBlend(TexelSample, fX, fY);

#if 0
                    if (NormalMap)
                    {
                        bilinear_sample NormalSample = BilenearSample(NormalMap, X, Y);
    
                        v4 NormalA = UnPack(NormalSample.A);
                        v4 NormalB = UnPack(NormalSample.B);
                        v4 NormalC = UnPack(NormalSample.C);
                        v4 NormalD = UnPack(NormalSample.D);
    
                        v4 Normal = Lerp(Lerp(NormalA, fX, NormalB),                  fY,
                                         Lerp(NormalC, fX, NormalD));
    
                        Normal = UnscaledAndBiasNormal(Normal);
    
                        Normal.xy = Normal.x * NxAxis + Normal.y * NyAxis;
                        Normal.z *= NZScale;
                        Normal.xyz = Normalize(Normal.xyz);
    
                        v3 BoundDirection = 2.f * Normal.z * Normal.xyz;
                        BoundDirection.z -= 1.f;
    
                        BoundDirection.z = -BoundDirection.z;
    
                        environment_map *FarMap = 0;
    
                        r32 Pz      = OriginZ + ZDiff;
                        r32 MapZ    = 1.f;
                        r32 tEnvMap = BoundDirection.y;
                        r32 tFarMap = 0.f;
    
                        if (tEnvMap < -.5f)
                        {
                            FarMap  = Bottom;
                            tFarMap = -2.f * tEnvMap - 1.f;
                        } else if (tEnvMap > 0.5f)
                        {
                       
                            FarMap  = Top;
                            tFarMap = 2.f * (tEnvMap - .5f);
                        }
                        tFarMap *= tFarMap;
                        tFarMap *= tFarMap;
    
                        v3 LightColor = V3(0, 0, 0);// SampleEnvironmentMap(ScreenSpaceUV, Normal.rgb, Normal.w, Middle);
    
                        if (FarMap)
                        {
                            r32 DistanceFromMapInZ = FarMap->Pz - Pz;
    
                            v3 FarColorMap = SampleEnvironmentMap(ScreenSpaceUV, BoundDirection, Normal.w, FarMap, DistanceFromMapInZ);
                            LightColor     = Lerp(LightColor, tFarMap, FarColorMap);
                        }
    
                        Texel.rgb = Texel.rgb + Texel.a * LightColor;
                    }

#endif
                Texel   = Hadamard(Texel, Color);
                Texel.r = Clamp01(Texel.r);
                Texel.g = Clamp01(Texel.g);
                Texel.b = Clamp01(Texel.b);

                v4 Dest = {(r32) ((*Pixel >> RED_SPACE) & 0xFF),
                           (r32) ((*Pixel >> GREEN_SPACE) & 0xFF),
                           (r32) ((*Pixel >> BLUE_SPACE) & 0xFF),
                           (r32) ((*Pixel >> 24) & 0xFF)};

                Dest = SRGB255ToLinear1(Dest);

                v4 Blended    = (1.0f - Texel.a) * Dest + Texel;
                v4 Blended255 = Linear1ToSRGB255(Blended);

                *Pixel = (((u32) (Blended255.r + 0.5f) << RED_SPACE) |
                          ((u32) (Blended255.g + 0.5f) << GREEN_SPACE) |
                          ((u32) (Blended255.b + 0.5f) << BLUE_SPACE)) |
                         ((u32) (Blended255.a + 0.5f) << 24);
            }
            ++Pixel;
        }
        Row += Buffer->Pitch;
    }
    END_TIMED_BLOCK_COUNTED(ProcessPixel, (MaxX - MinX + 1) * (MaxY - MinY + 1));

    END_TIMED_BLOCK(DrawRectangleSlowly);
}

internal void
DrawRectangle(loaded_bitmap *DrawBuffer, v2 vMin, v2 vMax, v4 Color, rectangle2i ClipRect, b32 Even)
{
    r32 R = Color.r;
    r32 G = Color.g;
    r32 B = Color.b;
    r32 A = Color.a;

    rectangle2i FillRect;
    FillRect.MinX = RoundReal32ToInt32(vMin.x);
    FillRect.MinY = RoundReal32ToInt32(vMin.y);
    FillRect.MaxX = RoundReal32ToInt32(vMax.x);
    FillRect.MaxY = RoundReal32ToInt32(vMax.y);

    FillRect = Intersect(FillRect, ClipRect);

    if ((!Even) == (FillRect.MinY & 1))
    {
        FillRect.MinY += 1;
    }

    s32 BytesPerPixel = BYTES_PER_PIXEL;
    u32 Color32       = ((RoundReal32ToUInt32(A * 255.0f) << 24) |
                   (RoundReal32ToUInt32(R * 255.0f) << RED_SPACE) |
                   (RoundReal32ToUInt32(G * 255.0f) << GREEN_SPACE) |
                   (RoundReal32ToUInt32(B * 255.0f) << BLUE_SPACE));
    u8 *Row           = ((u8 *) DrawBuffer->Memory +
               FillRect.MinX * BytesPerPixel +
               FillRect.MinY * DrawBuffer->Pitch);
    for (int Y = FillRect.MinY; Y < FillRect.MaxY; Y += 2)
    {
        u32 *Pixel = (u32 *) Row;
        for (int X = FillRect.MinX; X < FillRect.MaxX; ++X)
        {
            *Pixel++ = Color32;
        }
        Row += 2 * DrawBuffer->Pitch;
    }
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
    u8 *DestRow = ((u8 *) DrawBuffer->Memory +
                   MinX * BYTES_PER_PIXEL +
                   MinY * DrawBuffer->Pitch);

    for (int Y = MinY; Y < MaxY; ++Y)
    {
        u32 *Dest   = (u32 *) DestRow;
        u32 *Source = (u32 *) SourceRow;
        for (int X = MinX; X < MaxX; ++X)
        {
            v4 Texel = {(r32) ((*Source >> RED_SPACE) & 0xFF),
                        (r32) ((*Source >> GREEN_SPACE) & 0xFF),
                        (r32) ((*Source >> BLUE_SPACE) & 0xFF),
                        (r32) ((*Source >> 24) & 0xFF)};

            Texel = SRGB255ToLinear1(Texel);
            Texel *= CAlpha;

            v4 D = {(r32) ((*Dest >> RED_SPACE) & 0xFF),
                    (r32) ((*Dest >> GREEN_SPACE) & 0xFF),
                    (r32) ((*Dest >> BLUE_SPACE) & 0xFF),
                    (r32) ((*Dest >> 24) & 0xFF)};

            D = SRGB255ToLinear1(D);

            v4 Result = (1.0f - Texel.a) * D + Texel;

            Result = Linear1ToSRGB255(Result);

            *Dest = (((u32) (Result.r + 0.5f) << RED_SPACE) |
                     ((u32) (Result.g + 0.5f) << GREEN_SPACE) |
                     ((u32) (Result.b + 0.5f) << BLUE_SPACE) |
                     ((u32) (Result.a + 0.5f) << 24));
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

struct entity_basis_p_result
{
    v2  P;
    r32 Scale;
    b32 Valid;
};

inline entity_basis_p_result
GetRenderEntityBasisP(render_group *RenderGroup, v2 ScreenDim, render_entity_basis *EntityBasis)
{
    entity_basis_p_result Result = {};

    v3  EntityBaseP   = EntityBasis->Basis->P;
    r32 DistanceToPz  = RenderGroup->RenderCamera.DistanceAboveGround - EntityBaseP.z;
    r32 NearClipPlane = .2f;
    v3  RawXY         = V3((EntityBaseP.xy + EntityBasis->Offset.xy), 1.f);

    if (DistanceToPz > NearClipPlane)
    {
        v2 ScreenCenter = ScreenDim * .5f;
        v3 ProjectedXY  = (1.f / DistanceToPz) * RenderGroup->RenderCamera.FocalLength * RawXY;
        Result.P        = ScreenCenter + ProjectedXY.xy * RenderGroup->MetersToPixel;
        Result.Scale    = ProjectedXY.z * RenderGroup->MetersToPixel;
        Result.Valid    = true;
    }

    return Result;
}

internal void
RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget, rectangle2i ClipRect, b32 Even)
{
    BEGIN_TIMED_BLOCK(RenderGroupToOutput);
    v2 ScreenDim = {(r32) OutputTarget->Width,
                    (r32) OutputTarget->Height};

    r32 PixelsToMeter = 1.f / RenderGroup->MetersToPixel;

    for (u32 BaseAddress = 0;
         BaseAddress < RenderGroup->PushBufferSize;)
    {
        render_group_entry_header *Header = (render_group_entry_header *) (RenderGroup->PushBufferBase + BaseAddress);
        BaseAddress += sizeof(*Header);
        void *Data = (u8 *) Header + sizeof(*Header);

        switch (Header->Type)
        {
            case RenderGroupEntryType_render_entry_clear: {
                render_entry_clear *Entry = (render_entry_clear *) Data;

                DrawRectangle(OutputTarget, V2(0, 0),
                              V2((r32) OutputTarget->Width, (r32) OutputTarget->Height),
                              Entry->Color, ClipRect, Even);

                BaseAddress += sizeof(*Entry);
                break;
            }
            case RenderGroupEntryType_render_entry_rectangle: {
                render_entry_rectangle *Entry = (render_entry_rectangle *) Data;
                BaseAddress += sizeof(*Entry);

                entity_basis_p_result Basis = GetRenderEntityBasisP(RenderGroup, ScreenDim, &Entry->EntityBasis);
                DrawRectangle(OutputTarget, Basis.P, Basis.P + Basis.Scale * Entry->Dim, Entry->Color, ClipRect, Even);

                break;
            }

            case RenderGroupEntryType_render_entry_bitmap: {
                render_entry_bitmap *Entry = (render_entry_bitmap *) Data;
                BaseAddress += sizeof(*Entry);
#if 1

                entity_basis_p_result Basis = GetRenderEntityBasisP(RenderGroup, ScreenDim, &Entry->EntityBasis);
                Assert(Entry->Bitmap);

#if 0
                // DrawBitmap(OutputTarget, Entry->Bitmap, P.x, P.y, Entry->Color.a);
                DrawRectangleSlowly(OutputTarget, Basis.P,
                                     Basis.Scale * V2(Entry->Size.x, 0),
                                     Basis.Scale * V2(0, Entry->Size.y),
                                     Entry->Color, Entry->Bitmap, 0, 0, 0, 0, PixelsToMeter);
#else
                DrawRectangleQuickly(OutputTarget, Basis.P,
                                     Basis.Scale * V2(Entry->Size.x, 0),
                                     Basis.Scale * V2(0, Entry->Size.y),
                                     Entry->Color, Entry->Bitmap, 0, 0, 0, 0, PixelsToMeter, ClipRect, Even);

#endif
#endif
                break;
            }
            case RenderGroupEntryType_render_entry_coordinate_system: {
                render_entry_coordinate_system *Entry = (render_entry_coordinate_system *) Data;
#if 0
                BaseAddress += sizeof(*Entry);

                v4 Color = V4(1.f, 1.0f, 0, 0);
                v2 Dim   = {4, 4};
                v2 P     = Entry->Origin;
                DrawRectangle(OutputTarget, P, P + Dim, Color);

                P = Entry->Origin + Entry->XAxis;
                DrawRectangle(OutputTarget, P, P + Dim, Color);

                P = Entry->Origin + Entry->YAxis;
                DrawRectangle(OutputTarget, P, P + Dim, Color);

                P = Entry->Origin + Entry->XAxis + Entry->YAxis;
                DrawRectangle(OutputTarget, P, P + Dim, Color);

                DrawRectangleQuickly(OutputTarget, Entry->Origin, Entry->XAxis, Entry->YAxis, Entry->Color,
                                     Entry->Texture, Entry->NormalMap, Entry->Top, Entry->Middle, Entry->Bottom,
                                     PixelsToMeter, ClipRect, false);
#if 0
                for (u32 I = 0; I < ArrayCount(Entry->Points); ++I)
                {
                    v2 Point = Entry->Points[I];
                    Point = Entry->Origin + Point.x * Entry->XAxis + Point.y * Entry->YAxis;
                    DrawRectangle(OutputTarget, Point, Point + Dim, Entry->Color.r, Entry->Color.g, Entry->Color.b);
                }
#endif
#endif
                break;
            }
                InvalidDefaultCase;
        }
    }

    END_TIMED_BLOCK(RenderGroupToOutput);
}

struct tile_render_work
{
    render_group * RenderGroup;
    loaded_bitmap *OutputTarget;
    rectangle2i    ClipRect;
};

internal
PLATFORM_WORK_QUEUE_CALLBACK(DoTileRenderWork)
{
    tile_render_work *Work = (tile_render_work *) Data;
    RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, true);
    RenderGroupToOutput(Work->RenderGroup, Work->OutputTarget, Work->ClipRect, false);
}

internal void
TiledRenderGroupToOutput(platform_work_queue *RenderQueue,
                         render_group *RenderGroup, loaded_bitmap *OutputTarget)
{
    s32 const TileCountX = 4;
    s32 const TileCountY = 4;

    tile_render_work WorkArray[TileCountX * TileCountY];

    s32 TileWidth  = OutputTarget->Width / TileCountX;
    s32 TileHeight = OutputTarget->Height / TileCountY;

    s32 WorkCount = 0;

    for (s32 TileY = 0; TileY < TileCountY; TileY++)
    {
        for (s32 TileX = 0; TileX < TileCountY; TileX++)
        {
            tile_render_work *Work = WorkArray + WorkCount++;
            rectangle2i       ClipRect;
            ClipRect.MinX = TileX * TileWidth + 4;
            ClipRect.MaxX = ClipRect.MinX + TileWidth - 4;

            ClipRect.MinY = TileY * TileHeight + 4;
            ClipRect.MaxY = ClipRect.MinY + TileHeight - 4;

            Work->ClipRect     = ClipRect;
            Work->OutputTarget = OutputTarget;
            Work->RenderGroup  = RenderGroup;
#if 1
            PlatformAddEntry(RenderQueue, DoTileRenderWork, Work);
#else
            DoTileRenderWork(RenderQueue, Work);
#endif            
        }
    }

    PlatformCompleteAllWork(RenderQueue);
}

internal render_group *
         AllocateRenderGroup(memory_arena *Arena, u32 MaxPushBufferSize, u32 ResolutionPixelX, u32 ResolutionPixelY)
{
    render_group *Result = PushStruct(Arena, render_group);

    Result->PushBufferBase                 = (u8 *) PushSize(Arena, MaxPushBufferSize);
    Result->DefaultBasis                   = PushStruct(Arena, render_basis);
    Result->DefaultBasis->P                = V3(0, 0, 0);
    Result->MaxPushBufferSize              = MaxPushBufferSize;
    Result->PushBufferSize                 = 0;
    Result->GlobalAlpha                    = 1.f;
    Result->GameCamera.FocalLength         = .6f;
    Result->GameCamera.DistanceAboveGround = 9.f;
    Result->RenderCamera                   = Result->GameCamera;
    // Result->RenderCamera.DistanceAboveGround = 39.f;

    Result->MetersToPixel          = (r32) ResolutionPixelX * 0.625f;
    r32 PixelsToMeter              = 1.f / Result->MetersToPixel;
    Result->MonitorHalfDimInMeters = V2(.5f * ResolutionPixelX * PixelsToMeter, .5f * ResolutionPixelY * PixelsToMeter);
    return (Result);
}

inline void
PushBitmap(render_group *Group, loaded_bitmap *Bitmap, v3 Offset, r32 Height, v4 Color = V4(1, 1, 1, 1))
{
    render_entry_bitmap *Entry = PushRenderElement(Group, render_entry_bitmap);
    if (Entry)
    {
        Entry->EntityBasis.Basis = Group->DefaultBasis;
        Entry->Bitmap            = Bitmap;
        Entry->Size              = V2(Height * Bitmap->WidthOverHeight, Height);

        v2 Align = Hadamard(Bitmap->AlignPercentage, Entry->Size);

        Entry->EntityBasis.Offset = Offset - V3(Align, 0);
        Entry->Color              = Color * Group->GlobalAlpha;
    }
}

inline void
PushRect(render_group *Group, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    render_entry_rectangle *Piece = PushRenderElement(Group, render_entry_rectangle);
    if (Piece)
    {
        Piece->EntityBasis.Basis  = Group->DefaultBasis;
        Piece->EntityBasis.Offset = (Offset - V3(0.5f * Dim, 0));
        Piece->Color              = Color;
        Piece->Dim                = Dim;
    }
}

inline void
PushRectOutline(render_group *Group, v3 Offset, v2 Dim, v4 Color = V4(1, 1, 1, 1))
{
    r32 Thickness = 0.1f;
    PushRect(Group, Offset - V3(0, 0.5f * Dim.y, 0), V2(Dim.x, Thickness), Color);
    PushRect(Group, Offset + V3(0, 0.5f * Dim.y, 0), V2(Dim.x, Thickness), Color);

    PushRect(Group, Offset - V3(0.5f * Dim.x, 0, 0), V2(Thickness, Dim.y), Color);
    PushRect(Group, Offset + V3(0.5f * Dim.x, 0, 0), V2(Thickness, Dim.y), Color);
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
GetCoordinateSystem(render_group *Group, v2 Origin, v2 XAxis, v2 YAxis, v4 Color, loaded_bitmap *Texture, loaded_bitmap *NormalMap,
                    environment_map *Top, environment_map *Middle, environment_map *Bottom)
{
    render_entry_coordinate_system *Piece = PushRenderElement(Group, render_entry_coordinate_system);
    if (Piece)
    {
        Piece->Origin    = Origin;
        Piece->XAxis     = XAxis;
        Piece->YAxis     = YAxis;
        Piece->Color     = Color;
        Piece->Texture   = Texture;
        Piece->NormalMap = NormalMap;
        Piece->Top       = Top;
        Piece->Middle    = Middle;
        Piece->Bottom    = Bottom;
    }
    return (Piece);
}

inline v2
Unproject(render_group *Group, v2 ProjectedXY, r32 AtDistanceFromCamera)
{
    v2 WorldXY = (AtDistanceFromCamera / Group->GameCamera.FocalLength) * ProjectedXY;
    return (WorldXY);
}

inline rectangle2
GetCameraRectangleAtDistance(render_group *Group, r32 DistanceFromCamera)
{
    rectangle2 Result;

    v2 RawXY = Unproject(Group, Group->MonitorHalfDimInMeters, DistanceFromCamera);

    Result = RectCenterHalfDim(V2(0, 0), RawXY);
    return (Result);
}

inline rectangle2
GetCameraRectangleAtTarget(render_group *Group)
{
    rectangle2 Result = GetCameraRectangleAtDistance(Group, Group->GameCamera.DistanceAboveGround);
    return (Result);
}
