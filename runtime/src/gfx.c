#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <limits.h>

#define OPL_INCLUDE_VULKAN
#include <opl.h>

#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include "oe.h"
#include "internal.h"

#define MAX_FRAMES_COUNT     3
#define RESERVED_VERTS_COUNT 4000

#define CUR_GRAPHICS_CMDBUF \
  s_cmdbufs[QUEUE_INDEX_GRAPHICS][s_cur_frame_ind]

#define CUR_TRANSFER_CMDBUF \
  s_cmdbufs[QUEUE_INDEX_TRANSFER][s_cur_frame_ind]

enum queue_index {
  QUEUE_INDEX_GRAPHICS,
  QUEUE_INDEX_TRANSFER,
  QUEUE_INDEX_PRESENT,
  QUEUE_INDEX_MAX
};

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

static VkInstance s_instance;
static VkPhysicalDevice s_gpu;
static VkDevice s_device;
static uint32_t s_queue_families[QUEUE_INDEX_MAX];
static VkQueue s_queues[QUEUE_INDEX_MAX];
static VkSurfaceKHR s_surface;
static uint32_t s_frames_count;
static VkSwapchainKHR s_swapchain;
static VkExtent2D s_swapchain_extent;
static VkImage s_swapchain_images[MAX_FRAMES_COUNT];
static VkImageView s_swapchain_views[MAX_FRAMES_COUNT];
static VkRenderPass s_render_pass;
static VkPipelineLayout s_pipeline_layout;
static VkPipeline s_pipeline;
static VkFramebuffer s_framebufs[MAX_FRAMES_COUNT];
static VkCommandPool s_command_pools[QUEUE_INDEX_MAX];
static VkCommandBuffer s_cmdbufs[QUEUE_INDEX_MAX][MAX_FRAMES_COUNT];
static VkBuffer s_vert_buf;
static VkDeviceMemory s_vert_buf_mem;
static vert_t s_verts[RESERVED_VERTS_COUNT];
static i32 s_vert_count = 0;
static VkBuffer s_ind_buf;
static VkDeviceMemory s_ind_buf_mem;
static VkSemaphore s_image_available_semaphores[MAX_FRAMES_COUNT];
static VkSemaphore s_renderer_finished_semaphores[MAX_FRAMES_COUNT];
static VkFence s_in_flight_fences[MAX_FRAMES_COUNT];
static i32 s_cur_frame_ind = 0;
static uint32_t s_cur_image_ind;

inline static void _find_queue_families(void);
inline static void _check_device_exts(void);
inline static i32  _buf_create(VkBufferUsageFlags usage, VkDeviceSize size,
                               VkBuffer *buf, VkDeviceMemory *mem);
inline static void _buf_copy(VkBuffer src, VkBuffer dst, VkDeviceSize size);
inline static void _fill_memory(const void *data, VkDeviceSize size, VkBuffer buf);

inline static void _instance_create(void);
inline static void _select_gpu(void);
inline static void _device_create(void);
inline static void _obtain_queues(void);
inline static void _surface_create(opl_window_t window);
inline static void _swapchain_create(VkSwapchainKHR old_swapchain);
inline static void _swapchain_get_images(void);
inline static void _swapchain_image_views_create(void);
inline static void _render_pass_create(void);
inline static void _pipeline_layout_create(void);
inline static void _pipeline_create(void);
inline static void _framebufs_create(void);
inline static void _command_pools_create(void);
inline static void _command_bufs_create(void);
inline static void _vert_buf_create(void);
inline static void _ind_buf_create(void);
inline static void _sync_objects_create(void);

void _gfx_init(opl_window_t window)
{
  _instance_create();
  _select_gpu();
  _surface_create(window);
  _find_queue_families();
  _device_create();
  _obtain_queues();
  _check_device_exts();
  _swapchain_create(VK_NULL_HANDLE);
  _swapchain_get_images();
  _swapchain_image_views_create();
  _render_pass_create();
  _pipeline_layout_create();
  _pipeline_create();
  _framebufs_create();
  _command_pools_create();
  _command_bufs_create();
  _vert_buf_create();
  _ind_buf_create();
  _sync_objects_create();

  trace("gfx initialized");
}

