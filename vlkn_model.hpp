#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

#include "vlkn_device.hpp"

namespace vlkn {

class VlknModel {
public:
  struct Vertex {
    Vertex(glm::vec2 pos, glm::vec3 col);
    Vertex(glm::vec2 pos);
    Vertex() {}

    glm::vec2 position{};
    glm::vec3 color{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();
  };

  VlknModel(VlknDevice &device, const std::vector<Vertex> &vertices);
  ~VlknModel();

  VlknModel(const VlknModel &) = delete;
  VlknModel &operator=(const VlknModel &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);

  VlknDevice &vlknDevice;
  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  uint32_t vertexCount;
};

} // namespace vlkn
