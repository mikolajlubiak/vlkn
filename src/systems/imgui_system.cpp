// header
#include "imgui_system.hpp"

// std
#include <cassert>

namespace vlkn {

ImGuiSystem::ImGuiSystem(VlknDevice &device, VkRenderPass renderPass,
                         std::uint32_t minImageCount, std::uint32_t imageCount)
    : vlknDevice(device) {

  descriptorPool =
      VlknDescriptorPool::Builder(vlknDevice)
          .setMaxSets(imageCount)
          .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imageCount)
          .setPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
          .build();

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  imguiIO = &ImGui::GetIO();
  imguiIO->ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForVulkan(vlknDevice.getWindow().getGLFWwindow(), true);

  ImGui_ImplVulkan_InitInfo init_info = {};
  init_info.Instance = vlknDevice.getInstance();
  init_info.PhysicalDevice = vlknDevice.getPhysicalDevice();
  init_info.Device = vlknDevice.device();
  init_info.QueueFamily = vlknDevice.findPhysicalQueueFamilies().graphicsFamily;
  init_info.Queue = vlknDevice.graphicsQueue();
  init_info.PipelineCache = VK_NULL_HANDLE;
  init_info.DescriptorPool = descriptorPool->getDescriptorPool();
  init_info.RenderPass = renderPass;
  init_info.Subpass = 0;
  init_info.MinImageCount = minImageCount;
  init_info.ImageCount = imageCount;
  init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
  init_info.Allocator = VK_NULL_HANDLE;
  init_info.CheckVkResultFn = nullptr;
  ImGui_ImplVulkan_Init(&init_info);

  ImGui_ImplVulkan_CreateFontsTexture();
}

ImGuiSystem::~ImGuiSystem() {
  ImGui_ImplVulkan_DestroyFontsTexture();
  ImGui_ImplVulkan_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
}

void ImGuiSystem::update(const glm::quat &rotation) {
  ImGui_ImplVulkan_NewFrame();
  ImGui_ImplGlfw_NewFrame();
  ImGui::NewFrame();

  ImGui::Begin("Frame time");
  ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
              1000.0f / imguiIO->Framerate, imguiIO->Framerate);

  // Convert quaternion to Euler angles for display
  glm::vec3 eulerAngles = glm::eulerAngles(rotation);
  ImGui::Text("Rotation (Euler angles): X %.3f, Y %.3f, Z %.3f",
              glm::degrees(eulerAngles.x), glm::degrees(eulerAngles.y),
              glm::degrees(eulerAngles.z));

  ImGui::ColorPicker4("Point light color", (float *)&pointLightColor);
  ImGui::End();
}

void ImGuiSystem::render(const FrameInfo &frameInfo) const {
  ImGui::Render();
  ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(),
                                  frameInfo.commandBuffer);
}

} // namespace vlkn
