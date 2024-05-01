#include "GUI/imGUIHandler.hpp"
#include "rendering/rendering.hpp"

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
    init_info.DescriptorPool = _core->getFrameManager().getDescriptorAllocator()->grabPool();
    init_info.RenderPass = renderPass;
    init_info.Subpass = 0; 
    init_info.MinImageCount = getSettingsData(_core->getRegistry()).framesInFlight;
    init_info.ImageCount = getSettingsData(_core->getRegistry()).framesInFlight;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
    init_info.Allocator = nullptr;
    // init_info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&init_info);

}

void imGUI_handler::onFrameStart()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::ShowDemoWindow(); // Show the ImGui demo window

}

void imGUI_handler::onFrameEnd(uint32_t frameIndex)
{
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();


    VkCommandBuffer command_buffer = _core->getCommandBufferSystem().getCommandBuffer(frameIndex);

    ImGui_ImplVulkan_RenderDrawData(draw_data, command_buffer);

}

void imGUI_handler::cleanup()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext(_context);
}