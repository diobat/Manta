#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <array>

struct Model;
struct Mesh;

namespace shapes
{
    namespace quad
    {
        unsigned int VAO();
    }

    namespace cube
    {
        Model model(const glm::vec3& pos = glm::vec3(0.0f), const glm::vec3& color = {1.0f, 1.0f, 1.0f});
        Mesh mesh(const glm::vec3& color = {1.0f, 1.0f, 1.0f});
    }

    namespace sphere
    {
        unsigned int VAO();
        unsigned int vertexCount();
        unsigned int indexCount();
    }
}