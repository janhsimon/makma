#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D inDiffuseSampler;
layout(set = 0, binding = 1) uniform sampler2D inNormalSampler;
layout(set = 0, binding = 2) uniform sampler2D inOcclusionSampler;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec4 outGBuffer0;
layout(location = 1) out vec4 outGBuffer1;
layout(location = 2) out vec4 outGBuffer2;

void main()
{
  vec4 albedo = texture(inDiffuseSampler, inTexCoord);
  
	if (albedo.a < 0.5)
	{
		discard;
	}
		
	outGBuffer0 = vec4(inPosition, 1.0);
	outGBuffer1 = vec4(albedo.rgb, texture(inOcclusionSampler, inTexCoord).r);
  
	// calculate normal in tangent space
	mat3 TBN = mat3(inTangent, inBitangent, inNormal);
	outGBuffer2 = vec4(TBN * normalize(texture(inNormalSampler, inTexCoord).xyz * 2.0 - vec3(1.0)), 1.0);
}