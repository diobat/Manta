#include "GUI/imGUIHandler.hpp"
#include "rendering/rendering.hpp"
#include "rendering/swapChainManager.hpp"

#include "ECS/components/spatial.hpp"
#include "ECS/components/camera.hpp"

#include "util/time.hpp"

#include "core/settings.hpp"



imGUI_handler::imGUI_handler(rendering_system* core) :
    _core(core)
{
    ;
}

void imGUI_handler::init(VkInstance& instance, VkQueue& graphicsQueue, VkRenderPass& renderPass)
{
    _context = ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls
    io.FontGlobalScale = 2.0f; 

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();

    // Setup Platform/Renderer bindings
    ImGui_ImplGlfw_InitForVulkan(_core->getWindow(), true);

    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = instance;
    init_info.PhysicalDevice = _core->getPhysicalDevice();
    init_info.Device = _core->getLogicalDevice();
    // init_info.QueueFamily = _core->getQueueFamilyIndices().graphicsFamily.value();
    init_info.Queue = graphicsQueue;
    init_info.PipelineCache = VK_NULL_HANDLE;
    _descriptorPool = _core->getFrameManager().getDescriptorAllocator()->grabPool();
    init_info.DescriptorPool = _descriptorPool;
    init_info.RenderPass = renderPass;
    init_info.Subpass = 0; 
    init_info.MinImageCount = getSettingsData(_core->getRegistry()).framesInFlight;
    init_info.ImageCount = getSettingsData(_core->getRegistry()).framesInFlight;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    // init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

    FPSCounter::start(); 
}

void imGUI_handler::onFrameStart()
{
    FPSCounter::click();

    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // ImGui::ShowDemoWindow(); // Show the ImGui demo window
    createUI();

}

void imGUI_handler::createUI()
{
    if (!ImGui::Begin("Dear ImGui Demo", &_showGUI))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    createSceneCollapsible();
    createCameraCollapsible();
    createPerformanceCollapsible();

    ImGui::End();
}

void imGUI_handler::createSceneCollapsible()
{
    if (ImGui::CollapsingHeader("Scene"))
    {
        static int currentChosenItem;
        std::vector<std::string> modelNames = _core->getScene()->getAllModelNames();

        //  convert the vector to a char array
        std::vector<const char *> cStrArray;
        cStrArray.reserve(modelNames.size());
        for(int index = 0; index < modelNames.size(); ++index)
        {
            cStrArray.push_back(modelNames[index].c_str());
        }

        ImGui::ListBox("Models", &currentChosenItem, cStrArray.data(), modelNames.size(), 4);
    }
}

void imGUI_handler::createCameraCollapsible()
{
    if (ImGui::CollapsingHeader("Camera"))
    {
        entt::entity camera = _core->getScene()->getActiveCamera();

        // Camera position
        float& posX = _core->getScene()->getRegistry().get<position>(camera).value.x;
        float& posY = _core->getScene()->getRegistry().get<position>(camera).value.y;
        float& posZ = _core->getScene()->getRegistry().get<position>(camera).value.z;

        ImGui::Text("Camera Position:  X: %.*f, Y: %.*f, Z: %.*f", _ndp, posX, _ndp, posY, _ndp, posZ);

        // Camera rotation quaternion
        float& rotX = _core->getScene()->getRegistry().get<rotation>(camera).value.x;
        float& rotY = _core->getScene()->getRegistry().get<rotation>(camera).value.y;
        float& rotZ = _core->getScene()->getRegistry().get<rotation>(camera).value.z;
        glm::quat::value_type rotW = _core->getScene()->getRegistry().get<rotation>(camera).value.w;

        ImGui::Text("Camera Rotation:  X: %.*f, Y: %.*f, Z: %.*f", _ndp, rotX, _ndp, rotY, _ndp, rotZ);

        // Camera info
        float& fov = _core->getScene()->getRegistry().get<cameraSettings>(camera).fov;
        ImGui::Text("FOV: %.*f" , _ndp , fov);

        // Camera Planes
        float& nearPlane = _core->getScene()->getRegistry().get<cameraSettings>(camera).nearPlane;
        float& farPlane = _core->getScene()->getRegistry().get<cameraSettings>(camera).farPlane;

        ImGui::Text("Near Plane: %.*f, Far Plane: %.*f", _ndp, nearPlane, _ndp, farPlane);
    }
}

void imGUI_handler::createPerformanceCollapsible()
{
    if (ImGui::CollapsingHeader("Performance"))
    {
        // Get last value of the frame time buffer
        ImGui::Text("FPS: %.*f", _ndp, 1 / FPSCounter::timeBuffer[FPSCounter::timeBuffer.size() - 1]);

        // Frame time plot
        ImGui::PlotLines("Frame Times", FPSCounter::getTimes(), FPSCounter::getArraySize());
    }
}

void imGUI_handler::onFrameEnd(uint32_t frameIndex)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    VkCommandBuffer command_buffer = _core->getSwapChainSystem().getCommandBuffer(frameIndex);

    // Render GUI
    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);
}

void imGUI_handler::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(_context);

    // This pool is not created by the ImGui library, so we need to destroy it
    // ourselves. We must do this after ImGui_ImplVulkan_Shutdown()
    vkDestroyDescriptorPool( _core->getLogicalDevice(),_descriptorPool , nullptr);
}