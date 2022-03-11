#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform UniformBufferObject {
  mat4 mvp;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in uint inTexId;
layout(location = 3) in uint inTintIndex;

layout(location = 0) out vec2 fragTexCoord;
layout(location = 1) flat out uint fragTexId;
layout(location = 2) out vec4 fragColorMod;

void main() {
  gl_Position = ubo.mvp * vec4(inPosition, 1.0);
  fragTexCoord = inTexCoord;
  fragTexId = inTexId;
  fragColorMod = vec4(1, 1, 1, 1);

  // TODO: Remove this and sample biome in fragment shader
  // Jungle tints
  uint tintindex = inTintIndex & 0xFFFF;
  uint ao = inTintIndex >> 16;
  if (tintindex == 0) {
    fragColorMod = vec4(0.34, 0.78, 0.235, 1.0);
  } else if (tintindex == 1) {
    fragColorMod = vec4(0.188, 0.733, 0.043, 1.0);
  } else if (tintindex == 50) {
    fragColorMod = vec4(0.05, 0.3, 0.85, 1.0);
  }

  fragColorMod.rgb *= (0.25 + float(ao) * 0.25) * 0.675;
}
