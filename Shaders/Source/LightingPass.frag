#version 450

#extension GL_ARB_separate_shader_objects : enable
#extension GL_ARB_shading_language_420pack : enable

layout(set = 0, binding = 0) uniform sampler2D positionSampler;
layout(set = 0, binding = 1) uniform sampler2D albedoSampler;
layout(set = 0, binding = 2) uniform sampler2D normalSampler;

layout(set = 1, binding = 0) uniform Light
{
	mat4 data;
} light;

layout(location = 0) in vec2 inTexCoord;

layout(location = 0) out vec4 outColor;

void main()
{
  vec3 lightPosition = light.data[0].xyz;
  float lightType = light.data[0].w;
  vec3 lightColor = light.data[1].xyz;
  float lightDiffuseIntensity = light.data[2].x;
  float lightSpecularIntensity = light.data[2].y;
  float lightSpecularPower = light.data[2].z;
  
	vec3 albedo = texture(albedoSampler, inTexCoord).rgb;
	vec3 normal = normalize(texture(normalSampler, inTexCoord).rgb);
  float occlusion = 1.0 - texture(albedoSampler, inTexCoord).a;
  
  vec3 light = vec3(0.0, 0.0, 0.0);
  
  if (lightType < 0.5)
  // directional light
  {
    light = max(0.0, dot(normal, normalize(lightPosition))) * lightColor;
  }
  else if (lightType < 1.5)
  // point light
  {
    vec3 worldPosition = texture(positionSampler, inTexCoord).rgb;
    
    vec3 lightToFragment = worldPosition - lightPosition;
		float distance = length(lightToFragment);
		lightToFragment = normalize(lightToFragment);
		
		float diffuseFactor = dot(normal, -lightToFragment);
		
		vec3 diffuseColor = vec3(0.0, 0.0, 0.0);
		
		if (diffuseFactor > 0.0)
		{
			diffuseColor = lightColor * diffuseFactor;
		}
		
		float diffuseAttenuationFactor = 0.0;

		if (lightDiffuseIntensity > 0.0)
		{
			diffuseAttenuationFactor = 1.0 - sqrt(distance / lightDiffuseIntensity);
    }
		
		light = lightColor * clamp(diffuseAttenuationFactor, 0.0, 1.0);
  }
  
	outColor = vec4(light * albedo - occlusion, 1.0);
}