#pragma once

#include "vlkn_model.hpp"

#include <cstdint>
#include <memory>

namespace vlkn {

struct Transform2DComponent {
  glm::vec2 translation{};
  glm::vec2 scale{1.0f, 1.0f};
  float rotation;

  glm::mat2 mat2() {
    const float s = glm::sin(rotation);
    const float c = glm::cos(rotation);
    glm::mat2 rotationMat{{c, s}, {-s, c}};

    glm::mat2 scaleMat{{scale.x, 0.0f}, {0.0f, scale.y}};

    return rotationMat * scaleMat;
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
  Transform2DComponent transform2D{};

private:
  VlknGameObject(id_t objId) : id(objId){};
  id_t id;
};

} // namespace vlkn
