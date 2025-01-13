/*================================================================================
RayGene3D Framework
--------------------------------------------------------------------------------
RayGene3D is licensed under MIT License
================================================================================
The MIT License
--------------------------------------------------------------------------------
Copyright (c) 2021

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
================================================================================*/


#include "vlk_device.h"

namespace RayGene3D
{
  void VLKDevice::CreateInstance()
  {
    const auto extension_names = std::vector<const char*>
    {
      VK_KHR_SURFACE_EXTENSION_NAME,
      VK_EXT_DEBUG_UTILS_EXTENSION_NAME,
    #ifdef __linux__
      VK_KHR_XLIB_SURFACE_EXTENSION_NAME,
    #elif _WIN32
      VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
    #elif __OBJC__
      VK_EXT_METAL_SURFACE_EXTENSION_NAME,
    #endif
    };

    auto layer_names = std::vector<const char*>();
    if (debug) layer_names.push_back("VK_LAYER_KHRONOS_validation");

    auto property_count = uint32_t{ 0u };
    BLAST_ASSERT(VK_SUCCESS == vkEnumerateInstanceLayerProperties(&property_count, nullptr));
    auto property_array = std::vector<VkLayerProperties>(property_count);
    BLAST_ASSERT(VK_SUCCESS == vkEnumerateInstanceLayerProperties(&property_count, property_array.data()));

    auto application_info = VkApplicationInfo{};
    application_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    application_info.pApplicationName = "RayGene Demo";
    application_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.pEngineName = "RayGene3D Framework";
    application_info.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    application_info.apiVersion = VK_API_VERSION_1_2;

    auto create_info = VkInstanceCreateInfo{};
    create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    create_info.pApplicationInfo = &application_info;
    create_info.enabledExtensionCount = uint32_t(extension_names.size());
    create_info.ppEnabledExtensionNames = extension_names.data();
    create_info.enabledLayerCount = uint32_t(layer_names.size());
    create_info.ppEnabledLayerNames = layer_names.data();

    BLAST_ASSERT(VK_SUCCESS == vkCreateInstance(&create_info, nullptr, &instance));

    auto extension_count = uint32_t{ 0u };
    BLAST_ASSERT(VK_SUCCESS == vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr));
    auto extension_array = std::vector<VkExtensionProperties>(extension_count);
    BLAST_ASSERT(VK_SUCCESS == vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extension_array.data()));
  }

  void VLKDevice::DestroyInstance()
  {
    if (instance)
    {
      vkDestroyInstance(instance, nullptr);
      instance = nullptr;
    }
  }

  void VLKDevice::CreatePool()
  {
    VkCommandPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    pool_info.queueFamilyIndex = family;
    pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    BLAST_ASSERT(VK_SUCCESS == vkCreateCommandPool(device, &pool_info, nullptr, &command_pool));

    {
      VkCommandBufferAllocateInfo allocate_info = {};
      allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocate_info.commandPool = command_pool;
      allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocate_info.commandBufferCount = 1;
      BLAST_ASSERT(VK_SUCCESS == vkAllocateCommandBuffers(device, &allocate_info, &command_buffer));
    }

    {
      VkCommandBufferAllocateInfo allocate_info = {};
      allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      allocate_info.commandPool = command_pool;
      allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      allocate_info.commandBufferCount = 1;
      BLAST_ASSERT(VK_SUCCESS == vkAllocateCommandBuffers(device, &allocate_info, &present_command_buffer));
    }
  }

  void VLKDevice::DestroyPool()
  {
    if (device && command_pool)
    {
      vkDestroyCommandPool(device, command_pool, nullptr);
      command_pool = nullptr;
    }
  }

  void VLKDevice::CreateMessenger()
  {
    if (!debug) return;

    const auto debug_fn = [](VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity, VkDebugUtilsMessageTypeFlagsEXT messageType,
      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) -> VkBool32 //VKAPI_ATTR VkBool32 VKAPI_CALL
    {
      BLAST_LOG(pCallbackData->pMessage);

      return VK_FALSE;
    };

    VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = debug_fn;
    createInfo.pUserData = nullptr;

    auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    BLAST_ASSERT(VK_SUCCESS == vkCreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &messenger));
  }


  void VLKDevice::DestroyMessenger()
  {
    auto vkDestroyDebugUtilsMessengerEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");

    if (instance && messenger)
    {
      vkDestroyDebugUtilsMessengerEXT(instance, messenger, nullptr);
      messenger = nullptr;
    }
  }

  void VLKDevice::CreateSurface()
  {
#ifdef __linux__
    auto create_info = VkXlibSurfaceCreateInfoKHR{};
    create_info.sType = VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR;
    create_info.dpy = reinterpret_cast<Display*>(display);
    create_info.window = reinterpret_cast<Window>(window);
    create_info.flags = 0;
    BLAST_ASSERT(VK_SUCCESS == vkCreateXlibSurfaceKHR(instance, &create_info, nullptr, &surface));
#elif _WIN32
    auto create_info = VkWin32SurfaceCreateInfoKHR{};
    create_info.sType = VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR;
    create_info.hwnd = reinterpret_cast<HWND>(window);
    create_info.hinstance = reinterpret_cast<HINSTANCE>(instance);
    create_info.flags = 0;
    BLAST_ASSERT(VK_SUCCESS == vkCreateWin32SurfaceKHR(instance, &create_info, nullptr, &surface));
#elif __OBJC__
    auto create_info = VkMetalSurfaceCreateInfoEXT{};
    create_info.sType = VK_STRUCTURE_TYPE_METAL_SURFACE_CREATE_INFO_EXT;
    create_info.pLayer = reinterpret_cast<CAMetalLayer*>(window);
    BLAST_ASSERT(VK_SUCCESS == vkCreateMetalSurfaceEXT(instance, &create_info, nullptr, &surface));
#endif
  }


  void VLKDevice::DestroySurface()
  {
    if (instance && surface)
    {
      vkDestroySurfaceKHR(instance, surface, nullptr);
      surface = nullptr;
    }
  }

  void VLKDevice::CreateDevice()
  {
    auto extension_names = std::vector<const char*>
    {
      VK_KHR_SWAPCHAIN_EXTENSION_NAME,
    };


    auto adapter_count = uint32_t{ 0 };
    BLAST_ASSERT(VK_SUCCESS == vkEnumeratePhysicalDevices(instance, &adapter_count, nullptr));
    auto adapter_array = std::vector<VkPhysicalDevice>(adapter_count);
    BLAST_ASSERT(VK_SUCCESS == vkEnumeratePhysicalDevices(instance, &adapter_count, adapter_array.data()));

    BLAST_ASSERT(ordinal <= adapter_count);
    adapter = adapter_array[ordinal];

    vkGetPhysicalDeviceProperties(adapter, &properties);
    vkGetPhysicalDeviceFeatures(adapter, &features);
    vkGetPhysicalDeviceMemoryProperties(adapter, &memory);

    auto queue_count = uint32_t{ 0 };
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queue_count, nullptr);
    auto queue_array = std::vector<VkQueueFamilyProperties>(queue_count);
    vkGetPhysicalDeviceQueueFamilyProperties(adapter, &queue_count, queue_array.data());

    const auto queue_flags = VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_COMPUTE_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_SPARSE_BINDING_BIT;
    for (uint32_t i = 0; i < queue_count; ++i)
    {
      if (queue_array[i].queueFlags & queue_flags)
      {
        family = i;
        break;
      }
      //VkBool32 presentSupport = false;
      //vkGetPhysicalDeviceSurfaceSupportKHR(deviceItem, i, surface, &presentSupport);
      //if (!presentSupport) continue;
    }
    BLAST_ASSERT(family != -1);

    auto extension_count = uint32_t{ 0 };
    BLAST_ASSERT(VK_SUCCESS == vkEnumerateDeviceExtensionProperties(adapter, nullptr, &extension_count, nullptr));
    auto extension_array = std::vector<VkExtensionProperties>(extension_count);
    BLAST_ASSERT(VK_SUCCESS == vkEnumerateDeviceExtensionProperties(adapter, nullptr, &extension_count, extension_array.data()));

    const auto extension_check_fn = [&extension_array](const char* extension)
    {
      return std::any_of(extension_array.begin(), extension_array.end(),
        [extension](const VkExtensionProperties& e) { return strncmp(e.extensionName, extension, 256) == 0; } );
    };

    {
      ray_tracing_supported = extension_check_fn(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
      ray_tracing_supported &= extension_check_fn(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
      ray_tracing_supported &= extension_check_fn(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);

      if (ray_tracing_supported)
      {
        ray_tracing_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
        VkPhysicalDeviceProperties2 device_properties = {};
        device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        device_properties.pNext = &ray_tracing_properties;
        vkGetPhysicalDeviceProperties2(adapter, &device_properties);

        extension_names.push_back(VK_KHR_DEFERRED_HOST_OPERATIONS_EXTENSION_NAME);
        extension_names.push_back(VK_KHR_ACCELERATION_STRUCTURE_EXTENSION_NAME);
        extension_names.push_back(VK_KHR_RAY_TRACING_PIPELINE_EXTENSION_NAME);
      }
    }

    {
      mesh_shader_supported = extension_check_fn(VK_EXT_MESH_SHADER_EXTENSION_NAME);

      if (mesh_shader_supported)
      {
        mesh_shader_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_PROPERTIES_EXT;
        VkPhysicalDeviceProperties2 device_properties = {};
        device_properties.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
        device_properties.pNext = &mesh_shader_properties;
        vkGetPhysicalDeviceProperties2(adapter, &device_properties);

        extension_names.push_back(VK_EXT_MESH_SHADER_EXTENSION_NAME);
      }
    }

    VkPhysicalDeviceAccelerationStructureFeaturesKHR as_features = {};
    as_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_ACCELERATION_STRUCTURE_FEATURES_KHR;
    as_features.pNext = nullptr;
    as_features.accelerationStructure = true;

    VkPhysicalDeviceRayTracingPipelineFeaturesKHR rtp_features = {};
    rtp_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_FEATURES_KHR;
    rtp_features.pNext = &as_features;
    rtp_features.rayTracingPipeline = true;

    VkPhysicalDeviceBufferDeviceAddressFeatures bda_features = {};
    bda_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_BUFFER_DEVICE_ADDRESS_FEATURES;
    bda_features.pNext = &rtp_features;
    bda_features.bufferDeviceAddress = true;

    VkPhysicalDeviceMeshShaderFeaturesEXT ms_features = {};
    ms_features.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MESH_SHADER_FEATURES_EXT;
    ms_features.pNext = ray_tracing_supported ? &bda_features : nullptr;;
    ms_features.meshShader = true;
    ms_features.taskShader = true;

    void* extention_features = 
      mesh_shader_supported ? (void*) & ms_features : 
      ray_tracing_supported ? (void*) & bda_features: 
      nullptr;

    VkPhysicalDeviceFeatures enabled_features{};
    BLAST_ASSERT(features.samplerAnisotropy);          enabled_features.samplerAnisotropy = true;
    BLAST_ASSERT(features.robustBufferAccess);         enabled_features.robustBufferAccess = true;
    BLAST_ASSERT(features.geometryShader);             enabled_features.geometryShader = true;
    BLAST_ASSERT(features.tessellationShader);         enabled_features.tessellationShader = true;
    BLAST_ASSERT(features.imageCubeArray);             enabled_features.imageCubeArray = true;
    BLAST_ASSERT(features.multiViewport);              enabled_features.multiViewport = true;

    VkDeviceQueueCreateInfo queue_create_info = {};
    queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    queue_create_info.queueFamilyIndex = family;
    queue_create_info.queueCount = 1;
    const float priority = 1.0f; // 0.0...1.0
    queue_create_info.pQueuePriorities = &priority;

    VkDeviceCreateInfo device_create_info = {};
    device_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    device_create_info.pNext = extention_features;
    device_create_info.pQueueCreateInfos = &queue_create_info;
    device_create_info.queueCreateInfoCount = 1;
    device_create_info.enabledLayerCount = 0;
    device_create_info.ppEnabledLayerNames = nullptr;
    device_create_info.enabledExtensionCount = uint32_t(extension_names.size());
    device_create_info.ppEnabledExtensionNames = extension_names.data();
    device_create_info.pEnabledFeatures = &enabled_features;
    BLAST_ASSERT(VK_SUCCESS == vkCreateDevice(adapter, &device_create_info, nullptr, &device));

    vkGetDeviceQueue(device, family, 0, &queue);

    BLAST_LOG("Device is created on %s [RT:%s, MS:%s]",
      properties.deviceName,
      ray_tracing_supported ? "On" : "Off",
      mesh_shader_supported ? "On" : "Off");

    name = std::string(properties.deviceName) + " (Vulkan API)";
  }



  void VLKDevice::DestroyDevice()
  {
    if (device)
    {
      vkDestroyDevice(device, nullptr);
      device = nullptr;

      queue = nullptr;
      family = -1;
    }
  }

  void VLKDevice::CreateSwapchain()
  {
    auto capabilities = VkSurfaceCapabilitiesKHR{0};
    BLAST_ASSERT(VK_SUCCESS == vkGetPhysicalDeviceSurfaceCapabilitiesKHR(adapter, surface, &capabilities));
    auto format_count = uint32_t{ 0u };
    BLAST_ASSERT(VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &format_count, nullptr));
    auto format_array = std::vector<VkSurfaceFormatKHR>(format_count);
    BLAST_ASSERT(VK_SUCCESS == vkGetPhysicalDeviceSurfaceFormatsKHR(adapter, surface, &format_count, format_array.data()));

    auto mode_count = uint32_t{ 0u };
    BLAST_ASSERT(VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &mode_count, nullptr));
    auto mode_array = std::vector<VkPresentModeKHR>(mode_count);
    BLAST_ASSERT(VK_SUCCESS == vkGetPhysicalDeviceSurfacePresentModesKHR(adapter, surface, &mode_count, mode_array.data()));

    auto support = VkBool32{ false };
    BLAST_ASSERT(VK_SUCCESS == vkGetPhysicalDeviceSurfaceSupportKHR(adapter, family, surface, &support));

    if (support == VK_TRUE)
    {
      auto create_info = VkSwapchainCreateInfoKHR{};
      create_info.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
      create_info.surface = surface;
      create_info.minImageCount = 3;
      create_info.imageFormat = VK_FORMAT_B8G8R8A8_UNORM;
      create_info.imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
      create_info.imageExtent = { extent_x, extent_y };
      create_info.imageArrayLayers = 1;
      create_info.imageUsage = VK_IMAGE_USAGE_TRANSFER_DST_BIT;
      create_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
      create_info.queueFamilyIndexCount = 0;
      create_info.pQueueFamilyIndices = nullptr;
      create_info.preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
      create_info.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
      create_info.presentMode = VK_PRESENT_MODE_IMMEDIATE_KHR;
      create_info.clipped = VK_TRUE;
      create_info.oldSwapchain = VK_NULL_HANDLE;
      BLAST_ASSERT(VK_SUCCESS == vkCreateSwapchainKHR(device, &create_info, nullptr, &swapchain));

      auto image_count = uint32_t{ 0u };
      BLAST_ASSERT(VK_SUCCESS == vkGetSwapchainImagesKHR(device, swapchain, &image_count, nullptr));
      auto image_array = std::vector<VkImage>(image_count);
      BLAST_ASSERT(VK_SUCCESS == vkGetSwapchainImagesKHR(device, swapchain, &image_count, image_array.data()));

      if (image_count != 0)
      {
        images.resize(image_count);
        vkGetSwapchainImagesKHR(device, swapchain, &image_count, images.data());
      }
    }
  }

  void VLKDevice::DestroySwapchain()
  {
    if (device && swapchain)
    {
      vkDestroySwapchainKHR(device, swapchain, nullptr);
      swapchain = nullptr;
    }
  }


  void VLKDevice::Initialize()
  {
    CreateInstance();
    CreateMessenger();
    CreateDevice();
    CreateSurface();
    CreateSwapchain();
    CreatePool();
    CreateFence();
    CreateStaging();
    CreateScratch();
  }

  void VLKDevice::Use()
  {
    auto src_image = reinterpret_cast<VLKResource*>(screen.get())->GetImage();
    {

      {
        auto begin_info = VkCommandBufferBeginInfo{};
        begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        begin_info.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
        begin_info.pInheritanceInfo = nullptr;
        BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(command_buffer, &begin_info));

        for (auto& pass : passes)
        {
          pass->Use();
        }

        BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(command_buffer));
      }

      {
        BLAST_ASSERT(VK_SUCCESS == vkResetFences(device, 1, &fence));

        std::vector<VkSemaphore> waits;
        std::vector<VkPipelineStageFlags> stages;
        std::vector<VkSemaphore> signals;

        VkSubmitInfo submit_info = {};
        submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submit_info.waitSemaphoreCount = uint32_t(waits.size());
        submit_info.pWaitSemaphores = waits.data();
        submit_info.pWaitDstStageMask = stages.data();
        submit_info.commandBufferCount = 1;
        submit_info.pCommandBuffers = &command_buffer;
        submit_info.signalSemaphoreCount = uint32_t(signals.size());
        submit_info.pSignalSemaphores = signals.data();
        BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(queue, 1, &submit_info, fence));

        BLAST_ASSERT(VK_SUCCESS == vkWaitForFences(device, 1, &fence, true, UINT64_MAX));
      }
    }


    {
      uint32_t image_index = 0;
      vkAcquireNextImageKHR(device, swapchain, std::numeric_limits<uint64_t>::max(), imageAvailableSemaphore, VK_NULL_HANDLE, &image_index);

      auto dst_image = images.at(image_index);

      VkCommandBufferBeginInfo begin_info = {};
      begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      begin_info.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      begin_info.pInheritanceInfo = nullptr;
      BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(present_command_buffer, &begin_info));

      {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = dst_image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCmdPipelineBarrier(present_command_buffer,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
          0, nullptr,
          0, nullptr,
          1, &barrier);
      }

      {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = src_image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCmdPipelineBarrier(present_command_buffer,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
          0, nullptr,
          0, nullptr,
          1, &barrier);
      }

      VkImageCopy copy = {};
      copy.srcSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
      copy.srcOffset = { 0, 0, 0 };
      copy.dstSubresource = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 };
      copy.dstOffset = { 0, 0, 0 };
      copy.extent = { extent_x, extent_y, 1 };
      vkCmdCopyImage(present_command_buffer,
        src_image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        dst_image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1, &copy);

      {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = 0;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = dst_image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCmdPipelineBarrier(present_command_buffer,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
          0, nullptr,
          0, nullptr,
          1, &barrier);
      }

      {
        VkImageMemoryBarrier barrier = {};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
        barrier.dstAccessMask = 0;
        barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
        barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.image = src_image;
        barrier.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
        vkCmdPipelineBarrier(present_command_buffer,
          VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT, 0,
          0, nullptr,
          0, nullptr,
          1, &barrier);
      }

      BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(present_command_buffer));


      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      VkSemaphore waitSemaphores[] = { imageAvailableSemaphore };
      VkPipelineStageFlags waitStages[] = { VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT };
      submitInfo.waitSemaphoreCount = 1;
      submitInfo.pWaitSemaphores = waitSemaphores;
      submitInfo.pWaitDstStageMask = waitStages;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &present_command_buffer;
      VkSemaphore signalSemaphores[] = { renderFinishedSemaphore };
      submitInfo.signalSemaphoreCount = 1;
      submitInfo.pSignalSemaphores = signalSemaphores;
      BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE));

      VkPresentInfoKHR presentInfo = {};
      presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
      presentInfo.waitSemaphoreCount = 1;
      presentInfo.pWaitSemaphores = signalSemaphores;
      VkSwapchainKHR swapchains[] = { swapchain };
      presentInfo.swapchainCount = 1;
      presentInfo.pSwapchains = swapchains;
      presentInfo.pImageIndices = &image_index;
      presentInfo.pResults = nullptr;
      BLAST_ASSERT(VK_SUCCESS == vkQueuePresentKHR(queue, &presentInfo));
    }
  }

  void VLKDevice::CreateFence()
  {
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    BLAST_ASSERT(VK_SUCCESS == vkCreateSemaphore(device, &semaphoreInfo, nullptr, &imageAvailableSemaphore));
    BLAST_ASSERT(VK_SUCCESS == vkCreateSemaphore(device, &semaphoreInfo, nullptr, &renderFinishedSemaphore));

    VkFenceCreateInfo create_info = {};
    create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
    BLAST_ASSERT(VK_SUCCESS == vkCreateFence(device, &create_info, nullptr, &fence));
  }


  void VLKDevice::DestroyFence()
  {
    if (device && imageAvailableSemaphore)
    { 
      vkDestroySemaphore(device, imageAvailableSemaphore, nullptr); 
      imageAvailableSemaphore = nullptr;
    }

    if (device && renderFinishedSemaphore)
    {
      vkDestroySemaphore(device, renderFinishedSemaphore, nullptr);
      renderFinishedSemaphore = nullptr;
    }

    if (fence)
    {
      vkDestroyFence(device, fence, nullptr);
      fence = nullptr;
    }
  }


  void VLKDevice::CreateStaging()
  {
    const auto size = staging_size;
    const auto usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
    const auto buffer = CreateBuffer(size, usage);
    const auto requirements = GetRequirements(buffer);
    const auto property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
    const auto index = GetMemoryIndex(property, requirements.memoryTypeBits);
    const auto memory = AllocateMemory(requirements.size, index, false);

    BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device, buffer, memory, 0));

    staging_buffer = buffer;
    staging_memory = memory;
  }


  void VLKDevice::DestroyStaging()
  {
    if (staging_memory)
    {
      vkFreeMemory(device, staging_memory, nullptr);
      staging_memory = nullptr;
    }

    if (staging_buffer)
    {
      vkDestroyBuffer(device, staging_buffer, nullptr);
      staging_buffer = nullptr;
    }
  }


  void VLKDevice::CreateScratch()
  {
    if (!ray_tracing_supported) return;

    const auto size = scratch_size;
    const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    const auto buffer = CreateBuffer(size, usage);
    const auto requirements = GetRequirements(buffer);
    const auto property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
    const auto index = GetMemoryIndex(property, requirements.memoryTypeBits);
    const auto memory = AllocateMemory(requirements.size, index, true);

    BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device, buffer, memory, 0));

    scratch_buffer = buffer;
    scratch_memory = memory;

    scratch_address = GetAddress(buffer);
  }



  void VLKDevice::DestroyScratch()
  {
    if (!ray_tracing_supported) return;

    if (scratch_memory)
    {
      vkFreeMemory(device, scratch_memory, nullptr);
      scratch_memory = nullptr;
    }

    if (scratch_buffer)
    {
      vkDestroyBuffer(device, scratch_buffer, nullptr);
      scratch_buffer = nullptr;
    }
  }

  uint32_t VLKDevice::GetMemoryIndex(VkMemoryPropertyFlags flags, uint32_t bits) const
  {
    for (uint32_t i = 0; i < memory.memoryTypeCount; ++i)
    {
      if (((bits >> i) & 1) == 1 && (memory.memoryTypes[i].propertyFlags & flags) == flags) return i;
    }
    return memory.memoryTypeCount;
  }

  VkDeviceAddress VLKDevice::GetAddress(VkBuffer buffer) const
  {
    if (!buffer) return 0ULL;

    VkBufferDeviceAddressInfo info{};
    info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
    info.buffer = buffer;
    return vkGetBufferDeviceAddress(device, &info);
  };

  VkBuffer VLKDevice::CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags) const
  {
    VkBuffer buffer{ nullptr };

    VkBufferCreateInfo info{};
    info.sType                  = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
    info.size                   = size;
    info.usage                  = usage;
    info.flags                  = flags;
    info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount  = 0;
    info.pQueueFamilyIndices    = nullptr;

    BLAST_ASSERT(VK_SUCCESS == vkCreateBuffer(device, &info, nullptr, &buffer));

    return buffer;
  };

  VkImage VLKDevice::CreateImage(VkImageType type, VkFormat format, VkExtent3D extent,
    uint32_t mipmaps, uint32_t layers, VkImageUsageFlags usage, VkImageCreateFlags flags) const
  {
    VkImage image{ nullptr };

    VkImageCreateInfo info{};
    info.sType                  = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    info.flags                  = flags;
    info.imageType              = type;
    info.format                 = format;
    info.extent                 = extent;
    info.mipLevels              = mipmaps;
    info.arrayLayers            = layers;
    info.samples                = VK_SAMPLE_COUNT_1_BIT;
    info.tiling                 = VK_IMAGE_TILING_OPTIMAL;
    info.usage                  = usage;
    info.sharingMode            = VK_SHARING_MODE_EXCLUSIVE;
    info.queueFamilyIndexCount  = 0;
    info.pQueueFamilyIndices    = nullptr;
    info.initialLayout          = VK_IMAGE_LAYOUT_UNDEFINED; //properties.empty() ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_PREINITIALIZED;
    BLAST_ASSERT(VK_SUCCESS == vkCreateImage(device, &info, nullptr, &image));

    return image;
  };

  VkMemoryRequirements VLKDevice::GetRequirements(VkBuffer buffer) const
  {
    VkMemoryRequirements requirements{};
    vkGetBufferMemoryRequirements(device, buffer, &requirements);

    return requirements;
  };

  VkMemoryRequirements VLKDevice::GetRequirements(VkImage image) const
  {
    VkMemoryRequirements requirements{};
    vkGetImageMemoryRequirements(device, image, &requirements);

    return requirements;
  };

  VkDeviceMemory VLKDevice::AllocateMemory(VkDeviceSize size, uint32_t index, bool addressable) const
  {
    VkDeviceMemory memory{ nullptr };

    VkMemoryAllocateFlagsInfo flags_info{};
    flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
    flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

    VkMemoryAllocateInfo info{};
    info.sType             = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    info.pNext             = addressable ? &flags_info : nullptr;
    info.allocationSize    = size;
    info.memoryTypeIndex   = index;
    BLAST_ASSERT(VK_SUCCESS == vkAllocateMemory(device, &info, nullptr, &memory));

    return memory;
  };

  //void* VLKDevice::MapMemory(VkDeviceMemory memory) const
  //{
  //  void* mapped{ nullptr };

  //  BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &mapped));
  //  
  //  return mapped;
  //}

  //void VLKDevice::UnmapMemory(VkDeviceMemory memory) const
  //{
  //  vkUnmapMemory(device, memory);
  //}

  void VLKDevice::Discard()
  {
    if (device)
    {
      vkDeviceWaitIdle(device);
    }

    if (screen) // screen MUST be released only after device jobs have been completed
    {
      screen.reset();
    }

    //for (auto& pass : passes)
    //{
    //  if (pass) { /*BLAST_LOG("Discarding queue [%s]", name.c_str());*/ pass->Discard(); }
    //}

    //for (auto& resource : resources)
    //{
    //  if (resource) { /*BLAST_LOG("Discarding resource [%s]", name.c_str());*/ resource->Discard(); }
    //}

    DestroyScratch();
    DestroyStaging();
    DestroyFence();
    DestroyPool();
    DestroySwapchain();
    DestroySurface();
    DestroyDevice();
    DestroyMessenger();
    DestroyInstance();
  }

  VLKDevice::VLKDevice(const std::string& name) 
    : Device(name)
  {
  }

  VLKDevice::~VLKDevice()
  {
    Discard();
  }
}
