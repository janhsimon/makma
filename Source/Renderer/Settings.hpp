#pragma once

#define SETTINGS_RENDER_MODE_SERIAL								0
#define SETTINGS_RENDER_MODE_PARALLEL							1

#define SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL		0
#define SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL			1
// TODO: implement #define SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_SUPERGLOBAL	2 

class Settings
{
public:
	static int windowWidth;
	static int windowHeight;
	static int windowMode;
	static int renderMode;
	static bool mipMapping;
	static float mipLoadBias;
	static bool transientCommandPool;
	static bool reuseCommandBuffers;
	static bool vertexIndexBufferStaging;
	static bool keepUniformBufferMemoryMapped;
	static int dynamicUniformBufferStrategy;
	static bool flushDynamicUniformBufferMemoryIndividually;
	static int shadowMapResolution;
	static int shadowMapCascadeCount;
	static int blurKernelSize;
};