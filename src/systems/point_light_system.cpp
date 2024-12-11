// header
#include "point_light_system.hpp"

// std
#include <cassert>

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
  VlknPipeline::enableAlphaBlending(pipelineConfig);
  pipelineConfig.bindingDescriptions.clear();
  pipelineConfig.attributeDescriptions.clear();
  pipelineConfig.renderPass = renderPass;
  pipelineConfig.pipelineLayout = pipelineLayout;
  vlknPipeline = std::make_unique<VlknPipeline>(
      vlknDevice, "shaders/point_light.vert.spv",
      "shaders/point_light.frag.spv", pipelineConfig);
}

void PointLightSystem::update(FrameInfo &frameInfo, GlobalUbo &ubo) {
  glm::mat4 rotateLight =
      glm::rotate(glm::mat4(1.0f), frameInfo.frameDelta, {0.0f, -1.0f, 0.0f});
  float lightIntensity = 0.5f * glm::sin(frameInfo.frameTime) + 0.6f;

  std::size_t lightIndex = 0;

  for (auto &kv : frameInfo.gameObjects) {
    auto &obj = kv.second;
    if (obj.pointLight != nullptr) {
      assert(lightIndex < MAX_LIGHTS && "Exceeded maximum point light count");

      obj.transform.translation =
          glm::vec3(rotateLight * glm::vec4(obj.transform.translation, 1.0f));

      obj.pointLight->lightIntensity = lightIntensity;

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
