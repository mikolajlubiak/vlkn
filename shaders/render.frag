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
  mat4 inverseView;
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
  vec3 specularLight = vec3(0.0);
  vec3 surfaceNormal = normalize(fragNormalWorld);

  vec3 cameraPosWorld = ubo.inverseView[3].xyz;
  vec3 viewDirection = normalize(cameraPosWorld - fragPosWorld);

  for (uint i = 0; i < ubo.lightsNum; i++) {
    PointLight light = ubo.pointLights[i];

    vec3 directionToLight = light.position.xyz - fragPosWorld;
    vec3 normDirectionToLight = normalize(directionToLight);
    float attenuation = 1.0 / dot(directionToLight, directionToLight);
    float cosAngleIncidence = max(dot(surfaceNormal, normDirectionToLight), 0.0);

    vec3 lightContribution = light.color.xyz * light.color.w * attenuation;

    // diffuse
    diffuseLight += lightContribution * cosAngleIncidence;

    // specular
    vec3 halfAngle = normalize(normDirectionToLight + viewDirection);
    float blinnTerm = clamp(dot(surfaceNormal, halfAngle), 0.0, 1.0);
    blinnTerm = pow(blinnTerm, 512.0);
    specularLight += lightContribution * blinnTerm;
  }

  outColor = vec4((diffuseLight + specularLight) * fragColor, 1.0);
}
