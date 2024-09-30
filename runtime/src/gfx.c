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

#define MAX_SWAPCHAIN_IMAGES_COUNT 3

#define CUR_GRAPHICS_CMDBUF \
  s_cmdbufs[QUEUE_INDEX_GRAPHICS][s_cur_frame_ind]

enum queue_index {
  QUEUE_INDEX_GRAPHICS,
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
static VkImage s_swapchain_images[MAX_SWAPCHAIN_IMAGES_COUNT];
static VkImageView s_swapchain_views[MAX_SWAPCHAIN_IMAGES_COUNT];
static VkRenderPass s_render_pass;
static VkPipelineLayout s_pipeline_layout;
static VkPipeline s_pipeline;
static VkFramebuffer s_framebufs[MAX_SWAPCHAIN_IMAGES_COUNT];
static VkCommandPool s_command_pools[QUEUE_INDEX_MAX];


static VkCommandBuffer
  s_cmdbufs[QUEUE_INDEX_MAX][MAX_SWAPCHAIN_IMAGES_COUNT];

static int s_cur_frame_ind;
static uint32_t s_cur_image_ind;

static void _find_queue_families(void);
static void _check_device_exts(void);

static void _instance_create(void);
static void _select_gpu(void);
static void _device_create(void);
static void _obtain_queues(void);
static void _surface_create(opl_window_t window);
static void _swapchain_create(VkSwapchainKHR old_swapchain);
static void _swapchain_get_images(void);
static void _swapchain_image_views_create(void);
static void _render_pass_create(void);
static void _pipeline_layout_create(void);
static void _pipeline_create(void);
static void _framebufs_create(void);
static void _command_pools_create(void);
static void _command_bufs_create(void);

void _gfx_init(opl_window_t window) {
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

  trace("gfx initialized");
}

void _gfx_quit(void) {
  trace("destroying Vulkan objects...");

  for (int i = 0; i < QUEUE_INDEX_MAX; ++i)
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

void _find_queue_families(void) {
  uint32_t count;
  vkGetPhysicalDeviceQueueFamilyProperties(s_gpu, &count, NULL);

  VkQueueFamilyProperties props[count];
  vkGetPhysicalDeviceQueueFamilyProperties(s_gpu, &count, props);

  for (int i = 0; i < QUEUE_INDEX_MAX; ++i)
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

  for (int i = 0; i < QUEUE_INDEX_MAX; ++i) {
    if (s_queue_families[i] == UINT32_MAX)
      fatal("failed to find %d queue family", i);
  }
  debug("queue families:\n" "\tgraphics: %u\n" "\tpresent: %u",
        s_queue_families[QUEUE_INDEX_GRAPHICS],
        s_queue_families[QUEUE_INDEX_PRESENT]);
}

void _check_device_exts(void) {
  uint32_t count;
  vkEnumerateDeviceExtensionProperties(s_gpu, NULL, &count, NULL);

  VkExtensionProperties props[count];
  vkEnumerateDeviceExtensionProperties(s_gpu, NULL, &count, props);

  for (int i = 0; i < DEVICE_EXT_COUNT; ++i) {
    int available = 0;

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

void _device_create(void) {
  const float queue_priorities[] = { 1.0f };

  VkDeviceQueueCreateInfo queue_infos[2];

  for (int i = 0; i < QUEUE_INDEX_MAX; ++i) {
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
    .queueCreateInfoCount = 2,
    .pQueueCreateInfos = queue_infos,
  };

  const VkResult res = vkCreateDevice(s_gpu, &info, NULL, &s_device);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan device: %d", res);
  trace("Vulkan device created");
}

void _obtain_queues(void) {
  for (int i = 0; i < QUEUE_INDEX_MAX; ++i)
    vkGetDeviceQueue(s_device, s_queue_families[i], 0, &s_queues[i]);
}

void _surface_create(opl_window_t window) {
  const VkResult res = opl_vk_surface_create(window, s_instance, NULL,
                                             &s_surface);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan surface: %d", res);
  trace("Vulkan surface created");
}

void _swapchain_create(VkSwapchainKHR old_swapchain) {
  VkSurfaceCapabilitiesKHR capabs;
  vkGetPhysicalDeviceSurfaceCapabilitiesKHR(s_gpu, s_surface, &capabs);

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

void _swapchain_get_images(void) {
  vkGetSwapchainImagesKHR(s_device, s_swapchain, &s_frames_count,
                          s_swapchain_images);
  debug("obtained %u swapchain image%c", s_frames_count,
        s_frames_count > 1 ? 's' : '\0');
}

void _swapchain_image_views_create(void) {
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

void _render_pass_create(void) {
  const VkAttachmentDescription attachment_desc = {
    .flags = 0,
    .format = VK_FORMAT_B8G8R8A8_SRGB,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
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

  const VkRenderPassCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
    .attachmentCount = 1,
    .pAttachments = &attachment_desc,
    .dependencyCount = 0,
    .pDependencies = NULL,
    .subpassCount = 1,
    .pSubpasses = &subpass_desc,
  };

  const VkResult res = vkCreateRenderPass(s_device, &info, NULL,
                                          &s_render_pass);
  if (res != VK_SUCCESS)
    fatal("failed to create Vulkan render pass: %d", res);
  trace("render pass created");
}

void _pipeline_layout_create(void) {
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

void _shader_module_create(const char *path, VkShaderModule *module) {
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

void _pipeline_create(void) {
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
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = 0,
    },
    { // color
      .binding = 0,
      .location = 1,
      .format = VK_FORMAT_R8G8B8A8_UINT,
      .offset = offsetof(vert_t, color),
    },
    { // texture coordinates
      .binding = 0,
      .location = 2,
      .format = VK_FORMAT_R32G32_SFLOAT,
      .offset = offsetof(vert_t, tex_coord),
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

void _framebufs_create(void) {
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

void _command_pools_create(void) {
  VkCommandPoolCreateInfo info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .flags = 0,
    .pNext = NULL,
  };

  for (int i = 0; i < QUEUE_INDEX_MAX; ++i) {
    const VkResult res = vkCreateCommandPool(
      s_device, &info, NULL, &s_command_pools[i]);

    if (res != VK_SUCCESS)
      fatal("failed to create command pool for %d queue", i);
  }
  trace("Vulkan command pools created");
}

void _command_bufs_create(void) {
  VkCommandBufferAllocateInfo info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = NULL,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = s_frames_count,
  };

  for (int i = 0; i < QUEUE_INDEX_MAX; ++i) {
    info.commandPool = s_command_pools[i];

    const VkResult res = vkAllocateCommandBuffers(s_device, &info,
                                                  s_cmdbufs[i]);
    if (res != VK_SUCCESS)
      fatal("failed to allocoate command buffers for %d pool", i);
  }
  trace("Vulkan command buffers allocated");
}

void draw_begin(void) {
  vkAcquireNextImageKHR(s_device, s_swapchain, UINT64_MAX,
                        VK_NULL_HANDLE, VK_NULL_HANDLE,
                        &s_cur_image_ind);

  static const VkCommandBufferBeginInfo cmdbuf_begin_info = {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .flags = 0,
    .pNext = NULL,
    .pInheritanceInfo = NULL,
  };

  vkBeginCommandBuffer(CUR_GRAPHICS_CMDBUF, &cmdbuf_begin_info);

  const VkRenderPassBeginInfo render_pass_begin_info = {
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = NULL,
    .renderPass = s_render_pass,
    .framebuffer = s_framebufs[s_cur_frame_ind],
    .renderArea = (VkRect2D){
      .offset = { 0.0f, 0.0f },
      .extent = s_swapchain_extent,
    },
    .clearValueCount = 0,
    // .pClearValues
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

void draw_end(void) {
  vkCmdEndRenderPass(CUR_GRAPHICS_CMDBUF);

  vkEndCommandBuffer(CUR_GRAPHICS_CMDBUF);

  const VkSubmitInfo submit_info = {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = NULL,
    .commandBufferCount = 1,
    .pCommandBuffers = &CUR_GRAPHICS_CMDBUF,
    .waitSemaphoreCount = 0,
    // .pWaitSemaphores
    .signalSemaphoreCount = 0,
    // .pSignalSemaphores
  };

  const VkResult res = vkQueueSubmit(s_queues[QUEUE_INDEX_GRAPHICS], 1, 
                                     &submit_info, VK_NULL_HANDLE);
  if (res != VK_SUCCESS)
    fatal("failed to submit queue: %d", res);

  const VkPresentInfoKHR present_info = {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = NULL,
    .waitSemaphoreCount = 0,
    // .pWaitSemaphores
    .swapchainCount = 1,
    .pSwapchains = &s_swapchain,
    .pImageIndices = &s_cur_image_ind,
    .pResults = NULL
  };
  vkQueuePresentKHR(s_queues[QUEUE_INDEX_PRESENT], &present_info);

  s_cur_frame_ind = (s_cur_frame_ind + 1) % s_frames_count;
}

