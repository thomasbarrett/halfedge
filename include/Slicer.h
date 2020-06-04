#ifndef GEOM_SLICER_H
#define GEOM_SLICER_H

#include <map>
#include <Mesh.h>

namespace geom {

class Slicer {
public:
    Slicer(const std::vector<FaceVertexMesh> &g, double dz);
    std::vector<float> whitelist() const;
    std::vector<float> write(const std::string &prefix) const;

private:

   using Point = std::array<double, 2>;

    struct VertexData {
        Point position;
        uint32_t edge_count = 0;
        std::array<uint64_t, 2> edges; 
    };

    struct Intersection {
        uint64_t id = ~static_cast<uint64_t>(0);
        Point position;
    };

    using Graph = std::map<uint64_t, VertexData>;
    using Polygon = std::vector<Point>;
    using Polygons = std::vector<Polygon>;

    std::vector<Slicer::Graph> graph_; 
    std::vector<std::vector<Slicer::Polygons>> polygons_;
    int slice_count;

    static uint64_t make_vertex(uint32_t v) {
        return (uint64_t) v;
    }

    static uint64_t make_edge(uint32_t v1, uint32_t v2) {
        return v1 < v2 ? (uint64_t) v1 << 32 | v2: (uint64_t) v2 << 32 | v1;
    }

    std::array<float, 2> minmax(const std::vector<FaceVertexMesh> &meshes);
    void sliceTriangles(const FaceVertexMesh &geometry, std::array<float, 2> bounds, double dz);
    void computeContours(const FaceVertexMesh &g, std::vector<Slicer::Polygons> &polygons);
    std::vector<float> exportImages(const std::string &path_prefix) const;
};

}

#endif /* GEOM_SLICER_H */