void _gfx_quit(void)
{
  vkDeviceWaitIdle(s_device);

  trace("destroying Vulkan objects...");

  for (uint32_t i = 0; i < s_frames_count; ++i) {
    vkDestroyFence(s_device, s_in_flight_fences[i], NULL);

    vkDestroySemaphore(s_device, s_renderer_finished_semaphores[i],
                       NULL);

    vkDestroySemaphore(s_device, s_image_available_semaphores[i],
                       NULL);
  }

  vkFreeMemory(s_device, s_ind_buf_mem, NULL);
  vkDestroyBuffer(s_device, s_ind_buf, NULL);

  vkFreeMemory(s_device, s_vert_buf_mem, NULL);
  vkDestroyBuffer(s_device, s_vert_buf, NULL);

  for (i32 i = 0; i < QUEUE_INDEX_MAX; ++i)
    vkDestroyCommandPool(s_device, s_command_pools[i], NULL);

  for (uint32_t i = 0; i < s_frames_count; ++i)
    vkDestroyFramebuffer(s_device, s_framebufs[i], NULL);

  vkDestroyPipeline(s_device, s_pipeline, NULL);
  vkDestroyPipelineLayout(s_device, s_pipeline_layout, NULL);
  vkDestroyRenderPass(s_device, s_render_pass, NULL);

  for (uint32_t i = 0; i < s_frames_count; ++i)
    vkDestroyImageView(s_device, s_swapchain_views[i], NULL);

  vkDestroySwapchainKHR(s_device, s_swapchain, NULL);
  vkDestroyDevice(s_device, NULL);
  vkDestroySurfaceKHR(s_instance, s_surface, NULL);
  vkDestroyInstance(s_instance, NULL);

  trace("gfx terminated");
}

void _find_queue_families(void)
{
  uint32_t count;
  vkGetPhysicalDeviceQueueFamilyProperties(s_gpu, &count, NULL);

  VkQueueFamilyProperties props[count];
  vkGetPhysicalDeviceQueueFamilyProperties(s_gpu, &count, props);

  for (i32 i = 0; i < QUEUE_INDEX_MAX; ++i)
    s_queue_families[i] = UINT32_MAX;

  for (uint32_t i = 0; i < count; ++i) {
    // graphics family
    if (
      s_queue_families[QUEUE_INDEX_GRAPHICS] == UINT32_MAX &&
      props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT
    ) {
      s_queue_families[QUEUE_INDEX_GRAPHICS] = i;
      continue;
    }

    // transfer family
    if (
      s_queue_families[QUEUE_INDEX_TRANSFER] == UINT32_MAX &&
      props[i].queueFlags & VK_QUEUE_TRANSFER_BIT
    ) {
      s_queue_families[QUEUE_INDEX_TRANSFER] = i;
      continue;
    }

    // present family
    VkBool32 support_present = 0;
    vkGetPhysicalDeviceSurfaceSupportKHR(s_gpu, i, s_surface,
                                         &support_present);
    if (
      s_queue_families[QUEUE_INDEX_PRESENT] == UINT32_MAX &&
      support_present
    )
      s_queue_families[QUEUE_INDEX_PRESENT] = i;
  }

  for (i32 i = 0; i < QUEUE_INDEX_MAX; ++i) {
    if (s_queue_families[i] == UINT32_MAX)
      fatal("failed to find %d queue family", i);
  }
  debug("queue families:\n"
        "\tgraphics: %u\n"
        "\ttranfser: %u\n"
        "\tpresent: %u",
        s_queue_families[QUEUE_INDEX_GRAPHICS],
        s_queue_families[QUEUE_INDEX_TRANSFER],
        s_queue_families[QUEUE_INDEX_PRESENT]);
}

void _check_device_exts(void)
{
  uint32_t count;
  vkEnumerateDeviceExtensionProperties(s_gpu, NULL, &count, NULL);

  VkExtensionProperties props[count];
  vkEnumerateDeviceExtensionProperties(s_gpu, NULL, &count, props);

  for (i32 i = 0; i < DEVICE_EXT_COUNT; ++i) {
    i32 available = 0;

    for (uint32_t j = 0; j < count; ++j) {
      if (!strcmp(s_device_ext_names[i], props[j].extensionName)) {
        available = 1;
        break;
      }
    }

    if (!available)
      fatal("chosen gpu doesn't support %s extension",
            s_device_ext_names[i]);
  }
}

