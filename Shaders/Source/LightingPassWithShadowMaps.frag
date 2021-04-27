#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout (constant_id = 0) const int SHADOW_MAP_CASCADE_COUNT = 6;
layout (constant_id = 1) const float SHADOW_BIAS = 0.001;
layout (constant_id = 2) const int SHADOW_FILTER_RANGE = 2;
layout (constant_id = 3) const float BLOOM_THRESHOLD = 0.8;
layout (constant_id = 4) const float VOLUMETRIC_INTENSITY = 5.0;
layout (constant_id = 5) const int VOLUMETRIC_STEPS = 10;
layout (constant_id = 6) const float VOLUMETRIC_SCATTERING = 0.2;

#include "Lighting.include"

layout(set = 2, binding = 0) uniform sampler2D inAlbedoMetallic;
layout(set = 2, binding = 1) uniform sampler2D inNormalRoughness;
layout(set = 2, binding = 2) uniform sampler2D inDepth;

layout(set = 3, binding = 0) uniform sampler2DArray inShadowMap;

layout(set = 4, binding = 0) uniform Light { mat4 data; } light;

layout(set = 5, binding = 0) uniform ShadowMapCascade { mat4[SHADOW_MAP_CASCADE_COUNT] viewProjectionMatrices; } shadowMapCascade;
layout(set = 6, binding = 0) uniform ShadowMapCascadeSplits { mat4 splits; } shadowMapCascadeSplits;

layout(location = 0) in vec3 inEyePosition;
layout(location = 1) in vec3 inViewRay;
layout(location = 2) in vec3 inEyeForward;
layout(location = 3) in vec2 inCameraClip;

layout(location = 0) out vec4 outLBuffer0;
layout(location = 1) out vec4 outLBuffer1;

vec3 reconstructPositionFromDepth(float depth)
{
  depth = (inCameraClip.x * inCameraClip.y) / (inCameraClip.y + depth * (inCameraClip.x - inCameraClip.y));
  const vec3 viewRay = normalize(inViewRay);
  const float viewZDist = dot(inEyeForward, viewRay);
  return inEyePosition + viewRay * (depth / viewZDist);
}

void main()
{
  const vec2 uv = gl_FragCoord.xy / textureSize(inAlbedoMetallic, 0).xy;
  
  const vec3 lightPosition = light.data[0].xyz;
  const float lightType = light.data[0].w;
  const vec3 lightDirection = light.data[1].xyz;
  const float lightRange = light.data[1].w;
  const vec3 lightColor = light.data[2].xyz;
  const float lightIntensity = light.data[2].w;
  const bool lightCastShadows = light.data[3].x > 0.5;
  const float lightCutoffCosine = light.data[3].y;
  
  const vec3 position = reconstructPositionFromDepth(texture(inDepth, uv).r);

	const vec4 albedoMetallic = texture(inAlbedoMetallic, uv);
	const vec3 albedo = albedoMetallic.rgb;
	const float metallic = albedoMetallic.a;
	
	const vec4 normalRoughness = texture(inNormalRoughness, uv);
	const vec3 normal = normalize(normalRoughness.rgb * 2.0 - vec3(1.0));
	const float roughness = normalRoughness.a;
  
  vec3 light = vec3(0.0);
  
  vec3 lightToFragment = normalize(lightDirection);
  if (lightType > 0.5)
  // point or spotlight
  {
    lightToFragment = normalize(position - lightPosition);
  }
  
  light += Diffuse(normal, lightToFragment, lightDirection, lightColor, lightIntensity, lightType > 1.5, lightCutoffCosine);
  light += Specular(inEyePosition, position, lightToFragment, lightDirection, normal, lightColor, lightIntensity, roughness, lightType > 1.5, lightCutoffCosine);
   
  if (lightType > 0.5)
  // point or spotlight
  {
    light *= Attenuation(length(position - lightPosition), lightRange);
  }
  
  light *= max(albedo, 0.0);
  
  uint cascadeIndex = 0;
  if (lightCastShadows)
  {
    const float distance = length(inEyePosition - position);
    cascadeIndex = GetCascadeIndex(distance, shadowMapCascadeSplits.splits, SHADOW_MAP_CASCADE_COUNT);
    light *= ShadowFiltered(shadowMapCascade.viewProjectionMatrices[cascadeIndex], position, inShadowMap, cascadeIndex, SHADOW_BIAS, SHADOW_FILTER_RANGE);
  }
  
  outLBuffer0 = vec4(light, 1.0);
  
  float brightness = 0.2126 * light.r + 0.7152 * light.g + 0.0722 * light.b;
  if (brightness > BLOOM_THRESHOLD)
  {
    outLBuffer1 = vec4(light, 1.0);
  }
  else
  {
    outLBuffer1 = vec4(0.0, 0.0, 0.0, 1.0);
  }
  
  if (lightCastShadows)
  {
    outLBuffer1 += vec4(Volumetric(position, inEyePosition, shadowMapCascade.viewProjectionMatrices[cascadeIndex], inShadowMap, cascadeIndex, lightToFragment, lightColor, lightIntensity, SHADOW_BIAS), 0.0);
  }
}