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
#include "d11_batch.h"
#include "d11_state.h"
#include "d11_pass.h"
#include "d11_device.h"
#include "d11_resource.h"
#include "d11_view.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

namespace RayGene3D
{
  void D11Batch::Initialize()
  {
    auto effect = reinterpret_cast<D11State*>(&this->GetState());
    auto pass = reinterpret_cast<D11Pass*>(&effect->GetPass());
    auto device = reinterpret_cast<D11Device*>(&pass->GetDevice());

    const auto get_filter = [this](Sampler::Filtering filtering)
    {
      switch (filtering)
      {
      default: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      case Sampler::FILTERING_NEAREST: return D3D11_FILTER_MIN_MAG_MIP_POINT;
      case Sampler::FILTERING_LINEAR: return D3D11_FILTER_MIN_MAG_MIP_LINEAR;
      case Sampler::FILTERING_ANISOTROPIC: return D3D11_FILTER_ANISOTROPIC;
      }
    };

    const auto get_addressing = [this](Sampler::Addressing addressing)
    {
      switch (addressing)
      {
      default: return D3D11_TEXTURE_ADDRESS_CLAMP;
      case Sampler::ADDRESSING_REPEAT: return D3D11_TEXTURE_ADDRESS_WRAP;
      case Sampler::ADDRESSING_MIRROR: return D3D11_TEXTURE_ADDRESS_MIRROR;
      case Sampler::ADDRESSING_CLAMP: return D3D11_TEXTURE_ADDRESS_CLAMP;
      case Sampler::ADDRESSING_BORDER: return D3D11_TEXTURE_ADDRESS_BORDER;
      }
    };

    const auto get_comparison = [this](Sampler::Comparison comparison)
    {
      switch (comparison)
      {
      default: return D3D11_COMPARISON_NEVER;
      case Sampler::COMPARISON_NEVER: return D3D11_COMPARISON_NEVER;
      case Sampler::COMPARISON_LESS: return D3D11_COMPARISON_LESS;
      case Sampler::COMPARISON_EQUAL: return D3D11_COMPARISON_EQUAL;
      case Sampler::COMPARISON_LESS_EQUAL: return D3D11_COMPARISON_LESS_EQUAL;
      case Sampler::COMPARISON_GREATER: return D3D11_COMPARISON_GREATER;
      case Sampler::COMPARISON_NOT_EQUAL: return D3D11_COMPARISON_NOT_EQUAL;
      case Sampler::COMPARISON_GREATER_EQUAL: return D3D11_COMPARISON_GREATER_EQUAL;
      case Sampler::COMPARISON_ALWAYS: return D3D11_COMPARISON_ALWAYS;
      }
    };

    sampler_states.resize(samplers.size());
    for (uint32_t i = 0; i < sampler_states.size(); ++i)
    {
      D3D11_SAMPLER_DESC sampler_desc{};
      sampler_desc.Filter = get_filter(samplers[i].filtering);
      sampler_desc.AddressU = get_addressing(samplers[i].addressing);
      sampler_desc.AddressV = get_addressing(samplers[i].addressing);
      sampler_desc.AddressW = get_addressing(samplers[i].addressing);
      sampler_desc.MipLODBias = samplers[i].bias_lod;
      sampler_desc.MaxAnisotropy = 16;
      sampler_desc.ComparisonFunc = get_comparison(samplers[i].comparison);
      sampler_desc.BorderColor[0] = samplers[i].color[0];
      sampler_desc.BorderColor[1] = samplers[i].color[1];
      sampler_desc.BorderColor[2] = samplers[i].color[2];
      sampler_desc.BorderColor[3] = samplers[i].color[3];
      sampler_desc.MinLOD =-FLT_MAX; // samplers[i].min_lod;
      sampler_desc.MaxLOD = FLT_MAX; // samplers[i].max_lod;

      BLAST_ASSERT(S_OK == device->GetDevice()->CreateSamplerState(&sampler_desc, &sampler_states[i]));
    }


    const auto ub_count = uint32_t(ub_views.size());
    ub_items.resize(std::min(ub_count, uint32_t(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)), nullptr);
    for (uint32_t i = 0; i < uint32_t(ub_items.size()); ++i)
    {
      if (i < ub_count)
      {
        const auto& ub_view = ub_views[i];
        if (ub_view)
        {
          ub_items[i] = (reinterpret_cast<D11Resource*>(&ub_view->GetResource()))->GetBuffer();
        }
      }
    }

    const auto sb_count = uint32_t(sb_views.size());
    sb_items.resize(std::min(sb_count, uint32_t(D3D11_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)), nullptr);
    for (uint32_t i = 0; i < uint32_t(sb_items.size()); ++i)
    {
      if (i < sb_count)
      {
        const auto& sb_view = sb_views[i];
        if (sb_view)
        {
          sb_items[i] = (reinterpret_cast<D11Resource*>(&sb_view->GetResource()))->GetBuffer();
        }
      }
    }

    const auto rr_count = uint32_t(rb_views.size() + ri_views.size());
    rr_items.resize(std::min(rr_count, uint32_t(D3D11_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)), nullptr);
    for (uint32_t i = 0; i < uint32_t(rr_items.size()); ++i)
    {
      if (i < rr_count)
      {
        uint32_t offset = 0;
        if (i - offset < rb_views.size() && rb_views[i - offset]) { rr_items[i] = (reinterpret_cast<D11View*>(rb_views[i - offset].get()))->GetSRView(); continue; }
        offset += uint32_t(rb_views.size());
        if (i - offset < ri_views.size() && ri_views[i - offset]) { rr_items[i] = (reinterpret_cast<D11View*>(ri_views[i - offset].get()))->GetSRView(); continue; }
      }
    }

    const auto wr_count = uint32_t(wb_views.size() + wi_views.size());
    wr_items.resize(std::min(wr_count, uint32_t(D3D11_PS_CS_UAV_REGISTER_COUNT)), nullptr);
    for (uint32_t i = 0; i < uint32_t(wr_items.size()); ++i)
    {
      if (i < wr_count)
      {
        uint32_t offset = 0;
        if (i - offset < wb_views.size() && wb_views[i - offset]) { wr_items[i] = (reinterpret_cast<D11View*>(wb_views[i - offset].get()))->GetUAView(); continue; }
        offset += uint32_t(wb_views.size());
        if (i - offset < wi_views.size() && wi_views[i - offset]) { wr_items[i] = (reinterpret_cast<D11View*>(wi_views[i - offset].get()))->GetUAView(); continue; }
      }
    }
  }

