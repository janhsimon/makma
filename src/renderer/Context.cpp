#include "Context.hpp"
#include "Settings.hpp"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <SDL/SDL_vulkan.h>
#include <glm/glm.hpp>

#include <sstream>

#ifdef _DEBUG
static VKAPI_ATTR VkBool32 VKAPI_CALL debugReportCallbackFunction(VkDebugReportFlagsEXT flags,
                                                                  VkDebugReportObjectTypeEXT objectType,
                                                                  uint64_t object,
                                                                  size_t location,
                                                                  int32_t messageCode,
                                                                  const char* pLayerPrefix,
                                                                  const char* pMessage,
                                                                  void* pUserData)
{
  Window::showMessageBox("Debug Report", pMessage);
  return false;
}
#endif

vk::Instance* Context::createInstance(const std::shared_ptr<Window> window)
{
  std::vector<const char*> layers;

#ifdef _DEBUG
  layers.push_back("VK_LAYER_KHRONOS_validation");
#endif

  unsigned int extensionCount = 0;
  if (!SDL_Vulkan_GetInstanceExtensions(window->getWindow(), &extensionCount, nullptr))
  {
    throw std::runtime_error("Failed to get instance extensions: " + std::string(SDL_GetError()));
  }

  std::vector<const char*> extensions(extensionCount);
  if (!SDL_Vulkan_GetInstanceExtensions(window->getWindow(), &extensionCount, extensions.data()))
  {
    throw std::runtime_error("Failed to get instance extensions: " + std::string(SDL_GetError()));
  }

#ifdef _DEBUG
  extensions.push_back(VK_EXT_DEBUG_REPORT_EXTENSION_NAME);
#endif

  auto applicationInfo = vk::ApplicationInfo()
                           .setApiVersion(VK_API_VERSION_1_0)
                           .setPApplicationName("Makma")
                           .setPEngineName("Makma")
                           .setApplicationVersion(1)
                           .setEngineVersion(1);
  auto instanceCreateInfo = vk::InstanceCreateInfo()
                              .setPApplicationInfo(&applicationInfo)
                              .setEnabledLayerCount(static_cast<uint32_t>(layers.size()));
  instanceCreateInfo.setPpEnabledLayerNames(layers.data())
    .setEnabledExtensionCount(static_cast<uint32_t>(extensions.size()))
    .setPpEnabledExtensionNames(extensions.data());
  auto instance = vk::createInstance(instanceCreateInfo);
  return new vk::Instance(instance);
}

vk::SurfaceKHR* Context::createSurface(const std::shared_ptr<Window> window, const vk::Instance* instance)
{
  auto temp = new VkSurfaceKHR();

  if (!SDL_Vulkan_CreateSurface(window->getWindow(), *instance, temp))
  {
    throw std::runtime_error("Failed to create Vulkan surface.");
  }

  return (vk::SurfaceKHR*)temp;
}

vk::PhysicalDevice* Context::selectPhysicalDevice(const std::shared_ptr<Window> window, const vk::Instance* instance)
{
  uint32_t physicalDeviceCount = 0;
  if (instance->enumeratePhysicalDevices(&physicalDeviceCount, nullptr, vk::DispatchLoaderStatic()) !=
      vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to enumerate physical devices.");
  }

  if (physicalDeviceCount <= 0)
  {
    throw std::runtime_error("No physical devices found.");
  }

  std::vector<vk::PhysicalDevice> physicalDevices(physicalDeviceCount);
  if (instance->enumeratePhysicalDevices(&physicalDeviceCount, physicalDevices.data()) != vk::Result::eSuccess)
  {
    throw std::runtime_error("Failed to enumerate physical devices.");
  }

  std::stringstream message;
  message << "Physical device list:" << std::endl;
  auto physicalDeviceIndex = -1;

  for (auto i = 0; i < physicalDevices.size(); ++i)
  {
    const auto physicalDeviceProperties = physicalDevices[i].getProperties();
    message << "#" << i << ": " << physicalDeviceProperties.deviceName << std::endl;

    if (physicalDeviceIndex < 0 && physicalDeviceProperties.deviceType == vk::PhysicalDeviceType::eDiscreteGpu)
    {
      physicalDeviceIndex = i;
    }
  }

  if (physicalDeviceIndex < 0)
  {
    throw std::runtime_error("No suitable physical device found.");
  }

  message << std::endl << "Picked physical device #" << physicalDeviceIndex << ".";
  window->showMessageBox("Makma", message.str());

  return new vk::PhysicalDevice(physicalDevices[0]);
}

