#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 1, binding = 0) uniform WM
{
	mat4 worldMatrix;
} wm;

layout(set = 2, binding = 0) uniform VPM
{
	mat4 viewProjectionMatrix;
} vpm;

layout(location = 0) in vec3 inPosition;
layout(location = 1) in vec2 inTexCoord;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec3 inTangent;
layout(location = 4) in vec3 inBitangent;

layout(location = 0) out vec3 outPosition;
layout(location = 1) out vec2 outTexCoord;
layout(location = 2) out vec3 outNormal;
layout(location = 3) out vec3 outTangent;
layout(location = 4) out vec3 outBitangent;

void main()
{
  // vertex transform and position in worldspace
	vec4 worldPosition = wm.worldMatrix * vec4(inPosition, 1.0);
	gl_Position = vpm.viewProjectionMatrix * worldPosition;
	outPosition = worldPosition.xyz;
  
	// normal, tangent and bitangent also in worldspace
	outNormal = (wm.worldMatrix * vec4(inNormal, 0.0)).xyz;	
	outTangent = (wm.worldMatrix * vec4(inTangent, 0.0)).xyz;
	outBitangent = (wm.worldMatrix * vec4(inBitangent, 0.0)).xyz;
	
	outTexCoord = inTexCoord;
}