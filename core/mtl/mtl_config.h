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
#include "mtl_batch.h"
#include <array>

namespace RayGene3D
{
  class MTLConfig : public Config
  {
  protected:
    void* library{ nullptr };           // id<MTLLibrary> compiled from MSL source
    void* vertex_function{ nullptr };   // id<MTLFunction>
    void* fragment_function{ nullptr }; // id<MTLFunction>
    void* mesh_function{ nullptr };     // id<MTLFunction> (mesh stage, from DXIL)
    void* kernel_function{ nullptr };   // id<MTLFunction> (compute)
    void* pipeline{ nullptr };          // MTLRenderPipelineState* or MTLComputePipelineState*
    std::array<uint32_t, 3> threads_per_group{ 1, 1, 1 }; // compute local_size from the kernel

  public:
    const std::array<uint32_t, 3>& GetThreadsPerGroup() const { return threads_per_group; }
    void* GetLibrary() const { return library; }
    void* GetVSFunction() const { return vertex_function; }
    void* GetPSFunction() const { return fragment_function; }
    void* GetMSFunction() const { return mesh_function; }
    void* GetCSFunction() const { return kernel_function; }
    void* GetPipeline() const { return pipeline; }

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
    void BuildDXILLibrary(void* mtl_device, const std::string& path, const std::string& file); // id<MTLDevice>
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
  };
}
