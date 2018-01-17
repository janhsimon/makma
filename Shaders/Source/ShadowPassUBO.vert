#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0) uniform GWM { mat4 geometryWorldMatrix; } gwm;
layout(set = 1) uniform SMVPM { mat4 shadowMapViewProjectionMatrix; } smvpm;

layout(location = 0) in vec3 inPosition;

void main()
{
	gl_Position = smvpm.shadowMapViewProjectionMatrix * gwm.geometryWorldMatrix * vec4(inPosition, 1.0);
}