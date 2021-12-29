//
// Created by AgentOfChaos on 12/14/2020.
//

#ifndef HANDMADEHERO_HANDMADE_MATH_H

inline r32
SafeRatioN(r32 Numerator, r32 Divisor, r32 N)
{
    r32 Result = N;

    if (Divisor != 0.0f)
    {
        Result = Numerator / Divisor;
    }

    return (Result);
}

inline r32
SafeRatio0(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 0.0f);
    return (Result);
}

inline r32
SafeRatio1(r32 Numerator, r32 Divisor)
{
    r32 Result = SafeRatioN(Numerator, Divisor, 1.0f);
    return (Result);
}

struct v2
{
    union
    {
        struct
        {
            r32 x, y;
        };
        struct
        {
            r32 u, v;
        };
        r32 E[2];
    };
};

struct v3
{
    union
    {
        struct
        {
            r32 x, y, z;
        };

        struct
        {
            r32 r, g, b;
        };

        struct
        {
            r32 u, v, w;
        };

        struct
        {
            v2  xy;
            r32 Ignore0_;
        };

        struct
        {
            r32 Ignore1_;
            v2  yz;
        };

        struct
        {
            v2  uv;
            r32 Ignore2_;
        };

        struct
        {
            v2  vw;
            r32 Ignore3_;
        };
        r32 e[3];
    };
};

union v4
{
    struct
    {
        union
        {
            v3 xyz;
            struct
            {
                r32 x, y, z;
            };
        };
        r32 w;
    };

    struct
    {
        union
        {
            v3 rgb;
            struct
            {
                r32 r, g, b;
            };
        };
        r32 a;
    };

    struct
    {
        v2  xy;
        r32 Ignore0_;
        r32 Ignore1_;
    };

    struct
    {
        r32 Ignore2_;
        v2  yz;
        r32 Ignore3_;
    };
    struct
    {
        r32 Ignore4_;
        r32 Ignore5_;
        v2  zw;
    };

    r32 e[4];
};

inline v2 V2(r32 X, r32 Y)
{
    v2 Result = {};
    Result.x  = X;
    Result.y  = Y;
    return (Result);
}

inline v2 Perp(v2 A)
{
    v2 Result = V2(-A.y, A.x);
    return (Result);
}

inline v3 V3(r32 X, r32 Y, r32 Z)
{
    v3 Result = {};
    Result.x  = X;
    Result.y  = Y;
    Result.z  = Z;

    return (Result);
}

inline v3 V3(v2 XY, r32 Z)
{
    v3 Result = {};
    Result.x  = XY.x;
    Result.y  = XY.y;
    Result.z  = Z;

    return (Result);
}

inline v4 V4(r32 R, r32 G, r32 B, r32 A)
{
    v4 Result = {};
    Result.x  = R;
    Result.y  = G;
    Result.z  = B;
    Result.w  = A;

    return (Result);
}

struct rectangle2
{
    v2 Min;
    v2 Max;
};

struct rectangle3
{
    v3 Min;
    v3 Max;
};

//
// NOTE(rahul): V2 Operations
//

inline v2
operator+(v2 A, v2 B)
{
    v2 Result = {};
    Result.x  = A.x + B.x;
    Result.y  = A.y + B.y;
    return (Result);
}

inline v2
operator*(r32 A, v2 B)
{
    v2 Result = {};
    Result.x  = A * B.x;
    Result.y  = A * B.y;
    return (Result);
}

inline v2
operator*(v2 B, r32 A)
{
    v2 Result = A * B;
    return (Result);
}

inline v2
operator-(v2 A)
{
    v2 Result = {};
    Result.x  = -A.x;
    Result.y  = -A.y;
    return (Result);
}

inline v2
operator-(v2 A, v2 B)
{
    v2 Result = {};
    Result.x  = A.x - B.x;
    Result.y  = A.y - B.y;
    return (Result);
}

inline v2 &
operator*=(v2 &B, r32 A)
{
    B = A * B;
    return (B);
}

inline v2 &
operator+=(v2 &B, v2 A)
{
    B = A + B;
    return (B);
}

inline r32
DotProduct(v2 A, v2 B)
{
    r32 Result = A.x * B.x + A.y * B.y;
    return (Result);
}

