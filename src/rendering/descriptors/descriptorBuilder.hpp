#pragma once 

// GLFW
#include "wrapper/glfw.hpp"

#include "rendering/descriptors/layoutCache.hpp"
#include "rendering/descriptors/descriptorAllocator.hpp"


class DescriptorBuilder
{
public:
    static DescriptorBuilder begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator );

	DescriptorBuilder& bindBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
	DescriptorBuilder& bindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

	bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
	bool build(VkDescriptorSet& set);
private:

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	DescriptorLayoutCache* cache;
	DescriptorAllocator* alloc;
};