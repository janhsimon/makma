#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0) uniform Geometry { mat4 worldMatrix; } geometry;

layout(set = 1) uniform Camera
{
  mat4 viewProjectionMatrix;
  vec3 position;
} camera;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec2 outTexCoord;
layout(location = 1) out vec3 outNormal;
layout(location = 2) out vec3 outTangent;
layout(location = 3) out vec3 outBitangent;

void main()
{
  gl_Position = camera.viewProjectionMatrix * geometry.worldMatrix * vec4(inPosition, 1.0);

  outTexCoord = inTexCoord;
  
	outNormal = (geometry.worldMatrix * vec4(inNormal, 0.0)).xyz;	
	outTangent = (geometry.worldMatrix * vec4(inTangent, 0.0)).xyz;
	outBitangent = (geometry.worldMatrix * vec4(inBitangent, 0.0)).xyz;
}