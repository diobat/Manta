#include "rendering/resources/texture.hpp"

#include "rendering/rendering.hpp"
#include "util/modelImporter.hpp"

#include "util/VertexShapes.hpp"
#include "ECS/components/spatial.hpp"

texture_system::texture_system(rendering_system* rendering) :
    _core(rendering),
    _mipLevels(1)
{
    ;
}

void texture_system::init()
{
    initTextureSampler();
    _defaultTexture = createTexture("X:/Repos/Manta/res/missingTexture.png",E_TextureType::DIFFUSE , true);
    _defaultTexture = createTexture("X:/Repos/Manta/res/missingTexture.png",E_TextureType::CUBEMAP , true);
}

void texture_system::initTextureSampler()
{
    VkSamplerCreateInfo samplerInfo{};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;

    VkPhysicalDeviceProperties properties{};
    vkGetPhysicalDeviceProperties(_core->getPhysicalDevice(), &properties);

    samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.mipLodBias = 0.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = static_cast<float>(_mipLevels);

    if(vkCreateSampler(_core->getLogicalDevice(), &samplerInfo, nullptr, &_textureSampler) != VK_SUCCESS){
        throw std::runtime_error("failed to create texture sampler!");
    }

    // Populate the descriptor
    _textureSamplerDescriptor.sampler = _textureSampler;
    _textureSamplerDescriptor.imageView = VK_NULL_HANDLE;
    _textureSamplerDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

}

VkSampler& texture_system::getTextureSampler()
{
    return _textureSampler;
}

VkDescriptorImageInfo& texture_system::getTextureSamplerDescriptor()
{
    return _textureSamplerDescriptor;
}

image texture_system::createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, bool isCubeMap)
{
    image img;

    img.mipLevels = mipLevels;

    // Populate the struct
    VkImageCreateInfo imageInfo = {};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    
    imageInfo.extent.width = width;
    imageInfo.extent.height = height;
    imageInfo.extent.depth = 1;

    imageInfo.mipLevels = mipLevels;
    if(isCubeMap)
    {
        imageInfo.arrayLayers = 6;
        imageInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
    }
    else
    {
        imageInfo.arrayLayers = 1;
    }
    
    imageInfo.format = format;
    imageInfo.tiling = tiling;

    imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;    
    imageInfo.usage = usage;

    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    // Create the image
    if(vkCreateImage(_core->getLogicalDevice(), &imageInfo, nullptr, &img.image) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create image");
    }

    // Probe the memory requirements
    VkMemoryRequirements memRequirements;
    vkGetImageMemoryRequirements(_core->getLogicalDevice(), img.image, &memRequirements);

    // Allocate the memory
    VkMemoryAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = _core->getMemorySystem().findMemoryType(memRequirements.memoryTypeBits, properties);

    if(vkAllocateMemory(_core->getLogicalDevice(), &allocInfo, nullptr, &img.memory) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to allocate image memory");
    }

    // Bind the memory
    vkBindImageMemory(_core->getLogicalDevice(), img.image, img.memory, 0);

    return img;
}

// Temporary and must be removed later then texture system is fully ported
image texture_system::createTexture(const std::string& path, E_TextureType type, bool addToCache)
{
    image img;

    // check if the image ends with .hdr
    if(path.substr(path.find_last_of(".") + 1) == "hdr")
    {
        loadedImageDataHDR imgData = STB_load_image_HDR(path);
        img = createTexture(imgData, VK_FORMAT_R32G32B32A32_SFLOAT, type, addToCache);
    }
    else
    {
        loadedImageDataRGB imgData = STB_load_image(path);
        img = createTexture(imgData, VK_FORMAT_R8G8B8A8_SRGB, type, addToCache);
    }

    return img;
}

image texture_system::createTexture(const loadedImageDataRGB imgData, VkFormat format, E_TextureType type,  bool addToCache)
{
    image img;

    // Check if imgData.data is not null
    if(imgData.data == nullptr)
    {
        throw std::runtime_error("Image data is null");
    }

    // Calculate memory size of the image, 4 bytes per pixel
    VkDeviceSize imageSize = imgData.width * imgData.height * 4;

    // Calculate the number of mip levels based on the image dimensions
    img.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imgData.width, imgData.height)))) + 1;

    memoryBuffer stagingBuffer = _core->getMemorySystem().createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(_core->getLogicalDevice(), stagingBuffer.memory, 0, imageSize, 0, &data);
    memcpy(data, imgData.data, static_cast<size_t>(imageSize));
    vkUnmapMemory(_core->getLogicalDevice(), stagingBuffer.memory);

    free_image(imgData.data);

    img = createImage(imgData.width, imgData.height, img.mipLevels, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    transitionImageLayout(img.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, img.mipLevels);
    copyBufferToImage(stagingBuffer.buffer, img.image, static_cast<uint32_t>(imgData.width), static_cast<uint32_t>(imgData.height));
    generateMipMaps(img.image, format, imgData.width, imgData.height, img.mipLevels);

    _core->getMemorySystem().freeBuffer(stagingBuffer);

    // Populate the image view
    createTextureImageView(img, format);

    // Populate the descriptor image info
    img.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img.descriptor.imageView = img.imageView;
    img.descriptor.sampler = _textureSampler;

    // Add to _textures
    if(addToCache)
    {
        addTextureToCache(type, img);
    }

    return img;
}

