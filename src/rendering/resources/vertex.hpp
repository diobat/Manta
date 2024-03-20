#pragma once 

#include "wrapper/glm.hpp"
#include <glm/gtx/hash.hpp>

#include <array>

struct VkVertexInputBindingDescription;
struct VkVertexInputAttributeDescription;

struct Vertex{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoords;
    glm::vec3 Color;
    glm::vec3 Tangent;

    static VkVertexInputBindingDescription getBindingDescription();
    
    static std::array<VkVertexInputAttributeDescription, 5> getAttributeDescriptions();

    bool operator==(const Vertex& other) const;
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.Position) ^
                   (hash<glm::vec3>()(vertex.Color) << 1)) >> 1 ^
                   (hash<glm::vec2>()(vertex.TexCoords) << 1) ^
                   (hash<glm::vec3>()(vertex.Normal) << 1) ^
                   (hash<glm::vec3>()(vertex.Tangent) << 1));
        }
    };
}