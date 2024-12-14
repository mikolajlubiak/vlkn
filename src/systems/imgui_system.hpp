#pragma once

// local
#include "vlkn_camera.hpp"
#include "vlkn_descriptors.hpp"
#include "vlkn_device.hpp"
#include "vlkn_frame_info.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_pipeline.hpp"

// libs
// GLM
#include <glm/glm.hpp>
// Vulkan
#include <vulkan/vulkan_core.h>
// imgui
#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_vulkan.h"

// std
#include <memory>
#include <vector>

namespace vlkn {

class ImGuiSystem {
public:
  ImGuiSystem(VlknDevice &device, VkRenderPass renderPass,
              std::uint32_t minImageCount, std::uint32_t imageCount);
  ~ImGuiSystem();

  ImGuiSystem(const ImGuiSystem &) = delete;
  ImGuiSystem &operator=(const ImGuiSystem &) = delete;

  void update(const glm::vec3 &rotationInfo);

  void render(const FrameInfo &frameInfo) const;

  glm::vec4 getPointLightColor() const {
    return glm::vec4(pointLightColor.x, pointLightColor.y, pointLightColor.z,
                     pointLightColor.w);
  }

private:
  VlknDevice &vlknDevice;
  std::unique_ptr<VlknDescriptorPool> descriptorPool;
  ImVec4 pointLightColor{};
  ImGuiIO *imguiIO;
};

} // namespace vlkn
