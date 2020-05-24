/**
 * \file Mesh.h
 * \author Thomas Barrett
 * \brief Mesh
 */

#include <string>
#include <vector>
#include <array>
#include <memory>
#include <cmath>

#ifndef MESH_HPP
#define MESH_HPP

struct HalfEdge;
struct Vertex;
struct Edge;
struct Corner;
struct Face;
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
struct Edge {
    int index = -1;
    HalfEdge *halfedge = nullptr;
    std::vector<Face*> adjacentFaces() const {
        return {halfedge->face, halfedge->twin->face};
    }
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

    std::vector<Edge*> adjacentEdges() const {
        return {
            halfedge->edge,
            halfedge->next->edge,
            halfedge->next->next->edge
        };
    }

    std::vector<Vertex*> adjacentVertices() const {
        return {
            halfedge->vertex,
            halfedge->next->vertex,
            halfedge->next->next->vertex
        };
    }

    std::vector<HalfEdge*> adjacentHalfEdges() const {
        return {
            halfedge,
            halfedge->next,
            halfedge->next->next
        };
    }
};

/**
 *
 */

struct Vertex {
    int index = -1;
    HalfEdge *halfedge = nullptr;
    std::vector<Face*> adjacentFaces() const {
        std::vector<Face*> faces;
        HalfEdge *h = halfedge;
        do {
            faces.push_back(h->face);
            h = h->prev->twin;
        } while (h != halfedge);
        return faces;
    }
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
    using Triangle = std::array<int, 3>;

    Geometry() = default;
    Geometry(std::istream &);
    virtual ~Geometry() = default;


    static std::pair<double, double> minmax(const Geometry &g, const Triangle &t, int axis) {
        double tminz = INFINITY;
        double tmaxz = -INFINITY;
        for (int i = 0; i < 3; i++) {
            double z = g.positions()[t[i]][axis];
            if (z < tminz) tminz = z;
            if (z > tmaxz) tmaxz = z;
        } 
        return {tminz, tmaxz};
    }


    static std::pair<double, double> minmax(const Geometry &g, int axis) {
        auto [eminz, emaxz] = std::minmax_element(
            g.positions().begin(),
            g.positions().end(),
            [axis](auto &a, auto &b) { return a[axis] < b[axis]; }
        );
        float minz = (*eminz)[axis];
        float maxz = (*emaxz)[axis];   
        return {minz, maxz};
    }

    const std::vector<Point>& positions() const;
    const std::vector<Triangle>& faces() const;

  

private:
    std::vector<Point> positions_;
    std::vector<Triangle> faces_;

    std::vector<std::array<uint32_t, 3>> triangles_;
    std::vector<std::array<uint32_t, 2>> edges_;

public:
    const std::vector<std::array<uint32_t, 3>>& triangles() const {
        return triangles_;
    }

    const std::vector<std::array<uint32_t, 2>>& edges() const {
        return edges_;
    }
};

#endif /* MESH_HPP */