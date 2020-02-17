
#ifndef SLICER_H
#define SLICER_H

#include <set>

class Slicer {
public:
    static void sliceGeometry(const Geometry &g /*, SliceJobSettings */);

private:
    using Point = std::pair<int, std::pair<double, double>>;
    using Dots = std::map<int, std::array<double, 2>>;
    using Triangles = std::set<int>;
    using Polygon = std::vector<std::pair<double, double>>;
    using Polygons = std::vector<Polygon>;

    static std::pair<Dots,Triangles> generateDots(const Geometry &g, double z);
    static Polygons connectDots(const Geometry &g, const Dots &, const Triangles &);
    static void diagnoseTriangles(const Geometry &g, const Dots& dots, double z);
};

#endif /* SLICER_H */