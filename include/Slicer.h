
#ifndef SLICER_H
#define SLICER_H

#include <set>
#include <variant>

class Slicer {
public:
    static void sliceGeometry(const Geometry &g /*, SliceJobSettings */);

private:
    using Intersection = std::variant<const Edge*, const Vertex*>;
    using Edges = std::multimap<Intersection, Intersection>;
    using Point = std::array<double, 2>;
    using Points = std::map<Intersection, Point>;
    using Polygon = std::vector<Point>;
    using Polygons = std::vector<Polygon>;

    static std::pair<Points, Edges> sliceTriangles(const Geometry &g, double z);
    static Polygons computeContours(const Geometry &g, const Points &points, Edges edges);
    static void exportPolygons(const Polygons &polygons, const std::string &path);
    static void exportPolygonsToPNG(const Polygons &polygons, const std::string &path);
};

#endif /* SLICER_H */