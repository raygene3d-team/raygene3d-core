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


#include "vlk_pass.h"
#include "vlk_config.h"
#include "vlk_layout.h"
#include "vlk_device.h"
#include "vlk_view.h"


namespace RayGene3D
{
  const auto get_format = [](Format format)
  {
    switch (format)
    {
    default: return VK_FORMAT_UNDEFINED;
    case FORMAT_R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
    case FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
    case FORMAT_R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
    case FORMAT_R32G32B32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
    case FORMAT_R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
    case FORMAT_R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
    case FORMAT_R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
    case FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
    case FORMAT_R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
    case FORMAT_R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
    case FORMAT_R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
    case FORMAT_R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
    case FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
    case FORMAT_R32G32_SINT: return VK_FORMAT_R32G32_SINT;
    case FORMAT_D32_FLOAT_S8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
    case FORMAT_R10G10B10A2_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
    case FORMAT_R10G10B10A2_UINT: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
    case FORMAT_R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
    case FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
    case FORMAT_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
    case FORMAT_R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
    case FORMAT_R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
    case FORMAT_R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
    case FORMAT_R16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
    case FORMAT_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
    case FORMAT_R16G16_UINT: return VK_FORMAT_R16G16_UINT;
    case FORMAT_R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
    case FORMAT_R16G16_SINT: return VK_FORMAT_R16G16_SINT;
    case FORMAT_D32_FLOAT: return VK_FORMAT_D32_SFLOAT;
    case FORMAT_R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
    case FORMAT_R32_UINT: return VK_FORMAT_R32_UINT;
    case FORMAT_R32_SINT: return VK_FORMAT_R32_SINT;
    case FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
    case FORMAT_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
    case FORMAT_R8G8_UINT: return VK_FORMAT_R8G8_UINT;
    case FORMAT_R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
    case FORMAT_R8G8_SINT: return VK_FORMAT_R8G8_SINT;
    case FORMAT_R16_FLOAT: return VK_FORMAT_R16_SFLOAT;
    case FORMAT_D16_UNORM: return VK_FORMAT_D16_UNORM;
    case FORMAT_R16_UNORM: return VK_FORMAT_R16_UNORM;
    case FORMAT_R16_UINT: return VK_FORMAT_R16_UINT;
    case FORMAT_R16_SNORM: return VK_FORMAT_R16_SNORM;
    case FORMAT_R16_SINT: return VK_FORMAT_R16_SINT;
    case FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
    case FORMAT_R8_UINT: return VK_FORMAT_R8_UINT;
    case FORMAT_R8_SNORM: return VK_FORMAT_R8_SNORM;
    case FORMAT_R8_SINT: return VK_FORMAT_R8_SINT;
    case FORMAT_R9G9B9E5_SHAREDEXP: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
    case FORMAT_R8G8_B8G8_UNORM: return VK_FORMAT_G8B8G8R8_422_UNORM;
    case FORMAT_G8R8_G8B8_UNORM: return VK_FORMAT_B8G8R8G8_422_UNORM;
    case FORMAT_BC1_UNORM: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
    case FORMAT_BC1_SRGB: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
    case FORMAT_BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
    case FORMAT_BC2_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
    case FORMAT_BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
    case FORMAT_BC3_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
    case FORMAT_BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
    case FORMAT_BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
    case FORMAT_BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
    case FORMAT_BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
    case FORMAT_B5G6R5_UNORM: return VK_FORMAT_B5G6R5_UNORM_PACK16;
    case FORMAT_B5G5R5A1_UNORM: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
    case FORMAT_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
      // case FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
    case FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
      // case FORMAT_B8G8R8X8_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
    case FORMAT_BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
    case FORMAT_BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
    case FORMAT_BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
    case FORMAT_BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
    }
  };

