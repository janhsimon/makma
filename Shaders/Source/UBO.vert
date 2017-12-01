#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform WM
{
    mat4 worldMatrix;
} wm;

layout(set = 2, binding = 0) uniform VPM
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
} vpm;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outTexCoord;

void main()
{
    gl_Position = vpm.projectionMatrix * vpm.viewMatrix * wm.worldMatrix * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
}