#include "OptimizedGeometry.h"
#include "Parser.h"
#include <fstream>
#include <iostream>

bool OptimizedGeometry::CreateOptimizedFile(const std::string& rawFile, const std::string& optimizedFile) {

    Parser parser(rawFile);
    meshes = parser.GetCombinedList();

    std::ofstream out(optimizedFile, std::ios::binary);
    if (!out) {
        return false;
    }
    // Записываем число мешей
    size_t numMeshes = meshes.size();
    out.write(reinterpret_cast<const char*>(&numMeshes), sizeof(size_t));
    for (const auto& mesh : meshes) {
        // Записываем число треугольников в меше
        size_t numTris = mesh.size();
        out.write(reinterpret_cast<const char*>(&numTris), sizeof(size_t));
        for (const auto& tri : mesh) {
            out.write(reinterpret_cast<const char*>(&tri.v0), sizeof(Vector3g));
            out.write(reinterpret_cast<const char*>(&tri.v1), sizeof(Vector3g));
            out.write(reinterpret_cast<const char*>(&tri.v2), sizeof(Vector3g));
        }
    }
    out.close();
    return true;
}

bool OptimizedGeometry::LoadFromFile(const std::string& optimizedFile) {
    std::ifstream in(optimizedFile, std::ios::binary);
    if (!in) {
       
        return false;
    }
    meshes.clear();
    size_t numMeshes;
    in.read(reinterpret_cast<char*>(&numMeshes), sizeof(size_t));
    for (size_t i = 0; i < numMeshes; ++i) {
        size_t numTris;
        in.read(reinterpret_cast<char*>(&numTris), sizeof(size_t));
        std::vector<TriangleCombined> mesh;
        mesh.resize(numTris);
        for (size_t j = 0; j < numTris; ++j) {
            in.read(reinterpret_cast<char*>(&mesh[j].v0), sizeof(Vector3g));
            in.read(reinterpret_cast<char*>(&mesh[j].v1), sizeof(Vector3g));
            in.read(reinterpret_cast<char*>(&mesh[j].v2), sizeof(Vector3g));
        }
        meshes.push_back(mesh);
    }
    in.close();
    return true;
}
