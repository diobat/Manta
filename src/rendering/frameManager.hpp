#pragma once

#include <entt/entt.hpp>

#include "resources/memory.hpp"
#include "rendering/resources/model.hpp"
#include "rendering/resources/texture.hpp"
#include "rendering/descriptors/descriptorBuilder.hpp"

#include <vector>
#include <memory>

// Third-party headers
#include <boost/uuid/uuid.hpp>  // UUID's for descriptor sets
#include <boost/functional/hash.hpp>



class rendering_system;

enum class bindingType : unsigned int
{
    UNIFORM_BUFFER,
    TEXTURE,
    TEXTURE_ARRAY,
    SAMPLER,
    SIZE                // used to get the size of the enum, not a valid type, must be last
};

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

// Data for a single descriptor binding
struct descriptorBindingData
{
    uint32_t binding = 0;
    VkDescriptorType descriptorType = VK_DESCRIPTOR_TYPE_MAX_ENUM;
    uint32_t count = 1;
    VkShaderStageFlags stageFlags = VK_SHADER_STAGE_VERTEX_BIT || VK_SHADER_STAGE_FRAGMENT_BIT;
    const void* data = nullptr;
};

// One descriptor set is made of many descriptor bindings
using descriptorSetBindings = std::vector<descriptorBindingData>;

// One descriptor set for each frame in flight
using descriptorSets = std::vector<VkDescriptorSet>;

class frame_manager
{
public:
    frame_manager(rendering_system* core);

    void initDescriptorBuilder();

    boost::uuids::uuid compileDescriptorSet(std::vector<descriptorSetBindings>& bindings);
    descriptorSets& getDescriptorSet(boost::uuids::uuid id);

    void allocateUniformBuffers(uint32_t count = 1);
    void updateUniformBuffers(uint32_t currentImage);

    memoryBuffers& getMemoryBuffer(descriptorSetType type);
    VkDescriptorSet& getDescriptorSet(descriptorSetType type, uint32_t index);

    DescriptorBuilder getReadyDescriptorBuilder();

    std::unique_ptr<DescriptorAllocator>& getDescriptorAllocator() { return _descriptorAllocator; }

    void cleanup();

private:
    void updateModelMatrices(uint32_t currentImage);
    void updateMVPMatrix(uint32_t currentImage);

    // Buffer descriptor sets
    bufferDescriptorSetData _bufferDescriptorSets;
    bufferDescriptorSetData& getBufferDescriptorSetData(descriptorSetType type);

    // Image descriptor sets
    std::vector<VkDescriptorImageInfo> _textureDiffuseDescriptorSets;
    std::vector<VkDescriptorImageInfo> _textureCubemapDescriptorSets;

    std::unordered_map<boost::uuids::uuid, descriptorSets, boost::hash<boost::uuids::uuid> > _descriptorSets;

    std::unique_ptr<DescriptorBuilder> _descriptorBuilder;

    std::unique_ptr<DescriptorLayoutCache> _descriptorLayoutCache;
    std::unique_ptr<DescriptorAllocator> _descriptorAllocator;
    rendering_system* _core;
};  