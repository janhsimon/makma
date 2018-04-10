#pragma once

#define SETTINGS_RENDER_MODE_SERIAL							0
#define SETTINGS_RENDER_MODE_PARALLEL						1

#define SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_INDIVIDUAL	0
#define SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL		1

class Settings
{
public:
	static int windowWidth;
	static int windowHeight;
	static int windowMode;
	static int renderMode;
	static bool transientCommandPool;
	static bool vertexIndexBufferStaging;
	static int dynamicUniformBufferStrategy;
	static int shadowMapResolution;
	static int shadowMapCascadeCount;
	static int blurKernelSize;
};