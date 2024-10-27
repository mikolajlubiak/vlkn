#include "vlkn_model.hpp"

#include <cassert>
#include <cstdint>
#include <cstring>
#include <glm/fwd.hpp>
#include <vector>
#include <vulkan/vulkan_core.h>

namespace vlkn {

VlknModel::VlknModel(VlknDevice &device, const Builder &builder)
    : vlknDevice(device) {
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
}
VlknModel::~VlknModel() {
  vkDestroyBuffer(vlknDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(vlknDevice.device(), vertexBufferMemory, nullptr);

  if (hasIndexBuffer) {
    vkDestroyBuffer(vlknDevice.device(), indexBuffer, nullptr);
    vkFreeMemory(vlknDevice.device(), indexBufferMemory, nullptr);
  }
}

void VlknModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<std::uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  vlknDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          vertexBuffer, vertexBufferMemory);

  void *data;
  vkMapMemory(vlknDevice.device(), vertexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(vlknDevice.device(), vertexBufferMemory);
}

void VlknModel::createIndexBuffers(const std::vector<std::uint32_t> &indices) {
  indexCount = static_cast<std::uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

  vlknDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          indexBuffer, indexBufferMemory);

  void *data;
  vkMapMemory(vlknDevice.device(), indexBufferMemory, 0, bufferSize, 0, &data);
  memcpy(data, indices.data(), static_cast<size_t>(bufferSize));
  vkUnmapMemory(vlknDevice.device(), indexBufferMemory);
}

void VlknModel::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

void VlknModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer, 0, VK_INDEX_TYPE_UINT32);
  }
}

std::vector<VkVertexInputBindingDescription>
VlknModel::Vertex::getBindingDescriptions() {
  std::vector<VkVertexInputBindingDescription> bindingDescriptions{1};
  bindingDescriptions[0].binding = 0;
  bindingDescriptions[0].stride = sizeof(Vertex);
  bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
  return bindingDescriptions;
}

std::vector<VkVertexInputAttributeDescription>
VlknModel::Vertex::getAttributeDescriptions() {
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions{2};

  attributeDescriptions[0].binding = 0;
  attributeDescriptions[0].location = 0;
  attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[0].offset = offsetof(Vertex, position);

  attributeDescriptions[1].binding = 0;
  attributeDescriptions[1].location = 1;
  attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
  attributeDescriptions[1].offset = offsetof(Vertex, color);

  return attributeDescriptions;
}

VlknModel::Vertex::Vertex(glm::vec3 pos, glm::vec3 col)
    : position(pos), color(col) {}

VlknModel::Vertex::Vertex(glm::vec3 pos) : position(pos) {}

} // namespace vlkn