  void D11Batch::Use()
  {
    auto effect = reinterpret_cast<D11State*>(&this->GetState());
    auto pass = reinterpret_cast<D11Pass*>(&effect->GetPass());
    auto device = reinterpret_cast<D11Device*>(&pass->GetDevice());

    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      device->GetContext()->VSSetShaderResources(0, rr_items.size(), rr_items.data());
      device->GetContext()->VSSetConstantBuffers(0, ub_items.size(), ub_items.data());
      device->GetContext()->VSSetSamplers(0, sampler_states.size(), sampler_states.data());

      device->GetContext()->HSSetShaderResources(0, rr_items.size(), rr_items.data());
      device->GetContext()->HSSetConstantBuffers(0, ub_items.size(), ub_items.data());
      device->GetContext()->HSSetSamplers(0, sampler_states.size(), sampler_states.data());

      device->GetContext()->DSSetShaderResources(0, rr_items.size(), rr_items.data());
      device->GetContext()->DSSetConstantBuffers(0, ub_items.size(), ub_items.data());
      device->GetContext()->DSSetSamplers(0, sampler_states.size(), sampler_states.data());

      device->GetContext()->GSSetShaderResources(0, rr_items.size(), rr_items.data());
      device->GetContext()->GSSetConstantBuffers(0, ub_items.size(), ub_items.data());
      device->GetContext()->GSSetSamplers(0, sampler_states.size(), sampler_states.data());

      device->GetContext()->PSSetShaderResources(0, rr_items.size(), rr_items.data());
      device->GetContext()->PSSetConstantBuffers(0, ub_items.size(), ub_items.data());
      device->GetContext()->PSSetSamplers(0, sampler_states.size(), sampler_states.data());

      for (const auto& chunk : entities)
      {
        const uint32_t va_limit = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
        uint32_t va_strides[va_limit]{ 0 };
        uint32_t va_offsets[va_limit]{ 0 };
        ID3D11Buffer* va_items[va_limit]{ nullptr };
        const uint32_t va_count = std::min(va_limit, uint32_t(chunk.va_views.size()));
        for (uint32_t i = 0; i < va_count; ++i)
        {
          const auto& va_view = chunk.va_views.at(i);
          if (va_view)
          {
            va_items[i] = (reinterpret_cast<D11Resource*>(&va_view->GetResource()))->GetBuffer();
            va_offsets[i] = va_view->GetMipmapsOrCount().offset;
            va_strides[i] = effect->GetStrides().at(i);
          }
        }
        device->GetContext()->IASetVertexBuffers(0, va_count, va_items, va_strides, va_offsets);

        const uint32_t ia_limit = 1;
        uint32_t ia_offsets[ia_limit]{ 0 };
        DXGI_FORMAT ia_formats[ia_limit]{ DXGI_FORMAT_UNKNOWN };
        ID3D11Buffer* ia_items[ia_limit]{ nullptr };
        const uint32_t ia_count = std::min(ia_limit, uint32_t(chunk.ia_views.size()));
        for (uint32_t i = 0; i < ia_count; ++i)
        {
          const auto& ia_view = chunk.ia_views.at(i);
          if (ia_view)
          {
            ia_items[i] = (reinterpret_cast<D11Resource*>(&ia_view->GetResource()))->GetBuffer();
            ia_offsets[i] = ia_view->GetMipmapsOrCount().offset;
            ia_formats[i] = effect->GetIAState().indexer
              == State::INDEXER_32_BIT ? DXGI_FORMAT_R32_UINT
              : State::INDEXER_16_BIT ? DXGI_FORMAT_R16_UINT
              : DXGI_FORMAT_UNKNOWN;
          }
        }
        device->GetContext()->IASetIndexBuffer(ia_items[0], ia_formats[0], ia_offsets[0]);

          if (!sb_views.empty())
        {
          const auto sb_limit = 4u;
          const auto sb_count = std::min(sb_limit, uint32_t(sb_views.size()));

          uint32_t sb_offsets[sb_limit] = {};
          uint32_t sb_strides[sb_limit] = {};

          for (uint32_t i = 0; i < sb_count; ++i)
          {
            const auto& sb_view = sb_views[i];
            if (sb_view)
            {
              sb_offsets[i] = chunk.sb_offset ? chunk.sb_offset.value()[i] / 16u : 0u;
              const auto sb_resource = reinterpret_cast<D11Resource*>(&sb_view->GetResource());
              sb_strides[i] = sb_resource->GetLayersOrStride() / 16u;
            }
          }

          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->VSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->HSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->DSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->GSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->PSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
        }

        if (chunk.arg_view)
        {
          const auto aa_buffer = (reinterpret_cast<D11Resource*>(&chunk.arg_view->GetResource()))->GetBuffer();
          const auto aa_stride = uint32_t(sizeof(Graphic));
          const auto aa_draws = 1;
          const auto aa_offset = chunk.arg_view->GetMipmapsOrCount().offset;
          device->GetContext()->DrawIndexedInstancedIndirect(aa_buffer, aa_offset);
        }
        else
        {
          const auto ins_count = 1u; // subset.ins_range.length;
          const auto ins_offset = 0u; // subset.ins_range.offset;
          const auto vtx_count = chunk.vtx_or_grid_y.length;
          const auto vtx_offset = chunk.vtx_or_grid_y.offset;
          const auto idx_count = chunk.idx_or_grid_z.length;
          const auto idx_offset = chunk.idx_or_grid_z.offset;
          device->GetContext()->DrawIndexedInstanced(idx_count, ins_count, idx_offset, vtx_offset, ins_offset);
        }
      }
    }