image texture_system::createTexture(const loadedImageDataHDR imgData, VkFormat format, E_TextureType type,  bool addToCache)
{
    image img;

    // Check if imgData.data is not null
    if(imgData.data == nullptr)
    {
        throw std::runtime_error("Image data is null");
    }

    // Calculate memory size of the image, 4 bytes per pixel
    VkDeviceSize imageSize = imgData.width * imgData.height * 4 * sizeof(float);

    // Calculate the number of mip levels based on the image dimensions
    img.mipLevels = static_cast<uint32_t>(std::floor(std::log2(std::max(imgData.width, imgData.height)))) + 1;

    memoryBuffer stagingBuffer = _core->getMemorySystem().createBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(_core->getLogicalDevice(), stagingBuffer.memory, 0, imageSize, 0, &data);
    memcpy(data, imgData.data, static_cast<size_t>(imageSize));
    vkUnmapMemory(_core->getLogicalDevice(), stagingBuffer.memory);

    free_image(imgData.data);

    img = createImage(imgData.width, imgData.height, img.mipLevels, format, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    transitionImageLayout(img.image, format, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, img.mipLevels);
    copyBufferToImage(stagingBuffer.buffer, img.image, static_cast<uint32_t>(imgData.width), static_cast<uint32_t>(imgData.height));
    generateMipMaps(img.image, format, imgData.width, imgData.height, img.mipLevels);

    _core->getMemorySystem().freeBuffer(stagingBuffer);

    // Populate the image view
    createTextureImageView(img, format);

    // Populate the descriptor image info
    img.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img.descriptor.imageView = img.imageView;
    img.descriptor.sampler = _textureSampler;

    // Add to _textures
    if(addToCache)
    {
        addTextureToCache(type, img);
    }

    return img;
}

image texture_system::bakeCubemap(const std::string& filePath, bool addToCache)
{
    image img = createTexture(filePath, E_TextureType::CUBEMAP, false);
    

    return bakeCubemapFromFlat(img, addToCache);
}

image texture_system::bakeCubemapFromFlat(image flatImg, bool addToCache)
{
    // 0 - Check if the image is valid
    if(flatImg.image == VK_NULL_HANDLE)
    {
        throw std::runtime_error("Image is not valid");
    }

    // 1 - Create Cubemap image
    uint32_t size_width = 2048;
    uint32_t size_height = 2048;

    image img = createImage(size_width, size_height, 1, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, true);

    // 2 - Create Cubemap image view
    createImageView(img, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_ASPECT_COLOR_BIT, 1, VK_IMAGE_VIEW_TYPE_CUBE);

    // 3 - Populate the descriptor image info
    img.descriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    img.descriptor.imageView = img.imageView;
    img.descriptor.sampler = _textureSampler;

    // 4 - Create Cubemap framebuffer
    VkFramebuffer framebuffer;

    std::array<VkImageView, 1> attachments = {
        img.imageView
    };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = _core->getPipelineSystem().getRenderPass(E_RenderPassType::CUBE_MAP);
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = size_width;
    framebufferInfo.height = size_height;
    framebufferInfo.layers = 6;

    if(vkCreateFramebuffer(_core->getLogicalDevice(), &framebufferInfo, nullptr, &framebuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create framebuffer!");
    }

    // 5 - Render the cubemap
    shaderPipeline cubemapPipeline;
    try{
        cubemapPipeline = _core->getPipelineSystem().getPipeline("equi2cube");
    }
    catch(const std::exception& e)
    {   
        _core->getPipelineSystem().createPipeline("equi2cube", E_RenderPassType::CUBE_MAP);
        cubemapPipeline = _core->getPipelineSystem().getPipeline("equi2cube");
    }

    renderRequest requestInfo;
    requestInfo.commandBuffer = _core->getCommandBufferSystem().generateCommandBuffer();
    requestInfo.renderPass = E_RenderPassType::CUBE_MAP;
    requestInfo.framebuffer = framebuffer;
    requestInfo.extent = {size_width, size_height};
    requestInfo.pipeline = cubemapPipeline;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    vkCreateFence(_core->getLogicalDevice(), &fenceInfo, nullptr, &requestInfo.fence);

    requestInfo.viewport = {0.0f, 0.0f, static_cast<float>(size_width), static_cast<float>(size_height), 0.0f, 1.0f};
    requestInfo.scissor = {{0, 0}, {size_width, size_height}};

    VkDescriptorSet textureDescriptorSet;
    // Populate the descriptor image info
    _core->getFrameManager().getReadyDescriptorBuilder()
        .bindImage(0, &flatImg.descriptor, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .build(textureDescriptorSet);

    requestInfo.descriptorSets.push_back(textureDescriptorSet);

    // Get the cube model
    Model& cube = _core->getModelMeshLibrary().createModelFromMesh("cube", shapes::cube::mesh(glm::vec3(1.0f)));
    requestInfo.models = std::vector<Model>(6, cube);

    _core->getCommandBufferSystem().beginRecordingCommandBuffer(requestInfo.commandBuffer, requestInfo.renderPass, requestInfo.framebuffer, requestInfo.extent);

    int faces[] = {0, 1, 2, 3, 4, 5};

    for (int i = 0; i < 6; i++)
    {   
        PushConstant pcModel;
        pcModel.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pcModel.data = &faces[i];
        pcModel.size = sizeof(int);
        pcModel.offset = PUSH_CONSTANT_VERTEX_OFFSET;
        requestInfo.perModelPC.push_back(pcModel);
    }

    _core->getCommandBufferSystem().recordCommandBuffer(requestInfo);

    VkFence safeToDestroyFence;
    _core->getCommandBufferSystem().endRecordingCommandBuffer(requestInfo.commandBuffer);
    _core->getCommandBufferSystem().submitCommandBuffer(requestInfo.commandBuffer, requestInfo.fence);

    // 7 - Add to cache
    addTextureToCache(E_TextureType::CUBEMAP, img);

    // 8 - Cleanup
    vkWaitForFences(_core->getLogicalDevice(), 1, &requestInfo.fence, VK_TRUE, std::numeric_limits<uint64_t>::max());
    cleanupImage(flatImg);
    _core->getCommandBufferSystem().freeCommandBuffers(requestInfo.commandBuffer);
    vkDestroyFramebuffer(_core->getLogicalDevice(), framebuffer, nullptr);
    vkDestroyFence(_core->getLogicalDevice(), requestInfo.fence, nullptr);


    if(addToCache)
    {
        addTextureToCache(E_TextureType::CUBEMAP, img);
    }

    return img;
}

VkImageView texture_system::createImageView(image& img, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType)
{
    img.imageView = createImageView(img.image, format, aspectFlags, mipLevels, viewType);
    return img.imageView;
}

VkImageView texture_system::createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels, VkImageViewType viewType)
{
    VkImageViewCreateInfo viewInfo = {};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = viewType;
    viewInfo.format = format;
    viewInfo.subresourceRange.aspectMask = aspectFlags;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = mipLevels;
    viewInfo.subresourceRange.baseArrayLayer = 0;

    switch (viewType)
    {
    case VK_IMAGE_VIEW_TYPE_CUBE:
        viewInfo.subresourceRange.layerCount = 6;
        break;
    default:
        viewInfo.subresourceRange.layerCount = 1;
        break;
    }

    VkImageView imageView;
    if(vkCreateImageView(_core->getLogicalDevice(), &viewInfo, nullptr, &imageView) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to create texture image view!");
    }

    return imageView;
}

VkImageView texture_system::createTextureImageView(image& img, VkFormat format)
{
    return createImageView(img, format, VK_IMAGE_ASPECT_COLOR_BIT, img.mipLevels);
}

std::vector<VkDescriptorImageInfo>& texture_system::aggregateDescriptorTextureInfos(E_TextureType type, size_t returnVectorSize)
{
    if(_textures.find(type) == _textures.end())
    {
        throw std::runtime_error("Texture type not found in cache");
    }

    if(_textures.at(type)->empty())
    {
        throw std::runtime_error("Texture cache is empty");
    }

    if(_textures.at(type)->size() > returnVectorSize)
    {
        throw std::runtime_error("The number of textures in cache set exceeds the maximum number of textures allowed by argument");
    }

    std::vector<VkDescriptorImageInfo> imageInfos;

    for(auto& texture : *_textures.at(type))
    {
        imageInfos.push_back(texture.descriptor);
    }

    if (imageInfos.size() < returnVectorSize)
    {
        for(size_t i = imageInfos.size(); i < returnVectorSize; i++)
        {
            imageInfos.push_back(_defaultTexture.descriptor);
        }
    }

    _textureDescriptors[type] = imageInfos;

    return _textureDescriptors[type];
}

void texture_system::copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height)
{
    VkCommandBuffer commandBuffer = _core->getCommandBufferSystem().beginSingleTimeCommands();

    VkBufferImageCopy region = {};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;

    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 1;

    region.imageOffset = {0, 0, 0};
    region.imageExtent = {width, height, 1};

    vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    _core->getCommandBufferSystem().endSingleTimeCommands(commandBuffer);
}

void texture_system::transitionImageLayout(VkImage& image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels)
{
    VkCommandBuffer commandBuffer =  _core->getCommandBufferSystem().beginSingleTimeCommands();
    
    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.image = image;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;

    VkPipelineStageFlags sourceStage;
    VkPipelineStageFlags destinationStage;

    if(newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;

        if(hasStencilComponent(format))
        {
            barrier.subresourceRange.aspectMask |= VK_IMAGE_ASPECT_STENCIL_BIT;
        }
    }
    else
    {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

        sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        destinationStage = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    }
    else
    {
        throw std::invalid_argument("Unsupported layout transition!");
    }

    vkCmdPipelineBarrier(
        commandBuffer, 
        sourceStage, destinationStage, 
        0, 
        0, nullptr, 
        0, nullptr, 
        1, &barrier);

    _core->getCommandBufferSystem().endSingleTimeCommands(commandBuffer);
}

void texture_system::cleanupImage(image& img)
{
    vkDestroyImageView(_core->getLogicalDevice(), img.imageView, nullptr);
    vkDestroyImage(_core->getLogicalDevice(), img.image, nullptr);
    vkFreeMemory(_core->getLogicalDevice(), img.memory, nullptr);
}

void texture_system::cleanup()
{
    for(auto& texture : _textures)
    {
        for(auto& img : *texture.second)
        {
            cleanupImage(img);
        }
    }

    cleanupImage(_defaultTexture);
    vkDestroySampler(_core->getLogicalDevice(), _textureSampler, nullptr);
}

void texture_system::addTextureToCache(const E_TextureType type , image& img)
{
    if(_textures.find(type) == _textures.end())
    {
        _textures[type] = std::make_shared<std::vector<image>>();
    }
    img.id = _textures[type]->size();
    _textures[type]->push_back(img);
}

bool texture_system::hasStencilComponent(VkFormat format) const
{
    return format == VK_FORMAT_D32_SFLOAT_S8_UINT || format == VK_FORMAT_D24_UNORM_S8_UINT;
}

void texture_system::generateMipMaps(VkImage& image, VkFormat format, uint32_t width, uint32_t height, uint32_t mipLevels)
{
    // check if linear blitting is supported
    VkFormatProperties formatProperties;
    vkGetPhysicalDeviceFormatProperties(_core->getPhysicalDevice(), format, &formatProperties);

    if(!(formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT))
    {
        throw std::runtime_error("Texture image format does not support linear blitting!");
    }

    VkCommandBuffer commandBuffer = _core->getCommandBufferSystem().beginSingleTimeCommands();

    VkImageMemoryBarrier barrier = {};
    barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    barrier.image = image;
    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    int32_t mipWidth = width;
    int32_t mipHeight = height;

    for(uint32_t i = 1; i < mipLevels; i++)
    {

        // Transition the previous mip level to VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL because we will read from it
        barrier.subresourceRange.baseMipLevel = i - 1;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        VkImageBlit blit = {};
        // Define the dimensions of the source level
        blit.srcOffsets[0] = {0, 0, 0};
        blit.srcOffsets[1] = {mipWidth, mipHeight, 1};
        // Define the component and level number of the source
        blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.srcSubresource.mipLevel = i - 1;
        blit.srcSubresource.baseArrayLayer = 0;
        blit.srcSubresource.layerCount = 1;
        // Define the dimensions of the destination level
        blit.dstOffsets[0] = {0, 0, 0};
        blit.dstOffsets[1] = {mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1};
        // Define the component and level number of the destination
        blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        blit.dstSubresource.mipLevel = i;
        blit.dstSubresource.baseArrayLayer = 0;
        blit.dstSubresource.layerCount = 1;

        vkCmdBlitImage(commandBuffer, 
        image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, 
        image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 
        1, &blit, 
        VK_FILTER_LINEAR);

        // Transition the previous mip level to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL because after this moment
        // we will only need to read from it in the shader
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

        vkCmdPipelineBarrier(
            commandBuffer,
            VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );

        if(mipWidth > 1) mipWidth /= 2;
        if(mipHeight > 1) mipHeight /= 2;
    }

    // Transition the last mip level to VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer,
        VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
        0, nullptr,
        0, nullptr,
        1, &barrier
    );

    _core->getCommandBufferSystem().endSingleTimeCommands(commandBuffer);
}

