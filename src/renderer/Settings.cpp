#include "Settings.hpp"

int Settings::windowWidth = 1280;
int Settings::windowHeight = 720;
int Settings::windowMode = 0;
int Settings::renderMode = SETTINGS_RENDER_MODE_PARALLEL;
bool Settings::mipMapping = true;
float Settings::mipLoadBias = -0.85f;
bool Settings::reuseCommandBuffers = true;
bool Settings::transientCommandPool = true;
bool Settings::vertexIndexBufferStaging = true;
bool Settings::keepUniformBufferMemoryMapped = true;
int Settings::dynamicUniformBufferStrategy = SETTINGS_DYNAMIC_UNIFORM_BUFFER_STRATEGY_GLOBAL;
bool Settings::flushDynamicUniformBufferMemoryIndividually = false;
int Settings::shadowMapResolution = 4096;
int Settings::shadowMapCascadeCount = 6;
float Settings::shadowBias = 0.001f;
int Settings::shadowFilterRange = 2;
float Settings::bloomThreshold = 0.8f;
int Settings::blurKernelSize = 11;
float Settings::blurSigma = 7.0f;
float Settings::volumetricIntensity = 5.0f;
int Settings::volumetricSteps = 10;
float Settings::volumetricScattering = 0.2f;