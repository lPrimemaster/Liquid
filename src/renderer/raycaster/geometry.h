#pragma once

#include "../../common.h"
#include "../../math/vector.h"
#include "../../math/ray.h"
#include "hit_record.h"
#include "../../math/aabb.h"

struct Geometry
{
    static void RegisterGeometry(std::string name, Geometry* geometry);
    static Geometry* GetGeometry(std::string name);
    static void UnloadAll();
};

struct Sphere : Geometry
{
    
};

struct TriangleMesh;

struct Triangle
{
    u64 indicesVertex[3];
    u64 indicesTexCoord[3];
    u64 indicesNormal[3];
    AABB box; // TODO: This should be avoided. use ref to points instead to save space

    bool hit(const TriangleMesh* mesh, const Ray* r, f32 tmin, f32 tmax, HitRecord* rec) const;
};

struct BVHNodeTri;

struct TriangleMesh : Geometry
{
    BVHNodeTri* bvh;
    bool boxConstructed = false;

    Vector2* texCoords;
    u64 texCoordCount;

    Vector3* normals;
    u64 normalCount;

    Vector3* vertices;
    u64 vertexCount;

    Triangle* triangles;
    u64 triangleCount;

    static TriangleMesh* CreateMeshFromFile(const std::string& filename);
    static void FreeMesh(TriangleMesh* mesh);

    ~TriangleMesh();
};