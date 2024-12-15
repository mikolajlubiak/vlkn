// header
#include "vlkn_camera.hpp"

// local
#include "vlkn_utils.hpp"

// std
#include <cassert>
#include <limits>

namespace vlkn {

void VlknCamera::setOrthographicProjection(float left, float right, float top,
                                           float bottom, float near,
                                           float far) {
  projectionMatrix = glm::ortho(left, right, bottom, top, near, far);
}

void VlknCamera::setPerspectiveProjection(float fovy, float aspect, float near,
                                          float far) {
  assert(glm::abs(aspect) >
         std::numeric_limits<decltype(glm::abs(aspect))>::epsilon());
  projectionMatrix = glm::perspective(fovy, aspect, near, far);
}

void VlknCamera::setViewDirection(glm::vec3 position, glm::vec3 direction,
                                  glm::vec3 up) {
  assert(nonZeroVector(direction));

  const glm::vec3 w{glm::normalize(direction)};
  const glm::vec3 u{glm::normalize(glm::cross(w, up))};
  const glm::vec3 v{glm::cross(w, u)};

  // Create a rotation matrix from the basis vectors
  glm::mat4 rotationMatrix =
      glm::mat4(glm::vec4(u, 0.0f), glm::vec4(v, 0.0f), glm::vec4(w, 0.0f),
                glm::vec4(0.0f, 0.0f, 0.0f, 1.0f));

  // Create a translation matrix from the position
  glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

  // Create the inverse view matrix by combining the rotation and translation
  // matrices
  inverseViewMatrix = translationMatrix * rotationMatrix;

  // Create view matrix by... inversing the *inverse* view matrix
  viewMatrix = glm::inverse(inverseViewMatrix);
}

void VlknCamera::setViewTarget(glm::vec3 position, glm::vec3 target,
                               glm::vec3 up) {
  setViewDirection(position, target - position, up);
}

void VlknCamera::setViewYXZ(glm::vec3 position, const glm::quat &rotation) {
  // Create a rotation matrix from the quaternion
  glm::mat4 rotationMatrix = glm::toMat4(rotation);

  // Create a translation matrix from the position
  glm::mat4 translationMatrix = glm::translate(glm::mat4(1.0f), position);

  // Create the inverse view matrix by combining the rotation and translation
  // matrices
  inverseViewMatrix = translationMatrix * rotationMatrix;

  // Create view matrix by... inversing the *inverse* view matrix
  viewMatrix = glm::inverse(inverseViewMatrix);
}

} // namespace vlkn