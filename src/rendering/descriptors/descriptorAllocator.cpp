#include "rendering/descriptors/descriptorAllocator.hpp"

#include <stdexcept>


DescriptorAllocator::DescriptorAllocator(VkDevice device)  :
    _device(device)
{
    resetCurrentPools();

    _descriptorSizes[poolType::POOL_TYPE_BASIC] = PoolSizes().basicSizes;
    _descriptorSizes[poolType::POOL_TYPE_BINDLESS] = PoolSizes().bindLessSizes;
}

void DescriptorAllocator::cleanup()
{
    // delete every pool held
    for (auto pools : _usedPools)
    {   
        for(auto pool : pools.second)
        {
            vkDestroyDescriptorPool(_device, pool, nullptr);
        }
    }

    for (auto pools : _freePools)
    {
        for(auto pool : pools.second)
        {
            vkDestroyDescriptorPool(_device, pool, nullptr);
        }
    }
}

VkDescriptorPool DescriptorAllocator::grabPool(poolType type)
{
    // Check if reusable pools are available
    if (_freePools.size() > 0)
    {
        VkDescriptorPool pool = _freePools[type].back();
        _freePools[type].pop_back();
        return pool;
    }
    else
    {
        // Create a new pool
        VkDescriptorPoolCreateFlags flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

        if (type == poolType::POOL_TYPE_BINDLESS)
        {
            flags |= VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT_EXT;
        }

        return createPool(type, _device, kMaxDescriptorSets, flags);
    }
}

VkDescriptorPool DescriptorAllocator::createPool(poolType type, VkDevice device, int count, VkDescriptorPoolCreateFlags flags)
{
    std::vector<VkDescriptorPoolSize> sizes = getPoolSizes(type);

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

std::vector<VkDescriptorPoolSize> DescriptorAllocator::getPoolSizes(poolType type)
{
    std::vector<VkDescriptorPoolSize> sizes;
    sizes.reserve(_descriptorSizes[type].size());

    for (auto& size : _descriptorSizes[type])
    {
        sizes.push_back({size.first, static_cast<uint32_t>(size.second * kMaxDescriptorSets)});
    }

    return sizes;
}


bool DescriptorAllocator::allocate(VkDescriptorSet* set, VkDescriptorSetLayout layout, poolType type)
{
    if (_currentPool[type] == VK_NULL_HANDLE)
    {
        _currentPool[type] = grabPool(type);
        _usedPools[type].push_back(_currentPool[type]);
    }

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;

    allocInfo.pSetLayouts = &layout;
    allocInfo.descriptorPool = _currentPool[type];
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
        // allocate a new pool and try again'
        _currentPool[type] = grabPool();
        _usedPools[type].push_back(_currentPool[type]);

        allocInfo.descriptorPool = _currentPool[type];
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
    for (auto pools : _usedPools)
    {
        for(auto pool : pools.second)
        {
            vkResetDescriptorPool(_device, pool, 0);
            _freePools[pools.first].push_back(pool);
        }
    }

    // Clear used pools
    _usedPools.clear();

    // Reset current pool
    resetCurrentPools();
}

void DescriptorAllocator::resetCurrentPools()
{
    // This resets the current pools to VK_NULL_HANDLE, POOL_TYPE_UNUSED must always be the last entry in the enum for this to work
    for (int i{0}; i < static_cast<int>(poolType::POOL_TYPE_UNUSED) ; ++i )
    {
        _currentPool[static_cast<poolType>(i)] = VK_NULL_HANDLE;
    }
}