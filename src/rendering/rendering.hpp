#pragma once

#include "wrapper/glfw.hpp"
#include "wrapper/glm.hpp"

#include <memory>
#include <iostream>

#include "rendering/resources/vertex.hpp"
#include "ECS/ECS.hpp"
#include "rendering/resources/model.hpp"
#include "rendering/commandBufferManager.hpp"
#include "rendering/resources/memory.hpp"
#include "rendering/resources/texture.hpp"
#include "rendering/shaderManager.hpp"
#include "rendering/pipelineManager.hpp"
#include "rendering/swapChainManager.hpp"
#include "rendering/frameManager.hpp"

#include "rendering/descriptors/layoutCache.hpp"
#include "rendering/descriptors/descriptorAllocator.hpp"


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
    rendering_system(std::shared_ptr<Scene> scene);

    // Initialize the renderer
    void initRender();

    // Draw a frame
    void drawFrame();

    // Release resources
    void cleanup();

    // // Blocking function to run renderer
    // void run();
    // Setters
    void setScene(std::shared_ptr<Scene> scene);
    // Getters
    entt::registry& getRegistry();                                                  // registry getter
    GLFWwindow* getWindow() { return _window; }                                     // window getter
    VkDevice getLogicalDevice() { return _device; }                                 // logical device getter
    VkPhysicalDevice getPhysicalDevice() { return _physicalDevice; }                // physical device getter

    std::shared_ptr<Scene> getScene() { return _scene; }                            // scene getter
    model_mesh_library& getModelMeshLibrary() { return _modelLibrary; }             // model mesh library getter
    memory_system& getMemorySystem() { return _memory; }                            // memory system getter 
    command_buffer_system& getCommandBufferSystem() { return _commandBuffer; }      // command buffer system getter
    texture_system& getTextureSystem() { return _texture; }                         // texture system getter
    shader_system& getShaderSystem() { return _shaders; }                           // shader system getter
    pipeline_system& getPipelineSystem() { return _pipelines; }                     // pipeline system getter
    swap_chain_system& getSwapChainSystem() { return _swapChains; }                 // swap chain system getter
    frame_manager& getFrameManager() { return _frames; }                            // frame manager getter

    void firstTimeSetup();
    bool firstTime = true;

    VkRenderPass getRenderPass() { return _renderPass; }
    VkSampler getTextureSampler() { return _textureSampler; }
private:
    // Initialization
    void init();
    // Window initialization
    void initWindow();
        static void framebufferResizeCallback(GLFWwindow* window, int width, int height) {
                auto app = reinterpret_cast<rendering_system*>(glfwGetWindowUserPointer(window));
                app->_framebufferResized = true;}

    // Vulkan initialization 
    void initVulkan();
        // Instance creation
        void createInstance();
            bool checkValidationLayerSupport();
            std::vector<const char*> getRequiredExtensions();
        void setupDebugMessenger();
            void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
        void createSurface();
        void pickPhysicalDevice();
            bool isDeviceSuitable(VkPhysicalDevice device);
                bool checkDeviceExtensionSupport(VkPhysicalDevice device);
                SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);\
        void createLogicalDevice();

        // Rendering setup
        void createRenderPass();
        void createCommandPool();

        // Resource setup
        void createTextureSampler();
        void createDescriptorSets();
        void createCommandBuffers();
        void createSyncObjects();

    // Where the magic happens
    // void mainLoop();
        //void drawFrame();
            void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
        void* pUserData)
    {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
        return VK_FALSE;
    };
     
    Manta* _core;                                           // core
    std::shared_ptr<Scene> _scene;                          // scene
    
    model_mesh_library _modelLibrary;                   // model mesh library
    memory_system _memory;                                  // memory system
    command_buffer_system _commandBuffer;                   // command buffer system
    texture_system _texture;                                // texture system
    shader_system _shaders;                                 // shader system
    pipeline_system _pipelines;                             // pipeline system
    swap_chain_system _swapChains;                          // swap chain system
    frame_manager _frames;                                  // frame manager

    std::unique_ptr<DescriptorLayoutCache> _descriptorLayoutCache;
    std::unique_ptr<DescriptorAllocator> _descriptorAllocator;

    // Runtime Variables
    VkDescriptorPool _descriptorPool;                       // descriptor pool
    std::vector<VkDescriptorSet> _descriptorSets;           // descriptor sets

    uint32_t _mipLevels = 1;                                // mip levels

    image _textureImage;                                    // texture image

    VkSampler _textureSampler;                              // texture sampler

    image _depthImage;                                      // depth image

    // Initialization variables
    GLFWwindow* _window;                                    // glfw window
    VkInstance _instance;                                   // vulkan instance
    VkDebugUtilsMessengerEXT _debugMessenger;               // debug messenger

    VkPhysicalDevice _physicalDevice = VK_NULL_HANDLE;      // physical device
    VkDevice _device;                                       // logical device

    VkQueue _graphicsQueue;                                 // graphics queue
    VkQueue _transferQueue;                                 // transfer queue
    VkQueue _presentationQueue;                             // presentation queue

    VkRenderPass _renderPass;                               // render pass
    VkSurfaceKHR _surface;                                  // surface

    VkCommandPool _commandPool;                             // command pool
    VkCommandPool _transferCommandPool;                     // transfer command pool
    std::vector<VkCommandBuffer> _commandBuffers;           // command buffer

    std::vector<VkSemaphore> _imageAvailableSemaphores;     // image available semaphore
    std::vector<VkSemaphore> _renderFinishedSemaphores;     // render finished semaphore
    std::vector<VkFence> _inFlightFences;                   // in flight fence
    uint32_t _currentFrame = 0;                             // current frame

    bool _framebufferResized = false;                       // framebuffer resized flag

};