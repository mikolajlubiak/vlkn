#pragma once

#include "vlkn_device.hpp"

#include <cstdint>
#include <string>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

struct PipelineConfigInfo {
  VkPipelineViewportStateCreateInfo viewportInfo;
  VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
  VkPipelineRasterizationStateCreateInfo rasterizationInfo;
  VkPipelineMultisampleStateCreateInfo multisampleInfo;
  VkPipelineColorBlendAttachmentState colorBlendAttachment;
  VkPipelineColorBlendStateCreateInfo colorBlendInfo;
  VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
  std::vector<VkDynamicState> dynamicStateEnables;
  VkPipelineDynamicStateCreateInfo dynamicStateInfo;
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
  VlknPipeline &operator=(const VlknPipeline &) = delete;

  static void defaultPipelineConfigInfo(PipelineConfigInfo &configInfo);

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
