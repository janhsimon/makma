#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D inLBuffer0;
layout(set = 0, binding = 1) uniform sampler2D inLBuffer1;

layout(location = 0) out vec4 outColor;

float normpdf(in float x, in float sigma)
{
	return 0.39894*exp(-0.5*x*x/(sigma*sigma))/sigma;
}

void main()
{
  const vec2 iResolution = textureSize(inLBuffer0, 0).xy;
  
  // declare stuff
  const int mSize = 11;
  const int kSize = (mSize-1)/2;
  float kernel[mSize];
  vec3 final_colour = vec3(0.0);
  
  // create the 1-D kernel
  float sigma = 7.0;
  float Z = 0.0;
  for (int j = 0; j <= kSize; ++j)
  {
    kernel[kSize+j] = kernel[kSize-j] = normpdf(float(j), sigma);
  }
  
  // get the normalization factor (as the gaussian has been clamped)
  for (int j = 0; j < mSize; ++j)
  {
    Z += kernel[j];
  }
  
  // read out the texels
  for (int i=-kSize; i <= kSize; ++i)
  {
    for (int j=-kSize; j <= kSize; ++j)
    {
      final_colour += kernel[kSize+j]*kernel[kSize+i]*texture(inLBuffer1, (gl_FragCoord.xy+vec2(float(i),float(j))) / iResolution.xy).rgb;

    }
  }
  
  outColor = vec4(texture(inLBuffer0, gl_FragCoord.xy / iResolution).rgb + final_colour/(Z*Z), 1.0);
}