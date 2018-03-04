#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 2, binding = 0) uniform sampler2D inDiffuseSampler;
layout(set = 2, binding = 1) uniform sampler2D inNormalSampler;
layout(set = 2, binding = 2) uniform sampler2D inOcclusionSampler;
layout(set = 2, binding = 3) uniform sampler2D inMetallicSampler;
layout(set = 2, binding = 4) uniform sampler2D inRoughnessSampler;

layout(location = 0) in vec2 inTexCoord;
layout(location = 1) in vec3 inNormal;
layout(location = 2) in vec3 inTangent;
layout(location = 3) in vec3 inBitangent;

layout(location = 0) out vec4 outGBuffer0;
layout(location = 1) out vec4 outGBuffer1;

vec2 encodeNormal(vec3 n)
{
    return vec2(n.xy * 0.5 + 0.5);
}

void main()
{
  vec4 albedo = texture(inDiffuseSampler, inTexCoord);
  
	if (albedo.a < 0.5)
	{
		discard;
	}
	
	// calculate normal in tangent space
	mat3 TBN = mat3(inTangent, inBitangent, inNormal);
	vec2 normal = encodeNormal(TBN * normalize(texture(inNormalSampler, inTexCoord).xyz * 2.0 - vec3(1.0)));
	
	// albedo rgb and occlusion
	outGBuffer0 = vec4(albedo.rgb, texture(inOcclusionSampler, inTexCoord).r);
  
  // world-space normal xy, metallic and roughness
	outGBuffer1 = vec4(normal.xy, texture(inMetallicSampler, inTexCoord).r, texture(inRoughnessSampler, inTexCoord).r);
}