#pragma once

// External includes
#include <glm/glm.hpp>

// STD includes
#include <vector>
#include <memory>

// First party includes
#include "rendering/resources/vertex.hpp"
#include "rendering/resources/memory.hpp"
#include "rendering/resources/texture.hpp"

struct Mesh
{
    std::vector<Vertex> vertexData;
    memoryBuffer vertexBuffer;
    std::vector<unsigned int> indexData;
    memoryBuffer indexBuffer;

    std::array<unsigned int, static_cast<size_t>(E_TextureType::SIZE)> textureIndices = {0};

    std::string path; 
};

struct Model
{
    std::string path;

    std::shared_ptr<std::vector<Mesh>> meshes = nullptr;

    glm::mat4 modelMatrix = glm::mat4(1.0f);
    
    void operator=(const Model& other) {
        path = other.path;
        meshes = other.meshes;
    }
};	

