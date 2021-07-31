//
// Created by AgentOfChaos on 4/10/2021.
//

#if HANDMADE_WIN32
#define RED_PLACE   16
#define GREEN_PLACE 8
#define BLUE_PLACE  0

static v2
GetRenderEntityBasisP(render_group *RenderGroup, v2 &ScreenCenter, render_entry_rectangle *Entry, r32 MetersToPixel);

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
    v4 Result = V4((r32) ((Packed >> RED_PLACE) & 0xFF),
                   (r32) ((Packed >> GREEN_PLACE) & 0xFF),
                   (r32) ((Packed >> BLUE_PLACE) & 0xFF),
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

internal void
DrawRectangleQuickly(loaded_bitmap *Buffer, v2 Origin, v2 XAxis, v2 YAxis, v4 Color,
                     loaded_bitmap *  Texture,
                     loaded_bitmap *  NormalMap,
                     environment_map *Top,
                     environment_map *Middle,
                     environment_map *Bottom,
                     r32              PixelsToMeter)
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


    s32 WidthMax  = (Buffer->Width - 1) - 3;
    s32 HeightMax = (Buffer->Height - 1) - 3;

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

        if (MinX > FloorX) { MinX = FloorX; };
        if (MinY > FloorY) { MinY = FloorY; };
        if (MaxX < CeilX) { MaxX = CeilX; };
        if (MaxY < CeilY) { MaxY = CeilY; };
    }
    if (MinX < 0) { MinX = 0; };
    if (MinY < 0) { MinY = 0; };
    if (MaxX > Buffer->Width - 1) { MaxX = Buffer->Width - 1; };
    if (MaxY > Buffer->Height - 1) { MaxY = Buffer->Height - 1; };

    u8 *Row = ((u8 *) Buffer->Memory +
               MinX * BYTES_PER_PIXEL +
               MinY * Buffer->Pitch);

    v2 nXAxis = XAxis * InvXAxisLengthSq;
    v2 nYAxis = YAxis * InvYAxisLengthSq;

