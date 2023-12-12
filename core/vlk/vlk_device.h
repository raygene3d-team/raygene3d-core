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
    VkPhysicalDeviceRayTracingPipelinePropertiesKHR raytracing_properties;


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

    VkDeviceAddress scratch_address{ 0 };
    VkBuffer scratch_buffer{ nullptr };
    VkDeviceMemory scratch_memory{ nullptr };
    VkDeviceSize scratch_size{ 4 * 1024 * 1024 };

  public:
    VkBuffer GetStagingBuffer() const { return staging_buffer; }
    VkDeviceMemory GetStagingMemory() const { return staging_memory; }
    VkDeviceSize GetStagingSize() const { return staging_size; }

  public:
    VkDeviceAddress GetScratchAddress() const { return scratch_address; };
    VkBuffer GetScratchBuffer() const { return scratch_buffer; }
    VkDeviceMemory GetScratchMemory() const { return scratch_memory; }
    VkDeviceSize GetScratchSize() const { return scratch_size; }


  public:
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new VLKResource(name, *this, desc, hint, interops));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new VLKResource(name, *this, desc, hint, interops));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new VLKResource(name, *this, desc, hint, interops));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new VLKResource(name, *this, desc, hint, interops));
    }

    //const std::shared_ptr<Layout>& CreateLayout(const std::string& name) override { return layouts.emplace_back(new VLKLayout(name, *this)); }
    const std::shared_ptr<Layout>& CreateLayout(const std::string& name,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views,
      const std::pair<const Layout::Sampler*, uint32_t>& samplers,
      const std::pair<const Layout::RTXEntity*, uint32_t>& rtx_entities) override
    {
      return layouts.emplace_back(new VLKLayout(name, *this, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views, samplers, rtx_entities));
    }

    const std::shared_ptr<Config>& CreateConfig(const std::string& name,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state) override
    {
      return configs.emplace_back(new VLKConfig(name, *this, source, compilation, defines, ia_state, rc_state, ds_state, om_state));
    }

    const std::shared_ptr<Pass>& CreatePass(const std::string& name,
      Pass::Type type,
      const std::pair<const Pass::Subpass*, uint32_t>& subpasses,
      const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments = {},
      const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments = {}) override
    {
      return passes.emplace_back(new VLKPass(name, *this, type, subpasses, rt_attachments, ds_attachments));
    }

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
    VkDeviceAddress GetAddress(VkBuffer buffer) const;
    VkBuffer CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkBufferCreateFlags flags = 0) const;
    VkImage CreateImage(VkImageType type, VkFormat format, VkExtent3D extent, 
      uint32_t mipmaps, uint32_t layers, VkImageUsageFlags usage, VkImageCreateFlags flags = 0) const;
    VkMemoryRequirements GetRequirements(VkBuffer buffer) const;
    VkMemoryRequirements GetRequirements(VkImage image) const;
    VkDeviceMemory AllocateMemory(VkDeviceSize size, uint32_t index,
      bool addressable = false) const;

  //public:
  //  void* MapMemory(VkDeviceMemory memory) const;
  //  void UnmapMemory(VkDeviceMemory memory) const;

    //const VkCommandBuffer& CreateCommand() const;
    //void DestroyCommand(const VkCommandBuffer& command);

  public:
    VkDevice GetDevice() { return device; }
    uint32_t GetFamily() { return family; }
    VkQueue GetQueue() { return queue; }

  public:
    bool GetRTXSupported() const { return raytracing_supported; }
    const VkPhysicalDeviceRayTracingPipelinePropertiesKHR& GetRTXProperties() const { return  raytracing_properties; }

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
    void CreateScratch();
    void DestroyScratch();

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