  void VLKPass::Initialize()
  {
    auto device = reinterpret_cast<VLKDevice*>(&this->GetDevice());

    if (subpasses.empty()) return;

  	if (device->GetRTXSupported())
    {
      vkCreateRayTracingPipelinesNV = reinterpret_cast<PFN_vkCreateRayTracingPipelinesNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkCreateRayTracingPipelinesNV"));
      vkGetRayTracingShaderGroupHandlesNV = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetRayTracingShaderGroupHandlesNV"));
      vkCmdTraceRaysNV = reinterpret_cast<PFN_vkCmdTraceRaysNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkCmdTraceRaysNV"));
    }

    if (type == TYPE_GRAPHIC)
    {
      {
        std::vector<VkClearValue> rt_attachment_values(rt_values.size());
        for (size_t i = 0; i < rt_views.size(); ++i)
        {
          const auto& rt_value = rt_values[i];

          rt_attachment_values[i].color.float32[0] = rt_value ? rt_value.value()[0] : 1.0f;
          rt_attachment_values[i].color.float32[1] = rt_value ? rt_value.value()[1] : 0.0f;
          rt_attachment_values[i].color.float32[2] = rt_value ? rt_value.value()[2] : 1.0f;
          rt_attachment_values[i].color.float32[3] = rt_value ? rt_value.value()[3] : 0.0f;
        }

        std::vector<VkClearValue> ds_attachment_values(ds_values.size());
        for (size_t i = 0; i < ds_views.size(); ++i)
        {
          const auto& ds_value = ds_values[i];

          ds_attachment_values[i].depthStencil.depth = ds_value.first ? ds_value.first.value() : 0.0f;
          ds_attachment_values[i].depthStencil.stencil = ds_value.second ? ds_value.second.value() : 0;
        }

        attachment_values.clear();
        std::move(rt_attachment_values.begin(), rt_attachment_values.end(), std::back_inserter(attachment_values));
        std::move(ds_attachment_values.begin(), ds_attachment_values.end(), std::back_inserter(attachment_values));
      }

      {
        std::vector<VkAttachmentDescription> rt_attachment_desc(rt_views.size());
        for (size_t i = 0; i < rt_views.size(); ++i)
        {
          const auto& rt_value = rt_values[i];

          rt_attachment_desc[i].format = get_format(rt_views[i]->GetResource().GetFormat());
          rt_attachment_desc[i].samples = VK_SAMPLE_COUNT_1_BIT;
          rt_attachment_desc[i].loadOp = rt_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
          rt_attachment_desc[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
          rt_attachment_desc[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          rt_attachment_desc[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          rt_attachment_desc[i].initialLayout = rt_value ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL;
          rt_attachment_desc[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        std::vector<VkAttachmentDescription> ds_attachment_desc(ds_views.size());
        for (size_t i = 0; i < ds_views.size(); ++i)
        {
          const auto& ds_value = ds_values[i];

          ds_attachment_desc[i].format = get_format(ds_views[i]->GetResource().GetFormat());
          ds_attachment_desc[i].samples = VK_SAMPLE_COUNT_1_BIT;
          ds_attachment_desc[i].loadOp = ds_value.first ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
          ds_attachment_desc[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
          ds_attachment_desc[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          ds_attachment_desc[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          ds_attachment_desc[i].initialLayout = ds_value.first ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL;
          ds_attachment_desc[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;
        }

        std::vector<VkAttachmentDescription> attachment_desc;
        std::move(rt_attachment_desc.begin(), rt_attachment_desc.end(), std::back_inserter(attachment_desc));
        std::move(ds_attachment_desc.begin(), ds_attachment_desc.end(), std::back_inserter(attachment_desc));


        std::vector<VkAttachmentReference> rt_attachment_refs(rt_views.size());
        for (size_t i = 0; i < rt_views.size(); ++i)
        {
          rt_attachment_refs[i].attachment = uint32_t(i);
          rt_attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        std::vector<VkAttachmentReference> ds_attachment_refs(ds_views.size());
        for (size_t i = 0; i < ds_views.size(); ++i)
        {
          ds_attachment_refs[i].attachment = uint32_t(i + rt_attachment_refs.size());
          ds_attachment_refs[i].layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
        }

        VkSubpassDescription subpass = {};
        subpass.flags = 0;
        subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
        subpass.inputAttachmentCount = 0;
        subpass.pInputAttachments = nullptr;
        subpass.colorAttachmentCount = uint32_t(rt_attachment_refs.size());
        subpass.pColorAttachments = rt_attachment_refs.data();
        subpass.pResolveAttachments = nullptr;
        subpass.pDepthStencilAttachment = ds_attachment_refs.empty() ? nullptr : &ds_attachment_refs.at(0);
        subpass.preserveAttachmentCount = 0;
        subpass.pPreserveAttachments = nullptr;

        VkSubpassDependency dependency = {};
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

        VkRenderPassCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
        create_info.attachmentCount = uint32_t(attachment_desc.size());
        create_info.pAttachments = attachment_desc.data();
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        create_info.dependencyCount = 1;
        create_info.pDependencies = &dependency;
        BLAST_ASSERT(VK_SUCCESS == vkCreateRenderPass(device->GetDevice(), &create_info, nullptr, &renderpass));
      }


      {
        std::vector<VkImageView> rt_attachment_views(rt_views.size());
        for (size_t i = 0; i < rt_views.size(); ++i)
        {
          const auto& rt_view = rt_views[i];
          rt_attachment_views[i] = (reinterpret_cast<VLKView*>(rt_view.get()))->GetView();
          extent_x = std::max(extent_x, rt_view->GetResource().GetExtentX());
          extent_y = std::max(extent_y, rt_view->GetResource().GetExtentY());
        }

        std::vector<VkImageView> ds_attachment_views(ds_views.size());
        for (size_t i = 0; i < ds_views.size(); ++i)
        {
          const auto& ds_view = ds_views[i];
          ds_attachment_views[i] = (reinterpret_cast<VLKView*>(ds_view.get()))->GetView();
          extent_x = std::max(extent_x, ds_view->GetResource().GetExtentX());
          extent_y = std::max(extent_y, ds_view->GetResource().GetExtentY());
        }

        attachment_views.clear();
        std::move(rt_attachment_views.begin(), rt_attachment_views.end(), std::back_inserter(attachment_views));
        std::move(ds_attachment_views.begin(), ds_attachment_views.end(), std::back_inserter(attachment_views));


        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = renderpass;
        create_info.attachmentCount = uint32_t(attachment_views.size());
        create_info.pAttachments = attachment_views.data();
        create_info.width = extent_x;
        create_info.height = extent_y;
        create_info.layers = 1;
        BLAST_ASSERT(VK_SUCCESS == vkCreateFramebuffer(device->GetDevice(), &create_info, nullptr, &framebuffer));
      }

      subpass_proxies.resize(subpasses.size());
      for (uint32_t j = 0; j < subpasses.size(); ++j)
      {
        const auto& subpass = subpasses[j];
        auto& subpass_proxy = subpass_proxies[j];

        auto shader = reinterpret_cast<VLKConfig*>(subpass.shader.get());
        auto layout = reinterpret_cast<VLKLayout*>(subpass.layout.get());

        VkGraphicsPipelineCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        create_info.flags = 0;
        create_info.stageCount = shader->GetStageCount();
        create_info.pStages = shader->GetStageArray();
        create_info.pVertexInputState = &shader->GetInputState();
        create_info.pInputAssemblyState = &shader->GetAssemblyState();
        create_info.pViewportState = &shader->GetViewportState();
        create_info.pRasterizationState = &shader->GetRasterizationState();
        create_info.pMultisampleState = &shader->GetMultisampleState();
        create_info.pDepthStencilState = &shader->GetDepthstencilState();
        create_info.pTessellationState = &shader->GetTessellationState();
        create_info.pColorBlendState = &shader->GetColorblendState();
        create_info.pDynamicState = nullptr;
        create_info.layout = layout->GetLayout();
        create_info.renderPass = renderpass;
        create_info.subpass = 0;
        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;
        BLAST_ASSERT(VK_SUCCESS == vkCreateGraphicsPipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &subpass_proxy.pipeline));
      }
    }

    if (type == TYPE_COMPUTE)
    {
      subpass_proxies.resize(subpasses.size());
      for (uint32_t j = 0; j < subpasses.size(); ++j)
      {
        const auto& subpass = subpasses[j];
        auto& subpass_proxy = subpass_proxies[j];

        auto shader = reinterpret_cast<VLKConfig*>(subpass.shader.get());
        auto layout = reinterpret_cast<VLKLayout*>(subpass.layout.get());

        VkComputePipelineCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
        create_info.flags = 0;
        create_info.stage = shader->GetStageArray()[0];
        create_info.layout = layout->GetLayout();
        create_info.basePipelineHandle = VK_NULL_HANDLE;
        create_info.basePipelineIndex = -1;
        BLAST_ASSERT(VK_SUCCESS == vkCreateComputePipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &subpass_proxy.pipeline));
      }
    }

    if (type == TYPE_RAYTRACING)
    {
      if (device->GetRTXSupported())
      {
        subpass_proxies.resize(subpasses.size());
        for (uint32_t j = 0; j < subpasses.size(); ++j)
        {
          const auto& subpass = subpasses[j];
          auto& subpass_proxy = subpass_proxies[j];

          auto shader = reinterpret_cast<VLKConfig*>(subpass.shader.get());
          auto layout = reinterpret_cast<VLKLayout*>(subpass.layout.get());

          VkRayTracingPipelineCreateInfoNV create_info = {};
          create_info.sType = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_NV;
          create_info.flags = 0;
          create_info.stageCount = shader->GetStageCount();
          create_info.pStages = shader->GetStageArray();
          create_info.groupCount = shader->GetGroupCount();
          create_info.pGroups = shader->GetGroupArray();
          create_info.maxRecursionDepth = 1;
          create_info.layout = layout->GetLayout();
          create_info.basePipelineHandle = VK_NULL_HANDLE;
          BLAST_ASSERT(VK_SUCCESS == vkCreateRayTracingPipelinesNV(device->GetDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &subpass_proxy.pipeline));

          const auto binding_size = device->GetRTXProperties().shaderGroupHandleSize;
          const auto binding_align = device->GetRTXProperties().shaderGroupBaseAlignment;

          VkBufferCreateInfo buffer_info = {};
          buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
          buffer_info.flags = 0;
          buffer_info.size = shader->GetGroupCount() * binding_align;
          buffer_info.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
          buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
          buffer_info.queueFamilyIndexCount = 0;
          buffer_info.pQueueFamilyIndices = nullptr;
          BLAST_ASSERT(VK_SUCCESS == vkCreateBuffer(device->GetDevice(), &buffer_info, nullptr, &subpass_proxy.bufferLayout));

          auto* binding_data = new uint8_t[shader->GetGroupCount() * binding_align];
          BLAST_ASSERT(VK_SUCCESS == vkGetRayTracingShaderGroupHandlesNV(device->GetDevice(), subpass_proxy.pipeline, 0, shader->GetGroupCount(), shader->GetGroupCount() * binding_align, binding_data));


          VkMemoryRequirements requirements;
          vkGetBufferMemoryRequirements(device->GetDevice(), subpass_proxy.bufferLayout, &requirements);
          VkMemoryAllocateInfo allocateInfo = {};
          allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
          allocateInfo.allocationSize = requirements.size;
          allocateInfo.memoryTypeIndex = device->GetMemoryIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT, requirements.memoryTypeBits);
          BLAST_ASSERT(VK_SUCCESS == vkAllocateMemory(device->GetDevice(), &allocateInfo, nullptr, &subpass_proxy.memoryLayout));

          uint8_t* mapped = nullptr;
          BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), subpass_proxy.memoryLayout, 0, VK_WHOLE_SIZE, 0, (void**)&mapped));
          for (uint32_t i = 0; i < shader->GetGroupCount(); ++i)
          {
            memcpy(mapped + i * binding_align, binding_data + i * binding_size, binding_size);

            const auto temp = reinterpret_cast<const uint64_t*>(mapped + i * binding_align);
            BLAST_LOG("RTX shader handle %d: %ld %ld %ld %ld", i, temp[0], temp[1], temp[2], temp[3]);
          }
          //BLAST_ASSERT(VK_SUCCESS == vkGetRayTracingShaderGroupHandlesNV(device->GetDevice(), vk_pipeline, 0, shader->GetGroupCount(), requirements.size, mapped));
          vkUnmapMemory(device->GetDevice(), subpass_proxy.memoryLayout);

          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), subpass_proxy.bufferLayout, subpass_proxy.memoryLayout, 0));
        }
      }
    }

  }



  void VLKPass::Use()
  {
    if (!enabled) return;

    auto device = reinterpret_cast<VLKDevice*>(&this->GetDevice());

    if (subpasses.empty()) return;

    const auto command_buffer = device->GetCommadBuffer();


    if (type == TYPE_GRAPHIC)
    {
      auto pass_info = VkRenderPassBeginInfo{};
      pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      pass_info.renderPass = renderpass;
      pass_info.framebuffer = framebuffer;
      pass_info.renderArea.offset = { 0, 0 };
      pass_info.renderArea.extent = { extent_x, extent_y };
      pass_info.clearValueCount = uint32_t(attachment_values.size());
      pass_info.pClearValues = attachment_values.data();

      vkCmdBeginRenderPass(command_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);

      for (uint32_t j = 0; j < subpasses.size(); ++j)
      {
        const auto& subpass = subpasses[j];
        auto& subpass_proxy = subpass_proxies[j];

        auto shader = reinterpret_cast<VLKConfig*>(subpass.shader.get());
        auto binding = reinterpret_cast<VLKLayout*>(subpass.layout.get());

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, subpass_proxy.pipeline);

        const auto layout = binding->GetLayout();
        const auto set_items = binding->GetSetItems();
        const auto set_count = binding->GetSetCount();
        const auto offset_items = subpass.sb_offsets.data();
        const auto offset_count = uint32_t(subpass.sb_offsets.size());
        if (set_count > 0)
        {
          vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, set_count, set_items, offset_count, offset_items);
        }


        const auto va_limit = 16u;
        std::array<uint32_t, va_limit> va_strides;
        std::array<VkDeviceSize, va_limit> va_offsets;
        std::array<VkBuffer, va_limit> va_items;

        const auto va_count = std::min(va_limit, uint32_t(subpass.va_views.size()));
        for (uint32_t k = 0; k < va_count; ++k)
        {
          const auto& va_view = subpass.va_views[k];
          if (va_view)
          {
            va_items[k] = (reinterpret_cast<VLKResource*>(&va_view->GetResource()))->GetBuffer();
            va_offsets[k] = va_view->GetByteOffset();
          }
        }

        if (va_count > 0)
        {
          vkCmdBindVertexBuffers(command_buffer, 0, va_count, va_items.data(), va_offsets.data());
        }


        const auto ia_limit = 1u;
        std::array<VkIndexType, ia_limit> ia_formats;
        std::array<VkDeviceSize, ia_limit> ia_offsets;
        std::array<VkBuffer, ia_limit> ia_items;

        const auto ia_count = std::min(ia_limit, uint32_t(subpass.ia_views.size()));
        for (uint32_t k = 0; k < ia_count; ++k)
        {
          const auto& ia_view = subpass.ia_views[k];
          if (ia_view)
          {
            ia_items[k] = (reinterpret_cast<VLKResource*>(&ia_view->GetResource()))->GetBuffer();
            ia_offsets[k] = ia_view->GetByteOffset();            
            ia_formats[k] = shader->GetIndexer()
              == Config::INDEXER_32_BIT ? VK_INDEX_TYPE_UINT32
              : Config::INDEXER_16_BIT ? VK_INDEX_TYPE_UINT16
              : VK_INDEX_TYPE_MAX_ENUM;
          }
        }

        if (ia_count > 0)
        {
          vkCmdBindIndexBuffer(command_buffer, ia_items[0], ia_offsets[0], ia_formats[0]);
        }

        const auto aa_limit = 32u;
        std::array<uint32_t, aa_limit> aa_strides;
        std::array<VkDeviceSize, aa_limit> aa_offsets;
        std::array<VkBuffer, aa_limit> aa_items;

        const auto aa_count = std::min(aa_limit, uint32_t(subpass.aa_views.size()));
        for (uint32_t k = 0; k < aa_count; ++k)
        {
          const auto& aa_view = subpass.aa_views[k];
          if (aa_view)
          {
            aa_items[k] = (reinterpret_cast<VLKResource*>(&aa_view->GetResource()))->GetBuffer();
            aa_strides[k] = aa_view->GetByteCount();
            aa_offsets[k] = aa_view->GetByteOffset();
          }
        }

        for (uint32_t k = 0; k < aa_count; ++k)
        {
          vkCmdDrawIndexedIndirect(command_buffer, aa_items[k], aa_offsets[k], 1, aa_strides[k]);
        }

        for (const auto& graphic_task : subpass.graphic_tasks)
        {
          vkCmdDrawIndexed(command_buffer, graphic_task.idx_count, graphic_task.ins_count, graphic_task.idx_offset, graphic_task.vtx_offset, graphic_task.ins_offset);
        }
      }

      vkCmdEndRenderPass(command_buffer);
    }


