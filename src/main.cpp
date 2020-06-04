#include <iostream>

#include <Mesh.h>
#include <Slicer.h>
#include <Timer.h>

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

    double dz = atof(argv[2]);

    geom::Timer t;
    printf("info: loading meshes\n");
    geom::FaceVertexMesh mesh0{argv[1]};
    std::vector<geom::FaceVertexMesh> meshes;
    for (int i = -1; i < 1; i++) {
        for (int j = -1; j < 1; j++) {
            geom::FaceVertexMesh mesh = mesh0;
            float matrix [4][4] = {
                {1, 0, 0, 20.0f * i},
                {0, 1, 0, 20.0f * j},
                {0, 0, 1, 0.0},
                {0, 0, 0, 1}
            };
            mesh.transform(matrix);
            meshes.push_back(mesh);
        }
    }
    printf("%.2f seconds\n\n", t.duration());

    printf("info: slicing\n");
    geom::Timer t2;
    geom::Slicer slices(meshes, dz);
    std::vector<float> whitelist = slices.whitelist();
    printf("%.2f seconds\n\n", t2.duration());

    return 0;
    
}
