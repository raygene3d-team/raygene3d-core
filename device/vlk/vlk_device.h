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


#pragma once
#include "../device.h"
#include "vlk_resource.h"
#include "vlk_layout.h"
#include "vlk_config.h"
#include "vlk_pass.h"

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#elif _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __OBJC__
#define VK_USE_PLATFORM_METAL_EXT
#endif
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>

namespace RayGene3D
{
  class VLKDevice : public Device
  {
  protected:
    VkInstance instance{ nullptr };
    VkPhysicalDevice adapter{ nullptr };

    VkPhysicalDeviceProperties properties{};
    VkPhysicalDeviceFeatures features{};
    VkPhysicalDeviceMemoryProperties memory{};
    
    bool raytracing_supported{ false };
    VkPhysicalDeviceRayTracingPropertiesNV raytracing_properties;


    VkDevice device{ nullptr };
    uint32_t family{ uint32_t(-1) };
    VkQueue queue{ nullptr };

    VkCommandPool command_pool{ nullptr };
    VkCommandBuffer command_buffer{ nullptr };

    VkSurfaceKHR surface{ nullptr };
    VkSwapchainKHR swapchain{ nullptr };
    std::vector<VkImage> images;
    VkCommandBuffer present_command_buffer{ nullptr };

    VkSemaphore imageAvailableSemaphore{ nullptr };
    VkSemaphore renderFinishedSemaphore{ nullptr };
    VkFence fence{ nullptr };

    VkDebugUtilsMessengerEXT messenger{ nullptr };

    VkBuffer staging_buffer{ nullptr };
    VkDeviceMemory staging_memory{ nullptr };
    VkDeviceSize staging_size{ 32 * 1024 * 1024 };

  public:
    VkBuffer GetStagingBuffer() { return staging_buffer; }
    VkDeviceMemory GetStagingMemory() { return staging_memory; }
    VkDeviceSize GetStagingSize() { return staging_size; }


  public:
    std::shared_ptr<Resource> CreateResource(const std::string& name) override { return resources.emplace_back(new VLKResource(name, *this)); }
    std::shared_ptr<Resource> ShareResource(const std::string& name) override { for (auto& resource : resources) if (resource->GetName() == name) return resource; return nullptr; }
    std::shared_ptr<Layout> CreateLayout(const std::string& name) override { return layouts.emplace_back(new VLKLayout(name, *this)); }
    std::shared_ptr<Config> CreateConfig(const std::string& name) override { return configs.emplace_back(new VLKConfig(name, *this)); }
    std::shared_ptr<Pass> CreatePass(const std::string& name) override { return passes.emplace_back(new VLKPass(name, *this)); }

  public:
    const VkPhysicalDeviceProperties& GetProperties() const { return properties; }
    const VkPhysicalDeviceFeatures& GetFeatures() const { return features; }
    //const VkPhysicalDeviceMemoryProperties& GetMemory()  const { return memory; }

  public:
    VkCommandPool GetCommandPool() const { return command_pool; } //TODO: Remove
    VkCommandBuffer GetCommadBuffer() const { return command_buffer; }


  //public:
  //  void AllocateMemory(VkDeviceMemory& memory, VkBuffer buffer, VkMemoryPropertyFlags flags);
  //  void AllocateMemory(VkDeviceMemory& memory, VkImage image, VkMemoryPropertyFlags flags);
  //  void ReleaseMemory(VkDeviceMemory& memory);


  public:
    uint32_t GetMemoryIndex(VkMemoryPropertyFlags flags, uint32_t bits) const;

    //const VkCommandBuffer& CreateCommand() const;
    //void DestroyCommand(const VkCommandBuffer& command);

  public:
    VkDevice GetDevice() { return device; }
    uint32_t GetFamily() { return family; }
    VkQueue GetQueue() { return queue; }

  public:
    bool GetRTXSupported() const { return raytracing_supported; }
    const VkPhysicalDeviceRayTracingPropertiesNV& GetRTXProperties() const { return  raytracing_properties; }

  protected:
    void CreateInstance();
    void DestroyInstance();
    void CreateSurface();
    void DestroySurface();
    void CreateDevice();
    void DestroyDevice();
    void CreateSwapchain();
    void DestroySwapchain();
    void CreatePool();
    void DestroyPool();
    void CreateFence();
    void DestroyFence();
    void CreateMessenger();
    void DestroyMessenger();
    void CreateStaging();
    void DestroyStaging();

  public:
    VkDevice GetDevice() const { return device; }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    VLKDevice(const std::string& name);
    virtual ~VLKDevice();
  };
}
