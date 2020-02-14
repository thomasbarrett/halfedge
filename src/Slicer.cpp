#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cassert>
#include <map>
#include <algorithm>
#include <Mesh.h>
#include <Slicer.h>

void Slicer::sliceGeometry(const Geometry &geometry /*, SliceJobSettings */) {

        auto [min, max] = std::minmax_element(
            geometry.positions().begin(),
            geometry.positions().end(),
            [](const auto &a, const auto &b){ return a[2] < b[2]; }
        );

        float minz = min->at(2);
        float maxz = max->at(2);

        std::cout << "info: start slicing" << std::endl;
        
        int sliceCount = 0;
        
        for (double z = minz; z <= maxz; z += (maxz - minz) / 300) {
            Slicer::Dots dots = Slicer::generateDots(geometry, z);
            Slicer::Contours contours = Slicer::connectDots(geometry, dots);

            std::ofstream ofile;
            ofile.open ("test/csv/slice" + std::to_string(sliceCount) + ".csv");
            for (auto &[key, val]: dots) {
               ofile << val[0] << " " << val[1] << std::endl;
            }
            ofile.close();

            assert(contours.size() == dots.size() / 2);
            sliceCount += 1;
        }    
}

Slicer::Dots Slicer::generateDots(const Geometry &geometry, double z) {
    Slicer::Dots dots;

    for (auto &edge: geometry.mesh().edges()) {
        int vi1 = edge.halfedge->vertex->index;
        int vi2 = edge.halfedge->next->vertex->index;
        auto &p1 = geometry.positions().at(vi1);
        auto &p2 = geometry.positions().at(vi2);
        
        auto [eminz, emaxz] = std::minmax(p1, p2, [](auto &a, auto &b) { return a[2] < b[2]; });

        if (eminz[2] < z && z < emaxz[2]) {
            double s = (z - eminz[2]) / (emaxz[2] - eminz[2]);
            
            std::array<double, 2> p {
                eminz[0] + s * (emaxz[0] - eminz[0]),
                eminz[1] + s * (emaxz[1] - eminz[1]),
            };

            assert(!edge.halfedge->onBoundary);

            dots.emplace(edge.halfedge->index, p);
            dots.emplace(edge.halfedge->twin->index, p);
        }
    }

    return dots;
}


Slicer::Contours Slicer::connectDots(const Geometry &geometry, const Slicer::Dots &dots) {
    Slicer::Contours contour_edges;

    for (auto &[halfedge_index, point]: dots) {
        auto &halfedge = geometry.mesh().halfedges().at(halfedge_index);
        

        if (dots.find(halfedge.next->index) != dots.end()) {
            contour_edges.push_back({halfedge.edge->index, halfedge.next->edge->index});
        }
    }

    return contour_edges;
}

void Slicer::diagnoseTriangles(const Geometry &geometry, const Slicer::Dots &dots, double z) {
    for (auto &face: geometry.mesh().faces()) {
        auto e1 = face.halfedge->index;
        auto e2 = face.halfedge->next->index;
        auto e3 = face.halfedge->next->next->index;

        int sum = (dots.find(e1) != dots.end() ? 1: 0) +
        (dots.find(e2) != dots.end() ? 1: 0) +
        (dots.find(e3) != dots.end() ? 1: 0);
        
        
        if (sum != 0 && sum != 2) {
            std::cout << "z: " << z << " " << sum << std::endl;

            int vi1 = face.halfedge->vertex->index;
            int vi2 = face.halfedge->next->vertex->index;
            auto &p1 = geometry.positions().at(vi1);
            auto &p2 = geometry.positions().at(vi2);
            auto [eminz, emaxz] = std::minmax(p1, p2, [](auto &a, auto &b) { return a[2] < b[2]; });
            std::cout << (eminz[2] < z && z < emaxz[2]) << " (" << p1[0] << ", " << p1[1] << ", " << p1[2] << ") (" << p2[0] << ", " << p2[1] << ", " << p2[2] << ")" << std::endl;

            vi1 = face.halfedge->next->vertex->index;
            vi2 = face.halfedge->next->next->vertex->index;
            auto &p3 = geometry.positions().at(vi1);
            auto &p4 = geometry.positions().at(vi2);
            auto [eminz1, emaxz1] = std::minmax(p3, p4, [](auto &a, auto &b) { return a[2] < b[2]; });
            std::cout << (eminz1[2] < z && z < emaxz1[2]) << " (" << p3[0] << ", " << p3[1] << ", " << p3[2] << ") (" << p4[0] << ", " << p4[1] << ", " << p4[2] << ")" << std::endl;

            vi1 = face.halfedge->next->next->vertex->index;
            vi2 = face.halfedge->next->next->next->vertex->index;
            auto &p5 = geometry.positions().at(vi1);
            auto &p6 = geometry.positions().at(vi2);
            auto [eminz2, emaxz2] = std::minmax(p5, p6, [](auto &a, auto &b) { return a[2] < b[2]; });
            std::cout << (eminz2[2] < z && z < emaxz2[2]) << " (" << p5[0] << ", " << p5[1] << ", " << p5[2] << ") (" << p6[0] << ", " << p6[1] << ", " << p6[2] << ")" << std::endl;
            std::cout << std::endl;
        }
        
    }
}
