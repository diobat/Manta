#pragma once

// External includes
#include <entt.hpp>
#include <glm/glm.hpp>

// STD includes
#include <vector>

// First party includes
#include "rendering/resources/vertex.hpp"

struct Mesh
{
    std::vector<Vertex> vertices;
    std::vector<unsigned int> indices;
    std::string path;
};

struct Model
{
    std::vector<entt::entity> meshes;
};	


entt::entity createModel(entt::registry& registry, const std::string& path);

bool isMeshLoaded(entt::registry& registry, const std::string& path);
std::vector<entt::entity> getMeshes(entt::registry& registry, const std::string& path);