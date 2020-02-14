#include <Mesh.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <map>
#include <filesystem>

Mesh::Mesh(int vertexCount, const std::vector<std::array<int, 3>> &faces) {
    // The size of some fields are known ahead of time based on simple
    // geometrix properties of a pure simplicial complex. 
    vertices_.resize(vertexCount);
    faces_.resize(faces.size());
    halfedges_.resize(3 * faces_.size());
    corners_.resize(3 * faces_.size());
    
    // Associate each edge with one of its half edges and a boolean marking
    // whether or not it is a boundary edge. The key is a pair of sorted
    // vertex indices for each edge, which is necessary and sufficient to 
    // uniquelly identify each edge.
    std::map<std::pair<int, int>, std::pair<HalfEdge*, bool>> edges;

    // Associate each vertex with an index.
    for (int i = 0; i < vertexCount; i++) {
        vertices_[i].index = i;
    }

    for (int i = 0; i < faces.size(); i++) {
        auto &face = faces[i];
        
        // Associate each face with an index and one of its half-edges.
        faces_[i].index = i;
        faces_[i].halfedge = &halfedges_[3 * i];

        for (int j = 0; j < 3; j++) {
            auto halfedge = &halfedges_[3 * i + j];

            // Associate each corner with an index and its opposite halfedge.
            corners_[3 * i + j].index = 3 * i + j;
            corners_[3 * i + j].halfedge = halfedge;
            
            // Associate each vertex with a halfedge originating at that
            // vertex. Note that this might override a previous value, but that
            // is acceptable since we make no guarentee of what halfedge each
            // vertex is associated with.
            vertices_[face[j]].halfedge = halfedge;

            // Initialized each halfedge with an index, vertex, face, ...
            halfedge->index = 3 * i + j;
            halfedge->vertex = &vertices_[face[j]];
            halfedge->face = &faces_[i];
            halfedge->corner = &corners_[3 * i + j];
            halfedge->next = &halfedges_[3 * i + ((j + 1) % 3)];
            halfedge->prev = &halfedges_[3 * i + ((j + 2) % 3)];
            
            // Create a key which is the ordered indices of incident vertices
            int a = face[j];
            int b = face[(j + 1) % 3];
            if (a > b) std::swap(a, b);
            std::pair<int, int> key{a, b};

            auto it = edges.find(key);
            if (it == edges.end()) {
                // If the edge has not been created, create it and connect it
                // with the halfedge.
                Edge e;
                e.index = edges_.size();
                e.halfedge = halfedge;
                edges_.push_back(e);
                halfedge->edge = &edges_[e.index];
                edges.emplace(key, std::make_pair(halfedge, false));
            } else {
                // Otherwise, check that there are not already two halfedges
                // associated with the edge (non-manifold) and finish
                // connecting halfedge pairs
                if (it->second.second) {
                    throw std::runtime_error("error: non-manifold surface");
                } else {
                    it->second.second = true;
                    halfedge->twin = it->second.first;
                    halfedge->edge = halfedge->twin->edge;
                    halfedge->twin->twin = halfedge;
                    halfedge->onBoundary = false;
                    halfedge->twin->onBoundary = false;
                }
            }
        }
    }
}

int Mesh::eulerCharacteristic() const {
    return vertices_.size() - edges_.size() + faces_.size();
}

Geometry::Geometry(std::istream &f) {
    std::string first;
    float r[3];
    int i[3];
    std::vector<std::array<int, 3>> faces;

    while (!f.eof()) {
        f >> first;    

        if (first[0] == '#') {
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else if (first == "v") {
            f >> r[0] >> r[1] >> r[2];
            positions_.push_back({r[0], r[1], r[2]});
        } else if (first == "f") {
            f >> i[0] >> i[1] >> i[2];
            faces.push_back({i[0] - 1, i[1] - 1, i[2] - 1});
        } else if (first == "vt") {
            f >> r[0] >> r[1];
        } else if (first == "vn") {
            f >> r[0] >> r[1] >> r[2];
        } else if (first == "vp") {
            throw std::runtime_error("free-form geometry not supported");
        } else if (first == "l") {
            throw std::runtime_error("line elements not supported");
        } else {
            std::cout << "warning: unknown directive: " << first;
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        }

        if (f.fail()) {
            throw std::runtime_error("invalid obj file");
        }
    }

    mesh_ = std::make_unique<Mesh>(positions_.size(), faces);
}

const std::vector<std::array<float, 3>>& Geometry::positions() const {
    return positions_;
}

