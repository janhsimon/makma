#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform WM
{
	mat4 worldMatrix;
} wm;

layout(set = 1, binding = 0) uniform VPM
{
	mat4 viewProjectionMatrix;
} vpm;

layout(location = 0) in vec3 inPosition;

void main()
{
	gl_Position = vpm.viewProjectionMatrix * wm.worldMatrix * vec4(inPosition, 1.0);
}