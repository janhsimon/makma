#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(push_constant) uniform PC
{
	mat4 worldMatrix;
	mat4 viewMatrix;
	mat4 projectionMatrix;
} pc;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;

layout(location = 0) out vec3 outWorldPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;

void main()
{
  outWorldPosition = (pc.worldMatrix * vec4(inPosition, 1.0)).xyz;
  
  gl_Position = pc.projectionMatrix * pc.viewMatrix * vec4(outWorldPosition, 1.0);
     
  outTexCoord = inTexCoord;
  
  // world-space normal and tangent
	mat3 normalMatrix = transpose(inverse(mat3(pc.worldMatrix)));
	outNormal = normalMatrix * normalize(inNormal);	
	outTangent = normalMatrix * normalize(inTangent);
}