// header
#include "vlkn_descriptors.hpp"

// std
#include <cassert>
#include <stdexcept>

namespace vlkn {

// *************** Descriptor Set Layout Builder *********************

VlknDescriptorSetLayout::Builder &VlknDescriptorSetLayout::Builder::addBinding(
    uint32_t binding, VkDescriptorType descriptorType,
    VkShaderStageFlags stageFlags, uint32_t count) {
  assert(bindings.count(binding) == 0 && "Binding already in use");
  VkDescriptorSetLayoutBinding layoutBinding{};
  layoutBinding.binding = binding;
  layoutBinding.descriptorType = descriptorType;
  layoutBinding.descriptorCount = count;
  layoutBinding.stageFlags = stageFlags;
  bindings[binding] = layoutBinding;
  return *this;
}

std::unique_ptr<VlknDescriptorSetLayout>
VlknDescriptorSetLayout::Builder::build() const {
  return std::make_unique<VlknDescriptorSetLayout>(vlknDevice, bindings);
}

// *************** Descriptor Set Layout *********************

VlknDescriptorSetLayout::VlknDescriptorSetLayout(
    VlknDevice &vlknDevice,
    std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings)
    : vlknDevice{vlknDevice}, bindings{bindings} {
  std::vector<VkDescriptorSetLayoutBinding> setLayoutBindings{};
  for (auto kv : bindings) {
    setLayoutBindings.push_back(kv.second);
  }

  VkDescriptorSetLayoutCreateInfo descriptorSetLayoutInfo{};
  descriptorSetLayoutInfo.sType =
      VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
  descriptorSetLayoutInfo.bindingCount =
      static_cast<uint32_t>(setLayoutBindings.size());
  descriptorSetLayoutInfo.pBindings = setLayoutBindings.data();

  if (vkCreateDescriptorSetLayout(vlknDevice.device(), &descriptorSetLayoutInfo,
                                  nullptr,
                                  &descriptorSetLayout) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor set layout!");
  }
}

VlknDescriptorSetLayout::~VlknDescriptorSetLayout() {
  vkDestroyDescriptorSetLayout(vlknDevice.device(), descriptorSetLayout,
                               nullptr);
}

// *************** Descriptor Pool Builder *********************

VlknDescriptorPool::Builder &
VlknDescriptorPool::Builder::addPoolSize(VkDescriptorType descriptorType,
                                         uint32_t count) {
  poolSizes.push_back({descriptorType, count});
  return *this;
}

VlknDescriptorPool::Builder &
VlknDescriptorPool::Builder::setPoolFlags(VkDescriptorPoolCreateFlags flags) {
  poolFlags = flags;
  return *this;
}
VlknDescriptorPool::Builder &
VlknDescriptorPool::Builder::setMaxSets(uint32_t count) {
  maxSets = count;
  return *this;
}

std::unique_ptr<VlknDescriptorPool> VlknDescriptorPool::Builder::build() const {
  return std::make_unique<VlknDescriptorPool>(vlknDevice, maxSets, poolFlags,
                                              poolSizes);
}

// *************** Descriptor Pool *********************

VlknDescriptorPool::VlknDescriptorPool(
    VlknDevice &vlknDevice, uint32_t maxSets,
    VkDescriptorPoolCreateFlags poolFlags,
    const std::vector<VkDescriptorPoolSize> &poolSizes)
    : vlknDevice{vlknDevice} {
  VkDescriptorPoolCreateInfo descriptorPoolInfo{};
  descriptorPoolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
  descriptorPoolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
  descriptorPoolInfo.pPoolSizes = poolSizes.data();
  descriptorPoolInfo.maxSets = maxSets;
  descriptorPoolInfo.flags = poolFlags;

  if (vkCreateDescriptorPool(vlknDevice.device(), &descriptorPoolInfo, nullptr,
                             &descriptorPool) != VK_SUCCESS) {
    throw std::runtime_error("failed to create descriptor pool!");
  }
}

VlknDescriptorPool::~VlknDescriptorPool() {
  vkDestroyDescriptorPool(vlknDevice.device(), descriptorPool, nullptr);
}

bool VlknDescriptorPool::allocateDescriptorSet(
    const VkDescriptorSetLayout descriptorSetLayout,
    VkDescriptorSet &descriptor) const {
  VkDescriptorSetAllocateInfo allocInfo{};
  allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
  allocInfo.descriptorPool = descriptorPool;
  allocInfo.pSetLayouts = &descriptorSetLayout;
  allocInfo.descriptorSetCount = 1;

  // Might want to create a "DescriptorPoolManager" class that handles this
  // case, and builds a new pool whenever an old pool fills up. But this is
  // beyond our current scope
  if (vkAllocateDescriptorSets(vlknDevice.device(), &allocInfo, &descriptor) !=
      VK_SUCCESS) {
    return false;
  }
  return true;
}

void VlknDescriptorPool::freeDescriptors(
    std::vector<VkDescriptorSet> &descriptors) const {
  vkFreeDescriptorSets(vlknDevice.device(), descriptorPool,
                       static_cast<uint32_t>(descriptors.size()),
                       descriptors.data());
}

void VlknDescriptorPool::resetPool() {
  vkResetDescriptorPool(vlknDevice.device(), descriptorPool, 0);
}

// *************** Descriptor Writer *********************

VlknDescriptorWriter::VlknDescriptorWriter(VlknDescriptorSetLayout &setLayout,
                                           VlknDescriptorPool &pool)
    : setLayout{setLayout}, pool{pool} {}

VlknDescriptorWriter &
VlknDescriptorWriter::writeBuffer(uint32_t binding,
                                  VkDescriptorBufferInfo *bufferInfo) {
  assert(setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount == 1 &&
         "Binding single descriptor info, but binding expects multiple");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.descriptorType = bindingDescription.descriptorType;
  write.dstBinding = binding;
  write.pBufferInfo = bufferInfo;
  write.descriptorCount = 1;

  writes.push_back(write);
  return *this;
}

VlknDescriptorWriter &
VlknDescriptorWriter::writeImage(uint32_t binding,
                                 VkDescriptorImageInfo *imageInfo) {
  return writeImageArray(binding, imageInfo, 1);
}

VlknDescriptorWriter &VlknDescriptorWriter::writeImageArray(
    uint32_t binding, VkDescriptorImageInfo *imageInfo, uint32_t count) {
  assert(setLayout.bindings.count(binding) == 1 &&
         "Layout does not contain specified binding");

  auto &bindingDescription = setLayout.bindings[binding];

  assert(bindingDescription.descriptorCount != count + 1 &&
         "Binding expects different number of descriptors");

  VkWriteDescriptorSet write{};
  write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
  write.dstBinding = binding;
  write.descriptorType = bindingDescription.descriptorType;
  write.descriptorCount = count;
  write.pImageInfo = imageInfo;

  writes.push_back(write);
  return *this;
}

bool VlknDescriptorWriter::build(VkDescriptorSet &set) {
  bool success =
      pool.allocateDescriptorSet(setLayout.getDescriptorSetLayout(), set);
  if (!success) {
    return false;
  }
  overwrite(set);
  return true;
}

void VlknDescriptorWriter::overwrite(VkDescriptorSet &set) {
  for (auto &write : writes) {
    write.dstSet = set;
  }
  vkUpdateDescriptorSets(pool.vlknDevice.device(), writes.size(), writes.data(),
                         0, nullptr);
}

} // namespace vlkn