inline v2
Hadamard(v2 A, v2 B)
{
    v2 Result = {A.x * B.x, A.y * B.y};
    return (Result);
}

inline r32
LengthSq(v2 A)
{
    r32 Result = DotProduct(A, A);
    return (Result);
}

inline r32
Length(v2 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return (Result);
}

//
// Note(rahul): V3 Operations
//

inline v3
operator+(v3 A, v3 B)
{
    v3 Result = {};
    Result.x  = A.x + B.x;
    Result.y  = A.y + B.y;
    Result.z  = A.z + B.z;
    return (Result);
}

inline v3
operator*(r32 A, v3 B)
{
    v3 Result = {};
    Result.x  = A * B.x;
    Result.y  = A * B.y;
    Result.z  = A * B.z;

    return (Result);
}

inline v3
operator*(v3 B, r32 A)
{
    v3 Result = A * B;
    return (Result);
}

inline v3
operator-(v3 A)
{
    v3 Result = {};
    Result.x  = -A.x;
    Result.y  = -A.y;
    Result.z  = -A.z;
    return (Result);
}

inline v3
operator-(v3 A, v3 B)
{
    v3 Result = {};
    Result.x  = A.x - B.x;
    Result.y  = A.y - B.y;
    Result.z  = A.z - B.z;
    return (Result);
}

inline v3 &
operator*=(v3 &B, r32 A)
{
    B = A * B;
    return (B);
}

inline v3 &
operator+=(v3 &B, v3 A)
{
    B = A + B;
    return (B);
}

inline v3
Hadamard(v3 A, v3 B)
{
    v3 Result = {A.x * B.x, A.y * B.y, A.z * B.z};
    return (Result);
}

inline r32
DotProduct(v3 A, v3 B)
{
    r32 Result = A.x * B.x + A.y * B.y + A.z * B.z;
    return (Result);
}

inline r32
LengthSq(v3 A)
{
    r32 Result = DotProduct(A, A);
    return (Result);
}

inline r32
Length(v3 A)
{
    r32 Result = SquareRoot(LengthSq(A));
    return (Result);
}

inline v3
Lerp(v3 A, r32 t, v3 B)
{
    v3 Result = (1.0f - t) * A + t * B;
    return (Result);
}

inline v3
Normalize(v3 A)
{
    v3 Result;
    Result = A * (1.f / Length(A));

    return (Result);
}

//
// NOTE(rahul): Scalar operations
//

inline r32
Square(r32 A)
{
    r32 Result = A * A;
    return (Result);
}

inline r32
Lerp(r32 A, r32 t, r32 B)
{
    r32 Result = (1.0f - t) * A + t * B;
    return (Result);
}

inline r32
Clamp(r32 Min, r32 Value, r32 Max)
{
    r32 Result = Value;

    if (Result < Min)
    {
        Result = Min;
    } else if (Result > Max)
    {
        Result = Max;
    }
    return (Result);
}

inline r32
Clamp01(r32 Value)
{
    r32 Result = Clamp(0.0f, Value, 1.0f);
    return (Result);
}

inline r32
Clamp01MapToRange(r32 Min, r32 t, r32 Max)
{
    r32 Result = 0.f;

    r32 Range = Max - Min;
    if (Range != 0.f)
    {
        Result = Clamp01((t - Min) / Range);
    }
    return (Result);
}

//
// NOTE(rahul): rectangle2 operations
//

inline v2
V2i(s32 X, s32 Y)
{
    v2 Result = {(r32) X, (r32) Y};
    return (Result);
}

inline v2
V2i(u32 X, u32 Y)
{
    v2 Result = {(r32) X, (r32) Y};
    return (Result);
}

inline v2
GetDim(rectangle2 Rect)
{
    v2 Result = Rect.Max - Rect.Min;
    return (Result);
}

inline v2
GetMinCorner(rectangle2 Rect)
{
    v2 Result = Rect.Min;
    return (Result);
}

inline v2
GetMaxCorner(rectangle2 Rect)
{
    v2 Result = Rect.Max;
    return (Result);
}

inline v2
GetCenter(rectangle2 Rect)
{
    v2 Result = 0.5f * (Rect.Min + Rect.Max);
    return (Result);
}

