// header
#include "render_system.hpp"

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

struct PushConstantData {
  glm::mat4 transform{1.0f};
  alignas(16) glm::vec3 color;
};

RenderSystem::RenderSystem(VlknDevice &device, VkRenderPass renderPass)
    : vlknDevice(device) {
  createPipelineLayout();
  createPipeline(renderPass);
}

RenderSystem::~RenderSystem() {
  vkDestroyPipelineLayout(vlknDevice.device(), pipelineLayout, nullptr);
}

void RenderSystem::createPipelineLayout() {

  VkPushConstantRange pushConstantRange{};
  pushConstantRange.stageFlags =
      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
  pushConstantRange.offset = 0;
  pushConstantRange.size = sizeof(PushConstantData);

  VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
  pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
  pipelineLayoutInfo.setLayoutCount = 0;
  pipelineLayoutInfo.pSetLayouts = nullptr;
  pipelineLayoutInfo.pushConstantRangeCount = 1;
  pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

  if (vkCreatePipelineLayout(vlknDevice.device(), &pipelineLayoutInfo, nullptr,
                             &pipelineLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create pipeline layout");
  }
}

void RenderSystem::createPipeline(VkRenderPass renderPass) {
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  VlknPipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  vlknPipeline = std::make_unique<VlknPipeline>(
      vlknDevice, "shaders/vert.spv", "shaders/frag.spv", pipelineConfig);
}

void RenderSystem::renderGameObjects(VkCommandBuffer commandBuffer,
                                     std::vector<VlknGameObject> &gameObjects,
                                     const VlknCamera &camera,
                                     float deltaTime) {
  vlknPipeline->bind(commandBuffer);

  glm::mat4 projectionView = camera.getProjection() * camera.getView();

  for (VlknGameObject &obj : gameObjects) {
    TransformComponent transform = obj.transform;

    transform.rotation.y = glm::mod<float>(
        obj.transform.rotation.y + deltaTime * 5.0f, glm ::two_pi<float>());

    transform.rotation.x = glm::mod<float>(
        obj.transform.rotation.x + deltaTime * 1.0f, glm::two_pi<float>());

    PushConstantData push{};
    push.color = obj.color;
    push.transform = projectionView * transform.mat4();

    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(PushConstantData), &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

} // namespace vlkn
