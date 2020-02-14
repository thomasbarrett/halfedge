#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <Mesh.h>

int main(int argc, char const *argv[]) {
    if (argc != 2) {
        std::cout << "usage: slicer [file.obj]" << std::endl;
        return 1;
    } else {
        std::string path{argv[1]};
        size_t ext_loc = path.find(".") + 1;
        std::string ext = path.substr(ext_loc);
        if (ext != "obj") {
            std::cout << "warning: input file does not have 'obj' extension" << std::endl;
        }
        std::ifstream f{path};
        if (f.good()) {
            Geometry geometry{f};

            auto [min, max] = std::minmax_element(
                geometry.positions().begin(),
                geometry.positions().end(),
                [](const auto &a, const auto &b){ return a[2] < b[2]; }
            );

            float minz = min->at(2);
            float maxz = max->at(2);

            for (double z = minz; z <= maxz; z += (maxz - minz) / 300) {
                
                std::map<int, std::array<double, 2>> edge_intersections;
                std::vector<std::pair<int, int>> contour_edges;

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

                        edge_intersections.emplace(edge.index, p);
                    }
                }
                
                for (auto &face: geometry.mesh().faces()) {
                    auto e1 = face.halfedge->edge->index;
                    auto e2 = face.halfedge->next->edge->index;
                    auto e3 = face.halfedge->next->next->edge->index;
                    int sum = (edge_intersections.find(e1) != edge_intersections.end() ? 1: 0) +
                    (edge_intersections.find(e2) != edge_intersections.end() ? 1: 0) +
                    (edge_intersections.find(e3) != edge_intersections.end() ? 1: 0);
                    if (sum != 0 && sum != 2) {
                        std::cout << sum << std::endl;
                    }
                }
                
                for (auto &[edge_index, point]: edge_intersections) {
                    auto &edge = geometry.mesh().edges().at(edge_index);
                    
                    assert(!edge.halfedge->onBoundary);

                    auto halfedge1 = edge.halfedge;
                    if (edge_intersections.find(halfedge1->next->edge->index) != edge_intersections.end()) {
                        contour_edges.push_back({edge.index, halfedge1->next->edge->index});
                    }

                    auto halfedge2 = edge.halfedge->twin;
                    if (edge_intersections.find(halfedge2->next->edge->index) != edge_intersections.end()) {
                        contour_edges.push_back({edge.index, halfedge2->next->edge->index});
                    }
                    
                }
                
                std::cout << edge_intersections.size() << " " << contour_edges.size() << std::endl;
            }
            
        } else {
            std::cout << "error: input file not found" << std::endl;
        }
        return 0;
    }
}
