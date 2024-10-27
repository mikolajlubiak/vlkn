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
VlknModel::~VlknModel() {
  vkDestroyBuffer(vlknDevice.device(), vertexBuffer, nullptr);
  vkFreeMemory(vlknDevice.device(), vertexBufferMemory, nullptr);

  if (hasIndexBuffer) {
    vkDestroyBuffer(vlknDevice.device(), indexBuffer, nullptr);
    vkFreeMemory(vlknDevice.device(), indexBufferMemory, nullptr);
  }
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
  VkDeviceSize bufferSize = sizeof(vertices[0]) * vertexCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  vlknDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(vlknDevice.device(), stagingBufferMemory, 0, bufferSize, 0,
              &data);
  memcpy(data, vertices.data(), static_cast<std::size_t>(bufferSize));
  vkUnmapMemory(vlknDevice.device(), stagingBufferMemory);

  vlknDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, vertexBuffer, vertexBufferMemory);

  vlknDevice.copyBuffer(stagingBuffer, vertexBuffer, bufferSize);

  vkDestroyBuffer(vlknDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(vlknDevice.device(), stagingBufferMemory, nullptr);
}

void VlknModel::createIndexBuffers(const std::vector<std::uint32_t> &indices) {
  indexCount = static_cast<std::uint32_t>(indices.size());
  hasIndexBuffer = indexCount > 0;

  if (!hasIndexBuffer) {
    return;
  }

  VkDeviceSize bufferSize = sizeof(indices[0]) * indexCount;

  VkBuffer stagingBuffer;
  VkDeviceMemory stagingBufferMemory;

  vlknDevice.createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                          VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                              VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                          stagingBuffer, stagingBufferMemory);

  void *data;
  vkMapMemory(vlknDevice.device(), stagingBufferMemory, 0, bufferSize, 0,
              &data);
  memcpy(data, indices.data(), static_cast<std::size_t>(bufferSize));
  vkUnmapMemory(vlknDevice.device(), stagingBufferMemory);

  vlknDevice.createBuffer(
      bufferSize,
      VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, indexBuffer, indexBufferMemory);

  vlknDevice.copyBuffer(stagingBuffer, indexBuffer, bufferSize);

  vkDestroyBuffer(vlknDevice.device(), stagingBuffer, nullptr);
  vkFreeMemory(vlknDevice.device(), stagingBufferMemory, nullptr);
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

        auto colorIndex = 3 * index.vertex_index + 2;

        if (colorIndex < attrib.colors.size()) {
          vertex.color = {
              attrib.colors[colorIndex - 2],
              attrib.colors[colorIndex - 1],
              attrib.colors[colorIndex - 0],
          };
        } else {
          vertex.color = glm::vec3(1.0f);
        }
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
