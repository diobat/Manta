#include "rendering/frameManager.hpp"

#include "ECS/components/camera.hpp"
#include "ECS/components/spatial.hpp"
#include "rendering/rendering.hpp"
#include "rendering/resources/memory.hpp"
#include "core/settings.hpp"
#include <boost/uuid/uuid_generators.hpp> // UUID's for descriptor sets


frame_manager::frame_manager(rendering_system* core) : 
    _core(core)
{
    ;
}

void frame_manager::initDescriptorBuilder()
{
    _descriptorLayoutCache = std::make_unique<DescriptorLayoutCache>(_core->getLogicalDevice());
    _descriptorAllocator = std::make_unique<DescriptorAllocator>(_core->getLogicalDevice());
    _descriptorBuilder = std::make_unique<DescriptorBuilder>();
}

boost::uuids::uuid frame_manager::compileDescriptorSet(std::vector<descriptorSetBindings>& singleFrameBindings)
{
    boost::uuids::uuid id = boost::uuids::random_generator()();

    descriptorSets dSets;

    for(auto& descriptorSetBindings : singleFrameBindings)
    {
        DescriptorBuilder builder = DescriptorBuilder::begin(_descriptorLayoutCache.get(), _descriptorAllocator.get());
        for(auto& binding : descriptorSetBindings)
        {   
            switch(binding.descriptorType)
            {
                case VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER:
                    builder.bindBuffer(binding.binding, (VkDescriptorBufferInfo*)binding.data, binding.descriptorType, binding.stageFlags);
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE:
                    if (binding.count == 1)
                    {
                        builder.bindImage(binding.binding, (VkDescriptorImageInfo*)binding.data, binding.descriptorType, binding.stageFlags);
                    }
                    else
                    {
                        builder.bindImageArray(binding.binding, (std::vector<VkDescriptorImageInfo>*)binding.data, binding.count, binding.descriptorType, binding.stageFlags);
                    }
                    break;
                case VK_DESCRIPTOR_TYPE_SAMPLER:
                    builder.bindImageSampler(binding.binding, (VkDescriptorImageInfo*)binding.data, binding.stageFlags);
                    break;
                default:
                    throw std::runtime_error("Invalid descriptor type");
            }
        }
        VkDescriptorSet descriptorSet;
        builder.build(descriptorSet);
        dSets.push_back(descriptorSet);
    }

    _descriptorSets[id] = dSets;

    return id;
}

descriptorSets& frame_manager::getDescriptorSet(boost::uuids::uuid id)
{
    return _descriptorSets[id];
}

void frame_manager::allocateUniformBuffers(uint32_t count)
{
    uint32_t framesInFlight = getSettingsData(_core->getRegistry()).framesInFlight;

    auto& dsBuffer = _bufferDescriptorSets.buffer.buffers;
    dsBuffer.resize(framesInFlight);
    dsBuffer = _core->getMemorySystem().createUniformBuffers(sizeof(glm::mat4) * count, framesInFlight);
}

void frame_manager::updateUniformBuffers(uint32_t currentImage)
{
    updateModelMatrices(currentImage);
    updateMVPMatrix(currentImage);
}

memoryBuffers& frame_manager::getMemoryBuffer(descriptorSetType type)
{
    switch(type)
    {
        case descriptorSetType::MVP_MATRICES:
            _bufferDescriptorSets.type = descriptorSetType::MVP_MATRICES;
            _bufferDescriptorSets.descriptorSets.resize(getSettingsData(_core->getScene()->getRegistry()).framesInFlight);
            return _bufferDescriptorSets.buffer;
            break;
        default:
            throw std::runtime_error("Invalid descriptor set type");
            break;
    }
}

VkDescriptorSet& frame_manager::getDescriptorSet(descriptorSetType type,  uint32_t index)
{
    switch (type)
    {
        case descriptorSetType::MVP_MATRICES:
            return _bufferDescriptorSets.descriptorSets[index];
            break;
        default:
            throw std::runtime_error("Invalid descriptor set type");
            break;
    }
}

DescriptorBuilder frame_manager::getReadyDescriptorBuilder()
{
    return _descriptorBuilder->begin(_descriptorLayoutCache.get(), _descriptorAllocator.get());
}

void frame_manager::cleanup()
{
    // Free the uniform buffers
    _core->getMemorySystem().freeBuffer(_bufferDescriptorSets.buffer);

    // Free the descriptor sets allocator and layout cache
    if(_descriptorLayoutCache)
    {
        _descriptorLayoutCache->cleanup();
    }

    if(_descriptorAllocator)
    {
        _descriptorAllocator->cleanup();
    }
}

void frame_manager::updateModelMatrices(uint32_t currentImage)
{
   // Update the model matrices
    std::vector<glm::mat4> modelMatrices;

    auto& registry = _core->getRegistry();
    auto entities = registry.view<Model>();

    for(auto& entity : entities)
    {
        glm::mat4 modelMatrix = glm::mat4(1.0f);

        position* pos = registry.try_get<position>(entity);
        if(pos != nullptr)
        {
            modelMatrix = glm::translate(modelMatrix, pos->value);
        }

        rotation* rot = registry.try_get<rotation>(entity);
        if(rot != nullptr)
        {
            modelMatrix = modelMatrix * glm::mat4_cast(rot->value);
        }

        modelMatrices.push_back(modelMatrix);
    }

    memcpy(_bufferDescriptorSets.buffer.buffers[currentImage].mappedTo, modelMatrices.data(), modelMatrices.size() * sizeof(glm::mat4));

}

void frame_manager::updateMVPMatrix(uint32_t currentImage)
{
    MVPMatrix ubo{};

    entt::entity activeCamera = _core->getScene()->getActiveCamera();

    const MVPMatrix& mvp = recalculateMVP(_core->getScene()->getRegistry(), activeCamera);

    ubo.view = mvp.view;
    ubo.model = mvp.model;
    ubo.projection = mvp.projection;
    ubo.projection[1][1] *= -1;

    auto& cameraUBOs = _core->getScene()->getRegistry().get<memoryBuffers>(activeCamera);

    memcpy(cameraUBOs.buffers[currentImage].mappedTo, &ubo, sizeof(ubo));
}