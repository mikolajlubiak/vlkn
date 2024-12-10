// header
#include "app.hpp"

// local
#include "keyboard_movement_controller.hpp"
#include "mouse_movement_controller.hpp"
#include "systems/point_light_system.hpp"
#include "systems/render_system.hpp"
#include "vlkn_buffer.hpp"
#include "vlkn_camera.hpp"
#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_model.hpp"
#include "vlkn_renderer.hpp"

// libs
#include <GLFW/glfw3.h>
#include <glm/detail/qualifier.hpp>
#include <glm/fwd.hpp>
#include <glm/gtc/constants.hpp>
#include <vulkan/vulkan_core.h>

// std
#include <bits/fs_fwd.h>
#include <memory>
#include <utility>
#include <vector>

namespace vlkn {

struct GlobalUbo {
  alignas(16) glm::mat4 projection{1.0f};
  alignas(16) glm::mat4 view{1.0f};
  alignas(16) glm::vec4 ambientLightColor{1.0f, 1.0f, 1.0f, 0.02f};
  alignas(16) glm::vec3 lightPosition{-1.0f};
  alignas(16) glm::vec4 lightColor{1.0f};
};

App::App() {
  globalPool = VlknDescriptorPool::Builder(vlknDevice)
                   .setMaxSets(VlknSwapChain::MAX_FRAMES_IN_FLIGHT)
                   .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                VlknSwapChain::MAX_FRAMES_IN_FLIGHT)
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

  auto globalSetLayout =
      VlknDescriptorSetLayout::Builder(vlknDevice)
          .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                      VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
          .build();

  std::vector<VkDescriptorSet> globalDescriptorSets(
      VlknSwapChain::MAX_FRAMES_IN_FLIGHT);

  for (std::size_t i = 0; i < globalDescriptorSets.size(); i++) {
    auto bufferInfo = uboBuffers[i]->descriptorInfo();
    VlknDescriptorWriter(*globalSetLayout, *globalPool)
        .writeBuffer(0, &bufferInfo)
        .build(globalDescriptorSets[i]);
  }

  RenderSystem renderSystem{vlknDevice, vlknRenderer.getSwapChainRenderPass(),
                            globalSetLayout->getDescriptorSetLayout()};

  PointLightSystem pointLightSystem{vlknDevice,
                                    vlknRenderer.getSwapChainRenderPass(),
                                    globalSetLayout->getDescriptorSetLayout()};

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
          .frameTime = deltaTime,
          .commandBuffer = commandBuffer,
          .camera = camera,
          .globalDescriptorSet = globalDescriptorSets[frameIndex],
          .gameObjects = gameObjects,
      };

      // update stage
      GlobalUbo ubo{};
      ubo.projection = camera.getProjection();
      ubo.view = camera.getView();
      uboBuffers[frameIndex]->writeToBuffer(&ubo);
      uboBuffers[frameIndex]->flush();

      // render stage
      vlknRenderer.beginSwapChainRenderPass(commandBuffer);
      renderSystem.renderGameObjects(frameInfo);
      pointLightSystem.render(frameInfo);
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
}

} // namespace vlkn
