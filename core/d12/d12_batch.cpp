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
#include "d12_batch.h"
#include "d12_config.h"
#include "d12_pass.h"
#include "d12_device.h"
#include "d12_resource.h"
#include "d12_view.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

namespace RayGene3D
{
  void D12Batch::Initialize()
  {
    auto config = reinterpret_cast<D12Config*>(&this->GetConfig());
    auto pass = reinterpret_cast<D12Pass*>(&config->GetPass());
    auto device = reinterpret_cast<D12Device*>(&pass->GetDevice());

    const auto get_filter = [this](Sampler::Filtering filtering, bool compare)
    {
      switch (filtering)
      {
      default: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
      case Sampler::FILTERING_NEAREST: return compare ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
      case Sampler::FILTERING_LINEAR: return compare ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
      case Sampler::FILTERING_ANISOTROPIC: return compare ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
      }
    };

    const auto get_addressing = [this](Sampler::Addressing addressing)
    {
      switch (addressing)
      {
      default: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
      case Sampler::ADDRESSING_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
      case Sampler::ADDRESSING_MIRROR: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
      case Sampler::ADDRESSING_CLAMP: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
      case Sampler::ADDRESSING_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
      }
    };

    const auto get_comparison = [this](Sampler::Comparison comparison)
    {
      switch (comparison)
      {
      default: return D3D12_COMPARISON_FUNC_NONE;
      case Sampler::COMPARISON_NEVER: return D3D12_COMPARISON_FUNC_NEVER;
      case Sampler::COMPARISON_LESS: return D3D12_COMPARISON_FUNC_LESS;
      case Sampler::COMPARISON_EQUAL: return D3D12_COMPARISON_FUNC_EQUAL;
      case Sampler::COMPARISON_LESS_EQUAL: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
      case Sampler::COMPARISON_GREATER: return D3D12_COMPARISON_FUNC_GREATER;
      case Sampler::COMPARISON_NOT_EQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
      case Sampler::COMPARISON_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
      case Sampler::COMPARISON_ALWAYS: return D3D12_COMPARISON_FUNC_ALWAYS;
      }
    };

    sampler_items.resize(samplers.size());
    for (uint32_t i = 0; i < sampler_items.size(); ++i)
    {
      D3D12_SAMPLER_DESC sampler_desc{};
      sampler_desc.Filter = get_filter(samplers[i].filtering, samplers[i].comparison != Sampler::COMPARISON_NEVER);
      sampler_desc.AddressU = get_addressing(samplers[i].addressing);
      sampler_desc.AddressV = get_addressing(samplers[i].addressing);
      sampler_desc.AddressW = get_addressing(samplers[i].addressing);
      sampler_desc.MipLODBias = samplers[i].bias_lod;
      sampler_desc.MaxAnisotropy = samplers[i].anisotropy;
      sampler_desc.ComparisonFunc = get_comparison(samplers[i].comparison);
      sampler_desc.BorderColor[0] = samplers[i].color[0];
      sampler_desc.BorderColor[1] = samplers[i].color[1];
      sampler_desc.BorderColor[2] = samplers[i].color[2];
      sampler_desc.BorderColor[3] = samplers[i].color[3];
      sampler_desc.MinLOD = samplers[i].min_lod;
      sampler_desc.MaxLOD = samplers[i].max_lod;

      device->GetDevice()->CreateSampler(&sampler_desc, sampler_items[i]);
    }


    const auto ub_count = ub_views.size();
    ub_items.resize(std::min(ub_count, size_t(D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)));
    for (size_t i = 0; i < ub_items.size(); ++i)
    {
      if (i < ub_count)
      {
        const auto& ub_view = ub_views[i];
        if (ub_view)
        {
          ub_items[i] = (reinterpret_cast<D12View*>(ub_view.get()))->GetGPUHandle();
        }
      }
    }

    const auto sb_count = sb_views.size();
    sb_items.resize(std::min(sb_count, size_t(D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)));
    for (size_t i = 0; i < sb_items.size(); ++i)
    {
      if (i < sb_count)
      {
        const auto& sb_view = sb_views[i];
        if (sb_view)
        {
          sb_items[i] = (reinterpret_cast<D12View*>(sb_view.get()))->GetGPUHandle();
        }
      }
    }

    const auto rr_count = rb_views.size() + ri_views.size();
    rr_items.resize(std::min(rr_count, size_t(D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)));
    for (size_t i = 0; i < rr_items.size(); ++i)
    {
      if (i < rr_count)
      {
        size_t offset = 0u;
        if (i - offset < rb_views.size() && rb_views[i - offset]) { rr_items[i] = (reinterpret_cast<D12View*>(rb_views[i - offset].get()))->GetGPUHandle(); continue; }
        offset += rb_views.size();
        if (i - offset < ri_views.size() && ri_views[i - offset]) { rr_items[i] = (reinterpret_cast<D12View*>(ri_views[i - offset].get()))->GetGPUHandle(); continue; }
      }
    }

    const auto wr_count = wb_views.size() + wi_views.size();
    wr_items.resize(std::min(wr_count, size_t(D3D12_PS_CS_UAV_REGISTER_COUNT)));
    for (size_t i = 0; i < wr_items.size(); ++i)
    {
      if (i < wr_count)
      {
        size_t offset = 0u;
        if (i - offset < wb_views.size() && wb_views[i - offset]) { wr_items[i] = (reinterpret_cast<D12View*>(wb_views[i - offset].get()))->GetGPUHandle(); continue; }
        offset += wb_views.size();
        if (i - offset < wi_views.size() && wi_views[i - offset]) { wr_items[i] = (reinterpret_cast<D12View*>(wi_views[i - offset].get()))->GetGPUHandle(); continue; }
      }
    }

    std::vector<D3D12_ROOT_PARAMETER> root_parameters;

    D3D12_ROOT_SIGNATURE_DESC signature_desc = {};
    signature_desc.NumParameters = root_parameters.size();
    signature_desc.pParameters = root_parameters.data();
    signature_desc.NumStaticSamplers = 0;
    signature_desc.pStaticSamplers = nullptr;
    signature_desc.Flags =
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
      D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

    ID3DBlob* signature{ nullptr };
    ID3DBlob* errors{ nullptr };
    HRESULT hr = D3D12SerializeRootSignature(&signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errors);

    if (errors)
    {
      BLAST_LOG("signature serialization output: \n%s", reinterpret_cast<char*>(errors->GetBufferPointer()));
      errors->Release();
    }

    if (signature)
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&root_signature)));
      signature->Release();
    }

    D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
    pso_desc.pRootSignature = root_signature;
    pso_desc.VS = config->GetVSBytecode();
    pso_desc.PS = config->GetPSBytecode();
    pso_desc.HS = config->GetHSBytecode();
    pso_desc.DS = config->GetDSBytecode();
    pso_desc.GS = config->GetGSBytecode();    
    pso_desc.StreamOutput = {};
    pso_desc.BlendState = config->GetBlendDesc();
    pso_desc.SampleMask = UINT_MAX;
    pso_desc.RasterizerState = config->GetRasterDesc();
    pso_desc.DepthStencilState = config->GetDepthDesc();
    pso_desc.InputLayout = config->GetLayoutDesc();
    pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
    pso_desc.PrimitiveTopologyType = config->GetTopologyType();
    pso_desc.NumRenderTargets = pass->GetRTCount();
    pso_desc.RTVFormats[0] = pass->GetRTFormat(0);
    pso_desc.RTVFormats[1] = pass->GetRTFormat(1);
    pso_desc.RTVFormats[2] = pass->GetRTFormat(2);
    pso_desc.RTVFormats[3] = pass->GetRTFormat(3);
    pso_desc.RTVFormats[4] = pass->GetRTFormat(4);
    pso_desc.RTVFormats[5] = pass->GetRTFormat(5);
    pso_desc.RTVFormats[6] = pass->GetRTFormat(6);
    pso_desc.RTVFormats[7] = pass->GetRTFormat(7);
    pso_desc.DSVFormat = pass->GetDSFormat(0);
    pso_desc.SampleDesc = { 1, 0 };
    pso_desc.NodeMask = 0;
    pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
    BLAST_ASSERT(S_OK == device->GetDevice()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state)));
  }

  void D12Batch::Use()
  {
    auto config = reinterpret_cast<D12Config*>(&this->GetConfig());
    auto pass = reinterpret_cast<D12Pass*>(&config->GetPass());
    auto device = reinterpret_cast<D12Device*>(&pass->GetDevice());

    //if (pass->GetType() == Pass::TYPE_GRAPHIC)
    //{
    //  device->GetContext()->VSSetShaderResources(0, rr_items.size(), rr_items.data());
    //  device->GetContext()->VSSetConstantBuffers(0, ub_items.size(), ub_items.data());
    //  device->GetContext()->VSSetSamplers(0, sampler_states.size(), sampler_states.data());

    //  device->GetContext()->HSSetShaderResources(0, rr_items.size(), rr_items.data());
    //  device->GetContext()->HSSetConstantBuffers(0, ub_items.size(), ub_items.data());
    //  device->GetContext()->HSSetSamplers(0, sampler_states.size(), sampler_states.data());

    //  device->GetContext()->DSSetShaderResources(0, rr_items.size(), rr_items.data());
    //  device->GetContext()->DSSetConstantBuffers(0, ub_items.size(), ub_items.data());
    //  device->GetContext()->DSSetSamplers(0, sampler_states.size(), sampler_states.data());

    //  device->GetContext()->GSSetShaderResources(0, rr_items.size(), rr_items.data());
    //  device->GetContext()->GSSetConstantBuffers(0, ub_items.size(), ub_items.data());
    //  device->GetContext()->GSSetSamplers(0, sampler_states.size(), sampler_states.data());

    //  device->GetContext()->PSSetShaderResources(0, rr_items.size(), rr_items.data());
    //  device->GetContext()->PSSetConstantBuffers(0, ub_items.size(), ub_items.data());
    //  device->GetContext()->PSSetSamplers(0, sampler_states.size(), sampler_states.data());

    //  for (const auto& chunk : entities)
    //  {
    //    const size_t va_limit = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
    //    uint32_t va_strides[va_limit]{ 0 };
    //    uint32_t va_offsets[va_limit]{ 0 };
    //    ID3D11Buffer* va_items[va_limit]{ nullptr };
    //    const auto va_count = std::min(va_limit, chunk.va_views.size());
    //    for (size_t i = 0; i < va_count; ++i)
    //    {
    //      const auto& va_view = chunk.va_views.at(i);
    //      if (va_view)
    //      {
    //        va_items[i] = (reinterpret_cast<D11Resource*>(&va_view->GetResource()))->GetBuffer();
    //        va_offsets[i] = va_view->GetLevelsOrLength().offset;
    //        va_strides[i] = config->GetStrides().at(i);
    //      }
    //    }
    //    device->GetContext()->IASetVertexBuffers(0, va_count, va_items, va_strides, va_offsets);

    //    const size_t ia_limit = 1u;
    //    uint32_t ia_offsets[ia_limit]{ 0 };
    //    DXGI_FORMAT ia_formats[ia_limit]{ DXGI_FORMAT_UNKNOWN };
    //    ID3D11Buffer* ia_items[ia_limit]{ nullptr };
    //    const auto ia_count = std::min(ia_limit, chunk.ia_views.size());
    //    for (size_t i = 0; i < ia_count; ++i)
    //    {
    //      const auto& ia_view = chunk.ia_views.at(i);
    //      if (ia_view)
    //      {
    //        ia_items[i] = (reinterpret_cast<D11Resource*>(&ia_view->GetResource()))->GetBuffer();
    //        ia_offsets[i] = ia_view->GetLevelsOrLength().offset;
    //        ia_formats[i] = config->GetIAState().indexer
    //          == Config::INDEXER_32_BIT ? DXGI_FORMAT_R32_UINT
    //          : Config::INDEXER_16_BIT ? DXGI_FORMAT_R16_UINT
    //          : DXGI_FORMAT_UNKNOWN;
    //      }
    //    }
    //    device->GetContext()->IASetIndexBuffer(ia_items[0], ia_formats[0], ia_offsets[0]);

    //      if (!sb_views.empty())
    //    {
    //      const auto sb_limit = size_t(4u);
    //      const auto sb_count = std::min(sb_limit, sb_views.size());

    //      uint32_t sb_offsets[sb_limit] = {};
    //      uint32_t sb_strides[sb_limit] = {};

    //      for (size_t i = 0; i < sb_count; ++i)
    //      {
    //        const auto& sb_view = sb_views[i];
    //        if (sb_view)
    //        {
    //          sb_offsets[i] = chunk.sb_offset ? chunk.sb_offset.value()[i] / 16u : 0u;
    //          const auto sb_resource = reinterpret_cast<D11Resource*>(&sb_view->GetResource());
    //          sb_strides[i] = sb_resource->GetLayersOrStride() / 16u;
    //        }
    //      }

    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->VSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->HSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->DSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->GSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->PSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //    }

    //    if (chunk.arg_view)
    //    {
    //      const auto aa_buffer = (reinterpret_cast<D11Resource*>(&chunk.arg_view->GetResource()))->GetBuffer();
    //      const auto aa_stride = uint32_t(sizeof(Graphic));
    //      const auto aa_draws = 1u;
    //      const auto aa_offset = chunk.arg_view->GetLevelsOrLength().offset;
    //      device->GetContext()->DrawIndexedInstancedIndirect(aa_buffer, aa_offset);
    //    }
    //    else
    //    {
    //      const auto ins_count = chunk.ins_or_grid_x.length;
    //      const auto ins_offset = chunk.ins_or_grid_x.offset;
    //      const auto vtx_count = chunk.vtx_or_grid_y.length;
    //      const auto vtx_offset = chunk.vtx_or_grid_y.offset;
    //      const auto idx_count = chunk.idx_or_grid_z.length;
    //      const auto idx_offset = chunk.idx_or_grid_z.offset;
    //      device->GetContext()->DrawIndexedInstanced(idx_count, ins_count, idx_offset, vtx_offset, ins_offset);
    //    }
    //  }
    //}


    //if (pass->GetType() == Pass::TYPE_COMPUTE)
    //{
    //  uint32_t wr_initials[8] = { 0 };
    //  device->GetContext()->CSSetUnorderedAccessViews(0, wr_items.size(), wr_items.data(), wr_initials);
    //  device->GetContext()->CSSetShaderResources(0, rr_items.size(), rr_items.data());
    //  device->GetContext()->CSSetConstantBuffers(0, ub_items.size(), ub_items.data());
    //  device->GetContext()->CSSetSamplers(0, sampler_states.size(), sampler_states.data());

    //  for (const auto& chunk : entities)
    //  {
    //    if (!sb_views.empty())
    //    {
    //      const auto sb_limit = size_t(4u);
    //      const auto sb_count = std::min(sb_limit, sb_views.size());

    //      uint32_t sb_offsets[sb_limit] = {};
    //      uint32_t sb_strides[sb_limit] = {};

    //      for (size_t i = 0; i < sb_count; ++i)
    //      {
    //        const auto& sb_view = sb_views[i];
    //        if (sb_view)
    //        {
    //          sb_offsets[i] = chunk.sb_offset ? chunk.sb_offset.value()[i] : 0u;
    //          const auto sb_resource = reinterpret_cast<D11Resource*>(&sb_view->GetResource());
    //          sb_strides[i] = sb_resource->GetLayersOrStride() / 16u;
    //        }
    //      }

    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->VSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->HSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->DSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->GSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //      reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->PSSetConstantBuffers1(ub_items.size(),
    //        sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
    //    }


    //    if (chunk.arg_view)
    //    {
    //      const auto aa_buffer = (reinterpret_cast<D11Resource*>(&chunk.arg_view->GetResource()))->GetBuffer();
    //      const auto aa_stride = uint32_t(sizeof(Compute));
    //      const auto aa_offset = chunk.arg_view->GetLevelsOrLength().offset;
    //      device->GetContext()->DispatchIndirect(aa_buffer, aa_offset);
    //    }
    //    else
    //    {
    //      const auto grid_x = chunk.ins_or_grid_x.length;
    //      const auto grid_y = chunk.vtx_or_grid_y.length;
    //      const auto grid_z = chunk.idx_or_grid_z.length;
    //      device->GetContext()->Dispatch(grid_x, grid_y, grid_z);
    //    }
    //  }
    //}
  }

  void D12Batch::Discard()
  {
    for (uint32_t i = 0; i < sampler_items.size(); ++i)
    {
      //if (sampler_states[i])
      //{
      //  sampler_states[i]->Release();
      //  sampler_states[i] = nullptr;
      //}
    }

    if (root_signature)
    {
      root_signature->Release();
      root_signature = nullptr;
    }
  }

  D12Batch::D12Batch(const std::string& name,
    Config& config,
    const std::pair<const Entity*, size_t>& entities,
    const std::pair<const Sampler*, size_t>& samplers,
    const std::pair<const std::shared_ptr<View>*, size_t>& ub_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& sb_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& ri_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& wi_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& rb_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& wb_views
  )
    : Batch(name, config, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views)
  {
    D12Batch::Initialize();
  }

  D12Batch::~D12Batch()
  {
    D12Batch::Discard();
  }
}