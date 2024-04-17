#pragma once

#include <entt/entt.hpp>

#include "resources/memory.hpp"
#include "rendering/resources/model.hpp"
#include "rendering/descriptors/descriptorBuilder.hpp"

#include <vector>
#include <memory>

class rendering_system;

enum class bufferType
{
    MODEL_MATRICES
};

class frame_manager
{
public:

    frame_manager(rendering_system* core);

    void initDescriptorBuilder();

    void createDescriptorSets();

    void allocateUniformBuffers(bufferType type, uint32_t count = 1);
    void updateUniformBuffers(uint32_t currentImage);

    memoryBuffers& getModelMatrices();
    VkDescriptorSet& getDescriptorSet(uint32_t index);

    void cleanup();

private:
    void updateModelMatrices(uint32_t currentImage);
    void updateMVPMatrix(uint32_t currentImage);

    memoryBuffers _modelMatrices;
    VkDescriptorBufferInfo _modelMatricesDescriptorInfo;

    std::unique_ptr<DescriptorLayoutCache> _descriptorLayoutCache;
    std::unique_ptr<DescriptorAllocator> _descriptorAllocator;
    std::vector<VkDescriptorSet> _descriptorSets;           // descriptor sets, one per frame in flight

    rendering_system* _core;
};  