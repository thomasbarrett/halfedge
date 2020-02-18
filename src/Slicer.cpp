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
            const auto &dots = Slicer::generateDots(geometry, z);
            Slicer::Polygons contours = Slicer::connectDots(geometry, dots);

            std::ofstream ofile;
            ofile.open ("test/csv/slice" + std::to_string(sliceCount) + ".csv");
            for (auto &[key, val]: dots) {
               ofile << val[0] << " " << val[1] << std::endl;
            }
            ofile.close();
            sliceCount += 1;
        }    
}

Slicer::Dots Slicer::generateDots(const Geometry &geometry, double z) {
    Slicer::Dots dots;

    for (const Edge &edge: geometry.mesh().edges()) {

        assert(!edge.halfedge->onBoundary);

        Vertex *v1 = edge.halfedge->vertex;
        Vertex * v2 = edge.halfedge->next->vertex;
        auto &p1 = geometry.positions().at(v1->index);
        auto &p2 = geometry.positions().at(v2->index);
        
        auto [eminz, emaxz] = std::minmax(p1, p2, [](auto &a, auto &b) { return a[2] < b[2]; });

        if (eminz[2] == z && z == emaxz[2]) {
            std::array<double, 2> p {
                std::numeric_limits<double>::infinity(),
                std::numeric_limits<double>::infinity(),
            };
            // dots.emplace(edge.halfedge->edge, p);

        } else if (eminz[2] == z) {
            Vertex *vertex = eminz == p1 ? v1: v2;
            std::array<double, 2> p {
                eminz[0], eminz[1]
            };
            dots.emplace(vertex, p);

        } else if (emaxz[2] == z) {
            Vertex *vertex = emaxz == p1 ? v1: v2;
            std::array<double, 2> p {
                emaxz[0], emaxz[1]
            };
            dots.emplace(vertex, p);

        } else if (eminz[2] < z && z < emaxz[2]) {
            double s = (z - eminz[2]) / (emaxz[2] - eminz[2]);
            
            std::array<double, 2> p {
                eminz[0] + s * (emaxz[0] - eminz[0]),
                eminz[1] + s * (emaxz[1] - eminz[1]),
            };

            dots.emplace(edge.halfedge->edge, p);
        }
    }

    return dots;
}


Slicer::Polygons Slicer::connectDots(const Geometry &geometry, const Slicer::Dots &dots) {
    if (dots.size() == 0) return {};

    std::set<Face*> used;
    std::vector<std::array<double, 2>> points;

    auto first = dots.begin();
    auto current = first;

    outer: do {
        auto &[intersectionEntity, point] = *current;
        points.push_back(point);

        if (std::holds_alternative<const Vertex*>(intersectionEntity)) {
            const Vertex *vertex = std::get<const Vertex*>(intersectionEntity);
            for (Face *triangle: vertex->adjacentFaces()) {
                if (used.find(triangle) == used.end()) {
                    used.insert(triangle);
                    for (Edge *edge2: triangle->adjacentEdges()) {
                        if (dots.find(edge2) != dots.end()) {
                            current = dots.find(edge2);
                        }
                    }
                    for (Vertex *vertex2: triangle->adjacentVertices()) {
                        if (vertex != vertex2 && dots.find(vertex) != dots.end()) {
                            current = dots.find(vertex);
                        }
                    }
                    std::cout << "error 1: unreachable" << std::endl;
                }
            }
            std::cout << "error 2: unreachable" << std::endl;
        } else if (std::holds_alternative<const Edge*>(intersectionEntity)) {
            const Edge *edge = std::get<const Edge*>(intersectionEntity);
            for (Face *triangle: edge->adjacentFaces()) {
                if (used.find(triangle) == used.end()) {
                    used.insert(triangle);
                    for (Edge *edge2: triangle->adjacentEdges()) {
                        if (edge != edge2 && dots.find(edge2) != dots.end()) {
                            current = dots.find(edge2);
                        }
                    }
                    for (Vertex *vertex: triangle->adjacentVertices()) {
                        if (dots.find(vertex) != dots.end()) {
                            current = dots.find(vertex);
                        }
                    }
                    std::cout << "error 3: unreachable" << std::endl;
                }
            }
            std::cout << "error 4: unreachable" << std::endl;
        }
    } while (current != first);


    return {};
}

