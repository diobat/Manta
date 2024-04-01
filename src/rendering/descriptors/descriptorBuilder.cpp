#include "rendering/descriptors/descriptorBuilder.hpp"


DescriptorBuilder::DescriptorBuilder(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator) :
    cache(layoutCache),
    alloc(allocator)
{
    ;
}

void DescriptorBuilder::bindBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
    // Create a desciptor set layout binding
    VkDescriptorSetLayoutBinding bind = {};

    bind.descriptorCount = 1;
    bind.descriptorType = type;
    bind.pImmutableSamplers = nullptr;
    bind.stageFlags = stageFlags;
    bind.binding = binding;

    bindings.push_back(bind);

    // Create the write descriptor set

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;

    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pBufferInfo = bufferInfo;
    write.dstBinding = binding;

    writes.push_back(write);
}

void DescriptorBuilder::bindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags)
{
    // Create a desciptor set layout binding
    VkDescriptorSetLayoutBinding bind = {};

    bind.descriptorCount = 1;
    bind.descriptorType = type;
    bind.pImmutableSamplers = nullptr;
    bind.stageFlags = stageFlags;
    bind.binding = binding;

    bindings.push_back(bind);

    // Create the write descriptor set

    VkWriteDescriptorSet write = {};
    write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    write.pNext = nullptr;

    write.descriptorCount = 1;
    write.descriptorType = type;
    write.pImageInfo = imageInfo;
    write.dstBinding = binding;

    writes.push_back(write);
}

bool DescriptorBuilder::build(VkDescriptorSet& set, VkDescriptorSetLayout& layout)
{
    // Create the descriptor set layout
    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.pNext = nullptr;

    layoutInfo.pBindings = bindings.data();
    layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());

    layout = cache->createDescriptorLayout(&layoutInfo);

    // Allocate the descriptor set
    bool success = alloc->allocate(&set, layout);
    if(!success)
    {
        return false;
    }

    // write descriptor set
    for (VkWriteDescriptorSet& write : writes)
    {
        write.dstSet = set;
    }

    vkUpdateDescriptorSets(alloc->getDevice(), static_cast<uint32_t>(writes.size()), writes.data(), 0, nullptr);


    return true;
}