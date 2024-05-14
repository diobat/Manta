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

    std::string name = absolutePath.substr(absolutePath.find_last_of('/') + 1);
    name = name.substr(0, name.find_last_of('.'));     // Remove the file extension

    // If the meshes are already loaded, do not load them again
    if(_loadedModelPaths.find(absolutePath) == _loadedModelPaths.end())
    {
        Model& newModel = registry.emplace<Model>(modelEntity, _factory.importFromFile(absolutePath));
        newModel.name = name;
        _loadedModelPaths.insert(absolutePath);
    }
    else
    {
        registry.emplace<Model>(modelEntity, Model{absolutePath, getMeshes(absolutePath), name});
    }

    return modelEntity;
}

entt::entity model_mesh_library::createModelFromMesh(entt::registry& registry, const std::string& name, const Mesh& meshData )
{
    entt::entity modelEntity = _core->getScene()->newEntity();

    if(_loadedModelPaths.find(name) == _loadedModelPaths.end())
    {
        _meshes[name] = std::make_shared<std::vector<Mesh>>();
        Model& newModel = registry.emplace<Model>(modelEntity, _factory.importFromMeshData(name, meshData));
        _loadedModelPaths.insert(name);
    }
    else
    {
        registry.emplace<Model>(modelEntity, Model{name, getMeshes(name), name});
    }

    return modelEntity;
}

 Model model_mesh_library::createModelFromMesh(const std::string& name, const Mesh& meshData )
{
    Model result;

    if(_loadedModelPaths.find(name) == _loadedModelPaths.end())
    {
        _meshes[name] = std::make_shared<std::vector<Mesh>>();
        result = _factory.importFromMeshData(name, meshData);
        _loadedModelPaths.insert(name);
    }
    else
    {
        result = Model{name, getMeshes(name), name};
    }
    
    return result;
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
