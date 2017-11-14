#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 0) uniform sampler2D diffuseSampler;
layout(binding = 1) uniform sampler2D normalSampler;

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
    outColor = texture(diffuseSampler, fragTexCoord) * (texture(normalSampler, fragTexCoord) * 0.75);
}