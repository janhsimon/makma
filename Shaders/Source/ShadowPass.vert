#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (constant_id = 0) const int SHADOW_MAP_CASCADE_COUNT = 6;

layout(set = 0, binding = 0) uniform Geometry { mat4 worldMatrix; } geometry;
layout(set = 1, binding = 0) uniform ShadowMapCascade { mat4 viewProjectionMatrices[SHADOW_MAP_CASCADE_COUNT]; } shadowMapCascades;

layout(push_constant) uniform ShadowMapCascadeIndex { uint index; } shadowMapCascadeIndex;

layout(location = 0) in vec3 inPosition;

void main()
{
	gl_Position = shadowMapCascades.viewProjectionMatrices[shadowMapCascadeIndex.index] * geometry.worldMatrix * vec4(inPosition, 1.0);
}