inline rectangle2
RectMinMax(v2 Min, v2 Max)
{
    rectangle2 Result = {};
    Result.Min        = Min;
    Result.Max        = Max;
    return (Result);
}

inline rectangle2
RectMinDim(v2 Min, v2 Dim)
{
    rectangle2 Result = {};
    Result.Min        = Min;
    Result.Max        = Min + Dim;
    return (Result);
}

inline rectangle2
RectCenterHalfDim(v2 Center, v2 HalfDim)
{
    rectangle2 Result = {};
    Result.Min        = Center - HalfDim;
    Result.Max        = Center + HalfDim;
    return (Result);
}

inline rectangle2
RectCenterDim(v2 Center, v2 Dim)
{
    rectangle2 Result = RectCenterHalfDim(Center, 0.5f * Dim);

    return (Result);
}

inline b32
IsInRectangle(rectangle2 Rectangle, v2 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) &&
                  (Test.y >= Rectangle.Min.y) &&
                  (Test.x < Rectangle.Max.x) &&
                  (Test.y < Rectangle.Max.y));
    return (Result);
}

inline rectangle2
AddRadiusTo(rectangle2 A, v2 Radius)
{
    rectangle2 Result = {};
    Result.Min        = A.Min - Radius;
    Result.Max        = A.Max + Radius;
    return (Result);
}

inline v2
Clamp01(v2 Value)
{
    v2 Result;
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);

    return (Result);
}

inline v2
GetBarycentric(rectangle2 A, v2 P)
{
    v2 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);

    return (Result);
}

//
// NOTE(rahul): rectangle3 operations
//

inline v3
Clamp01(v3 Value)
{
    v3 Result;
    Result.x = Clamp01(Value.x);
    Result.y = Clamp01(Value.y);
    Result.z = Clamp01(Value.z);

    return (Result);
}

inline v3
GetMinCorner(rectangle3 Rect)
{
    v3 Result = Rect.Min;
    return (Result);
}

inline v3
GetMaxCorner(rectangle3 Rect)
{
    v3 Result = Rect.Max;
    return (Result);
}

inline v3
GetDim(rectangle3 Rect)
{
    v3 Result = Rect.Max - Rect.Min;
    return (Result);
}

inline v3
GetCenter(rectangle3 Rect)
{
    v3 Result = 0.5f * (Rect.Min + Rect.Max);
    return (Result);
}

inline rectangle3
RectMinMax(v3 Min, v3 Max)
{
    rectangle3 Result = {};
    Result.Min        = Min;
    Result.Max        = Max;
    return (Result);
}

inline rectangle3
RectMinDim(v3 Min, v3 Dim)
{
    rectangle3 Result = {};
    Result.Min        = Min;
    Result.Max        = Min + Dim;
    return (Result);
}

inline rectangle3
RectCenterHalfDim(v3 Center, v3 HalfDim)
{
    rectangle3 Result = {};
    Result.Min        = Center - HalfDim;
    Result.Max        = Center + HalfDim;
    return (Result);
}

inline rectangle3
RectCenterDim(v3 Center, v3 Dim)
{
    rectangle3 Result = RectCenterHalfDim(Center, 0.5f * Dim);

    return (Result);
}

inline b32
IsInRectangle(rectangle3 Rectangle, v3 Test)
{
    b32 Result = ((Test.x >= Rectangle.Min.x) &&
                  (Test.y >= Rectangle.Min.y) &&
                  (Test.z >= Rectangle.Min.z) &&
                  (Test.x < Rectangle.Max.x) &&
                  (Test.y < Rectangle.Max.y) &&
                  (Test.z < Rectangle.Max.z));

    return (Result);
}

inline rectangle3
AddRadiusTo(rectangle3 A, v3 Radius)
{
    rectangle3 Result = {};
    Result.Min        = A.Min - Radius;
    Result.Max        = A.Max + Radius;
    return (Result);
}

inline rectangle3
Offset(rectangle3 A, v3 Offset)
{
    rectangle3 Result;
    Result.Min = A.Min + Offset;
    Result.Max = A.Max + Offset;
    return (Result);
}