    if (type == TYPE_COMPUTE)
    {
      for (uint32_t j = 0; j < subpasses.size(); ++j)
      {
        const auto& subpass = subpasses[j];
        auto& subpass_proxy = subpass_proxies[j];

        auto shader = reinterpret_cast<VLKConfig*>(subpass.shader.get());
        auto binding = reinterpret_cast<VLKLayout*>(subpass.layout.get());

        vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, subpass_proxy.pipeline);

        const auto layout = binding->GetLayout();
        const auto set_items = binding->GetSetItems();
        const auto set_count = binding->GetSetCount();
        if (set_count > 0)
        {
          vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, set_count, set_items, 0, nullptr);
        }

        const auto aa_limit = 32u;
        std::array<uint32_t, aa_limit> aa_strides;
        std::array<VkDeviceSize, aa_limit> aa_offsets;
        std::array<VkBuffer, aa_limit> aa_items;

        const auto aa_count = std::min(aa_limit, uint32_t(subpass.aa_views.size()));
        for (uint32_t k = 0; k < aa_count; ++k)
        {
          const auto& aa_view = subpass.aa_views[k];
          if (aa_view)
          {
            aa_items[k] = (reinterpret_cast<VLKResource*>(&aa_view->GetResource()))->GetBuffer();
            aa_strides[k] = aa_view->GetByteCount();
            aa_offsets[k] = aa_view->GetByteOffset();
          }
        }
        for (uint32_t k = 0; k < aa_count; ++k)
        {
          vkCmdDispatchIndirect(command_buffer, aa_items[k], aa_offsets[k]);
        }

