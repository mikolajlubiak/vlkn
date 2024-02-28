#pragma once

#include "vlkn_device.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

struct PipelineConfigInfo {
  VkViewport viewport;
  VkRect2D scissor;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  VkPipelineLayout pipelineLayout = nullptr;
  VkRenderPass renderPass = nullptr;
  uint32_t subpass = 0;
};

class VlknPipeline {
public:
  VlknPipeline(VlknDevice &device, const std::string &vert,
               const std::string &frag, const PipelineConfigInfo &configInfo);
  ~VlknPipeline();

  VlknPipeline(const VlknPipeline &) = delete;
  void operator=(const VlknPipeline &) = delete;

  static PipelineConfigInfo defaultPipelineConfigInfo(uint32_t width,
                                                      uint32_t height);

  void bind(VkCommandBuffer commandBuffer);

private:
  static std::vector<char> readFile(const std::string &path);

  void createGraphicsPipeline(const std::string &vert, const std::string &frag,
                              const PipelineConfigInfo &configInfo);

  void createShaderModule(const std::vector<char> &code,
                          VkShaderModule *shaderModule);

  VlknDevice &vlknDevice;
  VkPipeline graphicsPipeline;
  VkShaderModule vertShaderModule;
  VkShaderModule fragShaderModule;
};

} // namespace vlkn
