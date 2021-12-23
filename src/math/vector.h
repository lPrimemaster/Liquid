#pragma once
#include "../common.h"

struct Vector3;

inline Vector3 operator/(const Vector3& lhs, const f32& rhs);

struct Vector3
{
    Vector3() : x(0), y(0), z(0) {  }
    Vector3(f32 x, f32 y, f32 z) : x(x), y(y), z(z) {  }
    
    union
    {
        struct
        {
            f32 x, y, z;
        };
        f32 data[3];
    };

    static inline f32 Dot(const Vector3& a, const Vector3& b)
    {
        return a.x * b.x + a.y * b.y + a.z * b.z;
    }

    static inline Vector3 Cross(const Vector3& u, const Vector3& v)
    {
        return Vector3(
            u.data[1] * v.data[2] - u.data[2] * v.data[1],
            u.data[2] * v.data[0] - u.data[0] * v.data[2],
            u.data[0] * v.data[1] - u.data[1] * v.data[0]
        );
    }

    inline f32 length() const
    {
        return sqrtf(Dot(*this, *this));
    }

    inline Vector3 normalized() const
    {
        return *this / length();
    }

    inline Vector3 sqrtComponents() const
    {
        return Vector3(sqrtf(x), sqrtf(y), sqrtf(z));
    }

    inline bool nearZero() const
    {
        const f32 s = 1E-8f;
        return (fabs(x) < s) && (fabs(y) < s) && (fabs(z) < s);
    }
};

inline Vector3 operator+(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z);
}

inline Vector3 operator-(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z);
}

inline Vector3 operator*(const Vector3& lhs, const Vector3& rhs)
{
    return Vector3(lhs.x * rhs.x, lhs.y * rhs.y, lhs.z * rhs.z);
}

inline Vector3 operator*(const Vector3& lhs, const f32& rhs)
{
    return Vector3(lhs.x * rhs, lhs.y * rhs, lhs.z * rhs);
}

inline Vector3 operator*(const f32& lhs, const Vector3& rhs)
{
    return Vector3(rhs.x * lhs, rhs.y * lhs, rhs.z * lhs);
}

inline Vector3 operator/(const Vector3& lhs, const f32& rhs)
{
    return Vector3(lhs.x / rhs, lhs.y / rhs, lhs.z / rhs);
}

inline Vector3 operator-(const Vector3& v)
{
    return Vector3(-v.x, -v.y, -v.z);
}

struct Vector2;

inline Vector2 operator/(const Vector2& lhs, const f32& rhs);

struct Vector2
{
    Vector2() : x(0), y(0) {  }
    Vector2(f32 x, f32 y) : x(x), y(y) {  }
    
    union
    {
        union
        {
            struct
            {
                f32 x, y;
            };
            
            struct
            {
                f32 u, v;
            };
        };
        f32 data[2];
    };

    static inline f32 Dot(const Vector2& a, const Vector2& b)
    {
        return a.x * b.x + a.y * b.y;
    }

    inline f32 length() const
    {
        return sqrtf(Dot(*this, *this));
    }

    inline Vector2 normalized() const
    {
        return *this / length();
    }
};

inline Vector2 operator+(const Vector2& lhs, const Vector2& rhs)
{
    return Vector2(lhs.x + rhs.x, lhs.y + rhs.y);
}

inline Vector2 operator-(const Vector2& lhs, const Vector2& rhs)
{
    return Vector2(lhs.x - rhs.x, lhs.y - rhs.y);
}

inline Vector2 operator*(const Vector2& lhs, const f32& rhs)
{
    return Vector2(lhs.x * rhs, lhs.y * rhs);
}

inline Vector2 operator*(const f32& lhs, const Vector2& rhs)
{
    return Vector2(rhs.x * lhs, rhs.y * lhs);
}

inline Vector2 operator/(const Vector2& lhs, const f32& rhs)
{
    return Vector2(lhs.x / rhs, lhs.y / rhs);
}

inline Vector2 operator-(const Vector2& v)
{
    return Vector2(-v.x, -v.y);
}