vk::Device* Context::createDevice(const vk::SurfaceKHR* surface,
                                  const vk::PhysicalDevice* physicalDevice,
                                  uint32_t& queueFamilyIndex)
{
  uint32_t queueFamilyPropertyCount = 0;
  physicalDevice->getQueueFamilyProperties(&queueFamilyPropertyCount, nullptr, vk::DispatchLoaderStatic());

  if (queueFamilyPropertyCount <= 0)
  {
    throw std::runtime_error("Failed to enumerate queue family properties for physical device.");
  }

  std::vector<vk::QueueFamilyProperties> queueFamilyProperties(queueFamilyPropertyCount);
  physicalDevice->getQueueFamilyProperties(&queueFamilyPropertyCount, queueFamilyProperties.data());

  bool queueFamilyFound = false;
  for (uint32_t i = 0; i < queueFamilyProperties.size(); ++i)
  {
    if (queueFamilyProperties[i].queueCount > 0 && queueFamilyProperties[i].queueFlags & vk::QueueFlagBits::eGraphics)
    {
      if (physicalDevice->getSurfaceSupportKHR(i, *surface))
      {
        queueFamilyIndex = i;
        queueFamilyFound = true;
        break;
      }
    }
  }

  if (!queueFamilyFound)
  {
    throw std::runtime_error("Failed to find suitable queue family for physical device.");
  }

  std::vector<float> queuePriorities = { 1.0f };
  auto deviceQueueCreateInfo =
    vk::DeviceQueueCreateInfo().setQueueFamilyIndex(queueFamilyIndex).setPQueuePriorities(queuePriorities.data());
  deviceQueueCreateInfo.setQueueCount(static_cast<uint32_t>(queuePriorities.size()));
  std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
  auto deviceFeatures = vk::PhysicalDeviceFeatures().setSamplerAnisotropy(true);
  auto deviceCreateInfo = vk::DeviceCreateInfo()
                            .setQueueCreateInfoCount(1)
                            .setPQueueCreateInfos(&deviceQueueCreateInfo)
                            .setPEnabledFeatures(&deviceFeatures);
  deviceCreateInfo.setEnabledExtensionCount(static_cast<uint32_t>(deviceExtensions.size()))
    .setPpEnabledExtensionNames(deviceExtensions.data());
  auto device = physicalDevice->createDevice(deviceCreateInfo);
  return new vk::Device(device);
}

vk::CommandPool* Context::createCommandPoolOnce(const vk::Device* device, uint32_t queueFamilyIndex)
{
  auto commandPoolCreateInfo = vk::CommandPoolCreateInfo().setQueueFamilyIndex(queueFamilyIndex);
  auto commandPool = device->createCommandPool(commandPoolCreateInfo);
  return new vk::CommandPool(commandPool);
}

vk::CommandPool* Context::createCommandPoolRepeat(const vk::Device* device, uint32_t queueFamilyIndex)
{
  auto commandPoolCreateInfo = vk::CommandPoolCreateInfo()
                                 .setQueueFamilyIndex(queueFamilyIndex)
                                 .setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

  if (Settings::transientCommandPool)
  {
    commandPoolCreateInfo.flags |= vk::CommandPoolCreateFlagBits::eTransient;
  }

  auto commandPool = device->createCommandPool(commandPoolCreateInfo);
  return new vk::CommandPool(commandPool);
}

