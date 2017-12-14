#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D diffuseSampler;
layout(set = 0, binding = 1) uniform sampler2D normalSampler;
layout(set = 0, binding = 2) uniform sampler2D occlusionSampler;

layout(location = 0) in vec3 inWorldPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec4 outWorldPosition;
layout(location = 1) out vec4 outAlbedo;
layout(location = 2) out vec4 outNormal;

void main()
{
	if (texture(diffuseSampler, inTexCoord).a < 0.5)
	{
		discard;
	}
		
	outWorldPosition = vec4(inWorldPosition, 1.0);
	outAlbedo = vec4(texture(diffuseSampler, inTexCoord).rgb, texture(occlusionSampler, inTexCoord).r);
  
	// calculate normal in tangent space
	vec3 N = normalize(inNormal);
	N.y = -N.y;
	vec3 T = normalize(inTangent);
	vec3 B = cross(N, T);
	mat3 TBN = mat3(T, B, N);
	outNormal = vec4(TBN * normalize(texture(normalSampler, inTexCoord).xyz * 2.0 - vec3(1.0)), 1.0);
}