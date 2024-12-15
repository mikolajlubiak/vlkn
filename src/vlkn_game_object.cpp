// header
#include "vlkn_game_object.hpp"

namespace vlkn {

glm::mat4 TransformComponent::mat4() {
  // Start with an identity matrix
  glm::mat4 transformationMatrix = glm::mat4(1.0f);

  // Apply scaling
  transformationMatrix = glm::scale(transformationMatrix, scale);

  // Apply rotation
  transformationMatrix *= glm::toMat4(rotation);

  // Apply translation
  transformationMatrix = glm::translate(transformationMatrix, translation);

  // Return the final transformation matrix
  return transformationMatrix;
}

glm::mat3 TransformComponent::normalMatrix() {
  // Extract the upper-left 3x3 part of the model-view matrix
  glm::mat4 normalMatrix = glm::mat3(mat4());

  // Compute the inverse of the normal matrix
  normalMatrix = glm::inverse(normalMatrix);

  // Transpose the inverse to get the normal matrix
  normalMatrix = glm::transpose(normalMatrix);

  return normalMatrix;
}

VlknGameObject VlknGameObject::makePointLight(float intensity, float radius,
                                              glm::vec3 color) {
  VlknGameObject gameObj = VlknGameObject::createGameObject();
  gameObj.color = color;
  gameObj.transform.scale.x = radius;
  gameObj.pointLight = std::make_unique<PointLightComponent>(intensity);

  return gameObj;
}

} // namespace vlkn