i32 _buf_create(VkBufferUsageFlags usage, VkDeviceSize size,
                VkBuffer *buf, VkDeviceMemory *mem)
{
  // create buffer
  const VkBufferCreateInfo buf_info = {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .size = size,
    .usage = usage | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    // .pQueueFamilyIndices
  };

  VkResult res = vkCreateBuffer(s_device, &buf_info, NULL, buf);
  if (res != VK_SUCCESS) {
    error("failed to create Vulkan buffer: %d", res);
    return 0;
  }

  // get the requirements and find suitable memory type
  VkMemoryRequirements mem_requirements;
  vkGetBufferMemoryRequirements(s_device, s_vert_buf, &mem_requirements);

  VkPhysicalDeviceMemoryProperties mem_props;
  vkGetPhysicalDeviceMemoryProperties(s_gpu, &mem_props);

  uint32_t mem_type_ind = UINT32_MAX;

  static const VkMemoryPropertyFlags target_mem_prop_flags =
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;

  for (uint32_t i = 0; i < mem_props.memoryTypeCount; i++) {
    VkMemoryPropertyFlags mem_prop_flags =
      mem_props.memoryTypes[i].propertyFlags;

    if (
      (mem_requirements.memoryTypeBits & (1 << i)) &&
      (mem_prop_flags & target_mem_prop_flags) == target_mem_prop_flags 
    ) {
      mem_type_ind = i;
      break;
    }
  }

  if (mem_type_ind == UINT32_MAX) {
    error("failed to find suitable memory type");
    return 0;
  }

  // allocate and bind memory
  const VkMemoryAllocateInfo mem_info = {
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = NULL,
    .allocationSize = mem_requirements.size,
    .memoryTypeIndex = mem_type_ind,
  };

  res = vkAllocateMemory(s_device, &mem_info, NULL, mem);
  if (res != VK_SUCCESS) {
    error("failed to allocate memory: %d", res);
    return 0;
  }

  vkBindBufferMemory(s_device, *buf, *mem, 0);

  return 1;
}

void _buf_copy(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
  const VkCommandBufferAllocateInfo cmd_buf_alloc_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandPool = s_command_pools[QUEUE_INDEX_TRANSFER],
    .commandBufferCount = 1,
  };

  VkCommandBuffer cmd_buf;
  vkAllocateCommandBuffers(s_device, &cmd_buf_alloc_info, &cmd_buf);

  const VkCommandBufferBeginInfo begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
  };

  vkBeginCommandBuffer(cmd_buf, &begin_info);

  const VkBufferCopy copy_region = {
    .srcOffset = 0, // Optional
    .dstOffset = 0, // Optional
    .size = size,
  };
  vkCmdCopyBuffer(cmd_buf, src, dst, 1, &copy_region);

  vkEndCommandBuffer(cmd_buf);

  const VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .commandBufferCount = 1,
    .pCommandBuffers = &cmd_buf,
  };

  vkQueueSubmit(s_queues[QUEUE_INDEX_TRANSFER], 1, &submit_info,
                VK_NULL_HANDLE);
  vkQueueWaitIdle(s_queues[QUEUE_INDEX_TRANSFER]);

  vkFreeCommandBuffers(s_device, s_command_pools[QUEUE_INDEX_TRANSFER],
                       1, &cmd_buf);
}

void _fill_memory(const void *data, VkDeviceSize size, VkBuffer buf)
{
  VkBuffer staging_buf;
  VkDeviceMemory stagin_buf_mem;

  _buf_create(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, size, &staging_buf,
              &stagin_buf_mem);

  void *stagind_buf_mem_ptr;
  vkMapMemory(s_device, stagin_buf_mem, 0, size, 0, &stagind_buf_mem_ptr);
  memcpy(stagind_buf_mem_ptr, data, size);
  vkUnmapMemory(s_device, stagin_buf_mem);

  _buf_copy(staging_buf, buf, size);

  vkFreeMemory(s_device, stagin_buf_mem, NULL);
  vkDestroyBuffer(s_device, staging_buf, NULL);
}

