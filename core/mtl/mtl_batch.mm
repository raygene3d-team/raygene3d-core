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


#include "mtl_batch.h"
#include "mtl_config.h"
#include "mtl_pass.h"
#include "mtl_device.h"
#include "mtl_resource.h"
#include "mtl_view.h"

#include <algorithm>
#include <cstdlib>

#import <Metal/Metal.h>

namespace RayGene3D
{
  void MTLBatch::Initialize()
  {
    auto config = reinterpret_cast<MTLConfig*>(&this->GetConfig());
    auto pass = reinterpret_cast<MTLPass*>(&config->GetPass());
    auto device = reinterpret_cast<MTLDevice*>(&pass->GetDevice());
    id<MTLDevice> mtl_device = (__bridge id<MTLDevice>)device->GetDevice();

    const auto get_min_mag_filter = [](Sampler::Filtering filtering)
    {
      switch (filtering)
      {
      default: return MTLSamplerMinMagFilterLinear;
      case Sampler::FILTERING_NEAREST: return MTLSamplerMinMagFilterNearest;
      case Sampler::FILTERING_LINEAR: return MTLSamplerMinMagFilterLinear;
      }
    };

    const auto get_mip_filter = [](Sampler::Filtering filtering)
    {
      switch (filtering)
      {
      default: return MTLSamplerMipFilterLinear;
      case Sampler::FILTERING_NEAREST: return MTLSamplerMipFilterNearest;
      case Sampler::FILTERING_LINEAR: return MTLSamplerMipFilterLinear;
      }
    };

    const auto get_addressing = [](Sampler::Addressing addressing)
    {
      switch (addressing)
      {
      default: return MTLSamplerAddressModeRepeat;
      case Sampler::ADDRESSING_REPEAT: return MTLSamplerAddressModeRepeat;
      case Sampler::ADDRESSING_MIRROR: return MTLSamplerAddressModeMirrorRepeat;
      case Sampler::ADDRESSING_CLAMP: return MTLSamplerAddressModeClampToEdge;
      case Sampler::ADDRESSING_BORDER: return MTLSamplerAddressModeClampToZero;
      }
    };

    const auto get_compare = [](Sampler::Comparison comparison)
    {
      switch (comparison)
      {
      default: return MTLCompareFunctionNever;
      case Sampler::COMPARISON_NEVER: return MTLCompareFunctionNever;
      case Sampler::COMPARISON_LESS: return MTLCompareFunctionLess;
      case Sampler::COMPARISON_EQUAL: return MTLCompareFunctionEqual;
      case Sampler::COMPARISON_LESS_EQUAL: return MTLCompareFunctionLessEqual;
      case Sampler::COMPARISON_GREATER: return MTLCompareFunctionGreater;
      case Sampler::COMPARISON_NOT_EQUAL: return MTLCompareFunctionNotEqual;
      case Sampler::COMPARISON_GREATER_EQUAL: return MTLCompareFunctionGreaterEqual;
      case Sampler::COMPARISON_ALWAYS: return MTLCompareFunctionAlways;
      }
    };

    sampler_states.resize(samplers.size());
    for (uint32_t i = 0; i < uint32_t(sampler_states.size()); ++i)
    {
      const auto& sampler = samplers.at(i);

      MTLSamplerDescriptor* desc = [[MTLSamplerDescriptor alloc] init];
      desc.minFilter = get_min_mag_filter(sampler.filtering);
      desc.magFilter = get_min_mag_filter(sampler.filtering);
      desc.mipFilter = get_mip_filter(sampler.filtering);
      desc.sAddressMode = get_addressing(sampler.addressing);
      desc.tAddressMode = get_addressing(sampler.addressing);
      desc.rAddressMode = get_addressing(sampler.addressing);
      desc.maxAnisotropy = sampler.filtering == Sampler::FILTERING_ANISOTROPIC ? uint32_t(sampler.anisotropy) : 1;
      desc.compareFunction = get_compare(sampler.comparison);
      desc.lodMinClamp = sampler.min_lod;
      desc.lodMaxClamp = sampler.max_lod;

      id<MTLSamplerState> state = [mtl_device newSamplerStateWithDescriptor:desc];
      BLAST_ASSERT(state != nil);
      sampler_states.at(i) = (__bridge_retained void*)state;
    }

    // Total declared bindings (samplers + view slots), the Metal counterpart of Vulkan's descriptor write count.
    const auto binding_count = samplers.size() + ub_views.size() + sb_views.size()
      + ri_views.size() + wi_views.size() + rb_views.size() + wb_views.size();
    BLAST_LOG("Binding count: %d [%s]", int(binding_count), name.c_str());
  }

