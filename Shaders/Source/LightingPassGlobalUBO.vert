#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0) uniform LWCVPM { mat4 lightWorldCameraViewProjectionMatrix; } lwcvpm;

layout(set = 1) uniform UBO
{
  mat4 cameraViewProjectionMatrix;
	mat4 globals;
} ubo;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outEyePosition;
layout(location = 1) out vec2 outScreenSize;

void main()
{
	gl_Position = lwcvpm.lightWorldCameraViewProjectionMatrix * vec4(inPosition, 1.0);
	
	outEyePosition = ubo.globals[0].xyz;
	outScreenSize = ubo.globals[1].xy;
}