#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 2, binding = 0) uniform WVPM
{
	mat4 worldViewProjectionMatrix;
} wvpm;

layout(location = 0) in vec3 inPosition;

void main()
{
	gl_Position = wvpm.worldViewProjectionMatrix * vec4(inPosition, 1.0);
}