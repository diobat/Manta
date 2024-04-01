#include "rendering/descriptors/layoutCache.hpp"

#include <algorithm>
#include <stdexcept>

DescriptorLayoutCache::DescriptorLayoutCache(VkDevice device) :
    _device(device)
{
    ;
}   

void DescriptorLayoutCache::cleanup()
{
    for (auto& layout : layoutCache)
    {
        vkDestroyDescriptorSetLayout(_device, layout.second, nullptr);
    }
}

VkDescriptorSetLayout DescriptorLayoutCache::createDescriptorLayout(VkDescriptorSetLayoutCreateInfo* info)
{
    DescriptorLayoutInfo layoutInfo;
    layoutInfo.bindings.reserve(info->bindingCount);
    bool isSorted = true;
    int lastBinding = -1;

    // Copy the bindings from the info to the layoutInfo
    for (uint32_t i = 0; i < info->bindingCount; i++)
    {
        layoutInfo.bindings.push_back(info->pBindings[i]);
        
        // Check if the bindings are sorted
        if(info->pBindings[i].binding > lastBinding)
        {
            lastBinding = info->pBindings[i].binding;
        }
        else
        {
            isSorted = false;
        }
    }
    // Sort if they arent
    if(!isSorted)
    {
        std::sort(layoutInfo.bindings.begin(), layoutInfo.bindings.end(), [](const VkDescriptorSetLayoutBinding& a, const VkDescriptorSetLayoutBinding& b) { return a.binding < b.binding; });
    }

    // Try to grab from cache
    auto it = layoutCache.find(layoutInfo);
    if (it != layoutCache.end())
    {
        return it->second;
    }
    else
    {
        // Not found, create new layout
        VkDescriptorSetLayout layout;
        if (vkCreateDescriptorSetLayout(_device, info, nullptr, &layout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create descriptor set layout");
        }

        // Add to cache
        layoutCache[layoutInfo] = layout;
        return layout;
    }
}

bool DescriptorLayoutCache::DescriptorLayoutInfo::operator==(const DescriptorLayoutInfo& other) const
{
    if (bindings.size() != other.bindings.size())
    {
        return false;
    }
    else 
    {
        // Compare each of the bindings is the same. Bindings are sorted
        for (size_t i = 0; i < bindings.size(); i++)
        {
            if (bindings[i].binding != other.bindings[i].binding ||
                bindings[i].descriptorCount != other.bindings[i].descriptorCount ||
                bindings[i].descriptorType != other.bindings[i].descriptorType ||
                bindings[i].stageFlags != other.bindings[i].stageFlags)
            {
                return false;
            }
        }   
        return true;
    }
}
    

size_t DescriptorLayoutCache::DescriptorLayoutInfo::hash() const
{
    using std::size_t;  

    size_t result = std::hash<size_t>()(bindings.size());

    for(const VkDescriptorSetLayoutBinding& b : bindings)
    {
        // Pack the binding into a single size_t
        size_t binding_hash = b.binding | b.descriptorType << 8 | b.descriptorCount << 16 | b.stageFlags << 24;

        result ^= std::hash<size_t>()(binding_hash);
    }
    return result;
}
