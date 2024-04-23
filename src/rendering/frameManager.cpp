#include "rendering/frameManager.hpp"

#include "ECS/components/camera.hpp"
#include "ECS/components/spatial.hpp"
#include "rendering/rendering.hpp"
#include "rendering/resources/memory.hpp"
#include "core/settings.hpp"


frame_manager::frame_manager(rendering_system* core) : 
    _core(core)
{
    ;
}

void frame_manager::initDescriptorBuilder()
{
    _descriptorLayoutCache = std::make_unique<DescriptorLayoutCache>(_core->getLogicalDevice());
    _descriptorAllocator = std::make_unique<DescriptorAllocator>(_core->getLogicalDevice());
}

void frame_manager::createDescriptorSets()
{
    unsigned int framesinFlight = getSettingsData(_core->getScene()->getRegistry()).framesInFlight;

    // MVP Matrices descriptor set

    auto& mvpDS = _bufferDescriptorSets;

    mvpDS.type = descriptorSetType::MVP_MATRICES;
    mvpDS.descriptorSets.resize(framesinFlight);

    memoryBuffers& cameraBuffers = _core->getScene()->getRegistry().get<memoryBuffers>( _core->getScene()->getActiveCamera() );

    _textureArrayDescriptorSets = _core->getTextureSystem().aggregateDescriptorImageInfos(kTextureArraySize);
    VkDescriptorImageInfo& samplerDescriptor = _core->getTextureSystem().getTextureSamplerDescriptor();

    for(size_t i = 0; i < framesinFlight; i++)
    {
	    DescriptorBuilder::begin(_descriptorLayoutCache.get(), _descriptorAllocator.get())
		.bindBuffer(0, &cameraBuffers.buffers[i].descriptorInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .bindBuffer(1, &getMemoryBuffer(descriptorSetType::MVP_MATRICES).buffers[i].descriptorInfo, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .bindImageSampler(2, &samplerDescriptor, VK_SHADER_STAGE_FRAGMENT_BIT)
		.bindImageArray(3, _textureArrayDescriptorSets, kTextureArraySize, VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, VK_SHADER_STAGE_FRAGMENT_BIT)
		.build(mvpDS.descriptorSets[i]);
    }
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
            return _bufferDescriptorSets.buffer;
            break;
    }

    std::runtime_error("Invalid descriptor set type");
}

VkDescriptorSet& frame_manager::getDescriptorSet(descriptorSetType type,  uint32_t index)
{
    switch (type)
    {
        case descriptorSetType::MVP_MATRICES:
            return _bufferDescriptorSets.descriptorSets[index];
            break;
    }

    std::runtime_error("Invalid descriptor set type");
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