    if (pass->GetType() == Pass::TYPE_COMPUTE)
    {
      uint32_t wr_initials[8] = { 0 };
      device->GetContext()->CSSetUnorderedAccessViews(0, wr_items.size(), wr_items.data(), wr_initials);
      device->GetContext()->CSSetShaderResources(0, rr_items.size(), rr_items.data());
      device->GetContext()->CSSetConstantBuffers(0, ub_items.size(), ub_items.data());
      device->GetContext()->CSSetSamplers(0, sampler_states.size(), sampler_states.data());

      for (const auto& chunk : entities)
      {
        if (!sb_views.empty())
        {
          const auto sb_limit = 4u;
          const auto sb_count = std::min(sb_limit, uint32_t(sb_views.size()));

          uint32_t sb_offsets[sb_limit] = {};
          uint32_t sb_strides[sb_limit] = {};

          for (uint32_t i = 0; i < sb_count; ++i)
          {
            const auto& sb_view = sb_views[i];
            if (sb_view)
            {
              sb_offsets[i] = chunk.sb_offset ? chunk.sb_offset.value()[i] : 0u;
              const auto sb_resource = reinterpret_cast<D11Resource*>(&sb_view->GetResource());
              sb_strides[i] = sb_resource->GetLayersOrStride() / 16u;
            }
          }

          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->VSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->HSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->DSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->GSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
          reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->PSSetConstantBuffers1(ub_items.size(),
            sb_items.size(), sb_items.data(), sb_offsets, sb_strides);
        }


        if (chunk.arg_view)
        {
          const auto aa_buffer = (reinterpret_cast<D11Resource*>(&chunk.arg_view->GetResource()))->GetBuffer();
          const auto aa_stride = uint32_t(sizeof(Compute));
          const auto aa_offset = chunk.arg_view->GetMipmapsOrCount().offset;
          device->GetContext()->DispatchIndirect(aa_buffer, aa_offset);
        }
        else
        {
          const auto grid_x = chunk.ins_or_grid_x.length;
          const auto grid_y = chunk.vtx_or_grid_y.length;
          const auto grid_z = chunk.idx_or_grid_z.length;
          device->GetContext()->Dispatch(grid_x, grid_y, grid_z);
        }
      }
    }
  }

  void D11Batch::Discard()
  {
    for (uint32_t i = 0; i < sampler_states.size(); ++i)
    {
      if (sampler_states[i])
      {
        sampler_states[i]->Release();
        sampler_states[i] = nullptr;
      }
    }
  }

  D11Batch::D11Batch(const std::string& name,
    State& effect,
    const std::pair<const Entity*, uint32_t>& entities,
    const std::pair<const Sampler*, uint32_t>& samplers,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views
  )
    : Batch(name, effect, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views)
  {
    D11Batch::Initialize();
  }

  D11Batch::~D11Batch()
  {
    D11Batch::Discard();
  }
}