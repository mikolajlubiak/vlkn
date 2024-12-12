#pragma once

// local
#include "vlkn_device.hpp"

// libs
// vulkan
#include <vulkan/vulkan_core.h>
// stb_image
#include "stb_image.h"

// std
#include <filesystem>

namespace vlkn {

class VlknImage {
public:
  struct Image {
    int texWidth, texHeight, texChannels;
    stbi_uc *pixels;
  };

  struct Builder {
    Image image;
    bool isStbImage = false;

    ~Builder() {
      if (isStbImage) {
        stbi_image_free(image.pixels);
      }
    }

    void loadImage(const std::filesystem::path &path);
  };

  VlknImage(VlknDevice &device, const Builder &builder);

  VlknImage(const VlknImage &) = delete;
  VlknImage &operator=(const VlknImage &) = delete;

  ~VlknImage();

  static std::unique_ptr<VlknImage>
  createImageFromFile(VlknDevice &device, const std::filesystem::path &path);

  static std::unique_ptr<VlknImage> createEmptyImage(VlknDevice &device);

  VkDescriptorImageInfo descriptorInfo();

private:
  void createTextureImage(Image image);

  void createTextureImageView();

  void createTextureSampler();

  void createImage(std::uint32_t width, std::uint32_t height, VkFormat format,
                   VkImageTiling tiling, VkImageUsageFlags usage,
                   VkMemoryPropertyFlags properties);

  void transitionImageLayout(VkFormat format, VkImageLayout oldLayout,
                             VkImageLayout newLayout);

  VlknDevice &vlknDevice;
  VkImage textureImage;
  VkDeviceMemory textureImageMemory;
  VkImageView textureImageView;
  VkSampler textureSampler;
};

} // namespace vlkn
