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


#include "mtl_pass.h"
#include "mtl_config.h"
#include "mtl_batch.h"
#include "mtl_device.h"
#include "mtl_view.h"

#include <cstdlib>

#import <Metal/Metal.h>

namespace RayGene3D
{
  void MTLPass::Initialize()
  {
    if (type == TYPE_GRAPHIC)
    {
      MTLRenderPassDescriptor* descriptor = [MTLRenderPassDescriptor renderPassDescriptor];

      for (size_t i = 0; i < rt_attachments.size(); ++i)
      {
        const auto& rt_value = rt_attachments[i].value;
        const auto& rt_view = rt_attachments[i].view;

        id<MTLTexture> texture = reinterpret_cast<MTLView*>(rt_view.get())->GetView();
        MTLRenderPassColorAttachmentDescriptor* color = descriptor.colorAttachments[i];
        color.texture = texture;
        // Clear only when the attachment carries a clear value, otherwise keep the existing contents.
        color.loadAction = rt_value ? MTLLoadActionClear : MTLLoadActionLoad;
        color.storeAction = MTLStoreActionStore;
        color.clearColor = MTLClearColorMake(
          rt_value ? rt_value.value()[0] : 1.0,
          rt_value ? rt_value.value()[1] : 0.0,
          rt_value ? rt_value.value()[2] : 1.0,
          rt_value ? rt_value.value()[3] : 0.0);
      }

      for (size_t i = 0; i < ds_attachments.size(); ++i)
      {
        const auto& ds_value = ds_attachments[i].value;
        const auto& ds_view = ds_attachments[i].view;

        id<MTLTexture> texture = reinterpret_cast<MTLView*>(ds_view.get())->GetView();
        MTLRenderPassDepthAttachmentDescriptor* depth = descriptor.depthAttachment;
        depth.texture = texture;
        depth.loadAction = ds_value.first ? MTLLoadActionClear : MTLLoadActionLoad;
        depth.storeAction = MTLStoreActionStore;
        depth.clearDepth = ds_value.first ? ds_value.first.value() : 0.0f;
      }

      // renderPassDescriptor is autoreleased, so retain it for the lifetime of this pass.
      render_pass_descriptor = [descriptor retain];
    }
  }

  void MTLPass::Use()
  {
    if (!enabled) return;

    auto device = reinterpret_cast<MTLDevice*>(&this->GetDevice());
    id<MTLCommandBuffer> command_buffer = device->GetCommandBuffer();

    if (type == TYPE_GRAPHIC)
    {
      MTLRenderPassDescriptor* descriptor = render_pass_descriptor;
      id<MTLRenderCommandEncoder> render = [command_buffer renderCommandEncoderWithDescriptor:descriptor];
      encoder = render;

      for (const auto& config : configs)
      {
        config->Use();
      }

      [render endEncoding];
      encoder = nil;
    }

    if (type == TYPE_COMPUTE)
    {
      id<MTLComputeCommandEncoder> compute = [command_buffer computeCommandEncoder];
      encoder = compute;

      for (const auto& config : configs)
      {
        config->Use();
      }

      [compute endEncoding];
      encoder = nil;
    }

    // TYPE_TRACING is not supported on Metal (no ray tracing pipeline); configs are skipped.
  }

  void MTLPass::Discard()
  {
    [render_pass_descriptor release];
    render_pass_descriptor = nil;
  }

  MTLPass::MTLPass(const std::string& name,
    Device& device,
    Pass::Type type,
    uint32_t size_x,
    uint32_t size_y,
    size_t layers,
    const std::pair<const Pass::RTAttachment*, size_t>& rt_attachments,
    const std::pair<const Pass::DSAttachment*, size_t>& ds_attachments)
    : Pass(name, device, type, size_x, size_y, layers, rt_attachments, ds_attachments)
  {
    MTLPass::Initialize();
  }

  MTLPass::~MTLPass()
  {
    MTLPass::Discard();
  }
}