#if 0 // SSE2
#define mmSquare(a) _mm_mul_ps(a, a)
#define M(a, i)     ((r32 *) &(a))[i]
#define Mi(a, i)    ((u32 *) &(a))[i]

    __m128 Inv255_4x = _mm_set1_ps(1.0f / 255.f);
    __m128 One255_4x = _mm_set1_ps(255.f);

    __m128 One  = _mm_set1_ps(1.0f);
    __m128 Zero = _mm_set1_ps(0.0f);
    __m128 Half = _mm_set1_ps(0.5f);
    __m128 Four = _mm_set1_ps(4.0f);

    __m128 Colorr_4x = _mm_set1_ps(Color.r);
    __m128 Colorg_4x = _mm_set1_ps(Color.g);
    __m128 Colorb_4x = _mm_set1_ps(Color.b);
    __m128 Colora_4x = _mm_set1_ps(Color.a);

    __m128 Originx_4x = _mm_set1_ps(Origin.x);
    __m128 Originy_4x = _mm_set1_ps(Origin.y);

    __m128 nXAxisx_4x = _mm_set1_ps(nXAxis.x);
    __m128 nXAxisy_4x = _mm_set1_ps(nXAxis.y);
    __m128 nYAxisx_4x = _mm_set1_ps(nYAxis.x);
    __m128 nYAxisy_4x = _mm_set1_ps(nYAxis.y);

    __m128 WidthM2  = _mm_set1_ps((r32) (Texture->Width - 2));
    __m128 HeightM2 = _mm_set1_ps((r32) (Texture->Height - 2));

    __m128i MaskFF = _mm_set1_epi32(0xFF);

    BEGIN_TIMED_BLOCK(ProcessPixel);
    for (s32 Y = MinY; Y < MaxY; ++Y)
    {
        u32 *Pixel = (u32 *) Row;

        __m128 PixelPy = _mm_set1_ps((r32) Y);
        __m128 PixelPx = _mm_set_ps((r32) (MinX + 3),
                                    (r32) (MinX + 2),
                                    (r32) (MinX + 1),
                                    (r32) (MinX + 0));

        PixelPx = _mm_sub_ps(PixelPx, Originx_4x);
        PixelPy = _mm_sub_ps(PixelPy, Originy_4x);

        for (s32 XI = MinX; XI < MaxX; XI += 4)
        {
            __m128 U = _mm_add_ps(_mm_mul_ps(nXAxisx_4x, PixelPx), _mm_mul_ps(nXAxisy_4x, PixelPy));
            __m128 V = _mm_add_ps(_mm_mul_ps(nYAxisx_4x, PixelPx), _mm_mul_ps(nYAxisy_4x, PixelPy));

            __m128i WriteMask = _mm_castps_si128(_mm_and_ps(_mm_and_ps(_mm_cmple_ps(U, One), _mm_cmpge_ps(U, Zero)),
                                                            _mm_and_ps(_mm_cmple_ps(V, One), _mm_cmpge_ps(V, Zero))));

            // if (_mm_movemask_epi8(WriteMask))
            {
                __m128i OriginalDest = _mm_loadu_si128((__m128i *) Pixel);

                U = _mm_min_ps(_mm_max_ps(U, Zero), One);
                V = _mm_min_ps(_mm_max_ps(V, Zero), One);

                __m128 tX = _mm_mul_ps(U, WidthM2);
                __m128 tY = _mm_mul_ps(V, HeightM2);

                __m128i FetchX_4x = _mm_cvttps_epi32(tX);
                __m128i FetchY_4x = _mm_cvttps_epi32(tY);

                __m128 fX = _mm_sub_ps(tX, _mm_cvtepi32_ps(FetchX_4x));
                __m128 fY = _mm_sub_ps(tY, _mm_cvtepi32_ps(FetchY_4x));

                __m128i TexelSampleA;
                __m128i TexelSampleB;
                __m128i TexelSampleC;
                __m128i TexelSampleD;
                for (int I = 0; I < 4; ++I)
                {
                    s32 FetchX = Mi(FetchX_4x, I);
                    s32 FetchY = Mi(FetchY_4x, I);

                    Assert((FetchX >= 0) && (FetchX < Texture->Width));
                    Assert((FetchY >= 0) && (FetchY < Texture->Height));

                    //NOTE(rahul): Bilenear Sample
                    u8 *TexelPtr = ((u8 *) Texture->Memory) + (FetchY * Texture->Pitch) + FetchX * sizeof(u32);

                    Mi(TexelSampleA, I) = *(u32 *) TexelPtr;
                    Mi(TexelSampleB, I) = *(u32 *) (TexelPtr + sizeof(u32));
                    Mi(TexelSampleC, I) = *(u32 *) (TexelPtr + Texture->Pitch);
                    Mi(TexelSampleD, I) = *(u32 *) (TexelPtr + Texture->Pitch + sizeof(u32));
                }

                //NOTE(rahul): Convert texture from sRGB to "linear" brightness space

                __m128 TexelAr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleA, RED_PLACE), MaskFF));
                __m128 TexelAg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleA, GREEN_PLACE), MaskFF));
                __m128 TexelAb = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleA, BLUE_PLACE), MaskFF));
                __m128 TexelAa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleA, 24), MaskFF));

                __m128 TexelBr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleB, RED_PLACE), MaskFF));
                __m128 TexelBg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleB, GREEN_PLACE), MaskFF));
                __m128 TexelBb = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleB, BLUE_PLACE), MaskFF));
                __m128 TexelBa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleB, 24), MaskFF));

                __m128 TexelCr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleC, RED_PLACE), MaskFF));
                __m128 TexelCg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleC, GREEN_PLACE), MaskFF));
                __m128 TexelCb = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleC, BLUE_PLACE), MaskFF));
                __m128 TexelCa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleC, 24), MaskFF));

                __m128 TexelDr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleD, RED_PLACE), MaskFF));
                __m128 TexelDg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleD, GREEN_PLACE), MaskFF));
                __m128 TexelDb = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleD, BLUE_PLACE), MaskFF));
                __m128 TexelDa = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(TexelSampleD, 24), MaskFF));

                TexelAr = mmSquare(_mm_mul_ps(Inv255_4x, TexelAr));
                TexelAg = mmSquare(_mm_mul_ps(Inv255_4x, TexelAg));
                TexelAb = mmSquare(_mm_mul_ps(Inv255_4x, TexelAb));
                TexelAa = _mm_mul_ps(Inv255_4x, TexelAa);

                TexelBr = mmSquare(_mm_mul_ps(Inv255_4x, TexelBr));
                TexelBg = mmSquare(_mm_mul_ps(Inv255_4x, TexelBg));
                TexelBb = mmSquare(_mm_mul_ps(Inv255_4x, TexelBb));
                TexelBa = _mm_mul_ps(Inv255_4x, TexelBa);

                TexelCr = mmSquare(_mm_mul_ps(Inv255_4x, TexelCr));
                TexelCg = mmSquare(_mm_mul_ps(Inv255_4x, TexelCg));
                TexelCb = mmSquare(_mm_mul_ps(Inv255_4x, TexelCb));
                TexelCa = _mm_mul_ps(Inv255_4x, TexelCa);

                TexelDr = mmSquare(_mm_mul_ps(Inv255_4x, TexelDr));
                TexelDg = mmSquare(_mm_mul_ps(Inv255_4x, TexelDg));
                TexelDb = mmSquare(_mm_mul_ps(Inv255_4x, TexelDb));
                TexelDa = _mm_mul_ps(Inv255_4x, TexelDa);

                //NOTE(rahul): Bilinear texture blend
                __m128 ifX = _mm_sub_ps(One, fX);
                __m128 ifY = _mm_sub_ps(One, fY);
                __m128 l0  = _mm_mul_ps(ifY, ifX);
                __m128 l1  = _mm_mul_ps(ifY, fX);
                __m128 l2  = _mm_mul_ps(fY, ifX);
                __m128 l3  = _mm_mul_ps(fY, fX);

                __m128 Texelr = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAr), _mm_mul_ps(l1, TexelBr)),
                                           _mm_add_ps(_mm_mul_ps(l2, TexelCr), _mm_mul_ps(l3, TexelDr)));
                __m128 Texelg = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAg), _mm_mul_ps(l1, TexelBg)),
                                           _mm_add_ps(_mm_mul_ps(l2, TexelCg), _mm_mul_ps(l3, TexelDg)));
                __m128 Texelb = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAb), _mm_mul_ps(l1, TexelBb)),
                                           _mm_add_ps(_mm_mul_ps(l2, TexelCb), _mm_mul_ps(l3, TexelDb)));
                __m128 Texela = _mm_add_ps(_mm_add_ps(_mm_mul_ps(l0, TexelAa), _mm_mul_ps(l1, TexelBa)),
                                           _mm_add_ps(_mm_mul_ps(l2, TexelCa), _mm_mul_ps(l3, TexelDa)));

                //NOTE(rahul): Modulate by incoming color
                Texelr = _mm_mul_ps(Texelr, Colorr_4x);
                Texelg = _mm_mul_ps(Texelg, Colorg_4x);
                Texelb = _mm_mul_ps(Texelb, Colorb_4x);
                Texela = _mm_mul_ps(Texela, Colora_4x);

                //NOTE(rahul): Clamp colors to valid range
                Texelr = _mm_min_ps(_mm_max_ps(Texelr, Zero), One);
                Texelg = _mm_min_ps(_mm_max_ps(Texelg, Zero), One);
                Texelb = _mm_min_ps(_mm_max_ps(Texelb, Zero), One);

                //NOTE(rahul): Load Destinatioln
                __m128 Destr = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, RED_PLACE), MaskFF));
                __m128 Destg = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, GREEN_PLACE), MaskFF));
                __m128 Destb = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, BLUE_PLACE), MaskFF));
                __m128 Desta = _mm_cvtepi32_ps(_mm_and_si128(_mm_srli_epi32(OriginalDest, 24), MaskFF));

                //NOTE(rahul): SRGB to  Linear
                Destr = mmSquare(_mm_mul_ps(Inv255_4x, Destr));
                Destg = mmSquare(_mm_mul_ps(Inv255_4x, Destg));
                Destb = mmSquare(_mm_mul_ps(Inv255_4x, Destb));
                Desta = _mm_mul_ps(Inv255_4x, Desta);

                //NOTE(rahul): Destination blend
                __m128 InvTexelA = _mm_sub_ps(One, Texela);

                __m128 Blendedr = _mm_add_ps(_mm_mul_ps(InvTexelA, Destr), Texelr);
                __m128 Blendedg = _mm_add_ps(_mm_mul_ps(InvTexelA, Destg), Texelg);
                __m128 Blendedb = _mm_add_ps(_mm_mul_ps(InvTexelA, Destb), Texelb);
                __m128 Blendeda = _mm_add_ps(_mm_mul_ps(InvTexelA, Desta), Texela);

                //NOTE(rahul):Linear to SRGB
                Blendedr = _mm_mul_ps(One255_4x, _mm_sqrt_ps(Blendedr));
                Blendedg = _mm_mul_ps(One255_4x, _mm_sqrt_ps(Blendedg));
                Blendedb = _mm_mul_ps(One255_4x, _mm_sqrt_ps(Blendedb));
                Blendeda = _mm_mul_ps(One255_4x, Blendeda);

                __m128i Intr = _mm_cvtps_epi32(Blendedr);
                __m128i Intg = _mm_cvtps_epi32(Blendedg);
                __m128i Intb = _mm_cvtps_epi32(Blendedb);
                __m128i Inta = _mm_cvtps_epi32(Blendeda);

                __m128i Sr = _mm_slli_epi32(Intr, 16);
                __m128i Sg = _mm_slli_epi32(Intg, 8);
                __m128i Sb = Intb;
                __m128i Sa = _mm_slli_epi32(Inta, 24);

                __m128i Out = _mm_or_si128(_mm_or_si128(Sr, Sg), _mm_or_si128(Sb, Sa));

                __m128i MaskedOut = _mm_or_si128(_mm_and_si128(WriteMask, Out),
                                                 _mm_andnot_si128(WriteMask, OriginalDest));

                _mm_storeu_si128((__m128i *) Pixel, MaskedOut);
            }
            PixelPx = _mm_add_ps(PixelPx, Four);
            Pixel += 4;
        }
        Row += Buffer->Pitch;
    }
    END_TIMED_BLOCK_COUNTED(ProcessPixel, (MaxX - MinX + 1) * (MaxY - MinY + 1));