  void MTLBatch::Use()
  {
    auto config = reinterpret_cast<MTLConfig*>(&this->GetConfig());
    auto pass = reinterpret_cast<MTLPass*>(&config->GetPass());

    // The encoder was opened by MTLPass::Use() for this pass type.
    const auto encoder = pass->GetEncoder();
    if (!encoder) return;

    const auto pipeline = config->GetPipeline();
    if (!pipeline) return;

    // Binds a view as a buffer (at its offset) or a texture view (type/mip/slice from the view), at the shader index.
    const auto bind_resource = [](const std::shared_ptr<View>& view, uint32_t index, id<MTLRenderCommandEncoder> render, id<MTLComputeCommandEncoder> compute)
    {
      if (!view) return;
      auto resource = reinterpret_cast<MTLResource*>(&view->GetResource());
      if (!resource->GetNative()) return;

      if (resource->GetType() == Resource::TYPE_BUFFER)
      {
        id native = (__bridge id)resource->GetNative();
        const auto offset = view->GetLevelsOrLength().offset;
        if (render) { [render setVertexBuffer:native offset:offset atIndex:index]; [render setFragmentBuffer:native offset:offset atIndex:index]; }
        if (compute) [compute setBuffer:native offset:offset atIndex:index];
      }
      else
      {
        // Textures are bound through the view so a cube/array/mip view takes effect (not the base texture).
        id native = (__bridge id)reinterpret_cast<MTLView*>(view.get())->GetView();
        if (!native) return;
        if (render) [render setFragmentTexture:native atIndex:index];
        if (compute) [compute setTexture:native atIndex:index];
      }
    };

    // Shader resources use one flat binding namespace (layout(set=0/[[vk::binding(N)]]), see vlk_batch.cpp):
    // samplers, then ub, sb, rb, ri, wb, wi, each taking the next running index. SPIRV-Cross keeps
    // that N as the Metal index in the matching table (buffer / texture / sampler), so the running
    // index must start after the samplers and must include every declared view.
    const auto bind_views = [&](const std::shared_ptr<View>* views, size_t count, uint32_t& index,
      id<MTLRenderCommandEncoder> render, id<MTLComputeCommandEncoder> compute)
    {
      for (size_t i = 0; i < count; ++i, ++index)
      {
        bind_resource(views[i], index, render, compute);
      }
    };

    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      id<MTLRenderCommandEncoder> render = (__bridge id<MTLRenderCommandEncoder>)encoder;
      [render setRenderPipelineState:(__bridge id<MTLRenderPipelineState>)pipeline];

      // Rasterizer and depth-stencil state from the config, applied the same way as the Vulkan/D3D12 backends.
      const auto& rc_state = config->GetRCState();
      [render setTriangleFillMode:(rc_state.fill_mode == Config::FILL_LINE ? MTLTriangleFillModeLines : MTLTriangleFillModeFill)];
      switch (rc_state.cull_mode)
      {
      case Config::CULL_FRONT: [render setCullMode:MTLCullModeFront]; break;
      case Config::CULL_BACK: [render setCullMode:MTLCullModeBack]; break;
      default: [render setCullMode:MTLCullModeNone]; break;
      }
      // Front face is clockwise, matching VK_FRONT_FACE_CLOCKWISE and D3D12 FrontCounterClockwise = false.
      [render setFrontFacingWinding:MTLWindingClockwise];
      [render setDepthStencilState:(__bridge id<MTLDepthStencilState>)config->GetDepthStencilState()];

      for (uint32_t i = 0; i < uint32_t(sampler_states.size()); ++i)
      {
        [render setFragmentSamplerState:(__bridge id<MTLSamplerState>)sampler_states[i] atIndex:i];
      }

      for (const auto& entity : entities)
      {
        // Flat index: samplers occupy [0, S), resources follow in Vulkan order.
        uint32_t index = uint32_t(sampler_states.size());
        bind_views(ub_views.data(), ub_views.size(), index, render, nil);
        bind_views(sb_views.data(), sb_views.size(), index, render, nil);
        bind_views(rb_views.data(), rb_views.size(), index, render, nil);
        bind_views(ri_views.data(), ri_views.size(), index, render, nil);
        bind_views(wb_views.data(), wb_views.size(), index, render, nil);
        bind_views(wi_views.data(), wi_views.size(), index, render, nil);

        // Vertex arrays: each VA view is bound to its own vertex buffer slot.
        for (uint32_t i = 0; i < uint32_t(entity.va_views.size()); ++i)
        {
          const auto& va_view = entity.va_views[i];
          if (!va_view) continue;
          auto resource = reinterpret_cast<MTLResource*>(&va_view->GetResource());
          [render setVertexBuffer:(__bridge id<MTLBuffer>)resource->GetNative() offset:va_view->GetLevelsOrLength().offset atIndex:i];
        }

        const auto ins_count = entity.ins_or_grid_x.length;
        const auto ins_offset = entity.ins_or_grid_x.offset;
        const auto vtx_offset = entity.vtx_or_grid_y.offset;
        const auto idx_count = entity.idx_or_grid_z.length;
        const auto idx_offset = entity.idx_or_grid_z.offset;

        // Index buffer: the first IA view holds the index data.
        if (!entity.ia_views.empty() && entity.ia_views[0])
        {
          const auto& ia_view = entity.ia_views[0];
          auto resource = reinterpret_cast<MTLResource*>(&ia_view->GetResource());
          id<MTLBuffer> index_buffer = (__bridge id<MTLBuffer>)resource->GetNative();
          const auto is_32 = config->GetIAState().indexer == Config::INDEXER_32_BIT;
          const auto index_type = is_32 ? MTLIndexTypeUInt32 : MTLIndexTypeUInt16;
          const auto index_stride = is_32 ? 4u : 2u;
          const auto index_offset = ia_view->GetLevelsOrLength().offset + idx_offset * index_stride;

          if (entity.arg_view)
          {
            auto arg_resource = reinterpret_cast<MTLResource*>(&entity.arg_view->GetResource());
            [render drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                                indexType:index_type
                              indexBuffer:index_buffer
                        indexBufferOffset:index_offset
                           indirectBuffer:(__bridge id<MTLBuffer>)arg_resource->GetNative()
                     indirectBufferOffset:entity.arg_view->GetLevelsOrLength().offset];
          }
          else
          {
            [render drawIndexedPrimitives:MTLPrimitiveTypeTriangle
                               indexCount:idx_count
                                indexType:index_type
                              indexBuffer:index_buffer
                        indexBufferOffset:index_offset
                            instanceCount:ins_count
                               baseVertex:vtx_offset
                             baseInstance:ins_offset];
          }
        }
      }
    }

