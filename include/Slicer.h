
#ifndef SLICER_H
#define SLICER_H

#include <set>
#include <variant>

class Slicer {
public:
    static void sliceGeometry(const Geometry &g /*, SliceJobSettings */);

private:
    using IntersectionEntity = std::variant<const Edge*, const Vertex*>;
    using Dots = std::map<IntersectionEntity, std::array<double, 2>>;
    using Triangles = std::set<Face*>;
    using Polygon = std::vector<std::pair<double, double>>;
    using Polygons = std::vector<Polygon>;

    static Dots generateDots(const Geometry &g, double z);
    static Polygons connectDots(const Geometry &g, const Dots &);
    static void diagnoseTriangles(const Geometry &g, const Dots& dots, double z);
};

#endif /* SLICER_H */