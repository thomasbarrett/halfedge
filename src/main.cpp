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
    if (argc != 2) {
        std::cout << "usage: slicer [file.obj]" << std::endl;
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

    Geometry geometry{file};
    std::cout << "info: mesh loaded" << std::endl;

    Slicer::sliceGeometry(geometry);
    std::cout << "info: slicing complete" << std::endl;

    return 0;
    
}