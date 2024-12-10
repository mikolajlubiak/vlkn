#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;

layout (location = 0) out vec4 outColor;

struct PointLight {
  vec4 position;
  vec4 color;
};

layout(set = 0, binding = 0) uniform GlobalUbo {
  mat4 projection;
  mat4 view;
  vec4 ambientLightColor;
  PointLight pointLights[10];
  uint lightsNum;
} ubo;


layout(push_constant) uniform Push {
  mat4 modelMatrix;
  mat4 normalMatrix;
} push;

void main() {
  vec3 diffuseLight = ubo.ambientLightColor.xyz * ubo.ambientLightColor.w;
  vec3 surfaceNormal = normalize(fragNormalWorld);

  for (uint i = 0; i < ubo.lightsNum; i++) {
    PointLight light = ubo.pointLights[i];

    vec3 directionToLight = light.position.xyz - fragPosWorld;
    float attenuation = 1.0 / dot(directionToLight, directionToLight);
    float cosAngleIncidence = max(dot(surfaceNormal, normalize(directionToLight)), 0.0);

    vec3 lightContribution = light.color.xyz * light.color.w * attenuation;
    diffuseLight += lightContribution * cosAngleIncidence;
  }

  outColor = vec4(diffuseLight * fragColor, 1.0);
}
