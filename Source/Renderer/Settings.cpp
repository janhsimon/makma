#include "Settings.hpp"

int Settings::windowWidth = 1280;
int Settings::windowHeight = 720;
int Settings::windowMode = 0;
int Settings::renderMode = SETTINGS_RENDER_MODE_PARALLEL;
bool Settings::reuseCommandBuffers = true;
bool Settings::transientCommandPool = true;
bool Settings::vertexIndexBufferStaging = true;
bool Settings::keepUniformBufferMemoryMapped = true;
int Settings::dynamicUniformBufferStrategy = SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL;
bool Settings::flushDynamicUniformBufferMemoryIndividually = false;
int Settings::shadowMapResolution = 4096;
int Settings::shadowMapCascadeCount = 4;
int Settings::blurKernelSize = 11;