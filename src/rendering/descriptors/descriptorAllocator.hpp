#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include <vector>

class DescriptorAllocator
{
public: 

    DescriptorAllocator(VkDevice device);

    struct PoolSizes
    {
        const std::vector<std::pair<VkDescriptorType, float>> sizes = 
        {
            { VK_DESCRIPTOR_TYPE_SAMPLER, 0.5f },
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 4.f },
            { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 4.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2.f },
            { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1.f },
            { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1.f },
            { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 0.5f }
        };
    };

    void resetPools();
    bool allocate (VkDescriptorSet* set, VkDescriptorSetLayout layout);

    void cleanup();

    VkDevice& getDevice() { return _device; }
private:

    VkDescriptorPool grabPool();

    VkDescriptorPool currentPool{VK_NULL_HANDLE};

    VkDevice _device;
    PoolSizes _descriptorSizes;
    std::vector<VkDescriptorPool> _usedPools;
    std::vector<VkDescriptorPool> _freePools;

};