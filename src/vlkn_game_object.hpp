#pragma once

// local
#include "vlkn_model.hpp"

// lib
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>

// std
#include <cstdint>
#include <memory>
#include <unordered_map>

namespace vlkn {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  glm::vec3 rotation{};

  glm::mat4 mat4();
  glm::mat3 normalMatrix();
};

class VlknGameObject {
public:
  using id_t = uint32_t;
  using Map = std::unordered_map<id_t, VlknGameObject>;

  static VlknGameObject createGameObject() {
    static id_t current_id = 0;
    return VlknGameObject{current_id++};
  }

  VlknGameObject(const VlknGameObject &) = delete;
  VlknGameObject &operator=(const VlknGameObject &) = delete;
  VlknGameObject(VlknGameObject &&) = default;
  VlknGameObject &operator=(VlknGameObject &&) = default;

  id_t getId() const { return id; }

  std::shared_ptr<VlknModel> model;
  glm::vec3 color{};
  TransformComponent transform{};

private:
  VlknGameObject(id_t objId) : id(objId) {};
  id_t id;
};

} // namespace vlkn
