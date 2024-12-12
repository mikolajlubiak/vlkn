#pragma once

// local
#include "vlkn_device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace vlkn {

class VlknDescriptorSetLayout {
public:
  class Builder {
  public:
    Builder(VlknDevice &vlknDevice) : vlknDevice{vlknDevice} {}

    Builder &addBinding(uint32_t binding, VkDescriptorType descriptorType,
                        VkShaderStageFlags stageFlags, uint32_t count = 1);
    std::unique_ptr<VlknDescriptorSetLayout> build() const;

  private:
    VlknDevice &vlknDevice;
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
  };

  VlknDescriptorSetLayout(
      VlknDevice &vlknDevice,
      std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
  ~VlknDescriptorSetLayout();
  VlknDescriptorSetLayout(const VlknDescriptorSetLayout &) = delete;
  VlknDescriptorSetLayout &operator=(const VlknDescriptorSetLayout &) = delete;

  VkDescriptorSetLayout getDescriptorSetLayout() const {
    return descriptorSetLayout;
  }

private:
  VlknDevice &vlknDevice;
  VkDescriptorSetLayout descriptorSetLayout;
  std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

  friend class VlknDescriptorWriter;
};

class VlknDescriptorPool {
public:
  class Builder {
  public:
    Builder(VlknDevice &vlknDevice) : vlknDevice{vlknDevice} {}

    Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
    Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
    Builder &setMaxSets(uint32_t count);
    std::unique_ptr<VlknDescriptorPool> build() const;

  private:
    VlknDevice &vlknDevice;
    std::vector<VkDescriptorPoolSize> poolSizes{};
    uint32_t maxSets = 1000;
    VkDescriptorPoolCreateFlags poolFlags = 0;
  };

  VlknDescriptorPool(VlknDevice &vlknDevice, uint32_t maxSets,
                     VkDescriptorPoolCreateFlags poolFlags,
                     const std::vector<VkDescriptorPoolSize> &poolSizes);
  ~VlknDescriptorPool();
  VlknDescriptorPool(const VlknDescriptorPool &) = delete;
  VlknDescriptorPool &operator=(const VlknDescriptorPool &) = delete;

  bool allocateDescriptorSet(const VkDescriptorSetLayout descriptorSetLayout,
                             VkDescriptorSet &descriptor) const;

  void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

  VkDescriptorPool getDescriptorPool() { return descriptorPool; }

  void resetPool();

private:
  VlknDevice &vlknDevice;
  VkDescriptorPool descriptorPool;

  friend class VlknDescriptorWriter;
};

class VlknDescriptorWriter {
public:
  VlknDescriptorWriter(VlknDescriptorSetLayout &setLayout,
                       VlknDescriptorPool &pool);

  VlknDescriptorWriter &writeBuffer(uint32_t binding,
                                    VkDescriptorBufferInfo *bufferInfo);
  VlknDescriptorWriter &writeImage(uint32_t binding,
                                   VkDescriptorImageInfo *imageInfo);
  VlknDescriptorWriter &writeImageArray(uint32_t binding,
                                        VkDescriptorImageInfo *imageInfo,
                                        uint32_t count);

  bool build(VkDescriptorSet &set);
  void overwrite(VkDescriptorSet &set);

private:
  VlknDescriptorSetLayout &setLayout;
  VlknDescriptorPool &pool;
  std::vector<VkWriteDescriptorSet> writes;
};

} // namespace vlkn