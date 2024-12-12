// header
#include "app.hpp"

// local
#include "keyboard_movement_controller.hpp"
#include "mouse_movement_controller.hpp"
#include "systems/imgui_system.hpp"
#include "systems/point_light_system.hpp"
#include "systems/render_system.hpp"
#include "vlkn_buffer.hpp"
#include "vlkn_camera.hpp"
#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_image.hpp"
#include "vlkn_model.hpp"
#include "vlkn_renderer.hpp"

// libs
// GLFW
#define GLFW_INCLUDE_NONE
#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
// GLM
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
// Vulkan
#include <vulkan/vulkan_core.h>

// std
#include <memory>
#include <utility>
#include <vector>

#ifndef NDEBUG
#define APP_USE_VULKAN_DEBUG_REPORT
#endif

namespace vlkn {

App::App() {
  globalPool = VlknDescriptorPool::Builder(vlknDevice)
                   .setMaxSets(VlknSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VlknSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2)
                   .build();

  gameObjects.reserve(16);
  loadGameObjects();
}

App::~App() {}

void App::run() {
  std::vector<std::unique_ptr<VlknBuffer>> uboBuffers(
      VlknSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (std::size_t i = 0; i < uboBuffers.size(); i++) {
    uboBuffers[i] = std::make_unique<VlknBuffer>(
        vlknDevice, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);

    uboBuffers[i]->map();
  }

  std::unique_ptr<VlknImage> textureImage =
      VlknImage::createImageFromFile(vlknDevice, "textures/image.jpg");

  auto globalSetLayout =
      VlknDescriptorSetLayout::Builder(vlknDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .addBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                      VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(
      VlknSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (std::size_t i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    auto imageInfo = textureImage->descriptorInfo();

    VlknDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .writeImage(1, &imageInfo)
        .build(globalDescriptorSets[i]);
  }

  RenderSystem renderSystem{vlknDevice, vlknRenderer.getSwapChainRenderPass(),
                            globalSetLayout->getDescriptorSetLayout()};

  PointLightSystem pointLightSystem{vlknDevice,
                                    vlknRenderer.getSwapChainRenderPass(),
                                    globalSetLayout->getDescriptorSetLayout()};

  ImGuiSystem imguiSystem{vlknDevice, vlknRenderer.getSwapChainRenderPass(),
                          VlknSwapChain::MAX_FRAMES_IN_FLIGHT,
                          VlknSwapChain::MAX_FRAMES_IN_FLIGHT};

  VlknCamera camera{};
  VlknGameObject viewerObject = VlknGameObject::createGameObject();
  viewerObject.transform.translation = {0.0f, -1.0f, -2.0f};

  KeyboardMovementController keyboardController{viewerObject};
  MouseMovementController mouseController{viewerObject};

  float lastTime = static_cast<float>(glfwGetTime());
  float nowTime = 0.0f;
  float deltaTime = 0.0f;
  float accumulator = 0.0f;
  const float tickrate = 1.0f / 512; // 512 ticks per second

  float aspectRatio = vlknRenderer.getAspectRatio();

  while (!vlknWindow.shouldClose() && !keyboardController.shouldClose()) {
    glfwPollEvents();

    nowTime = static_cast<float>(glfwGetTime());
    deltaTime = nowTime - lastTime;
    accumulator += deltaTime;
    lastTime = nowTime;

    while (accumulator >
           tickrate + std::numeric_limits<decltype(tickrate)>::epsilon()) {
      keyboardController.move(tickrate);
      accumulator -= tickrate;
    }

    aspectRatio = vlknRenderer.getAspectRatio();

    mouseController.lookAround();

    keyboardController.lookAt(gameObjects);

    camera.setViewYXZ(viewerObject.transform.translation,
                      viewerObject.transform.rotation);

    camera.setPerspectiveProjection(mouseController.getFov(), aspectRatio, 0.1f,
                                    100.0f);

    if (auto commandBuffer = vlknRenderer.beginFrame()) {
      std::uint32_t frameIndex = vlknRenderer.getFrameIndex();
      FrameInfo frameInfo{
          .frameIndex = frameIndex,
          .frameDelta = deltaTime,
          .frameTime = nowTime,
          .commandBuffer = commandBuffer,
          .camera = camera,
          .globalDescriptorSet = globalDescriptorSets[frameIndex],
          .gameObjects = gameObjects,
      };

      // update stage
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      ubo.inverseView = camera.getInverseView();
      pointLightSystem.update(frameInfo, imguiSystem.getPointLightColor(), ubo);
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render stage
      vlknRenderer.beginSwapChainRenderPass(commandBuffer);

      renderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo, imguiSystem.getPointLightColor());
      imguiSystem.render(frameInfo);

      vlknRenderer.endSwapChainRenderPass(commandBuffer);
      vlknRenderer.endFrame();
    }
  }

  vkDeviceWaitIdle(vlknDevice.device());
}

void App::loadGameObjects() {
  std::shared_ptr<VlknModel> flatVaseModel =
      VlknModel::createModelFromFile(vlknDevice, "models/flat_vase.obj");

  std::shared_ptr<VlknModel> smoothVaseModel =
      VlknModel::createModelFromFile(vlknDevice, "models/smooth_vase.obj");

  std::shared_ptr<VlknModel> floorModel =
      VlknModel::createModelFromFile(vlknDevice, "models/quad.obj");

  VlknGameObject flatVase = VlknGameObject::createGameObject();
  flatVase.model = flatVaseModel;
  flatVase.transform.translation = {-1.0f, 0.0f, 0.0f};
  flatVase.transform.scale = glm::vec3(3.0f, 2.0f, 3.0f);

  gameObjects.emplace(flatVase.getId(), std::move(flatVase));

  VlknGameObject smoothVase = VlknGameObject::createGameObject();
  smoothVase.model = smoothVaseModel;
  smoothVase.transform.translation = {1.0f, 0.0f, 0.0f};
  smoothVase.transform.scale = glm::vec3(4.0f);

  gameObjects.emplace(smoothVase.getId(), std::move(smoothVase));

  VlknGameObject floor = VlknGameObject::createGameObject();
  floor.model = floorModel;
  floor.transform.translation = {0.0f, 0.0f, 0.0f};
  floor.transform.scale = glm::vec3(16.0f, 1.0f, 16.0f);

  gameObjects.emplace(floor.getId(), std::move(floor));

  std::array<glm::vec3, 7> rainbowColors = {
      glm::vec3(1.0f, 0.0f, 0.0f), // Red
      glm::vec3(1.0f, 0.5f, 0.0f), // Orange
      glm::vec3(1.0f, 1.0f, 0.0f), // Yellow
      glm::vec3(0.0f, 1.0f, 0.0f), // Green
      glm::vec3(0.0f, 0.0f, 1.0f), // Blue
      glm::vec3(0.5f, 0.0f, 1.0f), // Indigo
      glm::vec3(0.9f, 0.0f, 0.9f)  // Violet
  };

  for (std::size_t i = 0; i < MAX_LIGHTS; i++) {
    VlknGameObject pointLight = VlknGameObject::makePointLight(0.1f);

    // Calculate the index for the rainbow colors
    float t = static_cast<float>(i) / (MAX_LIGHTS - 1); // Normalize i to [0, 1]
    std::size_t colorIndex =
        static_cast<std::size_t>(t * (rainbowColors.size() - 1));
    std::size_t nextColorIndex = (colorIndex + 1) % rainbowColors.size();

    // Interpolate between the two colors
    float blendFactor = (t * (rainbowColors.size() - 1)) - colorIndex;
    pointLight.color = glm::mix(rainbowColors[colorIndex],
                                rainbowColors[nextColorIndex], blendFactor);

    glm::mat4 rotateLight =
        glm::rotate(glm::mat4(1.0f), i * glm::two_pi<float>() / MAX_LIGHTS,
                    {0.0f, -1.0f, 0.0f});
    pointLight.transform.translation =
        glm::vec3(rotateLight * glm::vec4(-1.0f, -2.0f, -1.0f, 1.0f));

    gameObjects.emplace(pointLight.getId(), std::move(pointLight));
  }
}

} // namespace vlkn
