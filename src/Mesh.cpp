#include <Mesh.h>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <sstream>
#include <map>
#include <filesystem>
#include <algorithm>
#include <string>
#include <Progress.h>

Mesh::Mesh(int vertexCount, const std::vector<std::array<int, 3>> &faces) {

    std::cout << "info: loading mesh"  << std::endl;
    ProgressBar progress;

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

    // Associate each vertex with an index.
    for (int i = 0; i < vertexCount; i++) {
        vertices_[i].index = i;
    }

    for (int i = 0; i < faces.size(); i++) {
        if (i % (faces.size() / 100) == 0) {
            progress.update((float) i / faces.size());
        }

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

    progress.finish();
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

Geometry::Geometry(std::istream &file) {
    
    printf("info: reading file\n");
    ProgressBar progress;

    std::stringstream ss;
    ss << file.rdbuf();
    std::string data = ss.str();
    uint32_t index = 0;
    uint32_t size = data.size();
    uint32_t last_progress = 0;

   auto next_word = [&]() {
        while (data[index] != ' ') {
            if (data[index] == '\0') return false;
            index++;
        }
        while (data[index] == ' ') {
            if (data[index] == '\0') return false;
            index++;
        }
        return true;
    };

    auto next_line = [&]() {
        while (data[index] != '\n' && data[index] != '\0') {
            index++;
        }
        if (data[index] == '\n') {
            index++;
            return true;
        } else {
            return false;
        }
    };

    do {

        if (data.substr(index, 2) == "v ") {
            next_word();
            float x = atof(&data[index]);
            next_word();
            float y = atof(&data[index]);
            next_word();
            float z = atof(&data[index]);
            positions_.push_back({x, y, z});
        } else if (data.substr(index, 2) == "f ") {
            next_word();
            int x = atoi(&data[index]);
            next_word();
            int y = atoi(&data[index]);
            next_word();
            int z = atoi(&data[index]);
            faces_.push_back({x - 1, y - 1, z - 1});

        }

        if ((int) (100.0 * index / size) > last_progress) {
            last_progress = (int) (100.0 * index / size);
            progress.update((float) index / size);
        }

    } while (next_line());

    
    progress.finish();

    std::vector<std::array<uint32_t, 3>> triangles;
    std::vector<std::array<uint32_t, 2>> edges;

    std::map<std::array<uint32_t, 2>, uint32_t> edge_map;
    for (auto &face: faces_) {

        std::array<uint32_t, 3> t_edges;

        for (int i = 0; i < 3; i++) {
            uint32_t v1 = face[i];
            uint32_t v2 = face[(i + 1) % 3];
            uint32_t vmin = v1 < v2 ? v1: v2;
            uint32_t vmax = v1 < v2 ? v2: v1;
            std::array<uint32_t, 2> edge = {vmin, vmax};

            auto it = edge_map.find(edge);
            if (it == edge_map.end()) {
                t_edges[i] = edge_map.size();
                edge_map.emplace(edge, (uint32_t) edge_map.size());
                edges.push_back(edge);
            } else {
                t_edges[i] = it->second;
            }   
        }
        triangles.push_back(t_edges);
    }

}

const std::vector<std::array<float, 3>>& Geometry::positions() const {
    return positions_;
}

const std::vector<Geometry::Triangle>& Geometry::faces() const {
    return faces_;
}