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
#include "d12_batch.h"

#include <dxgi.h>
#include <d3d12.h>

namespace RayGene3D
{
  class D12Config : public Config
  {
  protected:
    D3D12_PRIMITIVE_TOPOLOGY_TYPE topology_type;

  protected:
    std::vector<D3D12_INPUT_ELEMENT_DESC> element_descs;

  protected:
    D3D12_BLEND_DESC blend_desc{};
    D3D12_DEPTH_STENCIL_DESC depth_desc{};
    D3D12_RASTERIZER_DESC raster_desc{};

  public:
    D3D12_SHADER_BYTECODE GetVSBytecode() const { return { vert_bytecode.data(), vert_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetHSBytecode() const { return { tesc_bytecode.data(), tesc_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetDSBytecode() const { return { tese_bytecode.data(), tese_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetGSBytecode() const { return { geom_bytecode.data(), geom_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetPSBytecode() const { return { frag_bytecode.data(), frag_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetCSBytecode() const { return { comp_bytecode.data(), comp_bytecode.size() }; }
 
  public:
    D3D12_SHADER_BYTECODE GetTaskBytecode() const { return { task_bytecode.data(), task_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetMeshBytecode() const { return { mesh_bytecode.data(), mesh_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetRGenBytecode() const { return { rgen_bytecode.data(), rgen_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetCallBytecode() const { return { call_bytecode.data(), call_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetISecBytecode() const { return { isec_bytecode.data(), isec_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetCHitBytecode() const { return { chit_bytecode.data(), chit_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetAHitBytecode() const { return { ahit_bytecode.data(), ahit_bytecode.size() }; }
    D3D12_SHADER_BYTECODE GetMissBytecode() const { return { miss_bytecode.data(), miss_bytecode.size() }; }
  public:
    D3D12_PRIMITIVE_TOPOLOGY_TYPE GetTopologyType() const { return topology_type; }
    D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return primitive_topology; }

  public:
    D3D12_INPUT_LAYOUT_DESC GetLayoutDesc() const { return { element_descs.data(), uint32_t(element_descs.size()) }; }

  public:
    D3D12_BLEND_DESC GetBlendDesc() const { return blend_desc; }
    D3D12_DEPTH_STENCIL_DESC GetDepthDesc() const { return depth_desc; }
    D3D12_RASTERIZER_DESC GetRasterDesc() const { return raster_desc; }

  protected:
    std::vector<D3D12_VIEWPORT> vp_items;
    std::vector<D3D12_RECT> sr_items;

  protected:
    std::vector<uint32_t> strides; //TODO: Remove

  protected:
    D3D_PRIMITIVE_TOPOLOGY primitive_topology{ D3D_PRIMITIVE_TOPOLOGY_UNDEFINED };

  protected:
    bool use_mesh_pipeline{ false };

  public:
    const std::vector<uint32_t>& GetStrides() const { return strides; }

  public:
    bool UseMeshPipeline() const { return use_mesh_pipeline; }

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
      return batches.emplace_back(new D12Batch(name, *this, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D12Config(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, size_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    D12Config(const std::string& name,
      Pass& pass,
      const std::string& path,
      const std::string& file,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, size_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    virtual ~D12Config();
  };
}