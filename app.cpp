#include "app.hpp"
#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_model.hpp"
#include "vlkn_pipeline.hpp"
#include "vlkn_swap_chain.hpp"

#include <GLFW/glfw3.h>
#include <array>
#include <bits/fs_fwd.h>
#include <cassert>
#include <cstdint>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

struct PushConstantData {
  glm::mat2 transform{1.0f};
  alignas(4) glm::vec2 offset;
  alignas(16) glm::vec3 color;
};

App::App() {
  gameObjects.reserve(5);
  loadGameObjects();
  createPipelineLayout();
  recreateSwapChain();
  createCommandBuffers();
}

App::~App() {
  vkDestroyPipelineLayout(vlknDevice.device(), pipelineLayout, nullptr);
}

void App::run() {
  while (!vlknWindow.shouldClose()) {
    glfwPollEvents();
    drawFrame();
  }

  vkDeviceWaitIdle(vlknDevice.device());
}

void App::createPipelineLayout() {

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

void App::createPipeline() {
  assert(vlknSwapChain != nullptr &&
         "Cannot create pipeline before swap chain");
  assert(pipelineLayout != nullptr &&
         "Cannot create pipeline before pipeline layout");

  PipelineConfigInfo pipelineConfig{};
  VlknPipeline::defaultPipelineConfigInfo(pipelineConfig);
  pipelineConfig.renderPass = vlknSwapChain->getRenderPass();
  pipelineConfig.pipelineLayout = pipelineLayout;
  vlknPipeline = std::make_unique<VlknPipeline>(
      vlknDevice, "shaders/vert.spv", "shaders/frag.spv", pipelineConfig);
}

void App::recreateSwapChain() {
  VkExtent2D extent = vlknWindow.getExtent();
  while (extent.width == 0 || extent.height == 0) {
    extent = vlknWindow.getExtent();
    glfwWaitEvents();
  }

  vkDeviceWaitIdle(vlknDevice.device());

  if (vlknSwapChain == nullptr) {
    vlknSwapChain = std::make_unique<VlknSwapChain>(vlknDevice, extent);
  } else {
    vlknSwapChain = std::make_unique<VlknSwapChain>(vlknDevice, extent,
                                                    std::move(vlknSwapChain));
    if (vlknSwapChain->imageCount() != commandBuffers.size()) {
      freeCommandBuffers();
      createCommandBuffers();
    }
  }

  createPipeline();
}

void App::createCommandBuffers() {
  commandBuffers.resize(vlknSwapChain->imageCount());
  VkCommandBufferAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
  allocInfo.commandPool = vlknDevice.getCommandPool();
  allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers.size());

  if (vkAllocateCommandBuffers(vlknDevice.device(), &allocInfo,
                               commandBuffers.data()) != VK_SUCCESS) {
    throw std::runtime_error("failed to create command buffers");
  }
}

void App::freeCommandBuffers() {
  vkFreeCommandBuffers(vlknDevice.device(), vlknDevice.getCommandPool(),
                       static_cast<uint32_t>(commandBuffers.size()),
                       commandBuffers.data());
  commandBuffers.clear();
}

void App::drawFrame() {
  uint32_t imageIndex;
  VkResult result = vlknSwapChain->acquireNextImage(&imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR) {
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
    throw std::runtime_error("failed to acquire next image");
  }

  recordCommandBuffer(imageIndex);

  result = vlknSwapChain->submitCommandBuffers(&commandBuffers[imageIndex],
                                               &imageIndex);

  if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR ||
      vlknWindow.wasWindowResized()) {
    vlknWindow.resetWindowResizedFlag();
    recreateSwapChain();
    return;
  }

  if (result != VK_SUCCESS) {
    throw std::runtime_error("failed to submit command buffer");
  }
}

void App::sierpinski(std::vector<VlknModel::Vertex> &vertices, int depth,
                     glm::vec2 left, glm::vec2 right, glm::vec2 top,
                     glm::vec3 leftColor, glm::vec3 rightColor,
                     glm::vec3 topColor) {
  if (depth <= 0) {
    vertices.emplace_back(top, topColor);
    vertices.emplace_back(right, rightColor);
    vertices.emplace_back(left, leftColor);
  } else {
    glm::vec2 leftTop = 0.5f * (left + top);
    glm::vec2 rightTop = 0.5f * (right + top);
    glm::vec2 leftRight = 0.5f * (left + right);
    sierpinski(vertices, depth - 1, left, leftRight, leftTop, leftColor,
               rightColor, topColor);
    sierpinski(vertices, depth - 1, leftRight, right, rightTop, leftColor,
               rightColor, topColor);
    sierpinski(vertices, depth - 1, leftTop, rightTop, top, leftColor,
               rightColor, topColor);
  }
}

