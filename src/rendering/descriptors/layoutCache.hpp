#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include "rendering/descriptors/descriptorParams.hpp"

#include <unordered_map>

class DescriptorLayoutCache
{
public:

    DescriptorLayoutCache(VkDevice device);
    void cleanup();

    VkDescriptorSetLayout createDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info);

    struct DescriptorLayoutInfo {
        //good idea to turn this into a inlined array
        std::vector<VkDescriptorSetLayoutBinding> bindings;

        bool operator==(const DescriptorLayoutInfo& other) const;

        size_t hash() const;
    };

private:
    struct DescriptorLayoutHash		{

        std::size_t operator()(const DescriptorLayoutInfo& k) const{
            return k.hash();
        }
    };

    std::unordered_map<DescriptorLayoutInfo, VkDescriptorSetLayout, DescriptorLayoutHash> layoutCache;
    VkDevice _device;
};