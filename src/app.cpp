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

    // look at cube
    // see docs/look_at_rotation_vector
    if (keyboardController.isKeyPressed(GLFW_KEY_1)) {
      glm::vec3 direction =
          glm::normalize(gameObjects[0].transform.translation -
                         viewerObject.transform.translation);

      float yaw = std::atan2(direction.x, direction.z);

      float pitch = -std::asin(direction.y);

      pitch = glm::clamp(pitch, glm::radians(-89.0f), glm::radians(89.0f));

      yaw = glm::mod(yaw, glm::two_pi<decltype(yaw)>());

      viewerObject.transform.rotation = glm::vec3(pitch, yaw, 0.0f);
    }

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

std::unique_ptr<VlknModel> createCubeModel(VlknDevice &device,
                                           glm::vec3 offset) {
  std::vector<VlknModel::Vertex> vertices{

      // left face (white)
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, .5f}, {.9f, .9f, .9f}},
      {{-.5f, -.5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, -.5f}, {.9f, .9f, .9f}},
      {{-.5f, .5f, .5f}, {.9f, .9f, .9f}},

      // right face (yellow)
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, .5f}, {.8f, .8f, .1f}},
      {{.5f, -.5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .8f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .8f, .1f}},

      // top face (orange, remember y axis points down)
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, .5f}, {.9f, .6f, .1f}},
      {{-.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, -.5f}, {.9f, .6f, .1f}},
      {{.5f, -.5f, .5f}, {.9f, .6f, .1f}},

      // bottom face (red)
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, .5f}, {.8f, .1f, .1f}},
      {{-.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, -.5f}, {.8f, .1f, .1f}},
      {{.5f, .5f, .5f}, {.8f, .1f, .1f}},

      // nose face (blue)
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, .5f, 0.5f}, {.1f, .1f, .8f}},
      {{-.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, -.5f, 0.5f}, {.1f, .1f, .8f}},
      {{.5f, .5f, 0.5f}, {.1f, .1f, .8f}},

      // tail face (green)
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, .5f, -0.5f}, {.1f, .8f, .1f}},
      {{-.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, -.5f, -0.5f}, {.1f, .8f, .1f}},
      {{.5f, .5f, -0.5f}, {.1f, .8f, .1f}},

  };

  for (VlknModel::Vertex &v : vertices) {
    v.position += offset;
  }

  return std::make_unique<VlknModel>(device, vertices);
}

void App::loadGameObjects() {
  std::shared_ptr<VlknModel> cubeModel =
      createCubeModel(vlknDevice, {0.0f, 0.0f, 0.0f});

  VlknGameObject cube = VlknGameObject::createGameObject();
  cube.model = cubeModel;
  cube.transform.translation = {0.0f, 0.0f, 2.0f};
  cube.transform.scale = {0.5f, 0.5f, 0.5f};

  gameObjects.push_back(std::move(cube));
}

} // namespace vlkn
