#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(push_constant) uniform PC
{
	mat4 worldMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outTexCoord;

void main()
{
    gl_Position = pc.projectionMatrix * pc.viewMatrix * pc.worldMatrix * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
}