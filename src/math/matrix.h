#pragma once
#include "../common.h"
#include "vector.h"

struct Matrix4;

inline Matrix4 operator*(const Matrix4& lhs, const Matrix4& rhs);
inline Vector3 operator*(const Matrix4& lhs, const Vector3& rhs);
inline Matrix4 operator*(const f32& lhs, const Matrix4& rhs);

struct Matrix4
{
    Matrix4()
    {
        this->data[0][0] = 1;
        this->data[1][1] = 1;
        this->data[2][2] = 1;
        this->data[3][3] = 1;
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

    inline static Matrix4 Rotation(f32 ex, f32 ey, f32 ez)
    {
        Matrix4 Rx = Identity();
        Matrix4 Ry = Identity();
        Matrix4 Rz = Identity();

        Rx.data[1][1] = Rx.data[2][2] = cosf(ex);
        Rx.data[2][1] = -sinf(ex);
        Rx.data[1][2] = -Rx.data[2][1];

        Ry.data[0][0] = Ry.data[2][2] = cosf(ey);
        Ry.data[0][3] = -sinf(ey);
        Ry.data[3][0] = -Ry.data[0][3];

        Rz.data[0][0] = Rz.data[1][1] = cosf(ez);
        Rz.data[0][1] = -sinf(ez);
        Rz.data[1][0] = -Rz.data[0][1];

        Matrix4 R = Rx * Ry * Rz;

        return R;
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
        Matrix4 r;

        f32 D2R = PI / 180.0f;
        f32 range = tanf((fov/ 2) * D2R) * near;	
		f32 left = -range * aspect;
		f32 right = range * aspect;
		f32 bottom = -range;
		f32 top = range;
        
		r.data[0][0] = (2 * near) / (right - left);
		r.data[1][1] = (2 * near) / (top - bottom);
		r.data[2][2] = - (far + near) / (far - near);
		r.data[2][3] = - 1;
		r.data[3][2] = - (2 * far * near) / (far - near);

        return r;
    }

    inline static Matrix4 LookAt(const Vector3& from, const Vector3& to, const Vector3& up)
    {
        Matrix4 r;

        Vector3 f = (to - from).normalized();
		Vector3 u = up.normalized();
		Vector3 s = Vector3::Cross(f, u).normalized();
		u = Vector3::Cross(s, f);

		r.data[0][0] = s.x;
		r.data[1][0] = s.y;
		r.data[2][0] = s.z;
		r.data[0][1] = u.x;
		r.data[1][1] = u.y;
		r.data[2][1] = u.z;
		r.data[0][2] =-f.x;
		r.data[1][2] =-f.y;
		r.data[2][2] =-f.z;
		r.data[3][0] =-Vector3::Dot(s, from);
		r.data[3][1] =-Vector3::Dot(u, from);
		r.data[3][2] = Vector3::Dot(f, from);
		return r;
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
