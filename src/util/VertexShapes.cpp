#include "util/VertexShapes.hpp"

#include "rendering/resources/model.hpp"

#include <vector>

namespace
{
    // Quad
    bool quadVertices_init = false;
    unsigned int quadVAO_nr, quadVBO_nr;
    float quadVertices[] = {
        // positions        // texture Coords
        -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
        1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
    };

    // Cube
    bool cubeModel_init = false;
    Model cubeModel;
    std::vector<float> cubeVertices = {
        // positions
        -1.0f,  1.0f, -1.0f,        // 0 Front-top-left
        -1.0f, -1.0f, -1.0f,        // 1 Front-bottom-left
         1.0f, -1.0f, -1.0f,        // 2 Front-bottom-right
         1.0f,  1.0f, -1.0f,        // 3 Front-top-right

        -1.0f,  1.0f,  1.0f,        // 4 Back-top-left
        -1.0f, -1.0f,  1.0f,        // 5 Back-bottom-left
         1.0f, -1.0f,  1.0f,        // 6 Back-bottom-right
         1.0f,  1.0f,  1.0f         // 7 Back-top-right
    };

    std::vector<unsigned int> cubeIndices = {
        0, 2, 1, 2, 0, 3, // Front face
        4, 5, 6, 6, 7, 4, // Back face
        0, 7, 3, 7, 0, 4, // Top face
        1, 6, 5, 6, 1, 2, // Bottom face
        0, 5, 4, 5, 0, 1, // Left face
        3, 6, 2, 6, 3, 7  // Right face
    };

    std::vector<unsigned int> cubeIndicesFlipped = {
        0, 1, 2, 2, 3, 0, // Front face
        4, 6, 5, 6, 4, 7, // Back face
        0, 3, 7, 7, 4, 0, // Top face
        1, 5, 6, 6, 2, 1, // Bottom face
        0, 4, 5, 5, 1, 0, // Left face
        3, 2, 6, 6, 7, 3  // Right face
    };



    // Sphere
    bool sphereVertices_init = false;
    unsigned int sphereVAO_nr, sphereVBO_nr, sphereEBO_nr;
    unsigned int sphereVertexCount = 0;
    unsigned int sphereIndexCount = 0;
    // A function that returns the openGL VAO for a sphere object, it takes the radius and the number of stacks and slices as parameters.
    // The sphere is centered at the origin and drawn using triangle strips.
    // The sphere is drawn using the following parametric equation:
    // x = r * cos(theta) * sin(phi)
    // y = r * sin(theta) * sin(phi) 
    // z = r * cos(phi)
    // where theta is the angle around the y axis and phi is the angle around the x axis.
    unsigned int sphereVAO(unsigned int stacks, unsigned int slices, float radius)
    {
        // Calculate the number of vertices in the sphere
        sphereVertexCount = (stacks + 1) * (slices + 1);
        // Calculate the number of indices in the sphere
        sphereIndexCount = stacks * slices * 6;

        // Create a vector to hold the vertices
        std::vector<float> vertices(sphereVertexCount * 3);
        // Create a vector to hold the indices
        std::vector<unsigned int> indices(sphereIndexCount);

        // Calculate the vertex positions and the texture coordinates
        for (unsigned int i = 0; i <= stacks; i++)
        {
            for (unsigned int j = 0; j <= slices; j++)
            {
                float theta = (float)i / (float)stacks * 2.0f * glm::pi<float>();
                float phi = (float)j / (float)slices * glm::pi<float>();

                float x = radius * glm::cos(theta) * glm::sin(phi);
                float y = radius * glm::sin(theta) * glm::sin(phi);
                float z = radius * glm::cos(phi);

                vertices[(i * (slices + 1) + j) * 3] = x;
                vertices[(i * (slices + 1) + j) * 3 + 1] = y;
                vertices[(i * (slices + 1) + j) * 3 + 2] = z;
            }
        }

        // Calculate the indices
        for (unsigned int i = 0; i < stacks; i++)
        {
            for (unsigned int j = 0; j < slices; j++)
            {
                indices[(i * slices + j) * 6] = i * (slices + 1) + j;
                indices[(i * slices + j) * 6 + 1] = i * (slices + 1) + j + 1;
                indices[(i * slices + j) * 6 + 2] = (i + 1) * (slices + 1) + j;
                indices[(i * slices + j) * 6 + 3] = i * (slices + 1) + j + 1;
                indices[(i * slices + j) * 6 + 4] = (i + 1) * (slices + 1) + j + 1;
                indices[(i * slices + j) * 6 + 5] = (i + 1) * (slices + 1) + j;
            }
        }



        return sphereVAO_nr;
    } 

}

namespace shapes
{
    namespace quad
    {
        unsigned int VAO()
        {
            if (!quadVertices_init)
            {
                ;
            }
            return quadVAO_nr;
        }
    }

    namespace cube
    {
        Model model(const glm::vec3& pos, const glm::vec3& color)
        {
            if (!cubeModel_init)
            {
                cubeModel_init = true;

                cubeModel.path = "Cube";
                cubeModel.name = "Cube";

                Mesh cubeMesh;

                unsigned int cubeVerticesSize = cubeVertices.size();

                for (unsigned int i = 0; i < cubeVertices.size(); i += 3)
                {
                    Vertex vertex;
                    vertex.Position = glm::vec3(cubeVertices[i] , cubeVertices[i + 1], cubeVertices[i + 2]);
                    vertex.Color = glm::vec3(color[0], color[1], color[2]);
                    cubeMesh.vertexData.push_back(vertex);
                }
                cubeMesh.indexData = cubeIndices;
                cubeMesh.path = "Cube";
                cubeModel.meshes = std::make_shared<std::vector<Mesh>>();
                cubeModel.meshes->push_back(cubeMesh);
            }
            cubeModel.modelMatrix = glm::translate(glm::mat4(1.0f), pos);

            return cubeModel;
        }

        Mesh mesh(const glm::vec3& color)
        {
            Mesh cubeMesh;

            for (unsigned int i = 0; i < cubeVertices.size(); i += 3)
            {
                Vertex vertex;
                vertex.Position = glm::vec3(cubeVertices[i], cubeVertices[i + 1], cubeVertices[i + 2]);
                vertex.Color = glm::vec3(color[0], color[1], color[2]);
                cubeMesh.vertexData.push_back(vertex);
            }
            cubeMesh.indexData = cubeIndices;

            cubeMesh.path = "Cube";

            return cubeMesh;
        }

    }

    namespace cube_flipped
    {
        Mesh mesh(const glm::vec3& color)
        {
            Mesh cubeMesh;

            for (unsigned int i = 0; i < cubeVertices.size(); i += 3)
            {
                Vertex vertex;
                vertex.Position = glm::vec3(cubeVertices[i], cubeVertices[i + 1], cubeVertices[i + 2]);
                vertex.Color = glm::vec3(color[0], color[1], color[2]);
                cubeMesh.vertexData.push_back(vertex);
            }
            cubeMesh.indexData = cubeIndicesFlipped;

            cubeMesh.path = "CubeFlipped";

            return cubeMesh;
        }
    }

    namespace sphere
    {
        unsigned int VAO()
        {
            if(!sphereVertices_init)
            {
                sphereVAO(16, 8, 1.0f);
                sphereVertices_init = true;
            }
            return sphereVAO_nr;
        }

        unsigned int vertexCount()
        {
            return sphereVertexCount;
        }

        unsigned int indexCount()
        {
            return sphereIndexCount;
        }
    }
}