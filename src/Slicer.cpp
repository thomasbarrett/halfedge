#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <cassert>
#include <map>
#include <cmath>
#include <set>
#include <algorithm>
#include <Mesh.h>
#include <Slicer.h>

void Slicer::sliceGeometry(const Geometry &geometry /*, SliceJobSettings */) {
        assert(geometry.mesh().closed());

        auto compare_z = [](auto &a, auto &b) {
            return a[2] < b[2];
        };

        auto [min, max] = std::minmax_element(
            geometry.positions().begin(),
            geometry.positions().end(),
            compare_z
        );

        float minz = (*min)[2];
        float maxz = (*max)[2];

        std::cout << "info: start slicing" << std::endl;
        
        int sliceCount = 0;
        
        for (double z = minz; z <= maxz; z += (maxz - minz) / 300) {
            const auto &[dots, triangles] = Slicer::generateDots(geometry, z);
            Slicer::Polygons contours = Slicer::connectDots(geometry, dots, triangles);

            std::ofstream ofile;
            ofile.open ("test/csv/slice" + std::to_string(sliceCount) + ".csv");
            for (auto &[key, val]: dots) {
               ofile << val[0] << " " << val[1] << std::endl;
            }
            ofile.close();
            sliceCount += 1;
        }    
}

 std::pair<Slicer::Dots,Slicer::Triangles> Slicer::generateDots(const Geometry &geometry, double z) {
    Slicer::Dots dots;
    Slicer::Triangles triangles;

    for (auto &edge: geometry.mesh().edges()) {
        int vi1 = edge.halfedge->vertex->index;
        int vi2 = edge.halfedge->next->vertex->index;
        auto &p1 = geometry.positions().at(vi1);
        auto &p2 = geometry.positions().at(vi2);
        
        auto [eminz, emaxz] = std::minmax(p1, p2, [](auto &a, auto &b) { return a[2] < b[2]; });

        if (eminz[2] <= z && z <= emaxz[2]) {
            double s = (z - eminz[2]) / (emaxz[2] - eminz[2]);
            
            std::array<double, 2> p {
                eminz[0] + s * (emaxz[0] - eminz[0]),
                eminz[1] + s * (emaxz[1] - eminz[1]),
            };

            assert(!edge.halfedge->onBoundary);

            dots.emplace(edge.halfedge->index, p);
            dots.emplace(edge.halfedge->twin->index, p);
            triangles.insert(edge.halfedge->face->index);
            triangles.insert(edge.halfedge->twin->face->index);
        }
    }

    return {dots, triangles};
}


Slicer::Polygons Slicer::connectDots(const Geometry &geometry, const Slicer::Dots &dots, const Slicer::Triangles &triangles) {
    std::multimap<const HalfEdge*, const HalfEdge*> edge_bundle;

    for (int triangle_index: triangles) {
        auto triangle = geometry.mesh().faces()[triangle_index];
        auto halfedge = triangle.halfedge;

        // We define a Point as a pair of halfedge indices and intersection
        // points.
        using Point = std::pair<int, std::pair<double, double>>;

        auto less = [](const Point &a, const Point &b) {
            return a.second < b.second;
        };
        
        std::set<Point, decltype(less)> points{less};

        // Iterate through halfedges in a triangle. If the halfedge
        // intersects the plane, then add it to the set.
        for (int i = 0; i < 3; i++) {
            auto it = dots.find(halfedge->index);
            if (it != dots.end()) points.insert(*it);
            halfedge = halfedge->next;
        }

        if (points.size() == 2) {
            std::vector<Point> endpoints{points.begin(), points.end()};
            const HalfEdge *h1 = &geometry.mesh().halfedges()[endpoints[0].first];
            const HalfEdge *h2 = &geometry.mesh().halfedges()[endpoints[1].first];
            edge_bundle.emplace(h1, h2);
            edge_bundle.emplace(h2, h1);
        }
    }

    Polygons polygons;
    while(edge_bundle.size() != 0) {
        Polygon polygon;
        const HalfEdge *first = edge_bundle.begin()->first;

        const HalfEdge *current1 = first;
        const HalfEdge *current2 = edge_bundle.find(current1)->second;
        do {
            assert(edge_bundle.size() != 0 && "error: no loop found");

            auto magnitude = [](const std::array<double, 2> &a) {
                return sqrt(a[0] * a[0] + a[1] * a[1]);
            };
            
            auto p1 = dots.find(current2->index)->second;
            auto p2 = geometry.positions().at(current2->vertex->index);
            if (magnitude(std::array{p2[0] - p1[0], p2[1] - p1[1]}) < 1E-9) {
                std::cout << "close!" << std::endl;
            }

            if(current2->twin->vertex->index >= geometry.positions().size() || current2->twin->vertex->index < 0) {
                std::cout << current2->twin->vertex->index << std::endl;
            }
            
            assert(current2->twin->twin == current2);
            assert(current2->vertex->index < geometry.positions().size());
            assert(current2->twin->vertex->index < geometry.positions().size());

            auto p3 = geometry.positions().at(current2->twin->vertex->index);

            if (magnitude(std::array{p3[0] - p1[0], p3[1] - p1[1]}) < 1E-9) {
                std::cout << "close!" << std::endl;

                std::cout << (edge_bundle.find(current2->twin) != edge_bundle.end())<< std::endl;
                std::cout << (edge_bundle.find(current2->twin->next) != edge_bundle.end()) << std::endl;
                std::cout << (edge_bundle.find(current2->twin->prev) != edge_bundle.end()) << std::endl;
                
                const HalfEdge *next1 = current2->twin->prev;
                const HalfEdge *next2 = edge_bundle.find(next1)->second;
                polygon.push_back(dots.find(current1->index)->second);

                edge_bundle.erase(current1);
                edge_bundle.erase(current2);

                current1 = next1;
                current2 = next2;

            } else {    
                const HalfEdge *next1 = current2->twin;
                const HalfEdge *next2 = edge_bundle.find(next1)->second;

                polygon.push_back(dots.find(current1->index)->second);

                edge_bundle.erase(current1);
                edge_bundle.erase(current2);

                current1 = next1;
                current2 = next2;
            }

        } while(current1 != first);
        polygons.push_back(polygon);
    }
    std::cout << polygons.size() << " polygon(s)" << std::endl;
  
    return polygons;
}

