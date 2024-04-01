#include "rendering/descriptors/descriptorAllocator.hpp"

#include <stdexcept>

namespace 
{
    VkDescriptorPool createPool(VkDevice device, const DescriptorAllocator::PoolSizes& poolSizes, int count, VkDescriptorPoolCreateFlags flags)
    {
        std::vector<VkDescriptorPoolSize> sizes;
        sizes.reserve(poolSizes.sizes.size());

        for (auto& size : poolSizes.sizes)
        {
            sizes.push_back({size.first, static_cast<uint32_t>(size.second * count)});
        }

        VkDescriptorPoolCreateInfo poolInfo{};
        poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
        poolInfo.flags = flags;
        poolInfo.maxSets = count;
        poolInfo.poolSizeCount = static_cast<uint32_t>(sizes.size());
        poolInfo.pPoolSizes = sizes.data();

        VkDescriptorPool descriptorPool;
        if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &descriptorPool) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor pool");
        }

        return descriptorPool;
    }
}

DescriptorAllocator::DescriptorAllocator(VkDevice device)  :
    _device(device)
{
    ;
}

void DescriptorAllocator::cleanup()
{
    // delete every pool held
    for (auto pool : _usedPools)
    {
        vkDestroyDescriptorPool(_device, pool, nullptr);
    }

    for (auto pool : _freePools)
    {
        vkDestroyDescriptorPool(_device, pool, nullptr);
    }
}

VkDescriptorPool DescriptorAllocator::grabPool()
{
    // Check if reusable pools are available
    if (_freePools.size() > 0)
    {
        VkDescriptorPool pool = _freePools.back();
        _freePools.pop_back();
        return pool;
    }
    else
    {
        // Create a new pool
        return createPool(_device, _descriptorSizes, 1000, 0);
    }
}

bool DescriptorAllocator::allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout)
{
    if (currentPool == VK_NULL_HANDLE)
    {
        currentPool = grabPool();
        _usedPools.push_back(currentPool);
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;

    allocInfo.pSetLayouts = &layout;
    allocInfo.descriptorPool = currentPool;
    allocInfo.descriptorSetCount = 1;

    // Try to allocate the descriptor set
    VkResult allocResult = vkAllocateDescriptorSets(_device, &allocInfo, set);
    bool needReallocate = false;

    switch (allocResult)
    {
        case VK_SUCCESS:
            return true;
        case VK_ERROR_FRAGMENTED_POOL:
        case VK_ERROR_OUT_OF_POOL_MEMORY:
            needReallocate = true;
            break;
        default:
            return false;
    }

    if(needReallocate)
    {
        // allocate a new pool and try again
        currentPool = grabPool();
        _usedPools.push_back(currentPool);

        allocInfo.descriptorPool = currentPool;
        allocResult = vkAllocateDescriptorSets(_device, &allocInfo, set);

        if (allocResult == VK_SUCCESS)
        {
            return true;
        }

    }

    return false;
}

void DescriptorAllocator::resetPools()
{
    // Move all used pools to free pools
    for (auto pool : _usedPools)
    {
        vkResetDescriptorPool(_device, pool, 0);
        _freePools.push_back(pool);
    }

    // Clear used pools
    _usedPools.clear();

    // Reset current pool
    currentPool = VK_NULL_HANDLE;
}
