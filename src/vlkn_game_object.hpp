#pragma once

#include "vlkn_model.hpp"

#include <cstdint>
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

namespace vlkn {

struct TransformComponent {
  glm::vec3 translation{};
  glm::vec3 scale{1.0f, 1.0f, 1.0f};
  glm::vec3 rotation{};

  glm::mat4 mat4() {
    // Rotation: YXZ Tait–Bryan angles
    // Scale & Translation: XYZ
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    return glm::mat4{{
                         scale.x * (c1 * c3 + s1 * s2 * s3),
                         scale.x * (c2 * s3),
                         scale.x * (c1 * s2 * s3 - c3 * s1),
                         0.0f,
                     },
                     {
                         scale.y * (c3 * s1 * s2 - c1 * s3),
                         scale.y * (c2 * c3),
                         scale.y * (c1 * c3 * s2 + s1 * s3),
                         0.0f,
                     },
                     {
                         scale.z * (c2 * s1),
                         scale.z * (-s2),
                         scale.z * (c1 * c2),
                         0.0f,
                     },
                     {translation.x, translation.y, translation.z, 1.0f}};
  }
};

class VlknGameObject {
public:
  using id_t = uint32_t;

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
