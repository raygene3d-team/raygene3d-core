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
#include "vlk_state.h"
#include "vlk_batch.h"
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

    if (type == TYPE_GRAPHIC)
    {
      {
        std::vector<VkClearValue> rt_attachment_values(rt_attachments.size());
        std::vector<VkAttachmentDescription> rt_attachment_descs(rt_attachments.size());
        std::vector<VkImageView> rt_attachment_views(rt_attachments.size());
        for (size_t i = 0; i < rt_attachments.size(); ++i)
        {
          const auto& rt_value = rt_attachments[i].value;
          const auto& rt_view = rt_attachments[i].view;

          rt_attachment_values[i].color.float32[0] = rt_value ? rt_value.value()[0] : 1.0f;
          rt_attachment_values[i].color.float32[1] = rt_value ? rt_value.value()[1] : 0.0f;
          rt_attachment_values[i].color.float32[2] = rt_value ? rt_value.value()[2] : 1.0f;
          rt_attachment_values[i].color.float32[3] = rt_value ? rt_value.value()[3] : 0.0f;

          rt_attachment_descs[i].format = get_format(rt_view->GetResource().GetFormat());
          rt_attachment_descs[i].samples = VK_SAMPLE_COUNT_1_BIT;
          rt_attachment_descs[i].loadOp = rt_value ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
          rt_attachment_descs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
          rt_attachment_descs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          rt_attachment_descs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          rt_attachment_descs[i].initialLayout = rt_value ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL;
          rt_attachment_descs[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

          rt_attachment_views[i] = (reinterpret_cast<VLKView*>(rt_view.get()))->GetView();
          size_x = std::max(size_x, rt_view->GetResource().GetSizeX());
          size_y = std::max(size_y, rt_view->GetResource().GetSizeY());
          layers = std::max(rt_view->GetLayersOrStride().length == -1 ? 1 : rt_view->GetLayersOrStride().length, layers);
        }

        std::vector<VkClearValue> ds_attachment_values(ds_attachments.size());
        std::vector<VkAttachmentDescription> ds_attachment_descs(ds_attachments.size());
        std::vector<VkImageView> ds_attachment_views(ds_attachments.size());
        for (size_t i = 0; i < ds_attachments.size(); ++i)
        {
          const auto& ds_value = ds_attachments[i].value;
          const auto& ds_view = ds_attachments[i].view;

          ds_attachment_values[i].depthStencil.depth = ds_value.first ? ds_value.first.value() : 0.0f;
          ds_attachment_values[i].depthStencil.stencil = ds_value.second ? ds_value.second.value() : 0;

          ds_attachment_descs[i].format = get_format(ds_view->GetResource().GetFormat());
          ds_attachment_descs[i].samples = VK_SAMPLE_COUNT_1_BIT;
          ds_attachment_descs[i].loadOp = ds_value.first ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
          ds_attachment_descs[i].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
          ds_attachment_descs[i].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
          ds_attachment_descs[i].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
          ds_attachment_descs[i].initialLayout = ds_value.first ? VK_IMAGE_LAYOUT_UNDEFINED : VK_IMAGE_LAYOUT_GENERAL;
          ds_attachment_descs[i].finalLayout = VK_IMAGE_LAYOUT_GENERAL;

          ds_attachment_views[i] = (reinterpret_cast<VLKView*>(ds_view.get()))->GetView();
          size_x = std::max(size_x, ds_view->GetResource().GetSizeX());
          size_y = std::max(size_y, ds_view->GetResource().GetSizeY());
          layers = std::max(ds_view->GetLayersOrStride().length == -1 ? 1 : ds_view->GetLayersOrStride().length, layers);
        }

        attachment_values.clear();
        std::move(rt_attachment_values.begin(), rt_attachment_values.end(), std::back_inserter(attachment_values));
        std::move(ds_attachment_values.begin(), ds_attachment_values.end(), std::back_inserter(attachment_values));

        attachment_descs.clear();
        std::move(rt_attachment_descs.begin(), rt_attachment_descs.end(), std::back_inserter(attachment_descs));
        std::move(ds_attachment_descs.begin(), ds_attachment_descs.end(), std::back_inserter(attachment_descs));

        attachment_views.clear();
        std::move(rt_attachment_views.begin(), rt_attachment_views.end(), std::back_inserter(attachment_views));
        std::move(ds_attachment_views.begin(), ds_attachment_views.end(), std::back_inserter(attachment_views));
      }

      {
        std::vector<VkAttachmentReference> rt_attachment_refs(rt_attachments.size());
        for (size_t i = 0; i < rt_attachments.size(); ++i)
        {
          rt_attachment_refs[i].attachment = uint32_t(i);
          rt_attachment_refs[i].layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
        }

        std::vector<VkAttachmentReference> ds_attachment_refs(ds_attachments.size());
        for (size_t i = 0; i < ds_attachments.size(); ++i)
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
        create_info.attachmentCount = uint32_t(attachment_descs.size());
        create_info.pAttachments = attachment_descs.data();
        create_info.subpassCount = 1;
        create_info.pSubpasses = &subpass;
        create_info.dependencyCount = 1;
        create_info.pDependencies = &dependency;
        BLAST_ASSERT(VK_SUCCESS == vkCreateRenderPass(device->GetDevice(), &create_info, nullptr, &renderpass));
      }


      {
        VkFramebufferCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        create_info.renderPass = renderpass;
        create_info.attachmentCount = uint32_t(attachment_views.size());
        create_info.pAttachments = attachment_views.data();
        create_info.width = size_x;
        create_info.height = size_y;
        create_info.layers = layers;
        BLAST_ASSERT(VK_SUCCESS == vkCreateFramebuffer(device->GetDevice(), &create_info, nullptr, &framebuffer));
      }
    }
  }

  void VLKPass::Use()
  {
    if (!enabled) return;

    auto device = reinterpret_cast<VLKDevice*>(&this->GetDevice());

    const auto command_buffer = device->GetCommadBuffer();

    if (type == TYPE_GRAPHIC)
    {
      auto pass_info = VkRenderPassBeginInfo{};
      pass_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
      pass_info.renderPass = renderpass;
      pass_info.framebuffer = framebuffer;
      pass_info.renderArea.offset = { 0, 0 };
      pass_info.renderArea.extent = { size_x, size_y };
      pass_info.clearValueCount = uint32_t(attachment_values.size());
      pass_info.pClearValues = attachment_values.data();

      vkCmdBeginRenderPass(command_buffer, &pass_info, VK_SUBPASS_CONTENTS_INLINE);

      for (const auto& effect : effects)
      {
        effect->Use();
      }

      vkCmdEndRenderPass(command_buffer);
    }

    if (type == TYPE_COMPUTE)
    {
      for (const auto& effect : effects)
      {
        effect->Use();
      }
    }

    if (type == TYPE_RAYTRACING && device->GetRTXSupported())
    {
      for (const auto& effect : effects)
      {
        effect->Use();
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

  VLKPass::VLKPass(const std::string& name,
    Device& device,
    Pass::Type type,
    const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments,
    const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments,
    const View::Range& ins_or_grid_x,
    const View::Range& vtx_or_grid_y)
    : Pass(name, device, type, rt_attachments, ds_attachments, ins_or_grid_x, vtx_or_grid_y)
  {
    VLKPass:Initialize();
  }

  VLKPass::~VLKPass()
  {
    VLKPass::Discard();
  }
}
