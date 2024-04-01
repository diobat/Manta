#pragma once 

// GLFW
#include "wrapper/glfw.hpp"

#include "descriptors/layoutCache.hpp"
#include "descriptors/descriptorAllocator.hpp"


class DescriptorBuilder
{
public:
    DescriptorBuilder(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator);
	//static DescriptorBuilder begin(DescriptorLayoutCache* layoutCache, DescriptorAllocator* allocator );

	void bindBuffer(uint32_t binding, VkDescriptorBufferInfo* bufferInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);
	void bindImage(uint32_t binding, VkDescriptorImageInfo* imageInfo, VkDescriptorType type, VkShaderStageFlags stageFlags);

	bool build(VkDescriptorSet& set, VkDescriptorSetLayout& layout);
	bool build(VkDescriptorSet& set);
private:

	std::vector<VkWriteDescriptorSet> writes;
	std::vector<VkDescriptorSetLayoutBinding> bindings;

	DescriptorLayoutCache* cache;
	DescriptorAllocator* alloc;
};