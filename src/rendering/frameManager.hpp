#pragma once

#include <entt/entt.hpp>

#include "resources/memory.hpp"
#include "rendering/resources/model.hpp"
#include "rendering/resources/texture.hpp"
#include "rendering/descriptors/descriptorBuilder.hpp"

#include <vector>
#include <memory>

class rendering_system;



enum class descriptorSetType : unsigned int
{
    MVP_MATRICES,
    SIZE                // used to get the size of the enum, not a valid type, must be last
};

struct bufferDescriptorSetData
{
    VkDescriptorBufferInfo bufferInfo;
    descriptorSetType type;
    
    std::vector<VkDescriptorSet> descriptorSets;       // descriptor sets, one per frame in flight
    memoryBuffers buffer;
};


class frame_manager
{
public:

    frame_manager(rendering_system* core);

    void initDescriptorBuilder();

    void createDescriptorSets();

    void allocateUniformBuffers(uint32_t count = 1);
    void updateUniformBuffers(uint32_t currentImage);

    memoryBuffers& getMemoryBuffer(descriptorSetType type);
    // memoryBuffers& getModelMatrices();
    VkDescriptorSet& getDescriptorSet(descriptorSetType type, uint32_t index);

    void cleanup();

private:
    
    void updateModelMatrices(uint32_t currentImage);
    void updateMVPMatrix(uint32_t currentImage);

    // Buffer descriptor sets
    bufferDescriptorSetData _bufferDescriptorSets;
    bufferDescriptorSetData& getBufferDescriptorSetData(descriptorSetType type);

    // Image descriptor sets
    std::vector<VkDescriptorImageInfo> _textureArrayDescriptorSets;

    std::unique_ptr<DescriptorLayoutCache> _descriptorLayoutCache;
    std::unique_ptr<DescriptorAllocator> _descriptorAllocator;
    rendering_system* _core;
};  