#else // AVX2

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

    __m256 nXAxisx = _mm256_set1_ps(nXAxis.x);
    __m256 nXAxisy = _mm256_set1_ps(nXAxis.y);
    __m256 nYAxisx = _mm256_set1_ps(nYAxis.x);
    __m256 nYAxisy = _mm256_set1_ps(nYAxis.y);

    __m256 WidthM2  = _mm256_set1_ps((r32) (Texture->Width - 2));
    __m256 HeightM2 = _mm256_set1_ps((r32) (Texture->Height - 2));

    __m256i MaskFF  = _mm256_set1_epi32(0xFF);


    BEGIN_TIMED_BLOCK(ProcessPixel);
    for (s32 Y = MinY; Y < MaxY; ++Y)
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

        for (s32 XI = MinX; XI < MaxX; XI += 8)
        {
            __m256 U = _mm256_add_ps(_mm256_mul_ps(nXAxisx, PixelPx), _mm256_mul_ps(nXAxisy, PixelPy));
            __m256 V = _mm256_add_ps(_mm256_mul_ps(nYAxisx, PixelPx), _mm256_mul_ps(nYAxisy, PixelPy));

            __m256i WriteMask = _mm256_castps_si256(
            _mm256_and_ps(_mm256_and_ps(_mm256_cmp_ps(U, One, _CMP_LE_OQ), _mm256_cmp_ps(U, Zero, _CMP_GE_OQ)),
                          _mm256_and_ps(_mm256_cmp_ps(V, One, _CMP_LE_OQ), _mm256_cmp_ps(V, Zero, _CMP_GE_OQ))));

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

                __m256i TexelSampleA;
                __m256i TexelSampleB;
                __m256i TexelSampleC;
                __m256i TexelSampleD;
                for (int I = 0; I < 8; ++I)
                {
                    s32 FetchX = Mi(FetchX_8x, I);
                    s32 FetchY = Mi(FetchY_8x, I);

                    Assert((FetchX >= 0) && (FetchX < Texture->Width));
                    Assert((FetchY >= 0) && (FetchY < Texture->Height));

                    //NOTE(rahul): Bilenear Sample
                    u8 *TexelPtr = ((u8 *) Texture->Memory) + (FetchY * Texture->Pitch) + FetchX * sizeof(u32);

                    Mi(TexelSampleA, I) = *(u32 *) TexelPtr;
                    Mi(TexelSampleB, I) = *(u32 *) (TexelPtr + sizeof(u32));
                    Mi(TexelSampleC, I) = *(u32 *) (TexelPtr + Texture->Pitch);
                    Mi(TexelSampleD, I) = *(u32 *) (TexelPtr + Texture->Pitch + sizeof(u32));
                }

                //NOTE(rahul): Convert texture from sRGB to "linear" brightness space

                __m256 TexelAr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, RED_PLACE), MaskFF));
                __m256 TexelAg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, GREEN_PLACE), MaskFF));
                __m256 TexelAb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, BLUE_PLACE), MaskFF));
                __m256 TexelAa = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleA, 24), MaskFF));

                __m256 TexelBr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, RED_PLACE), MaskFF));
                __m256 TexelBg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, GREEN_PLACE), MaskFF));
                __m256 TexelBb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, BLUE_PLACE), MaskFF));
                __m256 TexelBa = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleB, 24), MaskFF));

                __m256 TexelCr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, RED_PLACE), MaskFF));
                __m256 TexelCg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, GREEN_PLACE), MaskFF));
                __m256 TexelCb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, BLUE_PLACE), MaskFF));
                __m256 TexelCa = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleC, 24), MaskFF));

                __m256 TexelDr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleD, RED_PLACE), MaskFF));
                __m256 TexelDg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleD, GREEN_PLACE), MaskFF));
                __m256 TexelDb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(TexelSampleD, BLUE_PLACE), MaskFF));
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
                __m256 Destr = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(OriginalDest, RED_PLACE), MaskFF));
                __m256 Destg = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(OriginalDest, GREEN_PLACE), MaskFF));
                __m256 Destb = _mm256_cvtepi32_ps(_mm256_and_si256(_mm256_srli_epi32(OriginalDest, BLUE_PLACE), MaskFF));
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
                Blendedr = _mm256_mul_ps(One255, _mm256_sqrt_ps(Blendedr));
                Blendedg = _mm256_mul_ps(One255, _mm256_sqrt_ps(Blendedg));
                Blendedb = _mm256_mul_ps(One255, _mm256_sqrt_ps(Blendedb));
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
        }
        Row += Buffer->Pitch;
    }
    END_TIMED_BLOCK_COUNTED(ProcessPixel, (MaxX - MinX + 1) * (MaxY - MinY + 1));
