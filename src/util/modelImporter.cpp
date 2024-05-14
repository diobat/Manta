#include "util/modelImporter.hpp"
#include "rendering/modelLibrary.hpp"
#include "rendering/rendering.hpp"
#include "util/imageData.hpp"

// Assimp includes
#include <assimp/postprocess.h>

////////////////// Importing from a model file //////////////////

ModelImporter::ModelImporter(model_mesh_library* core) :
    _meshLibrary(core)
{
    ;
}

Model ModelImporter::importFromFile(const std::string& absolutePath)
{
    // If the model is not loaded, load it
    const aiScene* scene = importer.ReadFile(absolutePath, aiProcess_Triangulate | aiProcess_FlipUVs);

    if(!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
    {
        std::runtime_error("ERROR::ASSIMP::" + std::string(importer.GetErrorString()));
    }

    // Load the model
    processNode(scene, scene->mRootNode, absolutePath);

    return Model{absolutePath, _meshLibrary->getMeshes(absolutePath)};
}

void ModelImporter::processNode(const aiScene* scene, aiNode* node, const std::string& absolutePath)
{
    // All existing meshes
    auto& allMeshes = _meshLibrary->_meshes;

    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

        Mesh importedMesh = processMesh(mesh, scene, absolutePath);

        if(allMeshes.find(absolutePath) == allMeshes.end())
        {
            allMeshes[absolutePath] = std::make_shared<std::vector<Mesh>>();
        }

        allMeshes[absolutePath]->push_back(importedMesh);
    }

    // we then recursively process each of the children nodes
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(scene, node->mChildren[i], absolutePath);
    }
}

Mesh ModelImporter::processMesh(const aiMesh* assimpMesh, const aiScene* scene, const std::string& absolutePath)
{
    Mesh importedMesh;

    importedMesh.vertexData = getVertexData(assimpMesh, scene);
    importedMesh.vertexBuffer = _meshLibrary->_core->getMemorySystem().createVertexBuffer(importedMesh.vertexData);
    importedMesh.indexData = getIndexData(assimpMesh);
    importedMesh.indexBuffer = _meshLibrary->_core->getMemorySystem().createIndexBuffer(importedMesh.indexData);
    importedMesh.textureIndices = getTextureData(scene, assimpMesh);

    importedMesh.path = absolutePath;
    
    return importedMesh;
}

std::vector<Vertex> ModelImporter::getVertexData(const aiMesh* mesh, const aiScene* scene) {
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

std::vector<unsigned int> ModelImporter::getIndexData(const aiMesh* mesh)
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
  
std::array<unsigned int, static_cast<size_t>(E_TextureType::SIZE)> ModelImporter::getTextureData(const aiScene* scene, const aiMesh* mesh)
{
    // Fill the initial answer with zeros as that is the index of the placeholder texture
    std::array<unsigned int, static_cast<size_t>(E_TextureType::SIZE)> textureIndices = {0}; 

    if(mesh->mMaterialIndex >= 0)
    {
        aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

        textureIndices[static_cast<size_t>(E_TextureType::DIFFUSE)] = loadMaterialTextures(scene, material, aiTextureType_DIFFUSE);

        textureIndices[static_cast<size_t>(E_TextureType::SPECULAR)] = loadMaterialTextures(scene, material, aiTextureType_SPECULAR);

        textureIndices[static_cast<size_t>(E_TextureType::NORMAL)] = loadMaterialTextures(scene, material, aiTextureType_NORMALS);

        textureIndices[static_cast<size_t>(E_TextureType::HEIGHT)] = loadMaterialTextures(scene, material, aiTextureType_UNKNOWN);        
    }

    return textureIndices;
}

unsigned int ModelImporter::loadMaterialTextures(const aiScene* scene, aiMaterial* mat, aiTextureType type)
{        
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);

        std::string texturePath = str.C_Str();

        // Check if the texture has already been loaded
        if(texturePath.empty())
        {
            return 0;
        }
        
        // Get the aiTexture
        const aiTexture* texture = scene->GetEmbeddedTexture(str.C_Str());

        // Load the texture
        loadedImageData img = ASSIMP_load_image(texture);

        // Create the texture, and return the index
        return _meshLibrary->_core->getTextureSystem().createTexture(img, VK_FORMAT_R8G8B8A8_SRGB, _textureTypeMap[type]).id;
    }
    return -1;
}


////////////////// Importing from vertex data //////////////////

Model ModelImporter::importFromMeshData(const std::string& name, const Mesh& meshData, glm::mat4 modelMatrix)
{
    Mesh importedMesh;

    importedMesh.vertexData = meshData.vertexData;
    importedMesh.vertexBuffer = _meshLibrary->_core->getMemorySystem().createVertexBuffer(importedMesh.vertexData);
    importedMesh.indexData = meshData.indexData;
    importedMesh.indexBuffer = _meshLibrary->_core->getMemorySystem().createIndexBuffer(importedMesh.indexData);
    importedMesh.textureIndices = meshData.textureIndices;
    importedMesh.path = name;

    if(_meshLibrary->_meshes.find(name) == _meshLibrary->_meshes.end())
    {
        _meshLibrary->_meshes[name] = std::make_shared<std::vector<Mesh>>();
    }

    _meshLibrary->_meshes[name]->push_back(importedMesh);

    return Model{name, _meshLibrary->getMeshes(name), name, modelMatrix};
}