#include <string.h>
#include <stdint.h>

#define OPL_INCLUDE_VULKAN
#include <opl.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "oe.h"
#include "internal.h"

/*
static const char* s_device_ext_names[] = {
#ifdef OE_PLATFORM_MACOS
#define DEVICE_EXT_COUNT 2
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
  "VK_KHR_portability_subset",
#else
#define DEVICE_EXT_COUNT 1
  VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#endif
};
*/

static VkInstance s_instance;
static VkPhysicalDevice s_gpu;
// static VkDevice s_device;

static void _instance_create(void);
static void _select_gpu(void);
// static void _device_create(void);

void _gfx_init(void) {
  _instance_create();
  _select_gpu();

  info("gfx initialized");
}

void _gfx_quit(void) {
  vkDestroyInstance(s_instance, NULL);
  trace("Vulkan instance destroyed");

  info("gfx terminated");
}

void _instance_create(void) {
  const VkApplicationInfo app_info = {
    .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
    .pNext = NULL,
    .apiVersion = VK_API_VERSION_1_3,
    .pApplicationName = NULL,
    .applicationVersion = 0,
    .pEngineName = "Osfabias Engine",
    .engineVersion = VK_MAKE_API_VERSION(1, 1, 0, 0),
  };

  VkInstanceCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .pApplicationInfo = &app_info,
    .enabledLayerCount = 0,
    .ppEnabledLayerNames = 0,
  };

  int ext_count;
  opl_vk_device_extensions(&ext_count, NULL);

  // +2 for the portability and debug extension names
  const char *ext_names[ext_count + 2];
  opl_vk_device_extensions(&ext_count, ext_names);

#ifdef OE_PLATFORM_MACOS
  ext_names[ext_count++] = VK_KHR_PORTABILITY_ENUMERATION_EXTENSION_NAME;
  info.flags |= VK_INSTANCE_CREATE_ENUMERATE_PORTABILITY_BIT_KHR;
#endif

#ifdef OE_DEBUG_BUILD
  const char* validation_layer_name = "VK_LAYER_KHRONOS_validation";

  uint32_t layer_count;
  vkEnumerateInstanceLayerProperties(&layer_count, NULL);

  VkLayerProperties layer_props[layer_count];
  vkEnumerateInstanceLayerProperties(&layer_count, layer_props);

  // check if layer presented
  for (uint32_t i = 0; i < layer_count; ++i) {
    if (!strcmp(validation_layer_name, layer_props[i].layerName)) {
      ext_names[ext_count++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
      info.enabledLayerCount = 1;
      info.ppEnabledLayerNames = &validation_layer_name;

      trace("Vulkan validation layer enabled");
    }
  }

  if (!info.enabledLayerCount)
    error("Vulkan validation layer isn't supported");
#endif

  info.enabledExtensionCount   = ext_count;
  info.ppEnabledExtensionNames = ext_names;

  const VkResult res = vkCreateInstance(&info, NULL, &s_instance);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan instance: %d", res);
  trace("Vulkan instance created");
}

void _select_gpu(void) {
  // TODO search for the best possible gpu
  uint32_t count = 1;
  vkEnumeratePhysicalDevices(s_instance, &count, &s_gpu);

  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(s_gpu, &props);
  debug("gpu: %s", props.deviceName);
}

/*
void _device_create(void) {
  const VkDeviceCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .enabledExtensionCount = DEVICE_EXT_COUNT,
    .ppEnabledExtensionNames = s_device_ext_names,
  };

  const VkResult res = vkCreateDevice(s_gpu, &info, NULL, &s_device);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan device: %d", res);
  trace("Vulkan device created");
}
*/

