#pragma once

// External includes
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// STD includes
#include <vector>
#include <memory>

// First party includes
#include "rendering/resources/vertex.hpp"

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

// Hash function for Mesh
// namespace std
// {
//     template <>
//     struct hash<Mesh>
//     {
//         std::size_t operator()(const Mesh& mesh) const
//         {
//             size_t seed = 0;
//             for(const Vertex& vertex : mesh.vertices)
//             {
//                 seed ^= std::hash<Vertex>{}(vertex) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//             }
//             for(const unsigned int& index : mesh.indices)
//             {
//                 seed ^= std::hash<unsigned int>{}(index) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
//             }
//             return seed;
//         }
//     };
// }

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::string path;
};

struct Model
{
    std::shared_ptr<std::vector<Mesh>> meshes = nullptr;
    std::string path;

    void operator=(const Model& other)
    {
        meshes = other.meshes;
        path = other.path;
    }
};	

class model_mesh_library
{
public:
    model_mesh_library();

    Model importFromFile(const std::string& absolutePath);
    entt::entity createModel(entt::registry& registry, const std::string& path);

    std::shared_ptr<std::vector<Mesh>> getMeshes(const std::string& path);

private:

    void processNode(const aiScene* scene, aiNode* node, const std::string& absolutePath);
    Mesh processMesh(const aiMesh* assimpMesh, const aiScene* scene, const std::string& absolutePath);

    bool isLoaded(const std::string& path) const;

    std::unordered_map<std::string, std::shared_ptr<std::vector<Mesh>>> _meshes;
    std::vector<std::string> _loadedModelPaths;
    Assimp::Importer importer;
};