void _instance_create(void)
{
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

  i32 ext_count;
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

void _select_gpu(void)
{
  // TODO search for the best possible gpu
  uint32_t count = 1;
  vkEnumeratePhysicalDevices(s_instance, &count, &s_gpu);

  VkPhysicalDeviceProperties props;
  vkGetPhysicalDeviceProperties(s_gpu, &props);
  debug("gpu: %s", props.deviceName);
}

void _device_create(void)
{
  const float queue_priorities[] = { 1.0f };

  VkDeviceQueueCreateInfo queue_infos[QUEUE_INDEX_MAX];

  for (i32 i = 0; i < QUEUE_INDEX_MAX; ++i) {
    queue_infos[i] = (VkDeviceQueueCreateInfo){
      .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
      .flags = 0,
      .pNext = NULL,
      .queueCount = 1,
      .queueFamilyIndex = s_queue_families[i],
      .pQueuePriorities = queue_priorities,
    };
  };

  const VkDeviceCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .enabledExtensionCount = DEVICE_EXT_COUNT,
    .ppEnabledExtensionNames = s_device_ext_names,
    .queueCreateInfoCount = QUEUE_INDEX_MAX,
    .pQueueCreateInfos = queue_infos,
  };

  const VkResult res = vkCreateDevice(s_gpu, &info, NULL, &s_device);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan device: %d", res);
  trace("Vulkan device created");
}

void _obtain_queues(void)
{
  for (i32 i = 0; i < QUEUE_INDEX_MAX; ++i)
    vkGetDeviceQueue(s_device, s_queue_families[i], 0, &s_queues[i]);
}

void _surface_create(opl_window_t window)
{
  const VkResult res = opl_vk_surface_create(window, s_instance, NULL,
                                             &s_surface);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan surface: %d", res);
  trace("Vulkan surface created");
}

void _swapchain_create(VkSwapchainKHR old_swapchain)
{
  VkSurfaceCapabilitiesKHR capabs;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_gpu, s_surface, &capabs);

  assert(
    capabs.minImageCount <= MAX_FRAMES_COUNT,
    "minimal image count for the surface is %u, which is bigger than"
    "MAX_FRAMES_COUNT macro", capabs.minImageCount
  );
  s_swapchain_extent = capabs.currentExtent;

  VkSwapchainCreateInfoKHR info = {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .flags = 0,
    .pNext = 0,
    .surface = s_surface,
    .minImageCount = capabs.minImageCount,

    // NOTE: assert for now that all hardware we gonna ship our games
    //       on supports this format
    .imageFormat = VK_FORMAT_B8G8R8A8_SRGB,


    // NOTE: assert for now that all hardware we gonna ship our games
    //       on supports this color space 
    .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,

    .imageExtent = capabs.currentExtent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                  VK_IMAGE_USAGE_TRANSFER_DST_BIT,
    .preTransform = capabs.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,

    // NOTE: assert for now that all hardware we gonna ship our games
    //       on supports this present mode
    .presentMode = VK_PRESENT_MODE_MAILBOX_KHR,

    .clipped = VK_TRUE,
    .oldSwapchain = old_swapchain
  };

  const uint32_t queue_families[2] = {
    s_queue_families[QUEUE_INDEX_GRAPHICS],
    s_queue_families[QUEUE_INDEX_PRESENT]
  };

  if (
    s_queue_families[QUEUE_INDEX_GRAPHICS] !=
    s_queue_families[QUEUE_INDEX_PRESENT]
  ) {
    info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
    info.queueFamilyIndexCount = 2;
    info.pQueueFamilyIndices = queue_families;
  } else {
    info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount = 0;
    info.pQueueFamilyIndices = NULL;
  }

  s_frames_count = capabs.minImageCount;

  const VkResult res = vkCreateSwapchainKHR(s_device, &info, NULL,
                                            &s_swapchain);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan swapchain: %d", res);
  debug("Vulkan swapchain created: %ux%u",
        capabs.currentExtent.width,
        capabs.currentExtent.height);
}

void _swapchain_get_images(void)
{
  vkGetSwapchainImagesKHR(s_device, s_swapchain, &s_frames_count,
                          s_swapchain_images);
  debug("obtained %u swapchain image%c", s_frames_count,
        s_frames_count > 1 ? 's' : '\0');
}

