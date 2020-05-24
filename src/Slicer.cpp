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
#include <cairo/cairo.h>
#include <Timer.h>
#include <stack>
#include <future>


void Slicer::sliceGeometry(const Geometry &geometry, double dz) {

    geom::Timer t1;
    std::cout << "info: slicing triangles" << std::endl;
    sliceTriangles(geometry, dz);
    printf("%.2f seconds\n\n", t1.duration());

    geom::Timer t2;
    printf("info: building contours\n");
    computeContours(geometry);
    printf("%.2f seconds\n\n", t2.duration());

    geom::Timer t3;
    printf("info: rasterizing images\n");
    exportImages("test/img/slice");
    printf("%.2f seconds\n", t3.duration());

}

void Slicer::sliceTriangles(const Geometry &geometry, double dz) {
   
    auto [minz, maxz] = Geometry::minmax(geometry, 2);

    slice_count_ = (int)((maxz - minz) / dz) + 1;
    points_.resize(slice_count_);
    graph_.resize(slice_count_);

    for (int tidx = 0; tidx < geometry.faces().size(); tidx++) {

        auto &triangle = geometry.faces()[tidx];

        auto [tminz, tmaxz] = Geometry::minmax(geometry, triangle, 2);
        int slice_min = (int) ((tminz - minz) / dz);
        int slice_max = (int) ((tmaxz - minz) / dz);

        for (int sidx = slice_min; sidx <= slice_max; sidx++) {
            double z = sidx * dz + minz;

            geom::FiniteSet<Slicer::Intersection> intersections;
            Slicer::Points &points = points_[sidx];
            Slicer::Graph &graph = graph_[sidx];
           
            for (int e = 0; e < 3; e++) {

                int v1 = triangle[e % 3];
                int v2 = triangle[(e + 1) % 3];

                auto &p1 = geometry.positions()[v1];
                auto &p2 = geometry.positions()[v2];

                auto p_min = p1[2] < p2[2] ? p1: p2;
                auto p_max = p1[2] < p2[2] ? p2: p1;
            
                // ignore triangles parallel to slicing plane 
                if (p_min[2] == z && z == p_max[2]) continue;

                if (p1[2] == z) {

                    Slicer::Point p {p1[0], p1[1]};
                    intersections.insert(Slicer::make_vertex(v1, p));

                } else if (p2[2] == z) {

                    Slicer::Point p {p2[0], p2[1]};
                    intersections.insert(Slicer::make_vertex(v2, p));

                } else if (p_min[2] < z && z < p_max[2]) {

                    double s = (z - p_min[2]) / (p_max[2] - p_min[2]);
                    Slicer::Point p;
                    p[0] = p_min[0] + s * (p_max[0] - p_min[0]);
                    p[1] = p_min[1] + s * (p_max[1] - p_min[1]);
                    intersections.insert(Slicer::make_edge(v1, v2, p));

                }
            }
            
            // ignore triangles with only one intersection point
            if (intersections.size() == 2) {
                auto &i0 = graph[intersections[0].first];
                auto &i1 = graph[intersections[1].first];

                i0.position = intersections[0].second;
                i0.edges[i0.edge_count++] = intersections[1].first;

                i1.position = intersections[1].second;
                i1.edges[i1.edge_count++] = intersections[0].first;
            }
        }
    }
}

void Slicer::computeContours(const Geometry &g) {

    polygons_.resize(graph_.size());

    std::vector<std::future<void>> workers;
    
    for (int i = 0; i < slice_count_; i++) {
        
        workers.push_back(std::async([&](int i) {

            Slicer::Graph &graph = graph_[i];
            Slicer::Points &points = points_[i];
            Slicer::Polygons &polygons = polygons_[i];

            std::set<uint64_t> visited;
            std::vector<uint64_t> polygon;
            std::vector<Point> polygon2;

            for (auto &[a, b]: graph) {
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

void Slicer::exportPolygons(const std::string &path) {

    std::ofstream ofile;
    ofile.open(path);
    ProgressBar progress;
    for (int i = 0; i < slice_count_; i++) {
        Polygons &polygons = polygons_[i];
        progress.update((float) i / slice_count_);
        uint32_t polygon_count = polygons.size();    
        ofile.write((char *) &polygon_count, sizeof(uint32_t));
        for (auto &polygon: polygons) {
            uint32_t point_count = polygon.size();
            ofile.write((char *) &point_count, sizeof(uint32_t));
            for (auto &point: polygon) {
                float p1 = point[0];
                float p2 = point[1];
                ofile.write((char *) &p1, sizeof(float));
                ofile.write((char *) &p2, sizeof(float));
            }
        }
    } 
    ofile.close();
    progress.finish();
}

void Slicer::exportImages(const std::string &path_prefix) {

    cairo_surface_t *surface;
    cairo_t *cr;

    surface = cairo_image_surface_create(CAIRO_FORMAT_RGB24,1920,1080);
   
    ProgressBar progress;
    for (int i = 0; i < slice_count_; i++) {

        cr = cairo_create(surface);

        Polygons &polygons = polygons_[i];
        progress.update((float) i / slice_count_);

        cairo_set_source_rgb(cr, 0, 0, 0);
        cairo_paint(cr);
        cairo_translate(cr, 960, 540);
        cairo_scale(cr, 20, 20);
        cairo_set_antialias(cr, CAIRO_ANTIALIAS_NONE);
        cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);

        for (const auto &polygon: polygons) {
            cairo_set_source_rgb(cr, 1, 1, 1);
            cairo_set_line_width(cr, 0.05);
            cairo_move_to (cr, polygon[0][0], polygon[0][1]);
            for (const auto &point: polygon) {
                cairo_line_to(cr, point[0], point[1]);
            }
        }

        cairo_stroke_preserve(cr);
        cairo_fill(cr);

        std::string path = path_prefix + std::to_string(i) + ".png";
        // cairo_surface_write_to_png(surface, path.c_str());
        cairo_destroy(cr);

    }
    progress.finish();

    cairo_surface_destroy(surface);
}
