#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D inGBuffer0;
layout(set = 0, binding = 1) uniform sampler2D inGBuffer1;
layout(set = 0, binding = 2) uniform sampler2D inGBuffer2;

layout(set = 1, binding = 0) uniform sampler2D inShadowMap;

layout(set = 3, binding = 0) uniform Light
{
	mat4 data;
} lightData;

layout(set = 4, binding = 0) uniform Globals
{
  mat4 viewProjMatrix;
	mat4 data;
} globals;

layout(set = 5, binding = 0) uniform ShadowMap
{
	mat4 viewProjMatrix;
} shadowMap;

layout(location = 0) out vec4 outColor;

const mat4 biasMat = mat4( 
	0.5, 0.0, 0.0, 0.0,
	0.0, 0.5, 0.0, 0.0,
	0.0, 0.0, 1.0, 0.0,
	0.5, 0.5, 0.0, 1.0 );

void main()
{
  vec3 eyePosition = globals.data[0].xyz;
  vec2 screenSize = globals.data[1].xy;
  
  vec2 uv = gl_FragCoord.xy / screenSize;
  
  vec3 lightPosition = lightData.data[0].xyz;
  float lightType = lightData.data[0].w;
  vec3 lightColor = lightData.data[1].xyz;
  float lightIntensity = lightData.data[1].w;
  float lightRange = lightData.data[2].x;
  float lightSpecularPower = lightData.data[2].y;
  bool lightCastShadows = lightData.data[2].z > 0.5;
  
  vec4 positionMetallic = texture(inGBuffer0, uv);
  vec3 position = positionMetallic.rgb;
  float metallic = positionMetallic.a;
  
  vec4 albedoOcclusion = texture(inGBuffer1, uv);
	vec3 albedo = albedoOcclusion.rgb;
	float occlusion = 1.0 - albedoOcclusion.a;
	
	vec4 normalRoughness = texture(inGBuffer2, uv);
	vec3 normal = normalize(normalRoughness.rgb);
	float roughness = normalRoughness.a;
  
  vec3 light = vec3(0.0, 0.0, 0.0);
  
  if (lightType < 0.5)
  // directional light
  {
    vec3 lightDirection = normalize(lightPosition);
    float diffuseFactor = dot(normal, -lightDirection);
    float specularFactor = 0.0;
		
		vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
		vec3 specularColor = vec3(0.0, 0.0, 0.0);
    
    if (diffuseFactor > 0.0)
		{
			diffuseColor = lightColor * lightIntensity * diffuseFactor;
    
      vec3 fragmentToEye = normalize(eyePosition - position);
			vec3 lightReflect = normalize(reflect(lightDirection, normal));
			specularFactor = pow(dot(fragmentToEye, lightReflect), lightSpecularPower);

			if (specularFactor > 0.0)
			{
				specularColor = lightColor * lightIntensity * specularFactor * roughness;
			}
		}
		
		light = diffuseColor + specularColor;
  }
  else if (lightType < 1.5)
  // point light
  {
    vec3 lightToFragment = position - lightPosition;
    float distance = length(lightToFragment);
    lightToFragment = normalize(lightToFragment);
    float diffuseFactor = dot(normal, -lightToFragment);
		float specularFactor = 0.0;
		
		vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
		vec3 specularColor = vec3(0.0, 0.0, 0.0);
		
		if (diffuseFactor > 0.0)
		{
			diffuseColor = lightColor * lightIntensity * diffuseFactor;
			
			vec3 fragmentToEye = normalize(eyePosition - position);
			vec3 lightReflect = normalize(reflect(lightToFragment, normal));
			specularFactor = pow(dot(fragmentToEye, lightReflect), lightSpecularPower);

			if (specularFactor > 0.0)
			{
				specularColor = lightColor * lightIntensity * specularFactor * roughness;
			}
		}
		
		float attenuationFactor = 0.0;

		if (lightRange > 0.0)
		{
      
			attenuationFactor = clamp(1.0 - sqrt(distance / lightRange), 0.0, 1.0);
    }
			
		light = (diffuseColor + specularColor) * attenuationFactor;
  }
  
  float shadow = 1.0;
  if (lightCastShadows)
  {
    vec4 shadowCoord = biasMat * shadowMap.viewProjMatrix * vec4(position, 1.0);	
    
    if (texture(inShadowMap, shadowCoord.xy).r < shadowCoord.z - 0.001)
    {
      shadow = 0.0;
    }
  }
  
  outColor = vec4(light * (albedo - occlusion) * shadow, 1.0);
}