#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0) uniform Geometry { mat4 worldMatrix; } geometry;
layout(set = 1) uniform ShadowMapCascade { mat4 viewProjectionMatrices[4]; } shadowMapCascade;

layout(push_constant) uniform ShadowMapCascadeIndex { uint index; } shadowMapCascadeIndex;

layout(location = 0) in vec3 inPosition;

void main()
{
	gl_Position = shadowMapCascade.viewProjectionMatrices[shadowMapCascadeIndex.index] * geometry.worldMatrix * vec4(inPosition, 1.0);
}