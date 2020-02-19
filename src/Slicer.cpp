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
            Slicer::generateDots(geometry, sliceCount, z);
            int barWidth = 70;
            std::cout << "[";
            int pos = barWidth * (sliceCount / 300.0);
            for (int i = 0; i < barWidth; ++i) {
                if (i < pos) std::cout << "=";
                else if (i == pos) std::cout << ">";
                else std::cout << " ";
            }
            std::cout << "] " << int((sliceCount / 300.0) * 100.0) << " %\r";
            std::cout.flush();

            sliceCount += 1;
        }    
        std::cout << std::endl;

}


void Slicer::generateDots(const Geometry &geometry, int sliceCount, double z) {
    std::map<Slicer::Intersection, Slicer::Point> points;
    std::multimap<Slicer::Intersection, Slicer::Intersection> edges; 

    for (const Face &face: geometry.mesh().faces()) {

        std::set<Slicer::Intersection> intersection_set;

        for (const HalfEdge *halfedge: face.adjacentHalfEdges()) {
            assert(!halfedge->onBoundary);

            Vertex * v1 = halfedge->vertex;
            Vertex * v2 = halfedge->next->vertex;
            auto &p1 = geometry.positions().at(v1->index);
            auto &p2 = geometry.positions().at(v2->index);
            
            auto [eminz, emaxz] = std::minmax(p1, p2, [](auto &a, auto &b) { return a[2] < b[2]; });

            if (eminz[2] == z && z == emaxz[2]) {
                // In the case that the line is parallel to an edge,
                // we do not count it as an intersection, since the other
                // two edges will properly intersect at their vertices.
            } else if (p1[2] == z) {
                std::array<double, 2> p {p1[0], p1[1]};
                points.emplace(v1, p);
                intersection_set.insert(v1);
            } else if (p2[2] == z) {
                // Ignore this case  
            } else if (eminz[2] < z && z < emaxz[2]) {
                double s = (z - eminz[2]) / (emaxz[2] - eminz[2]);
                
                std::array<double, 2> p {
                    eminz[0] + s * (emaxz[0] - eminz[0]),
                    eminz[1] + s * (emaxz[1] - eminz[1]),
                };

                points.emplace(halfedge->edge, p);
                intersection_set.insert(halfedge->edge);

            }
            
        }

        if (intersection_set.size() == 2) {
            std::vector<Slicer::Intersection> entities {intersection_set.begin(), intersection_set.end()}; 
            edges.emplace(entities[0], entities[1]);
            edges.emplace(entities[1], entities[0]);
        }
        
    };

    Slicer::Polygons polygons;
   
    while(edges.size() > 0) {
        Slicer::Intersection first = edges.begin()->first;
        Slicer::Intersection e = first;
        Slicer::Polygon polygon;

        // Define a local function that connects contours through a modified
        // algorithm used to seperate dipartite graphs. 
        std::function<void(Slicer::Intersection e)> build_polygon;
        build_polygon = [&](Slicer::Intersection e) {
            auto [begin, end] = edges.equal_range(e);
            std::vector<std::pair<Slicer::Intersection, Slicer::Intersection>> entries{begin, end};
            assert(entries.size() == 2);

            auto n1 = entries[0].second;
            auto n2 = entries[1].second;
            edges.erase(e);

            if (edges.find(n1) != edges.end()) {
                polygon.push_back(points.find(n1)->second);
                build_polygon(n1);
            } else if (edges.find(n2) != edges.end()) {
                polygon.push_back(points.find(n2)->second);
                build_polygon(n2);
            } else if (n1 != first && n2 != first) {
                assert ("unreachable");
            }
        };

        polygon.push_back(points.find(first)->second);
        build_polygon(e);
        polygon.push_back(points.find(first)->second);

        polygons.push_back(polygon);
    }

    const auto polygon_path = "test/csv/slice" + std::to_string(sliceCount);
    exportPolygons(polygons, polygon_path);
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