    if (pass->GetType() == Pass::TYPE_COMPUTE)
    {
      id<MTLComputeCommandEncoder> compute = (__bridge id<MTLComputeCommandEncoder>)encoder;
      [compute setComputePipelineState:(__bridge id<MTLComputePipelineState>)pipeline];

      for (uint32_t i = 0; i < uint32_t(sampler_states.size()); ++i)
      {
        [compute setSamplerState:(__bridge id<MTLSamplerState>)sampler_states[i] atIndex:i];
      }

      for (const auto& chunk : entities)
      {
        // Same flat index as the graphics path (samplers first, then resources in Vulkan order).
        uint32_t index = uint32_t(sampler_states.size());
        bind_views(ub_views.data(), ub_views.size(), index, nil, compute);
        bind_views(sb_views.data(), sb_views.size(), index, nil, compute);
        bind_views(rb_views.data(), rb_views.size(), index, nil, compute);
        bind_views(ri_views.data(), ri_views.size(), index, nil, compute);
        bind_views(wb_views.data(), wb_views.size(), index, nil, compute);
        bind_views(wi_views.data(), wi_views.size(), index, nil, compute);

        const auto& local = config->GetThreadsPerGroup();
        const auto threads = MTLSizeMake(local[0], local[1], local[2]);

        if (chunk.arg_view)
        {
          // Indirect dispatch: the Compute argument list (grid_x, grid_y, grid_z) lives in the arg buffer.
          auto arg_resource = reinterpret_cast<MTLResource*>(&chunk.arg_view->GetResource());
          [compute dispatchThreadgroupsWithIndirectBuffer:(__bridge id<MTLBuffer>)arg_resource->GetNative()
                                     indirectBufferOffset:chunk.arg_view->GetLevelsOrLength().offset
                                    threadsPerThreadgroup:threads];
          continue;
        }

        const auto grid = MTLSizeMake(chunk.ins_or_grid_x.length, chunk.vtx_or_grid_y.length, chunk.idx_or_grid_z.length);
        [compute dispatchThreadgroups:grid threadsPerThreadgroup:threads];
      }
    }

    // TYPE_TRACING is not supported on Metal; no work is encoded.
  }

  void MTLBatch::Discard()
  {
    for (auto& state : sampler_states)
    {
      if (state) { (void)(__bridge_transfer id)state; state = nullptr; }
    }
    sampler_states.clear();
  }

  MTLBatch::MTLBatch(const std::string& name,
    Config& config,
    const std::pair<const Batch::Entity*, size_t>& entities,
    const std::pair<const Batch::Sampler*, size_t>& samplers,
    const std::pair<const std::shared_ptr<View>*, size_t>& ub_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& sb_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& ri_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& wi_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& rb_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& wb_views)
    : Batch(name, config, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views)
  {
    MTLBatch::Initialize();
  }

  MTLBatch::~MTLBatch()
  {
    MTLBatch::Discard();
  }
}
