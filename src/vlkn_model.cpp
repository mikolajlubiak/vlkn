// header
#include "vlkn_model.hpp"

// local
#include "vlkn_utils.hpp"

// libs
// tinyobjloader
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
// glm
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/fwd.hpp>
#include <glm/gtx/hash.hpp>
// vulkan
#include <vulkan/vulkan_core.h>

// std
#include <cassert>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>

namespace std {
template <> struct hash<vlkn::VlknModel::Vertex> {
  size_t operator()(vlkn::VlknModel::Vertex const &vertex) const {
    size_t seed = 0;
    vlkn::hashCombine(seed, vertex.position, vertex.color, vertex.normal,
                      vertex.uv);
    return seed;
  }
};
} // namespace std

namespace vlkn {

VlknModel::VlknModel(VlknDevice &device, const Builder &builder)
    : vlknDevice(device) {
  createVertexBuffers(builder.vertices);
  createIndexBuffers(builder.indices);
}

std::unique_ptr<VlknModel>
VlknModel::createModelFromFile(VlknDevice &device,
                               const std::filesystem::path &path) {
  Builder builder{};
  builder.loadModel(path);

  return std::make_unique<VlknModel>(device, builder);
}

void VlknModel::createVertexBuffers(const std::vector<Vertex> &vertices) {
  vertexCount = static_cast<std::uint32_t>(vertices.size());
  assert(vertexCount >= 3 && "Vertex count must be at least 3");

  std::uint32_t vertexSize = sizeof(decltype(vertices[0]));

  VkDeviceSize bufferSize = vertexSize * vertexCount;

  VlknBuffer stagingBuffer{vlknDevice, vertexSize, vertexCount,
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer(reinterpret_cast<const void *>(vertices.data()));

  vertexBuffer = std::make_unique<VlknBuffer>(
      vlknDevice, vertexSize, vertexCount,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vlknDevice.copyBuffer(stagingBuffer.getBuffer(), vertexBuffer->getBuffer(),
                        bufferSize);
}

void VlknModel::createIndexBuffers(const std::vector<std::uint32_t> &indices) {
  indexCount = static_cast<std::uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  std::uint32_t indexSize = sizeof(decltype(indices[0]));

  VkDeviceSize bufferSize = indexSize * indexCount;

  VlknBuffer stagingBuffer{vlknDevice, indexSize, indexCount,
                           VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                           VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                               VK_MEMORY_PROPERTY_HOST_COHERENT_BIT};

  stagingBuffer.map();
  stagingBuffer.writeToBuffer(reinterpret_cast<const void *>(indices.data()));

  indexBuffer = std::make_unique<VlknBuffer>(
      vlknDevice, indexSize, indexCount,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

  vlknDevice.copyBuffer(stagingBuffer.getBuffer(), indexBuffer->getBuffer(),
                        bufferSize);
}

void VlknModel::draw(VkCommandBuffer commandBuffer) {
  if (hasIndexBuffer) {
    vkCmdDrawIndexed(commandBuffer, indexCount, 1, 0, 0, 0);
  } else {
    vkCmdDraw(commandBuffer, vertexCount, 1, 0, 0);
  }
}

void VlknModel::bind(VkCommandBuffer commandBuffer) {
  VkBuffer buffers[] = {vertexBuffer->getBuffer()};
  VkDeviceSize offsets[] = {0};
  vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

  if (hasIndexBuffer) {
    vkCmdBindIndexBuffer(commandBuffer, indexBuffer->getBuffer(), 0,
                         VK_INDEX_TYPE_UINT32);
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
  std::vector<VkVertexInputAttributeDescription> attributeDescriptions;

  attributeDescriptions.push_back(
      {0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});

  attributeDescriptions.push_back(
      {1, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, color)});

  attributeDescriptions.push_back(
      {2, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, normal)});

  attributeDescriptions.push_back(
      {3, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(Vertex, uv)});

  return attributeDescriptions;
}

VlknModel::Vertex::Vertex(glm::vec3 pos, glm::vec3 col)
    : position(pos), color(col) {}

VlknModel::Vertex::Vertex(glm::vec3 pos) : position(pos) {}

void VlknModel::Builder::loadModel(const std::filesystem::path &path) {
  tinyobj::attrib_t attrib;
  std::vector<tinyobj::shape_t> shapes;
  std::vector<tinyobj::material_t> materials;
  std::string warn, error;

  if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &error,
                        path.c_str())) {
    throw std::runtime_error(warn + error);
  }

  vertices.clear();
  indices.clear();

  std::unordered_map<Vertex, std::uint32_t> uniqueVertices{};

  for (const auto &shape : shapes) {
    for (const auto &index : shape.mesh.indices) {
      Vertex vertex{};

      if (index.vertex_index >= 0) {
        vertex.position = {
            attrib.vertices[3 * index.vertex_index + 0],
            attrib.vertices[3 * index.vertex_index + 1],
            attrib.vertices[3 * index.vertex_index + 2],
        };

        vertex.color = {
            attrib.colors[3 * index.vertex_index + 0],
            attrib.colors[3 * index.vertex_index + 1],
            attrib.colors[3 * index.vertex_index + 2],
        };
      }

      if (index.normal_index >= 0) {
        vertex.normal = {
            attrib.normals[3 * index.normal_index + 0],
            attrib.normals[3 * index.normal_index + 1],
            attrib.normals[3 * index.normal_index + 2],
        };
      }

      if (index.texcoord_index >= 0) {
        vertex.uv = {
            attrib.texcoords[2 * index.texcoord_index + 0],
            attrib.texcoords[2 * index.texcoord_index + 1],
        };
      }

      if (uniqueVertices.count(vertex) == 0) {
        uniqueVertices[vertex] = static_cast<std::uint32_t>(vertices.size());
        vertices.push_back(vertex);
      }

      indices.push_back(uniqueVertices[vertex]);
    }
  }
}

} // namespace vlkn
