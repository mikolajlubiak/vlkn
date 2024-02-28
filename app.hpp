#pragma once

#include "vlkn_device.hpp"
#include "vlkn_pipeline.hpp"
#include "vlkn_swap_chain.hpp"
#include "vlkn_window.hpp"

#include <cstdint>
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
  void createPipelineLayout();
  void createPipeline();
  void createCommandBuffers();
  void drawFrame();
  VlknWindow vlknWindow{WIDTH, HEIGH, "vlkn Demo"};
  VlknDevice vlknDevice{vlknWindow};
  VlknSwapChain vlknSwapChain{vlknDevice, vlknWindow.getExtent()};
  std::unique_ptr<VlknPipeline> vlknPipeline;
  VkPipelineLayout pipelineLayout;
  std::vector<VkCommandBuffer> commandBuffers;
};

} // namespace vlkn
