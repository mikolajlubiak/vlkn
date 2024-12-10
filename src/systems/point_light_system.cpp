// header
#include "point_light_system.hpp"

// local
#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_pipeline.hpp"

// libs
#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <bits/fs_fwd.h>
#include <cassert>
#include <memory>
#include <stdexcept>
#include <vector>

namespace vlkn {

struct PointLightPushConstants {
  glm::vec4 position{};
  glm::vec4 color{};
  float radius = 0.0f;
};

PointLightSystem::PointLightSystem(VlknDevice &device, VkRenderPass renderPass,
                                   VkDescriptorSetLayout globalSetLayout)
    : vlknDevice(device) {
  createPipelineLayout(globalSetLayout);
  createPipeline(renderPass);
}

PointLightSystem::~PointLightSystem() {
  vkDestroyPipelineLayout(vlknDevice.device(), pipelineLayout, nullptr);
}

void PointLightSystem::createPipelineLayout(
    VkDescriptorSetLayout globalSetLayout) {

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PointLightPushConstants);

  std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount =
      static_cast<std::uint32_t>(descriptorSetLayouts.size());
  pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(vlknDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout");
  }
}

void PointLightSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  VlknPipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.bindingDescriptions.clear();
  pipelineConfig.attributeDescriptions.clear();
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  vlknPipeline = std::make_unique<VlknPipeline>(
      vlknDevice, "shaders/point_light.vert.spv",
      "shaders/point_light.frag.spv", pipelineConfig);
}

void PointLightSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo) {
  std::size_t lightIndex = 0;
  for (auto &kv : frameInfo.gameObjects) {
    auto &obj = kv.second;
    if (obj.pointLight != nullptr) {
      ubo.pointLights[lightIndex].position =
          glm::vec4(obj.transform.translation, 1.0f);
      ubo.pointLights[lightIndex].color =
          glm::vec4(obj.color, obj.pointLight->lightIntensity);

      lightIndex++;
    }
  }

  ubo.lightsNum = lightIndex;
}

void PointLightSystem::render(FrameInfo &frameInfo) {
  vlknPipeline->bind(frameInfo.commandBuffer);

  vkCmdBindDescriptorSets(frameInfo.commandBuffer,
                          VK_PIPELINE_BIND_POINT_GRAPHICS, pipelineLayout, 0, 1,
                          &frameInfo.globalDescriptorSet, 0, nullptr);

  for (auto &kv : frameInfo.gameObjects) {
    auto &obj = kv.second;
    if (obj.pointLight != nullptr) {
      PointLightPushConstants push{};
      push.position = glm::vec4(obj.transform.translation, 1.0f);
      push.color = glm::vec4(obj.color, obj.pointLight->lightIntensity);
      push.radius = obj.transform.scale.x;

      vkCmdPushConstants(frameInfo.commandBuffer, pipelineLayout,
                         VK_SHADER_STAGE_VERTEX_BIT |
                             VK_SHADER_STAGE_FRAGMENT_BIT,
                         0, sizeof(PointLightPushConstants), &push);

      vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
    }
  }
}

} // namespace vlkn
