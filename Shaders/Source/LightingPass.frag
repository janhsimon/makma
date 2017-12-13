#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D positionSampler;
layout(set = 0, binding = 1) uniform sampler2D albedoSampler;
layout(set = 0, binding = 2) uniform sampler2D normalSampler;

layout(set = 1, binding = 0) uniform LightData
{
	vec4 directionalLightDirection[4];
	vec4 directionalLightColor[4];
} lightData;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
	vec3 albedo = texture(albedoSampler, inTexCoord).rgb;
	vec3 normal = normalize(texture(normalSampler, inTexCoord).rgb);
  
	vec3 directionalLight = vec3(0.0, 0.0, 0.0);
	for (int i = 0; i < 4; ++i)
	{
		directionalLight += max(0.0, dot(normal, normalize(lightData.directionalLightDirection[i].xyz))) * lightData.directionalLightColor[i].rgb;
	}
  
	float occlusion = 1.0 - texture(albedoSampler, inTexCoord).a;
  
	outColor = vec4(directionalLight * albedo - occlusion, 1.0);
}