void _swapchain_image_views_create(void)
{
  VkImageViewCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .format = VK_FORMAT_B8G8R8A8_SRGB,
    .viewType = VK_IMAGE_VIEW_TYPE_2D,
    .components = {
      .r = VK_COMPONENT_SWIZZLE_IDENTITY,
      .g = VK_COMPONENT_SWIZZLE_IDENTITY,
      .b = VK_COMPONENT_SWIZZLE_IDENTITY,
      .a = VK_COMPONENT_SWIZZLE_IDENTITY,
    },
    .subresourceRange = {
      .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .layerCount = 1,
      .levelCount = 1,
      .baseArrayLayer = 0,
      .baseMipLevel = 0,
    },
  };

  for (uint32_t i = 0; i < s_frames_count; ++i) {
    info.image = s_swapchain_images[i];

    const VkResult res = vkCreateImageView(s_device, &info, NULL,
                                           &s_swapchain_views[i]);
    if (res != VK_SUCCESS)
      fatal("failed to create image view for %d swapchain image", i);
  }

  trace("Vulkan swapchain image views created");
}

void _render_pass_create(void)
{
  const VkAttachmentDescription attachment_desc = {
    .flags = 0,
    .format = VK_FORMAT_B8G8R8A8_SRGB,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
  };

  const VkAttachmentReference attachment_ref = {
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
  };

  const VkSubpassDescription subpass_desc = {
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .colorAttachmentCount = 1,
    .pColorAttachments = &attachment_ref,
    .inputAttachmentCount = 0,
    .pInputAttachments = NULL,
    .pResolveAttachments = NULL,
    .pDepthStencilAttachment = NULL,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = NULL,
  };

  const VkSubpassDependency dependency = {
    .srcSubpass = VK_SUBPASS_EXTERNAL,
    .dstSubpass = 0,
    .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .srcAccessMask = 0,
    .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
    .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
  };

  const VkRenderPassCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .attachmentCount = 1,
    .pAttachments = &attachment_desc,
    .dependencyCount = 1,
    .pDependencies = &dependency,
    .subpassCount = 1,
    .pSubpasses = &subpass_desc,
  };

  const VkResult res = vkCreateRenderPass(s_device, &info, NULL,
                                          &s_render_pass);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan render pass: %d", res);
  trace("render pass created");
}

void _pipeline_layout_create(void)
{
  const VkPipelineLayoutCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .setLayoutCount = 0,
    .pSetLayouts = NULL,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges = NULL
  };

  const VkResult res = vkCreatePipelineLayout(s_device, &info, NULL,
                                              &s_pipeline_layout);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan pipeline layout: %d", res);
  trace("Vulkan pipeline layout created");
}

void _shader_module_create(const char *path, VkShaderModule *module)
{
  FILE *fd = fopen(path, "rb");
  if (!fd)
    fatal("failed to open %s file", path);

  fseek(fd, 0, SEEK_END);
  long size = ftell(fd);
  fseek(fd, 0, SEEK_SET);

  char code[size];
  fread(code, size, 1, fd);
  fclose(fd);

  const VkShaderModuleCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .pCode = (uint32_t*)code,
    .codeSize = size,
  };

  const VkResult res = vkCreateShaderModule(s_device, &info, NULL,
                                            module);
  if (res != VK_SUCCESS)
    fatal("failed to create shader module %s: %d", path, res);
  trace("Vulkan shader module created: %s", path);
}