        for (const auto& compute_task : subpass.compute_tasks)
        {
          vkCmdDispatch(command_buffer, compute_task.grid_x, compute_task.grid_y, compute_task.grid_z);
        }
      }
    }


    if (type == TYPE_RAYTRACING)
    {
      if (device->GetRTXSupported())
      {
        for (uint32_t j = 0; j < subpasses.size(); ++j)
        {
          const auto& subpass = subpasses[j];
          auto& subpass_proxy = subpass_proxies[j];

          auto shader = reinterpret_cast<VLKConfig*>(subpass.shader.get());
          auto binding = reinterpret_cast<VLKLayout*>(subpass.layout.get());

          vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, subpass_proxy.pipeline);
          const auto layout = binding->GetLayout();
          const auto set_items = binding->GetSetItems();
          const auto set_count = binding->GetSetCount();
          if (set_count > 0)
          {
            vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_NV, layout, 0, set_count, set_items, 0, nullptr);
          }
          const auto binding_size = device->GetRTXProperties().shaderGroupHandleSize;
          const auto binding_align = device->GetRTXProperties().shaderGroupBaseAlignment;
          const auto extent_x = device->GetExtentX();
          const auto extent_y = device->GetExtentY();
          vkCmdTraceRaysNV(command_buffer,
            subpass_proxy.bufferLayout, 0 * binding_align,
            subpass_proxy.bufferLayout, 1 * binding_align, binding_align,
            subpass_proxy.bufferLayout, 2 * binding_align, binding_align,
            VK_NULL_HANDLE, 0, 0,
            extent_x, extent_y, 1);
        }
      }
    }

    vkCmdPipelineBarrier(command_buffer,
      VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, 0,
      0, nullptr,
      0, nullptr,
      0, nullptr);

  }

  void VLKPass::Discard()
  {
    auto device = reinterpret_cast<VLKDevice*>(&this->GetDevice());


    for (uint32_t j = 0; j < subpass_proxies.size(); ++j)
    {
      auto& subpass_proxy = subpass_proxies[j];

      if (subpass_proxy.pipeline)
      {
        vkDestroyPipeline(device->GetDevice(), subpass_proxy.pipeline, nullptr);
        subpass_proxy.pipeline = nullptr;
      }

      //RTX section
      if (subpass_proxy.bufferLayout)
      {
        vkDestroyBuffer(device->GetDevice(), subpass_proxy.bufferLayout, nullptr);
        subpass_proxy.bufferLayout = nullptr;
      }

      if (subpass_proxy.memoryLayout)
      {
        vkFreeMemory(device->GetDevice(), subpass_proxy.memoryLayout, nullptr);
        subpass_proxy.memoryLayout = nullptr;
      }
    }
    subpass_proxies.clear();

    if (framebuffer)
    {
      vkDestroyFramebuffer(device->GetDevice(), framebuffer, nullptr);
      framebuffer = nullptr;
    }

    if (renderpass)
    {
      vkDestroyRenderPass(device->GetDevice(), renderpass, nullptr);
      renderpass = nullptr;
    }

  }

  VLKPass::VLKPass(const std::string& name, Device& device)
    : Pass(name, device)
  {
  }

  VLKPass::~VLKPass()
  {
    VLKPass::Discard();
  }
}
