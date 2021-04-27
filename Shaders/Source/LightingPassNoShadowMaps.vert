#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform Light { mat4 worldMatrix; } light;

layout(set = 1, binding = 0) uniform Camera
{
  mat4 viewProjectionMatrix;
  vec4 positionNearClip;
  vec4 forwardFarClip;
} camera;

layout(set = 3, binding = 0) uniform LightData { mat4 data; } lightData;

layout(location = 0) in vec3 inPosition;

layout(location = 0) out vec3 outEyePosition;
layout(location = 1) out vec3 outViewRay;
layout(location = 2) out vec3 outEyeForward;
layout(location = 3) out vec2 outCameraClip;

void main()
{
  outEyePosition = camera.positionNearClip.xyz;
  outEyeForward = camera.forwardFarClip.xyz;
  outCameraClip = vec2(camera.positionNearClip.w, camera.forwardFarClip.w);
  
  const float lightType = lightData.data[0].w;
  if (lightType < 0.5)
  // directional light
  {
    vec4 position = inverse(camera.viewProjectionMatrix) * vec4(inPosition, 1.0);
    position /= position.w;
    outViewRay = position.xyz - camera.positionNearClip.xyz;
    gl_Position = vec4(inPosition, 1.0);
  }
  else
  // point or spotlight
  {
    vec4 position = light.worldMatrix * vec4(inPosition, 1.0);
    outViewRay = position.xyz - camera.positionNearClip.xyz;
    gl_Position = camera.viewProjectionMatrix * position;
  }
}