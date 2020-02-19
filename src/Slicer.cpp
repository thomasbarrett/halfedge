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
#include <Progress.h>

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
        
        ProgressBar progress;
        int sliceCount = 0;
        for (double z = minz; z <= maxz; z += (maxz - minz) / 300) {
            auto [points, edges] = sliceTriangles(geometry, z);
            const Polygons &polygons = computeContours(geometry, points, std::move(edges));
            const auto polygon_path = "test/csv/slice" + std::to_string(sliceCount);
            exportPolygons(polygons, polygon_path);
            progress.update(sliceCount / 300.0);
            sliceCount += 1;
        } 
        progress.finish();
}

std::pair<Slicer::Points, Slicer::Edges> Slicer::sliceTriangles(const Geometry &geometry, double z) {
    Slicer::Points points;
    Slicer::Edges edges; 

    for (const Face &face: geometry.mesh().faces()) {

        std::set<Slicer::Intersection> intersections;

        for (const HalfEdge *halfedge: face.adjacentHalfEdges()) {
            assert(!halfedge->onBoundary);

            Vertex *v1 = halfedge->vertex;
            Vertex *v2 = halfedge->next->vertex;

            const Geometry::Point &p1 = geometry.positions().at(v1->index);
            const Geometry::Point &p2 = geometry.positions().at(v2->index);
            
            auto [eminz, emaxz] = std::minmax(p1, p2, [](auto &a, auto &b) {
                return a[2] < b[2];
            });

            /*
             * There are a couple of ways that an halfedge could intersect the 
             * plane:
             * 
             * 1. halfedge lies flat on plane
             * 2. halfedge intersects at start vertex
             * 3. halfedge intersects at end vertex 
             * 4. halfedge intersects between vertices
             * 
             * We ignore the first case, since it is meaningless to consider
             * the intersection in such a case. 
             */
            if (eminz[2] == z && z == emaxz[2]) {
                continue;
            } else if (p1[2] == z) {
                Slicer::Point p {p1[0], p1[1]};
                points.emplace(v1, p);
                intersections.insert(v1);
            } else if (p2[2] == z) {
                Slicer::Point p {p2[0], p2[1]};
                points.emplace(v2, p);
                intersections.insert(v2);
            } else if (eminz[2] < z && z < emaxz[2]) {
                double s = (z - eminz[2]) / (emaxz[2] - eminz[2]);
                Slicer::Point p {
                    eminz[0] + s * (emaxz[0] - eminz[0]),
                    eminz[1] + s * (emaxz[1] - eminz[1]),
                };
                points.emplace(halfedge->edge, p);
                intersections.insert(halfedge->edge);
            }
            
        }

        if (intersections.size() == 2) {
            std::vector<Slicer::Intersection> entities {intersections.begin(), intersections.end()}; 
            edges.emplace(entities[0], entities[1]);
            edges.emplace(entities[1], entities[0]);
        }
        
    }
    return {points, edges};
}

Slicer::Polygons Slicer::computeContours(const Geometry &g, const Points &points, Edges edges) {
    Slicer::Polygons polygons;
   
    while(edges.size() > 0) {
        Slicer::Intersection first = edges.begin()->first;
        Slicer::Intersection e = first;
        Slicer::Polygon polygon;

        // Define a local function that connects contours through a modified
        // algorithm used to seperate dipartite graphs. 
        std::function<void(Slicer::Intersection e)> build_polygon;
        build_polygon = [&](Slicer::Intersection e) {
            
            // Make sure there are no duplicate key value pairs
            auto [begin, end] = edges.equal_range(e);
            std::set<Slicer::Intersection> next_set;
            while (begin != end) {
                next_set.insert(begin->second);
                begin++;
            }
       
            std::vector<Slicer::Intersection> next{next_set.begin(), next_set.end()};
            assert(next.size() == 2);
            auto n1 = next[0];
            auto n2 = next[1];
            edges.erase(e);

            if (edges.find(n1) != edges.end()) {
                polygon.push_back(points.find(n1)->second);
                build_polygon(n1);
            } else if (edges.find(n2) != edges.end()) {
                polygon.push_back(points.find(n2)->second);
                build_polygon(n2);
            } else if (n1 != first && n2 != first) {
                assert ("error: unconnected loop");
            }
        };

        polygon.push_back(points.find(first)->second);
        build_polygon(e);
        polygon.push_back(points.find(first)->second);

        polygons.push_back(polygon);
    }

    return polygons;
}

void Slicer::exportPolygons(const Polygons &polygons, const std::string &path) {
    int polygon_index = 0;
    for (auto &polygon: polygons) {
        std::ofstream ofile;
        ofile.open(path + "_" + std::to_string(polygon_index) + ".csv");
        for (auto &point: polygon) {
            ofile << point[0] << " " << point[1] << "\n";
        }
        ofile.flush();
        ofile.close();
        polygon_index += 1;
    }
}
