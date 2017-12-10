#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 1, binding = 0) uniform WM
{
    mat4 worldMatrix;
} wm;

layout(set = 2, binding = 0) uniform VPM
{
    mat4 viewMatrix;
    mat4 projectionMatrix;
} vpm;

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
  outWorldPosition = (wm.worldMatrix * vec4(inPosition, 1.0)).xyz;
  
  gl_Position = vpm.projectionMatrix * vpm.viewMatrix * vec4(outWorldPosition, 1.0);
     
  outTexCoord = inTexCoord;
  
  // world-space normal and tangent
	mat3 normalMatrix = transpose(inverse(mat3(wm.worldMatrix)));
	outNormal = normalMatrix * normalize(inNormal);	
	outTangent = normalMatrix * normalize(inTangent);
}