void App::loadGameObjects() {
  std::vector<VlknModel::Vertex> vertices{{{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
                                          {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
                                          {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}};

  std::shared_ptr<VlknModel> vlknModel =
      std::make_shared<VlknModel>(vlknDevice, vertices);

  std::vector<glm::vec3> colors{
      {1.f, .7f, .73f},
      {1.f, .87f, .73f},
      {1.f, 1.f, .73f},
      {.73f, 1.f, .8f},
      {.73, .88f, 1.f} //
  };

  for (glm::vec3 &color : colors) {
    color = glm::pow(color, glm::vec3{2.2f});
  }

  for (int i = 0; i < 40; i++) {
    auto triangle = VlknGameObject::createGameObject();
    triangle.model = vlknModel;
    triangle.transform2D.scale = glm::vec2(.5f) + i * 0.025f;
    triangle.transform2D.rotation = i * glm::pi<float>() * .025f;
    triangle.color = colors[i % colors.size()];
    gameObjects.push_back(std::move(triangle));
  }
}

void App::recordCommandBuffer(uint32_t imageIndex) {
  VkCommandBufferBeginInfo beginInfo{};
  beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

  if (vkBeginCommandBuffer(commandBuffers[imageIndex], &beginInfo) !=
      VK_SUCCESS) {
    throw std::runtime_error("failed to begin buffer");
  }

  VkRenderPassBeginInfo renderPassInfo{};
  renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
  renderPassInfo.renderPass = vlknSwapChain->getRenderPass();
  renderPassInfo.framebuffer = vlknSwapChain->getFrameBuffer(imageIndex);

  renderPassInfo.renderArea.offset = {0, 0};
  renderPassInfo.renderArea.extent = vlknSwapChain->getSwapChainExtent();

  std::array<VkClearValue, 2> clearValues{};
  clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
  clearValues[1].depthStencil = {1.0f, 0};

  renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
  renderPassInfo.pClearValues = clearValues.data();

  vkCmdBeginRenderPass(commandBuffers[imageIndex], &renderPassInfo,
                       VK_SUBPASS_CONTENTS_INLINE);
  VkViewport viewport{};
  viewport.x = 0;
  viewport.y = 0;
  viewport.width =
      static_cast<float>(vlknSwapChain->getSwapChainExtent().width);
  viewport.height =
      static_cast<float>(vlknSwapChain->getSwapChainExtent().height);
  viewport.minDepth = 0.0f;
  viewport.maxDepth = 1.0f;
  VkRect2D scissor{{0, 0}, vlknSwapChain->getSwapChainExtent()};
  vkCmdSetViewport(commandBuffers[imageIndex], 0, 1, &viewport);
  vkCmdSetScissor(commandBuffers[imageIndex], 0, 1, &scissor);

  renderGameObjects(commandBuffers[imageIndex]);

  vkCmdEndRenderPass(commandBuffers[imageIndex]);

  if (vkEndCommandBuffer(commandBuffers[imageIndex]) != VK_SUCCESS) {
    throw std::runtime_error("failed to record command buffer");
  }
}

void App::renderGameObjects(VkCommandBuffer commandBuffer) {

  int i = 0;
  for (VlknGameObject &obj : gameObjects) {
    i += 1;
    obj.transform2D.rotation = glm::mod<float>(
        obj.transform2D.rotation + 0.00005f * i, 2.f * glm::pi<float>());
  }

  vlknPipeline->bind(commandBuffer);

  for (VlknGameObject &obj : gameObjects) {
    PushConstantData push{};
    push.offset = obj.transform2D.translation;
    push.color = obj.color;
    push.transform = obj.transform2D.mat2();

    vkCmdPushConstants(commandBuffer, pipelineLayout,
                       VK_SHADER_STAGE_VERTEX_BIT |
                           VK_SHADER_STAGE_FRAGMENT_BIT,
                       0, sizeof(PushConstantData), &push);
    obj.model->bind(commandBuffer);
    obj.model->draw(commandBuffer);
  }
}

} // namespace vlkn
