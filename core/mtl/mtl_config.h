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
#ifndef __OBJC__
#error "mtl_config.h must be included only from Objective-C++ (.mm) files"
#endif
#include "../config.h"
#include "mtl_batch.h"
#import <Metal/Metal.h>
#include <array>

namespace RayGene3D
{
  class MTLConfig : public Config
  {
  protected:
    id<MTLLibrary> library{ nil };                        // compiled from MSL source, owned
    id<MTLLibrary> fragment_library{ nil };               // holds fragment_function, owned
    id<MTLFunction> vertex_function{ nil };               // owned
    id<MTLFunction> fragment_function{ nil };             // owned
    id<MTLFunction> mesh_function{ nil };                 // mesh stage, from DXIL, owned
    id<MTLFunction> kernel_function{ nil };               // compute, owned
    id pipeline{ nil };                                   // id<MTLRenderPipelineState> or id<MTLComputePipelineState>, owned
    id<MTLDepthStencilState> depth_stencil_state{ nil };  // from DSState (graphic passes), owned
    std::array<uint32_t, 3> threads_per_group{ 1, 1, 1 }; // compute local_size from the kernel

  public:
    const std::array<uint32_t, 3>& GetThreadsPerGroup() const { return threads_per_group; }
    id<MTLDepthStencilState> GetDepthStencilState() const { return depth_stencil_state; }
    id<MTLLibrary> GetLibrary() const { return library; }
    id<MTLFunction> GetVSFunction() const { return vertex_function; }
    id<MTLFunction> GetPSFunction() const { return fragment_function; }
    id<MTLFunction> GetMSFunction() const { return mesh_function; }
    id<MTLFunction> GetCSFunction() const { return kernel_function; }
    id GetPipeline() const { return pipeline; }

  public:
    const std::shared_ptr<Batch>& CreateBatch(const std::string& name,
      const std::pair<const Batch::Entity*, size_t>& entities,
      const std::pair<const Batch::Sampler*, size_t>& samplers,
      const std::pair<const std::shared_ptr<View>*, size_t>& ub_views,
      const std::pair<const std::shared_ptr<View>*, size_t>& sb_views,
      const std::pair<const std::shared_ptr<View>*, size_t>& ri_views,
      const std::pair<const std::shared_ptr<View>*, size_t>& wi_views,
      const std::pair<const std::shared_ptr<View>*, size_t>& rb_views,
      const std::pair<const std::shared_ptr<View>*, size_t>& wb_views
    ) override
    {
      return batches.emplace_back(new MTLBatch(name, *this, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views));
    }

  protected:
    void BuildDXILLibrary(id<MTLDevice> mtl_device, const std::string& path, const std::string& file);
    std::vector<std::pair<std::string, std::string>> defines_list() const;

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    MTLConfig(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, size_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    MTLConfig(const std::string& name,
      Pass& pass,
      const std::string& path,
      const std::string& file,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, size_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    virtual ~MTLConfig();
    MTLConfig(const MTLConfig&) = delete;
    MTLConfig& operator=(const MTLConfig&) = delete;
  };
}
