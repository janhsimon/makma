#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 0, binding = 1) uniform sampler2D normalSampler;

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;

layout(location = 0) out vec4 outWorldPosition;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormal;

void main()
{
  outWorldPosition = vec4(inWorldPosition, 1.0);
  outAlbedo = texture(diffuseSampler, inTexCoord);
  outNormal = vec4(inNormal * texture(normalSampler, inTexCoord).rgb, 1.0);
}