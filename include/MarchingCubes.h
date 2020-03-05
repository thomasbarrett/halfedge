#ifndef MARCHING_CUBES_H
#define MARCHING_CUBES_H

#include "Mesh.h"
#include <iostream>

void MarchingCubes(const Geometry &g) {

    auto compare_x = [](auto &a, auto &b) {
        return a[0] < b[0];
    };

    auto [eminx, emaxx] = std::minmax_element(
        g.positions().begin(),
        g.positions().end(),
        compare_x
    );

    double minx = (*eminx)[0];
    double maxx = (*emaxx)[0];
    double deltax = maxx - minx;

    auto compare_y = [](auto &a, auto &b) {
        return a[1] < b[1];
    };

    auto [eminy, emaxy] = std::minmax_element(
        g.positions().begin(),
        g.positions().end(),
        compare_y
    );

    double miny = (*eminy)[1];
    double maxy = (*emaxy)[1];
    double deltay = maxy - miny;
    
    double delta = deltax < deltay ? deltax: deltay;
    double dx = delta / 1000;

    int M = (int) (deltax / dx);
    int N = (int) (deltay / dx);
    std::cout << M * N << " vertices sampled" << std::endl;
    bool grid[M][N];
    memset(grid, 0, M * N * sizeof(bool));

    for (auto &face: g.mesh().faces()) {
        std::vector<double> xs;
        std::vector<double> ys;
        for (auto v: face.adjacentVertices()) {
            xs.push_back(g.positions().at(v->index)[0]);
            ys.push_back(g.positions().at(v->index)[0]);
        }

        auto [tminx, tmaxx] = std::minmax_element(xs.begin(), xs.end());
        auto [tminy, tmaxy] = std::minmax_element(xs.begin(), xs.end());
        
        int i = (int)((*tmaxx - minx) / dx);
        int j = (int)((*tmaxy - miny) / dx);

        int i0 = (int)((*tminx - minx) / dx);
        int j0 = (int)((*tminy - miny) / dx);

        assert(i0 >= 0 && i0 < M);
        assert(j0 >= 0 && j0 < N);
        assert(i >= 0 && i < M);
        assert(j >= 0 && j < N);

        for (int i_ = i0; i_ <= i; i_++) {
            for (int j_ = j0; j_ <= j; j_++) {
                grid[i_][j_] = true;
            }
        }
    }

    int acc = 0;
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            if (grid[i][j]) {
                acc += 1;
            }
        }
    }
    std::cout << "Marching Cubes: " << acc << std::endl;
}


#endif /* MARCHING_CUBES_H */