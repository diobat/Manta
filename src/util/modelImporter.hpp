#pragma once

#include <vector>
#include <unordered_map>

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>

#include "rendering/resources/vertex.hpp"
#include "rendering/resources/texture.hpp"
#include "rendering/resources/model.hpp"

class model_mesh_library;

class ModelImporter{
public:
    ModelImporter(model_mesh_library* meshLibrary);
    Model importFromFile(const std::string& absolutePath);
    Model importFromMeshData(const std::string& name, const Mesh& meshData,  glm::mat4 modelMatrix = glm::mat4(1.0f));

private: 
    // Get data from the assimp struct
    std::vector<Vertex> getVertexData(const aiMesh* mesh, const aiScene* scene);
    std::vector<unsigned int> getIndexData(const aiMesh* mesh);
    std::array<unsigned int, static_cast<size_t>(E_TextureType::SIZE)> getTextureData(const aiScene* scene, const aiMesh* mesh);

    unsigned int loadMaterialTextures(const aiScene* scene, aiMaterial* mat, aiTextureType type);

    // STB calls
    void processNode(const aiScene* scene, aiNode* node, const std::string& absolutePath);
    Mesh processMesh(const aiMesh* assimpMesh, const aiScene* scene, const std::string& absolutePath);

    std::unordered_map<aiTextureType, E_TextureType> _textureTypeMap = {
        {aiTextureType_DIFFUSE, E_TextureType::DIFFUSE},
        {aiTextureType_SPECULAR, E_TextureType::SPECULAR},
        {aiTextureType_NORMALS, E_TextureType::NORMAL},
        {aiTextureType_HEIGHT, E_TextureType::HEIGHT},
        {aiTextureType_LIGHTMAP, E_TextureType::LIGHTMAP},
        {aiTextureType_SHININESS, E_TextureType::ROUGHNESS}
    };


    Assimp::Importer importer;

    model_mesh_library* _meshLibrary;

};