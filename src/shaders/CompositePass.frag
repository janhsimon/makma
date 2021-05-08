#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (constant_id = 0) const int BLUR_KERNEL_SIZE = 11;
layout (constant_id = 1) const float BLUR_SIGMA = 7;

layout(set = 0, binding = 0) uniform sampler2D inLBuffer0;
layout(set = 0, binding = 1) uniform sampler2D inLBuffer1;

layout(location = 0) out vec4 outColor;

float normpdf(in float x)
{
	return 0.39894 * exp(-0.5 * x * x / (BLUR_SIGMA * BLUR_SIGMA)) / BLUR_SIGMA;
}

void main()
{
  const vec2 iResolution = textureSize(inLBuffer0, 0).xy;
  
  // declare stuff
  const int kSize = (BLUR_KERNEL_SIZE - 1) / 2;
  float kernel[BLUR_KERNEL_SIZE];
  vec3 final_colour = vec3(0.0);
  
  // create the 1-D kernel
  for (int j = 0; j <= kSize; ++j)
  {
    kernel[kSize + j] = kernel[kSize - j] = normpdf(float(j));
  }
  
  // get the normalization factor (as the gaussian has been clamped)
  float Z = 0.0;
  for (int j = 0; j < BLUR_KERNEL_SIZE; ++j)
  {
    Z += kernel[j];
  }
  
  // read out the texels
  for (int i = -kSize; i <= kSize; ++i)
  {
    for (int j = -kSize; j <= kSize; ++j)
    {
      final_colour += kernel[kSize + j] * kernel[kSize + i] * texture(inLBuffer1, (gl_FragCoord.xy + vec2(float(i),float(j))) / iResolution.xy).rgb;

    }
  }
  
  outColor = vec4(texture(inLBuffer0, gl_FragCoord.xy / iResolution).rgb + final_colour / (Z * Z), 1.0);
}