void _pipeline_create(void)
{
  VkShaderModule vert_shader, frag_shader;

  _shader_module_create("shaders/main-vert.spv", &vert_shader);
  _shader_module_create("shaders/main-frag.spv", &frag_shader);

  const VkPipelineShaderStageCreateInfo stages[2] = {
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .flags = 0,
      .pNext = NULL,
      .stage = VK_SHADER_STAGE_VERTEX_BIT,
      .module = vert_shader,
      .pName = "main",
      .pSpecializationInfo = NULL
    },
    {
      .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
      .flags = 0,
      .pNext = NULL,
      .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
      .module = frag_shader,
      .pName = "main",
      .pSpecializationInfo = NULL
    },
  };

  const VkVertexInputAttributeDescription vert_input_attr_descs[3] = {
    { // position
      .binding = 0,
      .location = 0,
      .format = VK_FORMAT_R32G32B32_SFLOAT,
      .offset = 0,
    },
    { // color
      .binding = 0,
      .location = 1,
      .format = VK_FORMAT_R8G8B8A8_UNORM,
      .offset = offsetof(vert_t, color),
    },
    { // texture coordinates
      .binding = 0,
      .location = 2,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = offsetof(vert_t, uv),
    },
  };

  const VkVertexInputBindingDescription vert_input_bind_desc = {
    .binding = 0,
    .stride = sizeof(vert_t),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
  };

  const VkPipelineVertexInputStateCreateInfo vertex_input_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .vertexAttributeDescriptionCount = 3,
    .pVertexAttributeDescriptions = vert_input_attr_descs,
    .vertexBindingDescriptionCount = 1,
    .pVertexBindingDescriptions = &vert_input_bind_desc,
  };

  const VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
  };

  const VkPipelineTessellationStateCreateInfo tessellation_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .patchControlPoints = 0,
  };

  const VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = s_swapchain_extent.width,
    .height = s_swapchain_extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };

  const VkRect2D scissor = {
    .offset = {0, 0},
    .extent = s_swapchain_extent,
  };

  const VkPipelineViewportStateCreateInfo viewport_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .scissorCount = 1,
    .pScissors = &scissor,
    .viewportCount = 1,
    .pViewports = &viewport,
  };

  const VkPipelineRasterizationStateCreateInfo rasterization_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode = VK_POLYGON_MODE_FILL,
    .cullMode = VK_CULL_MODE_BACK_BIT,
    .frontFace = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable = VK_FALSE,
    // .depthBiasConstantFactor
    .depthClampEnable = VK_FALSE,
    // .depthBiasClamp
    // .depthBiasSlopeFactor
    .lineWidth = 1.0f,
  };

  const VkPipelineMultisampleStateCreateInfo multisample_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable = VK_FALSE,
    // .minSampleShading
    // .pSampleMask
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable = VK_FALSE
  };

  const VkPipelineColorBlendAttachmentState color_blend_attachment = {
    .colorWriteMask = VK_COLOR_COMPONENT_R_BIT |
                      VK_COLOR_COMPONENT_G_BIT |
                      VK_COLOR_COMPONENT_B_BIT | 
                      VK_COLOR_COMPONENT_A_BIT,
    .blendEnable = VK_TRUE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
    .colorBlendOp = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp = VK_BLEND_OP_ADD,
  };

  const VkPipelineColorBlendStateCreateInfo color_blend_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .logicOpEnable = VK_FALSE,
    // .logicOp
    .attachmentCount = 1,
    .pAttachments = &color_blend_attachment,
    .blendConstants = { 1.0f, 1.0f, 1.0f, 1.0f }
  };

  const VkDynamicState dynamic_states[2] = {
    VK_DYNAMIC_STATE_VIEWPORT,
    VK_DYNAMIC_STATE_SCISSOR
  };

  const VkPipelineDynamicStateCreateInfo dynamic_state = {
    .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
    .dynamicStateCount = 2,
    .pDynamicStates = dynamic_states,
  };

  const VkGraphicsPipelineCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .stageCount = 2,
    .pStages = stages,
    .pVertexInputState = &vertex_input_state,
    .pInputAssemblyState = &input_assembly_state,
    .pTessellationState = &tessellation_state,
    .pViewportState = &viewport_state,
    .pRasterizationState = &rasterization_state,
    .pMultisampleState = &multisample_state,
    .pDepthStencilState = NULL,
    .pColorBlendState = &color_blend_state,
    .pDynamicState = &dynamic_state,
    .layout = s_pipeline_layout,
    .renderPass = s_render_pass,
    .subpass = 0,
    .basePipelineHandle = VK_NULL_HANDLE,
    .basePipelineIndex = 0,
  };

  const VkResult res = vkCreateGraphicsPipelines(s_device, NULL, 1,
                                                 &info, NULL,
                                                 &s_pipeline);

  vkDestroyShaderModule(s_device, vert_shader, NULL);
  vkDestroyShaderModule(s_device, frag_shader, NULL);

  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan graphics pipeline: %d", res);
  trace("Vulkan graphics pipeline created");
}

void _framebufs_create(void)
{
  VkSurfaceCapabilitiesKHR swapchain_capabs;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_gpu, s_surface,
                                            &swapchain_capabs);

  VkFramebufferCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .width = swapchain_capabs.currentExtent.width,
    .height = swapchain_capabs.currentExtent.height,
    .layers = 1,
    .renderPass = s_render_pass,
    .attachmentCount = 1,
  };

  for (uint32_t i = 0; i < s_frames_count; i++) {
    info.pAttachments = &s_swapchain_views[i];

    const VkResult res = vkCreateFramebuffer(s_device, &info, NULL,
                                             &s_framebufs[i]);
    if (res != VK_SUCCESS)
      fatal("failed to create Vulkan framebuffer: %d", res);
  }

  trace("Vulkan framebuffer created");
}

