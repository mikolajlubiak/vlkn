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
    Vertex(glm::vec3 pos, glm::vec3 col);
    Vertex(glm::vec3 pos);
    Vertex() {}

    glm::vec3 position{};
    glm::vec3 color{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();
  };

  struct Builder {
    std::vector<Vertex> vertices{};

    std::vector<std::uint32_t> indices{};
  };

  VlknModel(VlknDevice &device, const Builder &builder);
  ~VlknModel();

  VlknModel(const VlknModel &) = delete;
  VlknModel &operator=(const VlknModel &) = delete;

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffers(const std::vector<std::uint32_t> &indices);

  VlknDevice &vlknDevice;

  VkBuffer vertexBuffer;
  VkDeviceMemory vertexBufferMemory;
  std::uint32_t vertexCount;

  bool hasIndexBuffer = false;
  VkBuffer indexBuffer;
  VkDeviceMemory indexBufferMemory;
  std::uint32_t indexCount;
};

} // namespace vlkn
