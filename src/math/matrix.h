#pragma once
#include "../common.h"
#include "vector.h"

struct Matrix4;

inline Matrix4 operator*(const Matrix4& lhs, const Matrix4& rhs);
inline Vector3 operator*(const Matrix4& lhs, const Vector3& rhs);
inline Matrix4 operator*(const f32& lhs, const Matrix4& rhs);

struct Matrix4
{
    Matrix4(const f32& v = 1)
    {
        this->data[0][0] = v;
        this->data[1][1] = v;
        this->data[2][2] = v;
        this->data[3][3] = v;
    }

    inline static Matrix4 Identity()
    {
        Matrix4 m;
        m.data[0][0] = 1;
        m.data[1][1] = 1;
        m.data[2][2] = 1;
        m.data[3][3] = 1;
        return m;
    }

    inline static Matrix4 Translation(const Vector3& v)
    {
        Matrix4 m = Identity();
        m.data[0][3] = v.x;
        m.data[1][3] = v.y;
        m.data[2][3] = v.z;
        return m;
    }

    inline static Matrix4 Rotation(f32 angle, const Vector3& v)
    {
        f32 a = angle;
		f32 c = cos(a);
		f32 s = sin(a);

		Vector3 axis = v.normalized();
		Vector3 temp = (1 - c) * axis;

        Matrix4 m(1);
		Matrix4 Rotate;
		Rotate.data[0][0] = c + temp.data[0] * axis.data[0];
		Rotate.data[0][1] = temp.data[0] * axis.data[1] + s * axis.data[2];
		Rotate.data[0][2] = temp.data[0] * axis.data[2] - s * axis.data[1];

		Rotate.data[1][0] = temp.data[1] * axis.data[0] - s * axis.data[2];
		Rotate.data[1][1] = c + temp.data[1] * axis.data[1];
		Rotate.data[1][2] = temp.data[1] * axis.data[2] + s * axis.data[0];

		Rotate.data[2][0] = temp.data[2] * axis.data[0] + s * axis.data[1];
		Rotate.data[2][1] = temp.data[2] * axis.data[1] - s * axis.data[0];
		Rotate.data[2][2] = c + temp.data[2] * axis.data[2];

		Matrix4 Result;
        for(i32 i = 0; i < 4; i++)
        {
            Result.data[0][i] = m.data[0][i] * Rotate.data[0][0] + m.data[1][i] * Rotate.data[0][1] + m.data[2][i] * Rotate.data[0][2];
            Result.data[1][i] = m.data[0][i] * Rotate.data[1][0] + m.data[1][i] * Rotate.data[1][1] + m.data[2][i] * Rotate.data[1][2];
            Result.data[2][i] = m.data[0][i] * Rotate.data[2][0] + m.data[1][i] * Rotate.data[2][1] + m.data[2][i] * Rotate.data[2][2];
            Result.data[3][i] = m.data[3][i];
        }
		return Result;
    }

    inline static Matrix4 Scale(f32 sx, f32 sy, f32 sz)
    {
        Matrix4 S = Identity();

        S.data[0][0] = sx;
        S.data[1][1] = sy;
        S.data[2][2] = sz;

        return S;
    }

    inline static Matrix4 Projection(f32 fov, f32 aspect, f32 near, f32 far)
    {
        Matrix4 Result(0);

        f32 D2R = PI / 180.0f;
        f32 tHfov = tanf((fov / 2) * D2R);
        
		Result.data[0][0] = 1 / (aspect * tHfov);
		Result.data[1][1] = 1 / (tHfov);
		Result.data[2][2] = -(far + near) / (far - near);
		Result.data[2][3] = -1;
		Result.data[3][2] = -(2 * far * near) / (far - near);

        return Result;
    }

