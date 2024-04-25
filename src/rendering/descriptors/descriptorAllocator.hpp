#pragma once

// GLFW
#include "wrapper/glfw.hpp"

#include "rendering/descriptors/descriptorParams.hpp"

#include <vector>
#include <unordered_map>

enum class poolType : uint8_t
{
    POOL_TYPE_BASIC,
    POOL_TYPE_BINDLESS,     // Currently unused
    POOL_TYPE_UNUSED        // This one must always be the last entry, see DescriptorAllocator::resetCurrentPools()
};

class DescriptorAllocator
{
public: 

    DescriptorAllocator(VkDevice device);

    struct PoolSizes
    {
        const std::vector<std::pair<VkDescriptorType, float>> basicSizes = 
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

        const std::vector<std::pair<VkDescriptorType, float>> bindLessSizes = 
        {
            { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10.0f},
        };

    };

    void resetPools();
    bool allocate (VkDescriptorSet* set, VkDescriptorSetLayout layout, poolType type = poolType::POOL_TYPE_BASIC);

    void cleanup();

    VkDevice& getDevice() { return _device; }
private:

    VkDescriptorPool grabPool(poolType type = poolType::POOL_TYPE_BASIC);

    VkDescriptorPool createPool(poolType type, VkDevice device, int count, VkDescriptorPoolCreateFlags flags);
    std::vector<VkDescriptorPoolSize> getPoolSizes(poolType type);


    std::unordered_map<poolType , VkDescriptorPool> _currentPool;
    void resetCurrentPools();

    VkDevice _device;
    // PoolSizes _descriptorSizes;
    std::unordered_map<poolType, std::vector<std::pair<VkDescriptorType, float>>> _descriptorSizes;

    std::unordered_map<poolType, std::vector<VkDescriptorPool>> _usedPools;
    std::unordered_map<poolType, std::vector<VkDescriptorPool>> _freePools;

};