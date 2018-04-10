#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "Lighting.include"

layout (constant_id = 0) const int SHADOW_MAP_CASCADE_COUNT = 4;

layout(set = 2, binding = 0) uniform sampler2D inAlbedoMetallic;
layout(set = 2, binding = 1) uniform sampler2D inNormalRoughness;
layout(set = 2, binding = 2) uniform sampler2D inDepth;

layout(set = 3) uniform sampler2DArray inShadowMap;

layout(set = 4) uniform Light { mat4 data; } light;

layout(set = 5) uniform ShadowMapCascade { mat4[SHADOW_MAP_CASCADE_COUNT] viewProjectionMatrices; } shadowMapCascade;
layout(set = 6) uniform ShadowMapCascadeSplits { mat4 splits; } shadowMapCascadeSplits;

layout(location = 0) in vec3 inEyePosition;
layout(location = 1) in vec3 inViewRay;
layout(location = 2) in vec3 inEyeForward;

layout(location = 0) out vec4 outLBuffer0;
layout(location = 1) out vec4 outLBuffer1;

vec3 reconstructPositionFromDepth(float depth)
{
  const float zNear = 0.1;
  const float zFar = 3000.0;
  depth = (zFar * zNear) / (zFar + depth * (zNear - zFar));
  const vec3 viewRay = normalize(inViewRay);
  const float viewZDist = dot(inEyeForward, viewRay);
  return inEyePosition + viewRay * (depth / viewZDist);
}

void main()
{
  const vec2 uv = gl_FragCoord.xy / textureSize(inAlbedoMetallic, 0).xy;
  
  const vec3 lightPosition = light.data[0].xyz;
  const float lightType = light.data[0].w;
  const vec3 lightColor = light.data[1].xyz;
  const float lightIntensity = light.data[1].w;
  const float lightRange = light.data[2].x;
  const bool lightCastShadows = light.data[2].y > 0.5;
  
  const vec3 position = reconstructPositionFromDepth(texture(inDepth, uv).r);

	const vec4 albedoMetallic = texture(inAlbedoMetallic, uv);
	const vec3 albedo = albedoMetallic.rgb;
	const float metallic = albedoMetallic.a;
	
	const vec4 normalRoughness = texture(inNormalRoughness, uv);
	const vec3 normal = normalize(normalRoughness.rgb * 2.0 - vec3(1.0));
	const float roughness = normalRoughness.a;
  
  vec3 light = vec3(0.0, 0.0, 0.0);
  
  vec3 lightDirection = normalize(lightPosition);
  if (lightType > 0.5)
  {
    lightDirection = normalize(position - lightPosition);
  }
  
  light += Diffuse(normal, lightDirection, lightColor, lightIntensity);
  light += Specular(inEyePosition, position, lightDirection, normal, lightColor, lightIntensity, roughness);
   
  if (lightType > 0.5)
  {
    light *= Attenuation(length(position - lightPosition), lightRange);
  }
  
  light *= max(albedo, 0.0);
  
  uint cascadeIndex = 0;
  if (lightCastShadows)
  {
    const float distance = length(inEyePosition - position);
    cascadeIndex = GetCascadeIndex(distance, shadowMapCascadeSplits.splits, SHADOW_MAP_CASCADE_COUNT);
    light *= ShadowFiltered(shadowMapCascade.viewProjectionMatrices[cascadeIndex], position, inShadowMap, cascadeIndex);
  }
  
  outLBuffer0 = vec4(light, 1.0);
  
  float brightness = 0.2126 * light.r + 0.7152 * light.g + 0.0722 * light.b;
  if (brightness > 0.8)
  {
    outLBuffer1 = vec4(light, 1.0);
  }
  else
  {
    outLBuffer1 = vec4(0.0, 0.0, 0.0, 1.0);
  }
  
  if (lightCastShadows)
  {
    outLBuffer1 += vec4(Volumetric(position, inEyePosition, shadowMapCascade.viewProjectionMatrices[cascadeIndex], inShadowMap, cascadeIndex, lightDirection, lightColor, lightIntensity), 0.0);
  }
}