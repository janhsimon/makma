#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(set = 0, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 0, binding = 1) uniform sampler2D normalSampler;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(diffuseSampler, inTexCoord) /*(texture(normalSampler, inTexCoord) * 0.75)*/;
}