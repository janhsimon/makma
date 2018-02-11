#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0) uniform Light { mat4 worldMatrix; } light;

layout(set = 1) uniform UBO
{
  mat4 cameraViewMatrix;
  mat4 cameraProjectionMatrix;
  mat4 globals;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outEyePosition;
layout(location = 1) out vec2 outScreenSize;
layout(location = 2) out vec3 outViewPosition;

void main()
{
	vec4 viewPosition = ubo.cameraViewMatrix * light.worldMatrix * vec4(inPosition, 1.0);
  outViewPosition = viewPosition.xyz;
	gl_Position = ubo.cameraProjectionMatrix * viewPosition;
	
	outEyePosition = ubo.globals[0].xyz;
	outScreenSize = ubo.globals[1].xy;
}