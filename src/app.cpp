// header
#include "app.hpp"

// local
#include "keyboard_movement_controller.hpp"
#include "mouse_movement_controller.hpp"
#include "render_system.hpp"
#include "vlkn_camera.hpp"
#include "vlkn_device.hpp"
#include "vlkn_game_object.hpp"
#include "vlkn_model.hpp"
#include "vlkn_renderer.hpp"
#include "vlkn_utils.hpp"

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

App::App() {
  gameObjects.reserve(16);
  loadGameObjects();
}

App::~App() {}

void App::run() {
  RenderSystem renderSystem{vlknDevice, vlknRenderer.getSwapChainRenderPass()};
  VlknCamera camera{};
  VlknGameObject viewerObject = VlknGameObject::createGameObject();

  camera.setViewTarget(glm::vec3(0.0f, 0.0f, 0.0f),
                       gameObjects[0].transform.translation);

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

    aspectRatio = vlknRenderer.getAspectRatio();

    mouseController.lookAround();

    while (accumulator >
           tickrate + std::numeric_limits<decltype(tickrate)>::epsilon()) {
      keyboardController.move(tickrate);
      accumulator -= tickrate;
    }

    keyboardController.lookAt(gameObjects);

    camera.setViewYXZ(viewerObject.transform.translation,
                      viewerObject.transform.rotation);

    camera.setPerspectiveProjection(mouseController.getFov(), aspectRatio, 0.1f,
                                    100.0f);

    if (auto commandBuffer = vlknRenderer.beginFrame()) {

      vlknRenderer.beginSwapChainRenderPass(commandBuffer);
      renderSystem.renderGameObjects(commandBuffer, gameObjects, camera);
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

  VlknGameObject flatVase = VlknGameObject::createGameObject();
  flatVase.model = flatVaseModel;
  flatVase.transform.translation = {0.0f, 0.0f, 2.0f};
  flatVase.transform.scale = glm::vec3(3.0f, 2.0f, 3.0f);

  gameObjects.push_back(std::move(flatVase));

  VlknGameObject smoothVase = VlknGameObject::createGameObject();
  smoothVase.model = smoothVaseModel;
  smoothVase.transform.translation = {1.0f, 0.0f, 2.0f};
  smoothVase.transform.scale = glm::vec3(3.0f);

  gameObjects.push_back(std::move(smoothVase));
}

} // namespace vlkn
