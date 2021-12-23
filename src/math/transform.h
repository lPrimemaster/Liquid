#pragma once

#include "vector.h"
#include "matrix.h"

struct Transform
{
    Matrix4 tmatrix;

    Vector3 position;
    Vector3 scaleValue;

    inline void rotate(const f32& ex, const f32& ey, const f32& ez)
    {
        tmatrix = Matrix4::Rotation(ex, ey, ez) * tmatrix;
    }

    inline void rotate(const Vector3& axis, const f32& v)
    {
        //TODO: Project axis on all directions and rotate
        std::cerr << "warn: Transform::rotate not implemented." << std::endl;
    }

    inline void translate(const Vector3& v)
    {
        tmatrix = Matrix4::Translation(v) * tmatrix;
    }

    inline void scale(const f32& sx, const f32& sy, const f32& sz)
    {
        Matrix4 sm = Matrix4::Identity();
        sm.data[0][0] = sx;
        sm.data[1][1] = sy;
        sm.data[2][2] = sz;
        tmatrix = sm * tmatrix;
    }
};