vk::QueryPool* Context::createQueryPool(const vk::Device* device)
{
  auto queryPoolCreateInfo = vk::QueryPoolCreateInfo().setQueryType(vk::QueryType::eTimestamp).setQueryCount(8);
  auto queryPool = device->createQueryPool(queryPoolCreateInfo);
  return new vk::QueryPool(queryPool);
}

#ifdef _DEBUG
VkDebugReportCallbackEXT* Context::createDebugReportCallback(const vk::Instance* instance)
{
  VkDebugReportCallbackEXT debugReportCallback;

  // NOTE: instance.createDebugReportCallbackEXT() would be much nicer but is not currently implemented by the API (only
  // declared)
  PFN_vkCreateDebugReportCallbackEXT vkCreateDebugReportCallbackEXT =
    reinterpret_cast<PFN_vkCreateDebugReportCallbackEXT>(instance->getProcAddr("vkCreateDebugReportCallbackEXT"));
  vk::DebugReportCallbackCreateInfoEXT debugReportCallbackCreateInfo(vk::DebugReportFlagBitsEXT::eError |
                                                                       vk::DebugReportFlagBitsEXT::eWarning |
                                                                       vk::DebugReportFlagBitsEXT::ePerformanceWarning,
                                                                     debugReportCallbackFunction);
  auto temp = VkDebugReportCallbackCreateInfoEXT(debugReportCallbackCreateInfo);

  if (vkCreateDebugReportCallbackEXT(*instance, &temp, nullptr, &debugReportCallback) != VK_SUCCESS)
  {
    throw std::runtime_error("Failed to create debug report callback.");
  }

  return new VkDebugReportCallbackEXT(debugReportCallback);
}
#endif

Context::Context(const std::shared_ptr<Window> window)
{
  instance = std::unique_ptr<vk::Instance, decltype(instanceDeleter)>(createInstance(window), instanceDeleter);

#ifdef _DEBUG
  debugReportCallback = std::unique_ptr<VkDebugReportCallbackEXT, decltype(debugReportCallbackDeleter)>(
    createDebugReportCallback(instance.get()), debugReportCallbackDeleter);
#endif

  surface =
    std::unique_ptr<vk::SurfaceKHR, decltype(surfaceDeleter)>(createSurface(window, instance.get()), surfaceDeleter);
  physicalDevice = std::unique_ptr<vk::PhysicalDevice>(selectPhysicalDevice(window, instance.get()));
  device = std::unique_ptr<vk::Device, decltype(deviceDeleter)>(createDevice(surface.get(), physicalDevice.get(),
                                                                             queueFamilyIndex),
                                                                deviceDeleter);
  commandPoolOnce = std::unique_ptr<vk::CommandPool, decltype(commandPoolDeleter)>(
    createCommandPoolOnce(device.get(), queueFamilyIndex), commandPoolDeleter);
  commandPoolRepeat = std::unique_ptr<vk::CommandPool, decltype(commandPoolDeleter)>(
    createCommandPoolRepeat(device.get(), queueFamilyIndex), commandPoolDeleter);
  queryPool =
    std::unique_ptr<vk::QueryPool, decltype(queryPoolDeleter)>(createQueryPool(device.get()), queryPoolDeleter);

  queue = device->getQueue(queueFamilyIndex, 0);
}

void Context::calculateUniformBufferDataAlignment()
{
  uint32_t minUniformBufferAlignment =
    static_cast<uint32_t>(physicalDevice->getProperties().limits.minUniformBufferOffsetAlignment);

  uniformBufferDataAlignment = sizeof(glm::mat4);
  if (minUniformBufferAlignment > 0)
  {
    uniformBufferDataAlignment =
      (uniformBufferDataAlignment + minUniformBufferAlignment - 1) & ~(minUniformBufferAlignment - 1);
  }

  uniformBufferDataAlignmentLarge = sizeof(glm::mat4) * Settings::shadowMapCascadeCount;
  if (minUniformBufferAlignment > 0)
  {
    uniformBufferDataAlignmentLarge =
      (uniformBufferDataAlignmentLarge + minUniformBufferAlignment - 1) & ~(minUniformBufferAlignment - 1);
  }
}