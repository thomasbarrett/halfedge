#include <iostream>
#include <fstream>
#include <vector>
#include <array>
#include <map>
#include <algorithm>
#include <Mesh.h>
#include <Slicer.h>
#include <locale>

bool isFileOBJ(std::string path) {
    std::string fileExtension = path.substr(path.find("."));
    
    if(fileExtension == ".obj") return true;
    if(fileExtension == ".Obj") return true;
    if(fileExtension == ".OBJ") return true;

    return false;
}

int main(int argc, char const *argv[]) {
    if (argc != 3) {
        std::cout << "usage: slicer file.obj dz" << std::endl;
        return 1;
    }

    if (!isFileOBJ(argv[1])) {
        std::cout << "error: input file does not have 'obj' extension" << std::endl;
        return 1;
    }

    std::ifstream file{argv[1]};
    if (file.bad()) {
        std::cout << "error: input file not found" << std::endl;
        return 1;
    }

    double dz = atof(argv[2]);

    auto start = std::chrono::steady_clock::now();
    Geometry geometry{file};
    auto end = std::chrono::steady_clock::now();
    float elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() / 1000.0;
    printf("%.2f seconds\n\n", elapsed);

    Slicer().sliceGeometry(geometry, dz);

    return 0;
    
}
