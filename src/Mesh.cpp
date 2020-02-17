#include <Mesh.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <map>
#include <filesystem>
#include <algorithm>
#include <string>

Mesh::Mesh(int vertexCount, const std::vector<std::array<int, 3>> &faces) {
    // The size of some fields are known ahead of time based on simple
    // geometrix properties of a pure simplicial complex. 
    vertices_.resize(vertexCount);
    faces_.resize(faces.size());
    halfedges_.resize(3 * faces_.size());
    corners_.resize(3 * faces_.size());
    edges_.reserve(3 * faces.size());
    
    // Associate each edge with one of its half edges and a boolean marking
    // whether or not it is a boundary edge. The key is a pair of sorted
    // vertex indices for each edge, which is necessary and sufficient to 
    // uniquelly identify each edge.
    std::map<std::pair<int, int>, std::pair<HalfEdge*, bool>> edges;

    std::cout << "vertices: " << vertices_.size() << std::endl;

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
            assert(face[j] >= 0 && face[j] < vertexCount);
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

            assert(halfedge->vertex->index < vertices_.size());

            // Create a key which is the ordered indices of incident vertices
            auto key = std::minmax(face[j], face[(j + 1) % 3]);
            auto it = edges.find(key);
            if (it == edges.end()) {
                // If the edge has not been created, create it and connect it
                // with the halfedge.
                edges_.push_back(Edge{static_cast<int>(edges_.size()), halfedge});
                halfedge->edge = &edges_.back();
                edges.emplace(key, std::make_pair(halfedge, false));
            } else {
                auto &[twin, hasTwin] = it->second;
                
                if (hasTwin) {
                    throw std::runtime_error("error: non-manifold surface");
                } else {
                    hasTwin = true;
                    halfedge->twin = twin;
                    twin->twin = halfedge;
                    halfedge->edge = twin->edge;
                    halfedge->onBoundary = false;
                    halfedge->twin->onBoundary = false; 
                }
            }
        }
    }

    for (auto &h: halfedges_) {
        assert(h.twin->twin == &h);
    }

}

bool Mesh::closed() const {
    for (auto &edge: edges_) {
        if (edge.halfedge->onBoundary) return false;
    }
    return true;
}

int Mesh::eulerCharacteristic() const {
    return vertices_.size() - edges_.size() + faces_.size();
}

std::vector<std::string> split(const std::string &str) {
    std::stringstream ss{str};
    std::vector<std::string> words;
    std::string word;
    while (ss >> word) words.push_back(word);
    return words;
}

template <typename A, typename B> 
std::vector<B> map(std::vector<A> input, const std::function<B(const A&)> &func) {
    std::vector<B> output;
    output.reserve(input.size());
    std::transform(input.begin(), input.end(), output.begin(), func);
    return output;
}

Geometry::Geometry(std::istream &f) {
    std::string first;
    float r[3];
    int i[3];
    std::vector<std::array<int, 3>> faces;

    while (!(f >> first).eof()) {  

        if (first[0] == '#') {
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else if (first == "v") {
            f >> r[0] >> r[1] >> r[2];
            positions_.push_back({r[0], r[1], r[2]});
        } else if (first == "f") {
            std::string line;
            std::getline(f, line); 
            auto words = split(line);
            std::vector<int> indices;
            std::transform(words.begin(), words.end(), std::back_inserter(indices), [](auto &s){
                return std::stoi(s);
            }); 
            if (indices.size() == 3) {
                faces.push_back({indices[0] - 1, indices[1] - 1, indices[2] - 1});
            } else if (indices.size() == 4) {
                faces.push_back({indices[0] - 1, indices[1] - 1, indices[2] - 1});
                faces.push_back({indices[0] - 1, indices[2] - 1, indices[3] - 1});
            } else {
                throw std::runtime_error("polygons not supported");
            }
        } else if (first == "vt") {
            f >> r[0] >> r[1];
        } else if (first == "vn") {
            f >> r[0] >> r[1] >> r[2];
        } else if (first == "vp") {
            throw std::runtime_error("free-form geometry not supported");
        } else if (first == "l") {
            throw std::runtime_error("line elements not supported");
        } else if (first == "g") {
            f.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        } else {
            std::cout << "warning: unknown directive: " << first << std::endl;
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

