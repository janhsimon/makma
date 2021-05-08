#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(location = 0) in vec3 inPosition;

void main()
{
  gl_Position = vec4(inPosition, 1.0);
}