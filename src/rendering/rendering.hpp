#pragma once

#include "wrapper/glfw.hpp"
#include "wrapper/glm.hpp"

#include <memory>
#include <iostream>

#include "rendering/resources/vertex.hpp"
#include "ECS/ECS.hpp"


#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger);

void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator);

static std::vector<char> readFile(const std::string& filename);

struct QueueFamilyIndices;

struct SwapChainSupportDetails;

struct UniformBufferObject;

class rendering_system {
public:
    // Blocking function to run renderer
    void run();
    // Setters
    void setScene(std::shared_ptr<Scene> scene);
private:
    // Window initialization
    void initWindow();
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
                auto app = reinterpret_cast<rendering_system*>(glfwGetWindowUserPointer(window));
                app->_framebufferResized = true;}

    // Vulkan initialization 
    void initVulkan();
        // Instance creation
        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        // Rendering setup
        void createSwapChain();
        void createImageViews();
        void createRenderPass();
            VkFormat findDepthFormat();
                VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        void createDescriptorSetLayout();
        void createGraphicsPipeline();
        void createCommandPool();
        void createDepthResources();
        void createFramebuffers();
        // Resource setup
        void createTextureImage();
            void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout, uint32_t mipLevels);
                    bool hasStencilComponent(VkFormat format);
        void createTextureImageView();
            VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags, uint32_t mipLevels);
        void createTextureSampler();
        void loadModel();
        void createVertexBuffer();
        void createIndexBuffer();
        void createUniformBuffers();
        void createDescriptorPool();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();

    // Where the magic happens
    void mainLoop();

    // Release resources
    void cleanup();


    void copyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void createImage(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void generateMipMaps(VkImage image, VkFormat imageFormat, int32_t texWidth, int32_t texHeight, uint32_t mipLevels);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);
    void recreateSwapChain();
    void cleanupSwapChain();
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void drawFrame();
    void updateUniformBuffer(uint32_t currentImage);
    bool isDeviceSuitable(VkPhysicalDevice device);
    bool checkDeviceExtensionSupport(VkPhysicalDevice device);
    std::vector<const char*> getRequiredExtensions();
    bool checkValidationLayerSupport();
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR> availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    };
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    VkShaderModule createShaderModule(const std::vector<char>& code);
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);


    std::shared_ptr<Scene> _scene;                          // scene

    GLFWwindow* _window;                                    // glfw window
    VkInstance _instance;                                   // vulkan instance
    VkDebugUtilsMessengerEXT _debugMessenger;               // debug messenger
    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;      // physical device
    VkDevice _device;                                       // logical device
    VkQueue _graphicsQueue;                                 // graphics queue
    VkQueue _transferQueue;                                 // transfer queue
    VkQueue _presentationQueue;                             // presentation queue
    VkSurfaceKHR _surface;                                  // surface
    VkSwapchainKHR _swapChain;                              // swap chain
    std::vector<VkImage> _swapChainImages;                  // swap chain images
    VkFormat _swapChainImageFormat;                         // swap chain image format
    VkExtent2D _swapChainExtent;                            // swap chain extent
    std::vector<VkImageView> _swapChainImageViews;          // swap chain image views
    VkRenderPass _renderPass;                               // render pass
    VkDescriptorSetLayout _descriptorSetLayout;             // descriptor set layout
    VkPipelineLayout _pipelineLayout;                       // pipeline layout
    VkPipeline _graphicsPipeline;                           // graphics pipeline
    std::vector<VkFramebuffer> _swapChainFramebuffers;      // swap chain framebuffers
    VkCommandPool _commandPool;                             // command pool
    VkCommandPool _transferCommandPool;                     // transfer command pool
    std::vector<VkCommandBuffer> _commandBuffers;           // command buffer

    std::vector<VkSemaphore> _imageAvailableSemaphores;     // image available semaphore
    std::vector<VkSemaphore> _renderFinishedSemaphores;     // render finished semaphore
    std::vector<VkFence> _inFlightFences;                   // in flight fence
    uint32_t _currentFrame = 0;                             // current frame

    bool _framebufferResized = false;                       // framebuffer resized flag

    std::vector<Vertex> _vertices;                          // vertices
    std::vector<uint32_t> _indices;                         // indices
    VkBuffer _vertexBuffer;                                 // vertex buffer
    VkDeviceMemory _vertexBufferMemory;                     // vertex buffer memory
    VkBuffer _indexBuffer;                                  // index buffer
    VkDeviceMemory _indexBufferMemory;                      // index buffer memory

    std::vector<VkBuffer> _uniformBuffers;                  // uniform buffers
    std::vector<VkDeviceMemory> _uniformBuffersMemory;      // uniform buffers memory
    std::vector<void*> _uniformBuffersMapped;               // uniform buffers mapped

    VkDescriptorPool _descriptorPool;                       // descriptor pool
    std::vector<VkDescriptorSet> _descriptorSets;           // descriptor sets

    uint32_t _mipLevels = 1;                                // mip levels
    VkImage _textureImage;                                  // texture image
    VkDeviceMemory _textureImageMemory;                     // texture image memory

    VkImageView _textureImageView;                          // texture image view
    VkSampler _textureSampler;                              // texture sampler

    VkImage _depthImage;                                    // depth image
    VkDeviceMemory _depthImageMemory;                       // depth image memory
    VkImageView _depthImageView;                            // depth image view
};