void _command_pools_create(void)
{
  VkCommandPoolCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .pNext = NULL,
  };

  for (i32 i = 0; i < QUEUE_INDEX_MAX; ++i) {
    info.queueFamilyIndex = s_queue_families[i];

    const VkResult res = vkCreateCommandPool(
      s_device, &info, NULL, &s_command_pools[i]);

    if (res != VK_SUCCESS)
      fatal("failed to create command pool for %d queue", i);
  }
  trace("Vulkan command pools created");
}

void _command_bufs_create(void)
{
  VkCommandBufferAllocateInfo info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = NULL,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = s_frames_count,
  };

  for (i32 i = 0; i < QUEUE_INDEX_MAX; ++i) {
    info.commandPool = s_command_pools[i];

    const VkResult res = vkAllocateCommandBuffers(s_device, &info,
                                                  s_cmdbufs[i]);
    if (res != VK_SUCCESS)
      fatal("failed to allocoate command buffers for %d pool", i);
  }
  trace("Vulkan command buffers allocated");
}

void _vert_buf_create(void)
{
  if (!_buf_create(VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                 sizeof(vert_t) * RESERVED_VERTS_COUNT,
                 &s_vert_buf, &s_vert_buf_mem))
    fatal("failed to create buffer");
  trace("Vulkan vertex buffer created");
}

void _ind_buf_create(void)
{
  if (!_buf_create(VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                   sizeof(uint16_t) * RESERVED_VERTS_COUNT,
                   &s_ind_buf, &s_ind_buf_mem))
    fatal("failed to create buffer");
  trace("Vulkan index buffer created");
}

void _sync_objects_create(void)
{
  const VkSemaphoreCreateInfo semaphore_info = {
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
  };

  const VkFenceCreateInfo fence_info = {
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    .pNext = NULL,
  };

  for (uint32_t i = 0; i < s_frames_count; ++i) {
    VkResult res1 = vkCreateSemaphore(
      s_device, &semaphore_info, NULL,
      &s_image_available_semaphores[i]);

    VkResult res2 = vkCreateSemaphore(
      s_device, &semaphore_info, NULL, 
      &s_renderer_finished_semaphores[i]);

    VkResult res3 = vkCreateFence(s_device, &fence_info, NULL,
                                  &s_in_flight_fences[i]);

    if (res1 != VK_SUCCESS || res2 != VK_SUCCESS || res3 != VK_SUCCESS)
      fatal("failed to create sync objecsts: %d;%d;%d",
            res1, res2, res3);
  }
  trace("Vulkan sync objects created");
}

void draw_begin(color_t color)
{
  vkWaitForFences(s_device, 1, &s_in_flight_fences[s_cur_frame_ind],
                  VK_TRUE, UINT64_MAX);
  vkResetFences(s_device, 1, &s_in_flight_fences[s_cur_frame_ind]);

  vkAcquireNextImageKHR(s_device, s_swapchain, UINT64_MAX,
                        s_image_available_semaphores[s_cur_frame_ind],
                        VK_NULL_HANDLE, &s_cur_image_ind);

  vkResetCommandBuffer(CUR_GRAPHICS_CMDBUF, 0 /* reset flags */);

  static const VkCommandBufferBeginInfo cmdbuf_begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = 0,
    .pNext = NULL,
    .pInheritanceInfo = NULL,
  };

  vkBeginCommandBuffer(CUR_GRAPHICS_CMDBUF, &cmdbuf_begin_info);

  const VkClearValue clear_color = {
    .color = {{
      (color >> 24) / 255.0f,
      (color >> 16 & 0xff) / 255.0f,
      (color >> 8 & 0xff) / 255.0f,
      (color & 0xff) / 255.0f
    }}
  };

  const VkRenderPassBeginInfo render_pass_begin_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = NULL,
    .renderPass = s_render_pass,
    .framebuffer = s_framebufs[s_cur_frame_ind],
    .renderArea = (VkRect2D){
      .offset = { 0.0f, 0.0f },
      .extent = s_swapchain_extent,
    },
    .clearValueCount = 1,
    .pClearValues = &clear_color,
  };

  vkCmdBeginRenderPass(CUR_GRAPHICS_CMDBUF, &render_pass_begin_info, 
                       VK_SUBPASS_CONTENTS_INLINE);

  vkCmdBindPipeline(CUR_GRAPHICS_CMDBUF,
                    VK_PIPELINE_BIND_POINT_GRAPHICS, s_pipeline);

  const VkViewport viewport = {
    .x = 0.0f,
    .y = 0.0f,
    .width = s_swapchain_extent.width,
    .height = s_swapchain_extent.height,
    .minDepth = 0.0f,
    .maxDepth = 1.0f,
  };
  vkCmdSetViewport(CUR_GRAPHICS_CMDBUF, 0, 1, &viewport);

  const VkRect2D scissor = {
    .offset = {0, 0},
    .extent = s_swapchain_extent,
  };
  vkCmdSetScissor(CUR_GRAPHICS_CMDBUF, 0, 1, &scissor);
}

