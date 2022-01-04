#include "geometry.h"
#include "../../utils/fileloader.h"
#include "accelerator/bvh.h"

#include <unordered_map>
#include <chrono>

internal std::unordered_map<std::string, Geometry*> GeometryRegistry;

bool Triangle::hit(const TriangleMesh* mesh, const Ray* r, f32 tmin, f32 tmax, HitRecord* rec) const
{
    Vector3 e1 = mesh->vertices[indicesVertex[0]] - mesh->vertices[indicesVertex[1]];
    Vector3 e2 = mesh->vertices[indicesVertex[2]] - mesh->vertices[indicesVertex[0]];
	Vector3 n = Vector3::Cross(e1, e2);

	Vector3 c = mesh->vertices[indicesVertex[0]] - r->origin;
	Vector3 r0 = Vector3::Cross(r->direction, c);
	f32 invDet = 1.0f / Vector3::Dot(n, r->direction);

	float u = Vector3::Dot(r0, e2) * invDet;
	float v = Vector3::Dot(r0, e1) * invDet;
	float w = 1.0f - u - v;

	// This order of comparisons guarantees that none of u, v, or t, are NaNs:
	// IEEE-754 mandates that they compare to false if the left hand side is a NaN.
	if (u >= 0.0f && v >= 0.0f && u + v <= 1.0f) {
		float t = Vector3::Dot(n, c) * invDet;
		if (t >= 0.0f && t < tmax) {
			rec->uv = Vector2(u, v);
			rec->t = t;
            Vector3 nup = mesh->normals[indicesNormal[1]] * u;
            Vector3 nvp = mesh->normals[indicesNormal[2]] * v;
            Vector3 nwp = mesh->normals[indicesNormal[0]] * w;
            rec->n = nup + nvp + nwp;
			rec->p = r->at(t);
			return true;
		}
	}
	return false;
}

internal TriangleMesh* ParseWavefrontFile(const std::string& filename)
{
    TriangleMesh* mesh = new TriangleMesh();

    std::vector<std::string> lines = FileLoader::readFile(filename);

    u64 currVertex = 0;
    mesh->vertexCount = FileLoader::countToken(lines, "v ");
	mesh->vertices = new Vector3[mesh->vertexCount];

    u64 currTexCoord = 0;
    mesh->texCoordCount = FileLoader::countToken(lines, "vt ");
	mesh->texCoords = new Vector2[mesh->texCoordCount];

    u64 currNormals = 0;
    mesh->normalCount = FileLoader::countToken(lines, "vn ");
	mesh->normals = new Vector3[mesh->normalCount];
	
    u64 currTriangles = 0;
	mesh->triangleCount = FileLoader::countToken(lines, "f ");
	mesh->triangles = new Triangle[mesh->triangleCount];

    for(auto line : lines)
    {
		if (line[0] == '#' || line[0] == '\0') 
        {
			continue;
		}
        else if (line[0] == 'o' || line[0] == 'g')
        {
            // TODO: Get mesh and group names
		}
        else if (line[0] == 'v' && line[1] == ' ')
        {
			mesh->vertices[currVertex++] = FileLoader::parseVector3(line);
		}
        else if (line[0] == 'v' && line[1] == 't')
        {
			mesh->texCoords[currTexCoord++] = FileLoader::parseVector2(line);
		}
        else if (line[0] == 'v' && line[1] == 'n')
        {
			mesh->normals[currNormals++] = FileLoader::parseVector3(line);
		}
        else if (line[0] == 's')
        {
			continue;
		}
        else if (line[0] == 'f')
        {
			Triangle* t = &mesh->triangles[currTriangles++];
            FileLoader::parseFaces(t->indicesVertex, t->indicesTexCoord, t->indicesNormal, line);
		}
        else // TODO: Don't Ignore usemtl and mtllib arguments
        {
            // std::cout << "Ignoring line: \"" << line << "\"\n";
		}
	}
    return mesh;
}

TriangleMesh* TriangleMesh::CreateMeshFromFile(const std::string& filename)
{
    auto t0 = std::chrono::steady_clock::now();
    std::cout << "Parsing wavefront file: " << filename << " ";
    TriangleMesh* m = ParseWavefrontFile(filename.c_str());
    std::cout << "Done [" << m->vertexCount / 1000 << "k vertices in " 
              << std::chrono::duration_cast<std::chrono::seconds>(
                  std::chrono::steady_clock::now() - t0
                 ).count() << "s].\n";

    t0 = std::chrono::steady_clock::now();
    std::cout << "Building BVH... ";
    m->bvh = BVHNodeTri::NewBVHTriTree(m);

    BVHNodeTri* node = m->bvh;
    u64 estimatedDepth = 0;

    while(node != nullptr)
    {
        node = node->nleft;
        estimatedDepth++;
    }

    std::cout << "Done [" << estimatedDepth << " estimated layers in " 
              << std::chrono::duration_cast<std::chrono::seconds>(
                  std::chrono::steady_clock::now() - t0
                 ).count() << "s].\n";
    return m;
}

TriangleMesh::~TriangleMesh()
{
    BVHNodeTri::FreeBVHTriTree(bvh);
    delete[] vertices;
    delete[] texCoords;
    delete[] normals;
    delete[] triangles;
}

void Geometry::RegisterGeometry(std::string name, Geometry* geometry)
{
    if(GeometryRegistry.find(name) != GeometryRegistry.end())
    {
        std::cerr << "warn: Global geometry registry already contains name " << name << " - ignoring..." << std::endl;
        delete geometry;
        return;
    }
    GeometryRegistry.emplace(name, geometry);
}

Geometry* Geometry::GetGeometry(std::string name)
{
    if(GeometryRegistry.find(name) != GeometryRegistry.end())
        return GeometryRegistry.at(name);
    else
        return nullptr;
}

void Geometry::UnloadAll()
{
    for(auto g : GeometryRegistry)
    {
        if(g.second != nullptr)
            delete g.second;
    }
}