inline b32
RectangleIntersect(rectangle3 A, rectangle3 B)
{
    b32 Result = !((B.Max.x <= A.Min.x) ||
                   (B.Min.x >= A.Max.x) ||
                   (B.Max.y <= A.Min.y) ||
                   (B.Min.y >= A.Max.y) ||
                   (B.Max.z <= A.Min.z) ||
                   (B.Min.z >= A.Max.z));
    return (Result);
}

inline v3
GetBarycentric(rectangle3 A, v3 P)
{
    v3 Result;

    Result.x = SafeRatio0(P.x - A.Min.x, A.Max.x - A.Min.x);
    Result.y = SafeRatio0(P.y - A.Min.y, A.Max.y - A.Min.y);
    Result.z = SafeRatio0(P.z - A.Min.z, A.Max.z - A.Min.z);

    return (Result);
}

//

inline v4
V4(v3 XYZ, r32 w)
{
    v4 Result;
    Result.xyz = XYZ;
    Result.w   = w;
    return (Result);
}
inline v4
operator+(v4 A, v4 B)
{
    v4 Result = {};
    Result.x  = A.x + B.x;
    Result.y  = A.y + B.y;
    Result.z  = A.z + B.z;
    Result.w  = A.w + B.w;
    return (Result);
}

inline v4
operator*(r32 A, v4 B)
{
    v4 Result = {};
    Result.x  = A * B.x;
    Result.y  = A * B.y;
    Result.z  = A * B.z;
    Result.w  = A * B.w;

    return (Result);
}

inline v4
operator*(v4 B, r32 A)
{
    v4 Result = A * B;
    return (Result);
}

inline v4
operator-(v4 A)
{
    v4 Result = {};
    Result.x  = -A.x;
    Result.y  = -A.y;
    Result.z  = -A.z;
    Result.w  = -A.w;

    return (Result);
}

inline v4
operator-(v4 A, v4 B)
{
    v4 Result = {};
    Result.x  = A.x - B.x;
    Result.y  = A.y - B.y;
    Result.z  = A.z - B.z;
    Result.w  = A.w - B.w;

    return (Result);
}

inline v4 &
operator*=(v4 &B, r32 A)
{
    B = A * B;
    return (B);
}

inline v4 &
operator+=(v4 &B, v4 A)
{
    B = A + B;
    return (B);
}

inline v4
Hadamard(v4 A, v4 B)
{
    v4 Result = {A.x * B.x, A.y * B.y, A.z * B.z, A.w * B.w};
    return (Result);
}

inline v4
Lerp(v4 A, r32 t, v4 B)
{
    v4 Result = (1.0f - t) * A + t * B;
    return (Result);
}

struct rectangle2i
{
    s32 MinX, MinY;
    s32 MaxX, MaxY;
};

inline rectangle2i
Intersect(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.MinX = (A.MinX < B.MinX) ? B.MinX : A.MinX;
    Result.MinY = (A.MinY < B.MinY) ? B.MinY : A.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? B.MaxX : A.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? B.MaxY : A.MaxY;

    return (Result);
}

inline rectangle2i
Union(rectangle2i A, rectangle2i B)
{
    rectangle2i Result;

    Result.MinX = (A.MinX < B.MinX) ? A.MinX : B.MinX;
    Result.MinY = (A.MinY < B.MinY) ? A.MinY : B.MinY;
    Result.MaxX = (A.MaxX > B.MaxX) ? A.MaxX : B.MaxX;
    Result.MaxY = (A.MaxY > B.MaxY) ? A.MaxY : B.MaxY;

    return (Result);
}

inline s32
GetClampedRectArea(rectangle2i A)
{
    s32 Width  = (A.MaxX - A.MinX);
    s32 Height = (A.MaxY - A.MinY);
    s32 Result = 0;
    if ((Width > 0) && (Height > 0))
    {
        Result = Width * Height;
    }
    return (Result);
}

inline b32
HasArea(rectangle2i A)
{
    b32 Result = ((A.MinX < A.MaxX) && (A.MinY < A.MaxY));
    return (Result);
}

inline rectangle2i
InvertedInfinityRectangle(void)
{
    rectangle2i Result;

    Result.MinX = Result.MinY = INT_MAX;
    Result.MaxX = Result.MaxY = -INT_MAX;
    return (Result);
}

#define HANDMADEHERO_HANDMADE_MATH_H

#endif//HANDMADEHERO_HANDMADE_MATH_H
