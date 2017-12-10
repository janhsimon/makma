#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D positionSampler;
layout(set = 0, binding = 1) uniform sampler2D albedoSampler;
layout(set = 0, binding = 2) uniform sampler2D normalSampler;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
  vec4 albedo = texture(albedoSampler, inTexCoord);
  vec3 normal = texture(normalSampler, inTexCoord).rgb;
  
  outColor = albedo * dot(normal, normalize(vec3(1.0, -0.5, 1.0)));
}