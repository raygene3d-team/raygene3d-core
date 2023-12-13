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
#include "../technique.h"

#include <dxgi.h>
#include <d3d11_1.h>

namespace RayGene3D
{
  class D11Technique : public Technique
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
    std::vector<uint32_t> strides;

  protected:
    D3D_PRIMITIVE_TOPOLOGY primitive_topology{ D3D_PRIMITIVE_TOPOLOGY_UNDEFINED };

  public:
    ID3D11VertexShader* GetVSShader() const { return vs_shader; }
    ID3D11HullShader* GetHSShader() const { return hs_shader; }
    ID3D11DomainShader* GetDSShader() const { return ds_shader; }
    ID3D11GeometryShader* GetGSShader() const { return gs_shader; }
    ID3D11PixelShader* GetPSShader() const { return ps_shader; }
    ID3D11ComputeShader* GetCSShader() const { return cs_shader; }

  public:
    D3D_PRIMITIVE_TOPOLOGY GetPrimitiveTopology() const { return primitive_topology; }
    ID3D11InputLayout* GetInputLayout() const { return input_layout; }
    ID3D11RasterizerState* GetRasterState() const { return raster_state; }
    ID3D11DepthStencilState* GetDepthState() const { return depth_state; }
    ID3D11BlendState* GetBlendState() const { return blend_state; }
    const D3D11_VIEWPORT* GetViewportItems() const { return vp_items.data(); }
    uint32_t GetViewportCount() const { return uint32_t(vp_items.size()); }

  public:
    const std::vector<uint32_t>& GetStrides() const { return strides; }

  public:
    const std::shared_ptr<Batch>& CreateBatch(const std::string& name,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views,
      const std::pair<const Batch::Sampler*, uint32_t>& samplers,
      const std::pair<const Batch::RTXEntity*, uint32_t>& rtx_entities) override
    {
      return layouts.emplace_back(new D11Batch(name, *this, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views, samplers, rtx_entities));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D11Technique(const std::string& name,
      Pass& pass,
      const std::string& source,
      Technique::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Technique::IAState& ia_state,
      const Technique::RCState& rc_state,
      const Technique::DSState& ds_state,
      const Technique::OMState& om_state);
    virtual ~D11Technique();
  };
}