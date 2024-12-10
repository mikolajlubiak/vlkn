// header
#include "vlkn_game_object.hpp"

namespace vlkn {

glm::mat4 TransformComponent::mat4() {
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

glm::mat3 TransformComponent::normalMatrix() {
  // Rotation: YXZ Tait–Bryan angles
  // Scale & Translation: XYZ
  const float c3 = glm::cos(rotation.z);
  const float s3 = glm::sin(rotation.z);
  const float c2 = glm::cos(rotation.x);
  const float s2 = glm::sin(rotation.x);
  const float c1 = glm::cos(rotation.y);
  const float s1 = glm::sin(rotation.y);
  const glm::vec3 inverseScale = 1.0f / scale;

  return glm::mat3{
      {
          inverseScale.x * (c1 * c3 + s1 * s2 * s3),
          inverseScale.x * (c2 * s3),
          inverseScale.x * (c1 * s2 * s3 - c3 * s1),
      },
      {
          inverseScale.y * (c3 * s1 * s2 - c1 * s3),
          inverseScale.y * (c2 * c3),
          inverseScale.y * (c1 * c3 * s2 + s1 * s3),
      },
      {
          inverseScale.z * (c2 * s1),
          inverseScale.z * (-s2),
          inverseScale.z * (c1 * c2),
      },
  };
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
