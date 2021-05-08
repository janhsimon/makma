#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 2, binding = 0) uniform sampler2D inDiffuseSampler;
layout(set = 2, binding = 1) uniform sampler2D inNormalSampler;
layout(set = 2, binding = 2) uniform sampler2D inMetallicSampler;
layout(set = 2, binding = 3) uniform sampler2D inRoughnessSampler;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;

layout(location = 0) out vec4 outAlbedoMetallic;
layout(location = 1) out vec4 outNormalRoughness;

void main()
{
  vec4 albedo = texture(inDiffuseSampler, inTexCoord);
  
	if (albedo.a < 0.5)
	{
		discard;
	}
	
	// calculate normal in tangent space
	mat3 TBN = mat3(inTangent, inBitangent, inNormal);
	vec3 normal = TBN * normalize(texture(inNormalSampler, inTexCoord).rgb * 2.0 - vec3(1.0));
	normal = (normal + vec3(1.0)) * 0.5;
	
	// albedo and metallic
	outAlbedoMetallic = vec4(albedo.rgb, texture(inMetallicSampler, inTexCoord).r);
	
	// world-space normal and roughness
	outNormalRoughness = vec4(normal, texture(inRoughnessSampler, inTexCoord).r);
}