#pragma once

// External includes
#include <entt/entt.hpp>
#include <glm/glm.hpp>

// STD includes
#include <vector>
#include <memory>

// First party includes
#include "rendering/resources/model.hpp"
#include "util/modelImporter.hpp"

// Forward declarations
class rendering_system;

class model_mesh_library
{
    friend class ModelImporter;
public:
    model_mesh_library(rendering_system* core);

    entt::entity createModel(entt::registry& registry, const std::string& path);

    std::shared_ptr<std::vector<Mesh>> getMeshes(const std::string& path);
    bool isLoaded(const std::string& path) const;

    void cleanup();

private:
    ModelImporter _factory;

    std::unordered_map<std::string, std::shared_ptr<std::vector<Mesh>>> _meshes;
    std::set<std::string> _loadedModelPaths;

    rendering_system* _core;
};

