#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0) uniform Light { mat4 worldMatrix; } light;

layout(set = 1) uniform Camera
{
  mat4 viewProjectionMatrix;
  vec3 position;
  vec3 forward;
} camera;

layout(set = 4) uniform LightData { mat4 data; } lightData;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outEyePosition;
layout(location = 1) out vec3 outViewRay;
layout(location = 2) out vec3 outEyeForward;

void main()
{
  outEyePosition = camera.position;
  outEyeForward = camera.forward;
  
  const float lightType = lightData.data[0].w;
  if (lightType < 0.5)
  // directional light
  {
    vec4 position = inverse(camera.viewProjectionMatrix) * vec4(inPosition, 1.0);
    position /= position.w;
    outViewRay = position.xyz - camera.position;
    gl_Position = vec4(inPosition, 1.0);
  }
  else
  // point or spotlight
  {
    vec4 position = light.worldMatrix * vec4(inPosition, 1.0);
    outViewRay = position.xyz - camera.position;
    gl_Position = camera.viewProjectionMatrix * position;
  }
}