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
#include "d11_layout.h"
#include "d11_device.h"
#include "d11_resource.h"
#include "d11_view.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

namespace RayGene3D
{
  void D11Layout::Initialize()
  {
    auto device = reinterpret_cast<D11Device*>(&this->GetDevice());

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

  void D11Layout::Use()
  {
    auto device = reinterpret_cast<D11Device*>(&this->GetDevice());
  }

  void D11Layout::Discard()
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

  D11Layout::D11Layout(const std::string& name, Device& device) 
    : Layout(name, device)
  {
  }

  D11Layout::~D11Layout()
  {
    D11Layout::Discard();
  }
}