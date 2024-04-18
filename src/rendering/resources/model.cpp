#include "rendering/resources/model.hpp"
#include "rendering/rendering.hpp"

// Assimp includes
#include <assimp/postprocess.h>

// First-party includes
#include "helpers/RootDir.hpp"

namespace
{
    std::vector<Vertex> getVertexData(const aiMesh* mesh, const aiScene* scene) {
        std::vector<Vertex> vertices;
        // VertexData
        for (unsigned int i = 0; i < mesh->mNumVertices; i++)
        {
            Vertex vertex;
            glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
            
            // Positions
            vector.x = mesh->mVertices[i].x;
            vector.y = mesh->mVertices[i].y;
            vector.z = mesh->mVertices[i].z;
            vertex.Position = vector;
            
            // Normals
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
            
            // texture coordinates
            if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
            {
                glm::vec2 vec;
                // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
                // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
                vec.x = mesh->mTextureCoords[0][i].x;
                vec.y = mesh->mTextureCoords[0][i].y;
                vertex.TexCoords = vec;
            }
            else
            {
                vertex.TexCoords = glm::vec2(0.0f, 0.0f);
            }
            
            // Colors
            aiColor4D diffuse;
            aiMaterial* mtl = scene->mMaterials[mesh->mMaterialIndex];

            if(AI_SUCCESS == aiGetMaterialColor(mtl, AI_MATKEY_COLOR_DIFFUSE, &diffuse))
            {
                vector.x = diffuse.r;
                vector.y = diffuse.g;
                vector.z = diffuse.b;
            }
            else
            {
                vector.x = 1.0f;
                vector.y = 1.0f;
                vector.z = 1.0f;
            }
            vertex.Color = vector;

            // Tangents
            if(mesh->HasTangentsAndBitangents())
            {
                vector.x = mesh->mTangents[i].x;
                vector.y = mesh->mTangents[i].y;
                vector.z = mesh->mTangents[i].z;
                vertex.Tangent = vector;
            }
            else
            {
                vertex.Tangent = glm::vec3(0.0f, 0.0f, 0.0f);
            }

            vertices.push_back(vertex);
        }
        return vertices;
    }

    std::vector<unsigned int> getIndexData(const aiMesh* mesh)
    {
        std::vector<unsigned int> indices;

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            // retrieve all indices of the face and store them in the indices vector
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                indices.push_back(face.mIndices[j]);
        }

        return indices;
    }

    std::vector<image> getTextureData(const aiMesh* mesh)
    {

    }

    std::vector<image> loadMaterialTextures(aiMaterial* mat, aiTextureType type, const std::string& typeName)
    {
 
    }
}

model_mesh_library::model_mesh_library(rendering_system* core)    :
    _core(core)
{
    ;
}

Model model_mesh_library::importFromFile(const std::string& absolutePath)
{
    // Determine if meshes have already been loaded
    if(isLoaded(absolutePath))
    {
        return Model{absolutePath, getMeshes(absolutePath), nullptr};
    }

    // If the model is not loaded, load it
    const aiScene* scene = importer.ReadFile(absolutePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        return Model{"", nullptr, nullptr};
    }

    // Load the model
    processNode(scene, scene->mRootNode, absolutePath);
    _loadedModelPaths.push_back(absolutePath);

    return Model{absolutePath, getMeshes(absolutePath), nullptr};
}

entt::entity model_mesh_library::createModel(entt::registry& registry, const std::string& path)
{
    entt::entity modelEntity = _core->getScene()->newEntity();

    std::string absolutePath = ROOT_DIR + path; 

    Model& model = importFromFile(absolutePath);
    registry.emplace<Model>(modelEntity, model);

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

void model_mesh_library::processNode(const aiScene* scene, aiNode* node, const std::string& absolutePath)
{
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        Mesh importedMesh = processMesh(mesh, scene, absolutePath);

        if(_meshes.find(absolutePath) == _meshes.end())
        {
            _meshes[absolutePath] = std::make_shared<std::vector<Mesh>>();
        }

        _meshes[absolutePath]->push_back(importedMesh);
    }

    // we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(scene, node->mChildren[i], absolutePath);
    }
}

Mesh model_mesh_library::processMesh(const aiMesh* assimpMesh, const aiScene* scene, const std::string& absolutePath)
{
    Mesh importedMesh;

    importedMesh.vertexData = getVertexData(assimpMesh, scene);
    importedMesh.vertexBuffer = _core->getMemorySystem().createVertexBuffer(importedMesh.vertexData);
    importedMesh.indexData = getIndexData(assimpMesh);
    importedMesh.indexBuffer = _core->getMemorySystem().createIndexBuffer(importedMesh.indexData);
    importedMesh.path = absolutePath;
    
    return importedMesh;
}

bool model_mesh_library::isLoaded(const std::string& path) const
{
    return std::find(_loadedModelPaths.begin(), _loadedModelPaths.end(), path) != _loadedModelPaths.end();
}

std::shared_ptr<std::vector<Mesh>> model_mesh_library::getMeshes(const std::string& path)
{
    return _meshes[path];
}
