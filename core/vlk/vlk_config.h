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
#include "../config.h"
#include "vlk_batch.h"

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
  class VLKConfig : public Config
  {
  protected:
    VkShaderModule vs_module{ nullptr };
    VkShaderModule hs_module{ nullptr };
    VkShaderModule ds_module{ nullptr };
    VkShaderModule gs_module{ nullptr };
    VkShaderModule ps_module{ nullptr };
    VkShaderModule cs_module{ nullptr };

  protected:
    VkShaderModule task_module{ nullptr };
    VkShaderModule mesh_module{ nullptr };

  protected:
    VkShaderModule rgen_module{ nullptr };
    VkShaderModule isec_module{ nullptr };
    VkShaderModule chit_module{ nullptr };
    VkShaderModule ahit_module{ nullptr };
    VkShaderModule miss_module{ nullptr };
    VkShaderModule call_module{ nullptr };

  protected:
    std::vector<VkPipelineShaderStageCreateInfo> stages;
    std::vector<VkRayTracingShaderGroupCreateInfoKHR> groups;

  //public:
  //  std::shared_ptr<Pipeline> CreatePipeline(const std::string& name) override { return pipelines.emplace_back(new VLKPipeline(name, *this)); }

  public:
    VkShaderModule GetVSModule() const { return vs_module; }
    VkShaderModule GetHSModule() const { return hs_module; }
    VkShaderModule GetDSModule() const { return ds_module; }
    VkShaderModule GetGSModule() const { return gs_module; }
    VkShaderModule GetPSModule() const { return ps_module; }
    VkShaderModule GetCSModule() const { return cs_module; }

  protected:
    std::vector<VkVertexInputBindingDescription> input_bindings;
    std::vector<VkVertexInputAttributeDescription> input_attributes;
    VkPipelineVertexInputStateCreateInfo input_state{};
    VkPipelineInputAssemblyStateCreateInfo assembly_state{};

    std::vector<VkViewport> raster_viewports;
    std::vector<VkRect2D> raster_scissors;
    VkPipelineRasterizationStateCreateInfo rasterization_state{};
    VkPipelineMultisampleStateCreateInfo multisample_state{};
    VkPipelineDepthStencilStateCreateInfo depthstencil_state{};
    VkPipelineTessellationStateCreateInfo tessellation_state{};
    std::vector<VkPipelineColorBlendAttachmentState> colorblend_attachments;
    VkPipelineColorBlendStateCreateInfo colorblend_state{};
    VkPipelineViewportStateCreateInfo viewport_state{};

  protected:
    VkPipelineLayout pipeline_layout{ nullptr };

  public:
    const VkPipelineVertexInputStateCreateInfo& GetInputState() const { return input_state; }
    const VkPipelineInputAssemblyStateCreateInfo& GetAssemblyState() const { return assembly_state; }
    const VkPipelineRasterizationStateCreateInfo& GetRasterizationState() const { return rasterization_state; }
    const VkPipelineMultisampleStateCreateInfo& GetMultisampleState() const { return multisample_state; }
    const VkPipelineDepthStencilStateCreateInfo& GetDepthstencilState() const { return depthstencil_state; }
    const VkPipelineTessellationStateCreateInfo& GetTessellationState() const { return tessellation_state; }
    const VkPipelineColorBlendStateCreateInfo& GetColorblendState() const { return colorblend_state; }
    const VkPipelineViewportStateCreateInfo& GetViewportState() const { return viewport_state; }

  public:
    uint32_t GetStageCount() const { return uint32_t(stages.size()); }
    const VkPipelineShaderStageCreateInfo* GetStageArray() const { return stages.data(); }
    uint32_t GetGroupCount() const { return uint32_t(groups.size()); }
    const VkRayTracingShaderGroupCreateInfoKHR* GetGroupArray() const { return groups.data(); }

  public:
    const std::shared_ptr<Batch>& CreateBatch(const std::string& name,
      const std::pair<const Batch::Entity*, uint32_t>& entities,
      const std::pair<const Batch::Sampler*, uint32_t>& samplers,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views
    ) override
    {
      return batches.emplace_back(new VLKBatch(name, *this, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    VLKConfig(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    virtual ~VLKConfig();
  };
}