#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

#include "Lighting.include"

layout(set = 2, binding = 0) uniform sampler2D inGBuffer0;
layout(set = 2, binding = 1) uniform sampler2D inGBuffer1;
layout(set = 2, binding = 2) uniform sampler2D inGBuffer2;

layout(set = 3) uniform sampler2DArray inShadowMap;

layout(set = 4) uniform Light { mat4 data; } light;

layout(set = 5) uniform ShadowMapCascade { mat4[4] viewProjectionMatrices; } shadowMapCascade;
layout(set = 6) uniform ShadowMapCascadeSplits { mat4 splits; } shadowMapCascadeSplits;

layout(location = 0) in vec3 inEyePosition;

layout(location = 0) out vec4 outColor;

void main()
{
  const vec2 uv = gl_FragCoord.xy / textureSize(inGBuffer0, 0).xy;
  
  const vec3 lightPosition = light.data[0].xyz;
  const float lightType = light.data[0].w;
  const vec3 lightColor = light.data[1].xyz;
  const float lightIntensity = light.data[1].w;
  const float lightRange = light.data[2].x;
  const bool lightCastShadows = light.data[2].y > 0.5;
  
  const vec4 positionMetallic = texture(inGBuffer0, uv);
  const vec3 position = positionMetallic.rgb;
  const float metallic = positionMetallic.a;
  
  const vec4 albedoOcclusion = texture(inGBuffer1, uv);
	const vec3 albedo = albedoOcclusion.rgb;
	const float occlusion = 1.0 - albedoOcclusion.a;
	
	const vec4 normalRoughness = texture(inGBuffer2, uv);
	const vec3 normal = normalize(normalRoughness.rgb);
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
  
  light *= max(albedo - occlusion, 0.0);
  
  if (lightCastShadows)
  {
    const float distance = length(inEyePosition - position);
    uint cascadeIndex = GetCascadeIndex(distance, shadowMapCascadeSplits.splits[0].xyzw);
    light *= ShadowFiltered(shadowMapCascade.viewProjectionMatrices[cascadeIndex], position, inShadowMap, cascadeIndex);
    light += Volumetric(position, inEyePosition, shadowMapCascade.viewProjectionMatrices[cascadeIndex], inShadowMap, cascadeIndex, lightDirection, lightColor, lightIntensity);
  }
  
  outColor = vec4(light, 1.0);
}