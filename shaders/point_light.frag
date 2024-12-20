#version 450

layout (location = 0) in vec2 fragOffset;
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
  PointLight pointLights[16];
  uint lightsNum;
} ubo;

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout (push_constant) uniform Push {
  vec4 position;
  vec4 color;
} push;

const float PI = 3.14;

void main() {
  float distance = dot(fragOffset, fragOffset);

  if (distance > 1.0) {
    discard;
  }

  float distanceCosine = 0.5 * (cos(sqrt(distance) * PI) + 0.5);

  outColor = vec4(push.color.xyz * push.color.w + pow(distanceCosine, 8.0), distanceCosine);
}
