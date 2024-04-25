#include "rendering/modelLibrary.hpp"
#include "rendering/rendering.hpp"

// First-party includes
#include "helpers/RootDir.hpp"

model_mesh_library::model_mesh_library(rendering_system* core)  :
    _factory(this),
    _core(core)
{
    ;
}

entt::entity model_mesh_library::createModel(entt::registry& registry, const std::string& path)
{
    entt::entity modelEntity = _core->getScene()->newEntity();
    std::string absolutePath = ROOT_DIR + path; 

    // If the model is already loaded, do not load it again
    if(_loadedModelPaths.find(absolutePath) == _loadedModelPaths.end())
    {
        registry.emplace<Model>(modelEntity, _factory.importFromFile(absolutePath));
        _loadedModelPaths.insert(absolutePath);
    }
    else
    {
        registry.emplace<Model>(modelEntity, Model{absolutePath, getMeshes(absolutePath)});
    }

    return modelEntity;
}

void model_mesh_library::cleanup()
{
    for(auto& mesh : _meshes)
    {
        for(auto& meshData : *mesh.second)
        {
            _core->getMemorySystem().freeBuffer(meshData.vertexBuffer);
            _core->getMemorySystem().freeBuffer(meshData.indexBuffer);
        }
    }
}

bool model_mesh_library::isLoaded(const std::string& path) const
{
    return std::find(_loadedModelPaths.begin(), _loadedModelPaths.end(), path) != _loadedModelPaths.end();
}

std::shared_ptr<std::vector<Mesh>> model_mesh_library::getMeshes(const std::string& path)
{
    return _meshes[path];
}
