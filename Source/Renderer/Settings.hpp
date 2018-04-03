#pragma once

#define SETTINGS_RENDER_MODE_SERIAL					0
#define SETTINGS_RENDER_MODE_PARALLEL				1

#define SETTINGS_UNIFORM_BUFFER_MODE_INDIVIDUAL		0 // TODO: currently broken
#define SETTINGS_UNIFORM_BUFFER_MODE_STATIC_DYNAMIC	1
#define SETTINGS_UNIFORM_BUFFER_MODE_DYNAMIC		2 // TODO: not implemented yet

class Settings
{
public:
	static int renderMode;
	static int uniformBufferMode;
	static int shadowMapCascadeCount;
	static int blurKernelSize;
};