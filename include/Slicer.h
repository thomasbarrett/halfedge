
#ifndef SLICER_H
#define SLICER_H

class Slicer {
public:
    static void sliceGeometry(const Geometry &g /*, SliceJobSettings */);

private:
    using Dots = std::map<int, std::array<double, 2>>;
    using Contours = std::vector<std::pair<int, int>>;

    static Dots generateDots(const Geometry &g, double z);
    static Contours connectDots(const Geometry &g, const Dots& dots);
    static void diagnoseTriangles(const Geometry &g, const Dots& dots, double z);
};

#endif /* SLICER_H */