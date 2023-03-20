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
#include "../layout.h"

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
  class VLKLayout : public Layout
  {
  protected:
    VkDescriptorPool pool{ nullptr };
    VkPipelineLayout layout{ nullptr };

  protected:
    std::vector<VkSampler> sampler_states;

  protected:
    std::array<VkDescriptorSet, 1> sets;

  protected:
    std::vector<VkPushConstantRange> constants; //TODO: ???
    std::vector<VkDescriptorSetLayout> tables;

  protected:
    std::vector<VkDeviceSize> bottom_sizes;
    std::vector<VkAccelerationStructureNV> bottom_accelerations;
    std::vector<VkDeviceMemory> bottom_memories;
    std::vector<uint64_t> bottom_handles;

    VkDeviceSize top_size{ 0 };
    VkAccelerationStructureNV top_acceleration{ nullptr };
    VkDeviceMemory top_memory{ nullptr };
    uint64_t top_handle{ 0 };

    VkDeviceMemory memoryInstances{ nullptr };
    VkBuffer bufferInstances{ nullptr };

    VkDeviceMemory memoryScratch{ nullptr };
    VkBuffer bufferScratch{ nullptr };

    //std::vector<VkGeometryNV> geometries;

    VkCommandBuffer commandBuffer{ nullptr };
    //VkCommandPool commandPool{ nullptr };
    VkFence fence{ nullptr };


  protected:
    PFN_vkCreateAccelerationStructureNV vkCreateAccelerationStructureNV{ nullptr };
    PFN_vkDestroyAccelerationStructureNV vkDestroyAccelerationStructureNV{ nullptr };
    PFN_vkBindAccelerationStructureMemoryNV vkBindAccelerationStructureMemoryNV{ nullptr };
    PFN_vkGetAccelerationStructureHandleNV vkGetAccelerationStructureHandleNV{ nullptr };
    PFN_vkGetAccelerationStructureMemoryRequirementsNV vkGetAccelerationStructureMemoryRequirementsNV{ nullptr };
    PFN_vkCmdBuildAccelerationStructureNV vkCmdBuildAccelerationStructureNV{ nullptr };

  //public:
  //  std::shared_ptr<Command> CreateCommand(const std::string& name) override { return commands.emplace_back(new VLKCommand(name, *this)); }

  public:
    const VkPipelineLayout& GetLayout() const { return layout; }

  public:
    const VkDescriptorSet* GetSetItems() const { return sets.data(); }
    uint32_t GetSetCount() const { return 1u; }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    VLKLayout(const std::string& name, Device& device);
    virtual ~VLKLayout();
  };
}