#endif
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

        if (MinX > FloorX) { MinX = FloorX; };
        if (MinY > FloorY) { MinY = FloorY; };
        if (MaxX < CeilX) { MaxX = CeilX; };
        if (MaxY < CeilY) { MaxY = CeilY; };
    }
    if (MinX < 0) { MinX = 0; };
    if (MinY < 0) { MinY = 0; };
    if (MaxX > Buffer->Width - 1) { MaxX = Buffer->Width - 1; };
    if (MaxY > Buffer->Height - 1) { MaxY = Buffer->Height - 1; };

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
    
                        v4 Normal = Lerp(Lerp(NormalA, fX, NormalB),
                                         fY,
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

                v4 Dest = {(r32) ((*Pixel >> RED_PLACE) & 0xFF),
                           (r32) ((*Pixel >> GREEN_PLACE) & 0xFF),
                           (r32) ((*Pixel >> BLUE_PLACE) & 0xFF),
                           (r32) ((*Pixel >> 24) & 0xFF)};

                Dest = SRGB255ToLinear1(Dest);

                v4 Blended    = (1.0f - Texel.a) * Dest + Texel;
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
    END_TIMED_BLOCK_COUNTED(ProcessPixel, (MaxX - MinX + 1) * (MaxY - MinY + 1));

    END_TIMED_BLOCK(DrawRectangleSlowly);
}

