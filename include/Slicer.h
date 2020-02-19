
#ifndef SLICER_H
#define SLICER_H

#include <set>
#include <variant>

class Slicer {
public:
    static void sliceGeometry(const Geometry &g /*, SliceJobSettings */);

private:
    using Point = std::array<double, 2>;
    using Intersection = std::variant<const Edge*, const Vertex*>;
    using Polygon = std::vector<Point>;
    using Polygons = std::vector<Polygon>;

    static void generateDots(const Geometry &g, int sliceCount, double z);
    static void exportPolygons(const Polygons &polygons, const std::string &path);
};

#endif /* SLICER_H */