    inline static Matrix4 LookAt(const Vector3& eye, const Vector3& center, const Vector3& up)
    {
        Vector3 f = (center - eye).normalized();
		Vector3 s = Vector3::Cross(f, up).normalized();
		Vector3 u = Vector3::Cross(s, f);

		Matrix4 Result(1);
		Result.data[0][0] = s.x;
		Result.data[1][0] = s.y;
		Result.data[2][0] = s.z;
		Result.data[0][1] = u.x;
		Result.data[1][1] = u.y;
		Result.data[2][1] = u.z;
		Result.data[0][2] =-f.x;
		Result.data[1][2] =-f.y;
		Result.data[2][2] =-f.z;
		Result.data[3][0] =-Vector3::Dot(s, eye);
		Result.data[3][1] =-Vector3::Dot(u, eye);
		Result.data[3][2] = Vector3::Dot(f, eye);
		return Result;
    }

    f32 data[4][4] = { 0 };
};

#define MULT_MAT_IJ(j, i) \
    lhs.data[j][0] * rhs.data[0][i] + \
    lhs.data[j][1] * rhs.data[1][i] + \
    lhs.data[j][2] * rhs.data[2][i] + \
    lhs.data[j][3] * rhs.data[3][i]

inline Matrix4 operator*(const Matrix4& lhs, const Matrix4& rhs)
{
    Matrix4 m;
    m.data[0][0] = MULT_MAT_IJ(0, 0);
    m.data[0][1] = MULT_MAT_IJ(0, 1);
    m.data[0][2] = MULT_MAT_IJ(0, 2);
    m.data[0][3] = MULT_MAT_IJ(0, 3);

    m.data[1][0] = MULT_MAT_IJ(1, 0);
    m.data[1][1] = MULT_MAT_IJ(1, 1);
    m.data[1][2] = MULT_MAT_IJ(1, 2);
    m.data[1][3] = MULT_MAT_IJ(1, 3);

    m.data[2][0] = MULT_MAT_IJ(2, 0);
    m.data[2][1] = MULT_MAT_IJ(2, 1);
    m.data[2][2] = MULT_MAT_IJ(2, 2);
    m.data[2][3] = MULT_MAT_IJ(2, 3);

    m.data[3][0] = MULT_MAT_IJ(3, 0);
    m.data[3][1] = MULT_MAT_IJ(3, 1);
    m.data[3][2] = MULT_MAT_IJ(3, 2);
    m.data[3][3] = MULT_MAT_IJ(3, 3);
    return m;
}

inline Vector3 operator*(const Matrix4& lhs, const Vector3& rhs)
{
    Vector3 v;

    v.x = rhs.x * lhs.data[0][0] + rhs.y * lhs.data[0][1] + rhs.z * lhs.data[0][2] + lhs.data[0][3];
    v.y = rhs.x * lhs.data[1][0] + rhs.y * lhs.data[1][1] + rhs.z * lhs.data[1][2] + lhs.data[1][3];
    v.z = rhs.x * lhs.data[2][0] + rhs.y * lhs.data[2][1] + rhs.z * lhs.data[2][2] + lhs.data[2][3];
    // NOTE: What to do with w value of vector ?
    // TODO: Use SIMD
    return v;
}

inline Matrix4 operator*(const f32& lhs, const Matrix4& rhs)
{
    Matrix4 m = rhs;
    m.data[0][0] *= lhs;
    m.data[0][1] *= lhs;
    m.data[0][2] *= lhs;
    m.data[0][3] *= lhs;
    
    m.data[1][0] *= lhs;
    m.data[1][1] *= lhs;
    m.data[1][2] *= lhs;
    m.data[1][3] *= lhs;

    m.data[2][0] *= lhs;
    m.data[2][1] *= lhs;
    m.data[2][2] *= lhs;
    m.data[2][3] *= lhs;

    m.data[3][0] *= lhs;
    m.data[3][1] *= lhs;
    m.data[3][2] *= lhs;
    m.data[3][3] *= lhs;
    return m;
}
