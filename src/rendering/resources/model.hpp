#pragma once

// External includes
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// STD includes
#include <vector>
#include <memory>

// First party includes
#include "rendering/resources/vertex.hpp"
#include "rendering/resources/memory.hpp"
#include "rendering/resources/texture.hpp"

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

// Forward declarations
class rendering_system;

struct Mesh
{
    std::vector<Vertex> vertexData;
    memoryBuffer vertexBuffer;
    std::vector<unsigned int> indexData;
    memoryBuffer indexBuffer;

    std::string path; 
};

struct Model
{
    std::string path;

    std::shared_ptr<std::vector<Mesh>> meshes = nullptr;
    std::shared_ptr<std::vector<image>> textures = nullptr;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    
    void operator=(const Model& other) {
        path = other.path;
        meshes = other.meshes;
        textures = other.textures;
    }
};	

class model_mesh_library
{
public:
    model_mesh_library(rendering_system* core);

    Model importFromFile(const std::string& absolutePath);
    entt::entity createModel(entt::registry& registry, const std::string& path);

    std::shared_ptr<std::vector<Mesh>> getMeshes(const std::string& path);
    std::shared_ptr<std::vector<image>> getTextures(const std::string& path);

    void cleanup();

private:
    void processNode(const aiScene* scene, aiNode* node, const std::string& absolutePath);
    Mesh processMesh(const aiMesh* assimpMesh, const aiScene* scene, const std::string& absolutePath);

    bool isLoaded(const std::string& path) const;

    std::unordered_map<std::string, std::shared_ptr<std::vector<Mesh>>> _meshes;

    std::vector<std::string> _loadedModelPaths;
    Assimp::Importer importer;

    rendering_system* _core;
};

