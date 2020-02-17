/**
 * \file Mesh.h
 * \author Thomas Barrett
 * \brief Mesh
 */

#include <string>
#include <vector>
#include <array>
#include <memory>

#ifndef MESH_HPP
#define MESH_HPP

struct HalfEdge;

/**
 * 
 */
struct Vertex {
    int index = -1;
    HalfEdge *halfedge = nullptr;
    friend class Mesh;
};

/**
 * 
 */
struct Edge {
    int index = -1;
    HalfEdge *halfedge = nullptr;
};

/**
 * 
 */
struct Corner {
    int index = -1;
    HalfEdge *halfedge = nullptr;
};

/**
 * 
 */
struct Face {
    int index = -1;
    HalfEdge *halfedge = nullptr;
};

/**
 *
 */
struct HalfEdge {
    int index = -1;
    Vertex *vertex = nullptr;
    Edge *edge = nullptr;
    Face *face = nullptr;
    Corner *corner = nullptr;
    HalfEdge *next = nullptr;
    HalfEdge *prev = nullptr;
    HalfEdge *twin = nullptr;
    bool onBoundary = true;
};

/**
 * 
 */
class Mesh {
private:
    std::vector<Vertex> vertices_;
    std::vector<Edge> edges_;
    std::vector<Face> faces_;
    std::vector<Corner> corners_;
    std::vector<HalfEdge> halfedges_;
public:
    Mesh(int vertexCount, const std::vector<std::array<int, 3>> &faces);

    bool closed() const;
    int eulerCharacteristic() const;

    const std::vector<Vertex>& vertices() const { return vertices_; }
    const std::vector<Edge>& edges() const { return edges_; }
    const std::vector<Face>& faces() const { return faces_; }
    const std::vector<Corner>& corners() const { return corners_; }
    const std::vector<HalfEdge>& halfedges() const { return halfedges_; }
};

/**
 * 
 */
class Geometry {
public:
    using Point = std::array<float, 3>;
    Geometry() = default;
    Geometry(std::istream &);
    virtual ~Geometry() = default;

    const Mesh& mesh() const { return *mesh_; }
    const std::vector<Point>& positions() const;
private:
    std::vector<std::array<float, 3>> positions_;
    std::unique_ptr<Mesh> mesh_;
};

#endif /* MESH_HPP */