static void _batch(void)
{
  _fill_memory(s_verts, sizeof(vert_t) * s_vert_count, s_vert_buf);

  const i32 ind_count = s_vert_count * 1.5f;
  uint16_t inds[ind_count];

  for (i32 i = 0; i < ind_count / 6; ++i) {
    inds[i * 6 + 0] = i * 4 + 0;
    inds[i * 6 + 1] = i * 4 + 1;
    inds[i * 6 + 2] = i * 4 + 2;
    inds[i * 6 + 3] = i * 4 + 2;
    inds[i * 6 + 4] = i * 4 + 3;
    inds[i * 6 + 5] = i * 4 + 0;
  }


  _fill_memory(inds, sizeof(inds), s_ind_buf);
}

void draw_end(void)
{
  _batch();

  static const VkDeviceSize offsets[1] = { 0 };
  vkCmdBindVertexBuffers(CUR_GRAPHICS_CMDBUF, 0, 1, &s_vert_buf, offsets);

  vkCmdBindIndexBuffer(CUR_GRAPHICS_CMDBUF, s_ind_buf, 0,
                       VK_INDEX_TYPE_UINT16);

  vkCmdDrawIndexed(CUR_GRAPHICS_CMDBUF, s_vert_count * 1.5f, 1, 0, 0, 0);

  s_vert_count = 0;

  vkCmdEndRenderPass(CUR_GRAPHICS_CMDBUF);

  vkEndCommandBuffer(CUR_GRAPHICS_CMDBUF);

  static const VkPipelineStageFlags wait_stages[] = {
    VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT
  };

  const VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = NULL,
    .commandBufferCount = 1,
    .pCommandBuffers = &CUR_GRAPHICS_CMDBUF,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &s_image_available_semaphores[s_cur_frame_ind],
    .pWaitDstStageMask = wait_stages,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &s_renderer_finished_semaphores[s_cur_frame_ind]
  };

  const VkResult res = vkQueueSubmit(
    s_queues[QUEUE_INDEX_GRAPHICS], 1, &submit_info,
    s_in_flight_fences[s_cur_frame_ind]);

  if (res != VK_SUCCESS)
    fatal("failed to submit queue: %d", res);

  const VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = NULL,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &s_renderer_finished_semaphores[s_cur_frame_ind],
    .swapchainCount = 1,
    .pSwapchains = &s_swapchain,
    .pImageIndices = &s_cur_image_ind,
    .pResults = NULL
  };
  vkQueuePresentKHR(s_queues[QUEUE_INDEX_PRESENT], &present_info);

  s_cur_frame_ind = (s_cur_frame_ind + 1) % s_frames_count;
}

void draw_rect(rect_t rect, color_t color, f32 depth)
{
  assert(s_vert_count < RESERVED_VERTS_COUNT,
         "verts number exceeds the limit");

  s_verts[s_vert_count++] = (vert_t){
    .pos   = { rect.x, rect.y, depth },
    .color = color,
    .uv    = { 0.0f, 0.0f }
  };

  s_verts[s_vert_count++] = (vert_t){
    .pos   = { rect.x + rect.width, rect.y, depth },
    .color = color,
    .uv    = { 0.0f, 0.0f }
  };

  s_verts[s_vert_count++] = (vert_t){
    .pos   = { rect.x + rect.width, rect.y + rect.height, depth },
    .color = color,
    .uv    = { 0.0f, 0.0f }
  };

  s_verts[s_vert_count++] = (vert_t){
    .pos   = { rect.x , rect.y + rect.height, depth },
    .color = color,
    .uv    = { 0.0f, 0.0f }
  };
}

