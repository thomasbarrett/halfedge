
#ifndef SLICER_H
#define SLICER_H

#include <set>
#include <variant>
#include <Progress.h>
#include <unordered_map>


namespace geom {

template <typename T> class FiniteSet {
private:
    int size_ = 0;
    std::array<T, 2> data_;
public:

    void insert(const T &x) {
        if (data_[0].first == x.first) return;
        if (data_[1].first == x.first) return;
        if (size_ < 2) data_[size_] = x;
        size_++;
    }

    size_t size() const { return size_; }

    const std::array<double, 2>& getPoint() const {
        return (data_[0].first < data_[1].first) ? data_[0].second: data_[1].second;
    }

    T& operator[](int i) {
        return data_[i];
    }
};

};


class Slicer {
public:

    void sliceGeometry(const Geometry &g, double dz);

private:

   using Point = std::array<double, 2>;

    struct VertexData {
        Point position;
        uint32_t edge_count = 0;
        std::array<uint64_t, 2> edges; 
    };

    using Intersection = std::pair<uint64_t, std::array<double, 2>>;
    using Graph = std::map<uint64_t, VertexData>;
    using Points = std::map<Intersection, Point>;
    using Polygon = std::vector<Point>;
    using Polygons = std::vector<Polygon>;

 

    std::vector<Slicer::Points> points_;
    std::vector<Slicer::Graph> graph_; 
    std::vector<Slicer::Polygons> polygons_;
    int slice_count_;

    static Intersection make_vertex(uint32_t v, const std::array<double, 2> &p) {
        return {(uint64_t) v, p};
    }

    static Intersection make_edge(uint32_t v1, uint32_t v2, const std::array<double, 2> &p) {
        return {v1 < v2 ? (uint64_t) v1 << 32 | v2: (uint64_t) v2 << 32 | v1, p};
    }

    void sliceTriangles(const Geometry &g, double dz);
    void computeContours(const Geometry &g);
    void exportPolygons(const std::string &path);
    void exportImages(const std::string &path_prefix);
};

#endif /* SLICER_H */