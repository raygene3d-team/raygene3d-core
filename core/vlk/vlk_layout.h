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
    std::vector<VkAccelerationStructureGeometryKHR> bottom_geometries;
    std::vector<VkAccelerationStructureBuildRangeInfoKHR> bottom_range_infos;
    std::vector<VkAccelerationStructureBuildSizesInfoKHR> bottom_sizes_infos;
    std::vector<VkDeviceSize> bottom_sizes;
    std::vector<VkAccelerationStructureKHR> bottom_accelerations;
    std::vector<VkDeviceMemory> bottom_memories;
    std::vector<VkBuffer> bottom_buffers;
    std::vector<uint64_t> bottom_handles;

    VkAccelerationStructureGeometryKHR top_geometry;
    VkAccelerationStructureBuildRangeInfoKHR top_range_info;
    VkAccelerationStructureBuildSizesInfoKHR top_sizes_info;
    VkDeviceSize top_size{ 0 };
    VkAccelerationStructureKHR top_acceleration{ nullptr };
    VkDeviceMemory top_memory{ nullptr };
    VkBuffer top_buffer{ nullptr };
    uint64_t top_handle{ 0 };

    VkDeviceMemory memory_instances{ nullptr };
    VkBuffer buffer_instances{ nullptr };

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
    VLKLayout(const std::string& name,
      Device& device,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views,
      const std::pair<const Layout::Sampler*, uint32_t>& samplers = {},
      const std::pair<const Layout::RTXEntity*, uint32_t>& rtx_entities = {});
    virtual ~VLKLayout();
  };
}