internal void
DrawRectangle(loaded_bitmap *DrawBuffer, v2 vMin, v2 vMax, v4 Color)
{
    r32 R = Color.r;
    r32 G = Color.g;
    r32 B = Color.b;
    r32 A = Color.a;

    s32 MinX = RoundReal32ToInt32(vMin.x);
    s32 MinY = RoundReal32ToInt32(vMin.y);
    s32 MaxX = RoundReal32ToInt32(vMax.x);
    s32 MaxY = RoundReal32ToInt32(vMax.y);

    if (MinX < 0) { MinX = 0; }
    if (MinY < 0) { MinY = 0; }
    if (MaxX > DrawBuffer->Width) { MaxX = DrawBuffer->Width; }
    if (MaxY > DrawBuffer->Height) { MaxY = DrawBuffer->Height; }

    s32 BytesPerPixel = BYTES_PER_PIXEL;
    u32 Color32       = ((RoundReal32ToUInt32(A * 255.0f) << 24) |
                   (RoundReal32ToUInt32(R * 255.0f) << RED_PLACE) |
                   (RoundReal32ToUInt32(G * 255.0f) << GREEN_PLACE) |
                   (RoundReal32ToUInt32(B * 255.0f) << BLUE_PLACE));
    u8 *Row           = ((u8 *) DrawBuffer->Memory +
               MinX * BytesPerPixel +
               MinY * DrawBuffer->Pitch);
    for (int Y = MinY; Y < MaxY; ++Y)
    {
        u32 *Pixel = (u32 *) Row;
        for (int X = MinX; X < MaxX; ++X)
        {
            *Pixel++ = Color32;
        }
        Row += DrawBuffer->Pitch;
    }
}

