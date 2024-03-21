#include "ECS/components/model.hpp"

// Assimp includes
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

// First-party includes
#include "helpers/RootDir.hpp"

Model importFromFile(entt::registry& registry, const std::string& absolutePath);
void processNode(Model& model, entt::registry& registry, const aiScene* scene, aiNode* node, const std::string& absolutePath);
entt::entity processMesh(entt::registry& registry, const aiMesh* assimpMesh, const aiScene* scene, const std::string& absolutePath);
std::vector<Vertex> getVertexData(const aiMesh* mesh, const aiScene* scene);
std::vector<unsigned int> getIndexData(const aiMesh* mesh);

entt::entity createModel(entt::registry& registry, const std::string& path)
{
    entt::entity modelEntity = registry.create();
    Model& model = registry.emplace<Model>(modelEntity);

    std::string absolutePath = ROOT_DIR + path; 

    model = importFromFile(registry, absolutePath);

    return modelEntity;
}

Model importFromFile(entt::registry& registry, const std::string& absolutePath)
{

    Model model;

    // Determine if meshes have already been loaded
    if(isMeshLoaded(registry, absolutePath))
    {
        auto existingMeshes = getMeshes(registry, absolutePath);

        for(auto mesh : existingMeshes)
        {
            model.meshes.push_back(mesh);
        }
    }
    // If the model is not loaded, load it
    else
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(absolutePath, aiProcess_Triangulate | aiProcess_FlipUVs);

        if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            //std::cerr << "ERROR::ASSIMP::" << importer.GetErrorString() << std::endl;
            return Model();
        }

        // Load the model
        processNode(model, registry, scene, scene->mRootNode, absolutePath);
    }

    return model;
}


void processNode(Model& model,entt::registry& registry, const aiScene* scene, aiNode* node, const std::string& absolutePath)
{

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
        model.meshes.push_back(processMesh(registry, mesh, scene, absolutePath));
    }


    // we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(model, registry, scene, node->mChildren[i], absolutePath);
    }

}

entt::entity processMesh(entt::registry& registry, const aiMesh* assimpMesh, const aiScene* scene, const std::string& absolutePath)
{
    entt::entity importedMeshEntity = registry.create();
    Mesh& meshData = registry.emplace<Mesh>(importedMeshEntity);

    meshData.vertices = getVertexData(assimpMesh, scene);
    meshData.indices = getIndexData(assimpMesh);
    meshData.path = absolutePath;
    
    return importedMeshEntity;
}

std::vector<Vertex> getVertexData(const aiMesh* mesh, const aiScene* scene)
{
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

bool isMeshLoaded(entt::registry& registry, const std::string& path)
{
    auto view = registry.view<Mesh>();

    for(auto entity : view)
    {
        if(registry.get<Mesh>(entity).path == path)
        {
            return true;
        }
    }

    return false;
}

std::vector<entt::entity> getMeshes(entt::registry& registry, const std::string& path)
{
    auto view = registry.view<Mesh>();

    std::vector<entt::entity> meshes;

    for(auto entity : view)
    {
        if(registry.get<Mesh>(entity).path == path)
        {
            meshes.push_back(entity);
        }
    }
    return meshes;
}

std::vector<Mesh> getMeshData(entt::registry& registry, const Model& model)
{
    std::vector<Mesh> meshes;

    for(auto mesh : model.meshes)
    {
        meshes.push_back(registry.get<Mesh>(mesh));
    }

    return meshes;
}