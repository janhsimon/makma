#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 1, binding = 0) uniform UBO
{
    mat4 worldMatrix;
    mat4 viewMatrix;
    mat4 projectionMatrix;
} ubo;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;

layout(location = 0) out vec2 outTexCoord;

void main()
{
    gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.worldMatrix * vec4(inPosition, 1.0);
    outTexCoord = inTexCoord;
}