inline void
DrawRectangleOutline(loaded_bitmap *DrawBuffer, v2 vMin, v2 vMax, v3 Color, r32 R = 2.0f)
{

    DrawRectangle(DrawBuffer, V2(vMin.x - R, vMin.y - R), V2(vMax.x + R, vMin.y + R),
                  V4(Color, 1.f));
    DrawRectangle(DrawBuffer, V2(vMin.x - R, vMax.y - R), V2(vMax.x + R, vMax.y + R),
                  V4(Color, 1.f));

    DrawRectangle(DrawBuffer, V2(vMin.x - R, vMin.y - R), V2(vMin.x + R, vMax.y + R),
                  V4(Color, 1.f));
    DrawRectangle(DrawBuffer, V2(vMax.x - R, vMin.y - R), V2(vMax.x + R, vMax.y + R),
                  V4(Color, 1.f));
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
            v4 Texel = {(r32) ((*Source >> RED_PLACE) & 0xFF),
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
                     ((u32) (Result.b + 0.5f) << BLUE_PLACE) |
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
RenderGroupToOutput(render_group *RenderGroup, loaded_bitmap *OutputTarget)
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
                              Entry->Color);

                BaseAddress += sizeof(*Entry);
                break;
            }
            case RenderGroupEntryType_render_entry_rectangle: {
                render_entry_rectangle *Entry = (render_entry_rectangle *) Data;
                BaseAddress += sizeof(*Entry);

                entity_basis_p_result Basis = GetRenderEntityBasisP(RenderGroup, ScreenDim, &Entry->EntityBasis);
                DrawRectangle(OutputTarget, Basis.P, Basis.P + Basis.Scale * Entry->Dim, Entry->Color);

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
                                     Entry->Color, Entry->Bitmap, 0, 0, 0, 0, PixelsToMeter);
#endif
#endif
                break;
            }
            case RenderGroupEntryType_render_entry_coordinate_system: {
                render_entry_coordinate_system *Entry = (render_entry_coordinate_system *) Data;
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
                                     PixelsToMeter);

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

    END_TIMED_BLOCK(RenderGroupToOutput);
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
