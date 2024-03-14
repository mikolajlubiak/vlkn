#pragma once

#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_pipeline.hpp"
#include "vlkn_swap_chain.hpp"
#include "vlkn_window.hpp"

#include <cstdint>
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

class App {
public:
  static constexpr uint32_t WIDTH = 800;
  static constexpr uint32_t HEIGH = 600;

  App();
  ~App();

  App(const App &) = delete;
  App &operator=(const App &) = delete;

  void run();

private:
  void sierpinski(std::vector<VlknModel::Vertex> &vertices, int depth,
                  glm::vec2 left, glm::vec2 right, glm::vec2 top,
                  glm::vec3 leftColor, glm::vec3 rightColor,
                  glm::vec3 topColor);

  void loadGameObjects();
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void freeCommandBuffers();
  void drawFrame();
  void recreateSwapChain();
  void recordCommandBuffer(uint32_t imageIndex);
  void renderGameObjects(VkCommandBuffer commandBuffer);

  VlknWindow vlknWindow{WIDTH, HEIGH, "vlkn Demo"};
  VlknDevice vlknDevice{vlknWindow};
  std::unique_ptr<VlknSwapChain> vlknSwapChain;
  std::unique_ptr<VlknPipeline> vlknPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
  std::vector<VlknGameObject> gameObjects;
};

} // namespace vlkn
