#include <cstdio>
#include <cmath>
#include <set>
#include <future>

#include <Mesh.h>
#include <Slicer.h>

#include <cairo/cairo.h>

using namespace geom;


Slicer::Slicer(const std::vector<FaceVertexMesh> &meshes, double dz) {

    auto bounds = minmax(meshes);
    slice_count = (int)((bounds[1] - bounds[0]) / dz) + 1;

    for (auto &mesh: meshes) {
        polygons_.push_back(std::vector<Slicer::Polygons>(slice_count));
        graph_ = std::vector<Slicer::Graph>(slice_count);
        sliceTriangles(mesh, bounds, dz);        
        computeContours(mesh, polygons_.back());
    }
    
}

std::vector<float> Slicer::whitelist() const {
    return exportImages("");
};

std::vector<float> Slicer::write(const std::string &prefix) const {
    return exportImages(prefix);
}

std::array<float, 2> Slicer::minmax(const std::vector<FaceVertexMesh> &meshes) {
    float min = +INFINITY;
    float max = -INFINITY;
    for (auto &mesh: meshes) {
        auto bounds = mesh.minmax(2);
        if (bounds[0] < min) min = bounds[0];
        if (bounds[1] > max) max = bounds[1];
    }
    return {min, max};
}

void Slicer::sliceTriangles(const FaceVertexMesh &geometry, std::array<float, 2> bounds, double dz) {
   
    float minz = bounds[0];
    float maxz = bounds[1];

    for (int tidx = 0; tidx < geometry.triangles().size(); tidx++) {

        auto &triangle = geometry.triangles()[tidx];

        auto bounds = geometry.minmax(triangle, 2);
        int slice_min = (int) ((bounds[0] - minz) / dz);
        int slice_max = (int) ((bounds[1] - minz) / dz);

        for (int sidx = slice_min; sidx <= slice_max; sidx++) {
            double z = sidx * dz + minz;

            int count = 0;
            std::array<Slicer::Intersection, 2> i;

            Slicer::Graph &graph = graph_[sidx];
           
            for (int e = 0; e < 3; e++) {

                int v1 = triangle[e % 3];
                int v2 = triangle[(e + 1) % 3];

                auto &p1 = geometry.vertices()[v1];
                auto &p2 = geometry.vertices()[v2];

                auto p_min = p1[2] < p2[2] ? p1: p2;
                auto p_max = p1[2] < p2[2] ? p2: p1;
            
                // ignore triangles parallel to slicing plane 
                if (p_min[2] == z && z == p_max[2]) continue;

                if (p1[2] == z) {
                    uint64_t id = Slicer::make_vertex(v1);
                    Slicer::Point p {p1[0], p1[1]};
                    if (i[0].id != id && i[1].id != id && count < 2) {
                        i[count++] = {id, p};
                    }
                } else if (p2[2] == z) {
                    uint64_t id = Slicer::make_vertex(v2);
                    Slicer::Point p {p2[0], p2[1]};
                    if (i[0].id != id && i[1].id != id && count < 2) {
                        i[count++] = {id, p};
                    }
                } else if (p_min[2] < z && z < p_max[2]) {
                    uint64_t id = Slicer::make_edge(v1, v2);
                    double s = (z - p_min[2]) / (p_max[2] - p_min[2]);
                    Slicer::Point p;
                    p[0] = p_min[0] + s * (p_max[0] - p_min[0]);
                    p[1] = p_min[1] + s * (p_max[1] - p_min[1]);
                    if (i[0].id != id && i[1].id != id && count < 2) {
                        i[count++] = {id, p};
                    }
                }
            }
            
            // ignore triangles with only one intersection point
            if (count == 2) {
                auto &i0 = graph[i[0].id];
                auto &i1 = graph[i[1].id];

                i0.position = i[0].position;
                i0.edges[i0.edge_count++] = i[1].id;

                i1.position = i[0].position;
                i1.edges[i1.edge_count++] = i[0].id;
            }
        }
    }
}


void Slicer::computeContours(const FaceVertexMesh &g, std::vector<Slicer::Polygons> &polygons_) {

    std::vector<std::future<void>> workers;
    
    for (int i = 0; i < slice_count; i++) {
        
        workers.push_back(std::async([&](int i) {

            Slicer::Graph &graph = graph_[i];
            Slicer::Polygons &polygons = polygons_[i];

            std::set<uint64_t> visited;
            std::vector<uint64_t> polygon;
            std::vector<Point> polygon2;

            for (auto &entry: graph) {
                auto &a = entry.first;
                auto &b = entry.second;
                if (visited.find(a) != visited.end()) continue;
                polygon = {a, b.edges[0]};
                polygon2 = {b.position};
                do {
                    auto curr = polygon.back();
                    auto next = graph[curr];
                    polygon2.push_back(next.position);
                    if (next.edges[0] != polygon[polygon.size() - 2]) {
                        polygon.push_back(next.edges[0]);
                    } else {
                        polygon.push_back(next.edges[1]);
                    }
                    visited.insert(curr);
                } while (polygon.back() != polygon.front());
                            
                polygons.push_back(polygon2);
            }
        }, i));
    }

    for (int i = 0; i < workers.size(); i++) {
        workers[i].wait();
    }
}

std::vector<float> Slicer::exportImages(const std::string &path_prefix) const {

    const float scene_width = 192;
    const float scene_height = 108;

    const int image_width = 1920;
    const int image_height = 1080;

    cairo_surface_t *surface = cairo_image_surface_create(CAIRO_FORMAT_A8, image_width, image_height);
   
    bool should_write_images = path_prefix != "";

    std::vector<float> whitelist;

    for (int i = 0; i < slice_count; i++) {

        cairo_t *cr = cairo_create(surface);

        cairo_set_operator (cr, CAIRO_OPERATOR_SOURCE);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
        cairo_set_line_width(cr, 0);

        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_paint(cr);

        cairo_translate(cr, image_width / 2, image_height / 2);
        cairo_scale(cr, image_width / scene_width, image_height / scene_height);

        for (const auto &polygons: polygons_) {
            cairo_new_path(cr);
            for (const auto &polygon: polygons[i]) {
                cairo_set_source_rgba(cr, 0, 0, 0, 1);
                cairo_set_line_width(cr, 0.05);
                cairo_move_to (cr, polygon[0][0], polygon[0][1]);
                for (const auto &point: polygon) {
                    cairo_line_to(cr, point[0], point[1]);
                }
            }
            cairo_fill(cr);
        }


        if (should_write_images) {
            std::string path = path_prefix + std::to_string(i) + ".png";
            cairo_surface_write_to_png(surface, path.c_str());
        }

        uint8_t *data = cairo_image_surface_get_data(surface);

        int sum = 0;
        for (int i = 0; i < image_width * image_height; i++) {
            sum += *(data++) ? 1: 0;
        }
      
        whitelist.push_back((float) sum / image_width / image_height);
        cairo_destroy(cr);
    }

    cairo_surface_destroy(surface);
    return whitelist;
}
