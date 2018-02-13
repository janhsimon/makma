#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0) uniform Light { mat4 lightWorldCameraViewProjectionMatrix; } light;

layout(set = 1) uniform Camera
{
  mat4 viewProjectionMatrix;
  vec3 position;
} camera;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outEyePosition;

void main()
{
  gl_Position = light.lightWorldCameraViewProjectionMatrix * vec4(inPosition, 1.0);

  outEyePosition = camera.position;
}