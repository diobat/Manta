#pragma once

#include <entt/entt.hpp>

#include "resources/memory.hpp"
#include "rendering/resources/model.hpp"

class rendering_system;

enum class bufferType
{
    MODEL_MATRICES
};

class frame_manager
{
public:

    frame_manager(rendering_system* core);

    void allocateUniformBuffers(bufferType type, uint32_t count = 1);
    memoryBuffer& updateUniformBuffers(uint32_t currentImage);

    memoryBuffers& getModelMatrices();

    void cleanup();

private:

    memoryBuffers _modelMatrices;
    VkDescriptorBufferInfo _modelMatricesDescriptorInfo;
    
    rendering_system* _core;
};