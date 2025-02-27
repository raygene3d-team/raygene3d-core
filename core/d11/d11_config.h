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
#include "d11_batch.h"

#include <dxgi.h>
#include <d3d11_1.h>

namespace RayGene3D
{
  class D11Config : public Config
  {
  protected:
    ID3D11VertexShader* vs_shader{ nullptr };
    ID3D11HullShader* hs_shader{ nullptr };
    ID3D11DomainShader* ds_shader{ nullptr };
    ID3D11GeometryShader* gs_shader{ nullptr };
    ID3D11PixelShader* ps_shader{ nullptr };
    ID3D11ComputeShader* cs_shader{ nullptr };

  //public:
  //  std::vector<char> CompileVSSource(const std::string& source) override;
  //  std::vector<char> CompileHSSource(const std::string& source) override;
  //  std::vector<char> CompileDSSource(const std::string& source) override;
  //  std::vector<char> CompileGSSource(const std::string& source) override;
  //  std::vector<char> CompilePSSource(const std::string& source) override;
  //  std::vector<char> CompileCSSource(const std::string& source) override;

  protected:
    ID3D11InputLayout* input_layout{ nullptr };

  protected:
    ID3D11RasterizerState* raster_state{ nullptr };
    ID3D11DepthStencilState* depth_state{ nullptr };
    ID3D11BlendState* blend_state{ nullptr };

  protected:
    std::vector<D3D11_VIEWPORT> vp_items;

  protected:
    std::vector<uint32_t> strides; //TODO: Remove

  protected:
    D3D_PRIMITIVE_TOPOLOGY primitive_topology{ D3D_PRIMITIVE_TOPOLOGY_UNDEFINED };

  public:
    const std::vector<uint32_t>& GetStrides() const { return strides; }

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
      return batches.emplace_back(new D11Batch(name, *this, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D11Config(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    D11Config(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    D11Config(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines);
    virtual ~D11Config();
  };
}