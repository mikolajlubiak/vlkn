#pragma once

// local
#include "vlkn_buffer.hpp"
#include "vlkn_device.hpp"

// libs
// glm
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/fwd.hpp>
#include <glm/glm.hpp>
// vulkan
#include <vulkan/vulkan_core.h>

// std
#include <filesystem>
#include <memory>
#include <vector>

namespace vlkn {

class VlknModel {
public:
  struct Vertex {
    Vertex(glm::vec3 pos, glm::vec3 col);
    Vertex(glm::vec3 pos);
    Vertex() {}

    glm::vec3 position{};
    glm::vec3 color{};
    glm::vec3 normal{};
    glm::vec2 uv{};

    static std::vector<VkVertexInputBindingDescription>
    getBindingDescriptions();
    static std::vector<VkVertexInputAttributeDescription>
    getAttributeDescriptions();

    bool operator==(const Vertex &other) const {
      return position == other.position && color == other.color &&
             normal == other.normal;
    }
  };

  struct Builder {
    std::vector<Vertex> vertices{};
    std::vector<std::uint32_t> indices{};

    void loadModel(const std::filesystem::path &path);
  };

  VlknModel(VlknDevice &device, const Builder &builder);

  VlknModel(const VlknModel &) = delete;
  VlknModel &operator=(const VlknModel &) = delete;

  static std::unique_ptr<VlknModel>
  createModelFromFile(VlknDevice &device, const std::filesystem::path &path);

  void bind(VkCommandBuffer commandBuffer);
  void draw(VkCommandBuffer commandBuffer);

private:
  void createVertexBuffers(const std::vector<Vertex> &vertices);
  void createIndexBuffers(const std::vector<std::uint32_t> &indices);

  VlknDevice &vlknDevice;

  std::unique_ptr<VlknBuffer> vertexBuffer;
  std::uint32_t vertexCount;

  bool hasIndexBuffer = false;
  std::unique_ptr<VlknBuffer> indexBuffer;
  std::uint32_t indexCount;
};

} // namespace vlkn
