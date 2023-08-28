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
#include "d11_pass.h"
#include "d11_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

namespace RayGene3D
{
  void D11Pass::Initialize()
  {
  }

  void D11Pass::Use()
  {
    if (!enabled) return;

    auto device = reinterpret_cast<D11Device*>(&this->GetDevice());

    if (type == TYPE_GRAPHIC)
    {
      const uint32_t rt_limit = D3D11_SIMULTANEOUS_RENDER_TARGET_COUNT;
      ID3D11RenderTargetView* rt_items[rt_limit]{ nullptr };
      const uint32_t rt_count = std::min(rt_limit, uint32_t(rt_views.size()));
      for (uint32_t i = 0; i < rt_count; ++i)
      {
        const auto& rt_view = rt_views[i];
        const auto& rt_value = rt_values[i];

        if (rt_view)
        {
          rt_items[i] = (reinterpret_cast<D11View*>(rt_view.get()))->GetRTView();

          if (rt_value)
          {
            const auto& clear_target = rt_value.value();
            device->GetContext()->ClearRenderTargetView(rt_items[i], clear_target.data());
          }
        }
      }

      const uint32_t ds_limit = 1;
      ID3D11DepthStencilView* ds_items[ds_limit]{ nullptr };
      const uint32_t ds_count = std::min(ds_limit, uint32_t(ds_views.size()));
      for (uint32_t i = 0; i < ds_count; ++i)
      {
        const auto& ds_view = ds_views[i];
        const auto& ds_value = ds_values[i];

        if (ds_view)
        {
          ds_items[i] = (reinterpret_cast<D11View*>(ds_view.get()))->GetDSView();

          uint32_t clear_flags = 0;
          float clear_depth = 0.0f;
          uint8_t clear_stencil = 0;

          if (ds_value.first)
          {
            clear_flags |= D3D11_CLEAR_DEPTH;
            clear_depth = ds_value.first.value();
          }

          if (ds_value.second)
          {
            clear_flags |= D3D11_CLEAR_STENCIL;
            clear_stencil = ds_value.second.value();
          }

          if (clear_flags)
          {
            device->GetContext()->ClearDepthStencilView(ds_items[i], clear_flags, clear_depth, clear_stencil);
          }
        }
      }

      device->GetContext()->OMSetRenderTargets(rt_count, rt_items, ds_items[0]);


      for (uint32_t j = 0; j < uint32_t(subpasses.size()); ++j)
      {
        const auto& subpass = subpasses[j];

        const auto config = reinterpret_cast<const D11Config*>(subpass.config.get());
        const auto layout = reinterpret_cast<const D11Layout*>(subpass.layout.get());

        device->GetContext()->VSSetShader(config->GetVSShader(), nullptr, 0);
        device->GetContext()->HSSetShader(config->GetHSShader(), nullptr, 0);
        device->GetContext()->DSSetShader(config->GetDSShader(), nullptr, 0);
        device->GetContext()->GSSetShader(config->GetGSShader(), nullptr, 0);
        device->GetContext()->PSSetShader(config->GetPSShader(), nullptr, 0);


        device->GetContext()->IASetInputLayout(config->GetInputLayout());
        device->GetContext()->IASetPrimitiveTopology(config->GetPrimitiveTopology());

        device->GetContext()->RSSetState(config->GetRasterState());

        uint32_t stencil_reference = 0xFFFFFFFF;
        device->GetContext()->OMSetDepthStencilState(config->GetDepthState(), stencil_reference);

        float blend_factors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        uint32_t sample_mask = 0xFFFFFFFF;
        device->GetContext()->OMSetBlendState(config->GetBlendState(), blend_factors, sample_mask);

        device->GetContext()->RSSetViewports(config->GetViewportCount(), config->GetViewportItems());

        device->GetContext()->VSSetShaderResources(0, layout->GetRRCount(), layout->GetRRItems());
        device->GetContext()->VSSetConstantBuffers(0, layout->GetUBCount(), layout->GetUBItems());
        device->GetContext()->VSSetSamplers(0, layout->GetSamplerCount(), layout->GetSamplerItems());

        device->GetContext()->HSSetShaderResources(0, layout->GetRRCount(), layout->GetRRItems());
        device->GetContext()->HSSetConstantBuffers(0, layout->GetUBCount(), layout->GetUBItems());
        device->GetContext()->HSSetSamplers(0, layout->GetSamplerCount(), layout->GetSamplerItems());

        device->GetContext()->DSSetShaderResources(0, layout->GetRRCount(), layout->GetRRItems());
        device->GetContext()->DSSetConstantBuffers(0, layout->GetUBCount(), layout->GetUBItems());
        device->GetContext()->DSSetSamplers(0, layout->GetSamplerCount(), layout->GetSamplerItems());

        device->GetContext()->GSSetShaderResources(0, layout->GetRRCount(), layout->GetRRItems());
        device->GetContext()->GSSetConstantBuffers(0, layout->GetUBCount(), layout->GetUBItems());
        device->GetContext()->GSSetSamplers(0, layout->GetSamplerCount(), layout->GetSamplerItems());

        device->GetContext()->PSSetShaderResources(0, layout->GetRRCount(), layout->GetRRItems());
        device->GetContext()->PSSetConstantBuffers(0, layout->GetUBCount(), layout->GetUBItems());
        device->GetContext()->PSSetSamplers(0, layout->GetSamplerCount(), layout->GetSamplerItems());


        const uint32_t va_limit = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
        uint32_t va_strides[va_limit]{ 0 };
        uint32_t va_offsets[va_limit]{ 0 };
        ID3D11Buffer* va_items[va_limit]{ nullptr };
        const uint32_t va_count = std::min(va_limit, uint32_t(subpass.va_views.size()));
        for (uint32_t i = 0; i < va_count; ++i)
        {
          const auto& va_view = subpass.va_views[i];
          if (va_view)
          {
            va_items[i] = (reinterpret_cast<D11Resource*>(&va_view->GetResource()))->GetBuffer();
            va_strides[i] = config->GetStrides().at(i);
          }
        }
        device->GetContext()->IASetVertexBuffers(0, va_count, va_items, va_strides, va_offsets);

        const uint32_t ia_limit = 1;
        uint32_t ia_offsets[ia_limit]{ 0 };
        DXGI_FORMAT ia_formats[ia_limit]{ DXGI_FORMAT_UNKNOWN };
        ID3D11Buffer* ia_items[ia_limit]{ nullptr };
        const uint32_t ia_count = std::min(ia_limit, uint32_t(subpass.ia_views.size()));
        for (uint32_t i = 0; i < ia_count; ++i)
        {
          const auto& ia_view = subpass.ia_views[i];
          if (ia_view)
          {
            ia_items[i] = (reinterpret_cast<D11Resource*>(&ia_view->GetResource()))->GetBuffer();
            ia_formats[i] = config->GetIndexer() 
              == Config::INDEXER_32_BIT ? DXGI_FORMAT_R32_UINT 
               : Config::INDEXER_16_BIT ? DXGI_FORMAT_R16_UINT
               : DXGI_FORMAT_UNKNOWN;
          }
        }
        device->GetContext()->IASetIndexBuffer(ia_items[0], ia_formats[0], ia_offsets[0]);

        for (const auto& command : subpass.commands)
        {
          if (!command.offsets.empty())
          {
            const auto offset_limit = 16u;
            const auto offset_count = std::min(offset_limit, uint32_t(command.offsets.size()));
            std::array<uint32_t, offset_limit> offset_array;
            for (uint32_t i = 0; i < offset_count; ++i) { offset_array[i] = command.offsets[i] / 16; }
            std::array<uint32_t, offset_limit> offset_range;
            for (uint32_t i = 0; i < offset_count; ++i) { offset_range[i] = 256 / 16; }
            
            reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->VSSetConstantBuffers1(layout->GetUBCount(),
              layout->GetSBCount(), layout->GetSBItems(), offset_array.data(), offset_range.data());
            reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->HSSetConstantBuffers1(layout->GetUBCount(),
              layout->GetSBCount(), layout->GetSBItems(), offset_array.data(), offset_range.data());
            reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->DSSetConstantBuffers1(layout->GetUBCount(),
              layout->GetSBCount(), layout->GetSBItems(), offset_array.data(), offset_range.data());
            reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->GSSetConstantBuffers1(layout->GetUBCount(),
              layout->GetSBCount(), layout->GetSBItems(), offset_array.data(), offset_range.data());
            reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->PSSetConstantBuffers1(layout->GetUBCount(),
              layout->GetSBCount(), layout->GetSBItems(), offset_array.data(), offset_range.data());
          }

          if (command.view)
          {
            device->GetContext()->DrawIndexedInstancedIndirect((reinterpret_cast<D11Resource*>(&command.view->GetResource()))->GetBuffer(), command.view->GetCount().offset + 0 * 4);
          }
          else
          {
            device->GetContext()->DrawIndexedInstanced(command.argument.idx_count, command.argument.ins_count, 
              command.argument.idx_offset, command.argument.vtx_offset, command.argument.ins_offset);
          }
        }
      }
    }

    if (type == TYPE_COMPUTE)
    {
      for (uint32_t j = 0; j < uint32_t(subpasses.size()); ++j)
      {
        const auto& subpass = subpasses[j];

        const auto config = reinterpret_cast<const D11Config*>(subpass.config.get());
        const auto layout = reinterpret_cast<const D11Layout*>(subpass.layout.get());

        device->GetContext()->CSSetShader(config->GetCSShader(), nullptr, 0);

        uint32_t wr_initials[8] = { 0 };
        device->GetContext()->CSSetUnorderedAccessViews(0, layout->GetWRCount(), layout->GetWRItems(), wr_initials);
        device->GetContext()->CSSetShaderResources(0, layout->GetRRCount(), layout->GetRRItems());
        device->GetContext()->CSSetConstantBuffers(0, layout->GetUBCount(), layout->GetUBItems());
        device->GetContext()->CSSetSamplers(0, layout->GetSamplerCount(), layout->GetSamplerItems());

        for (const auto& command : subpass.commands)
        {
          if (!command.offsets.empty())
          {
            const auto offset_limit = 16u;
            const auto offset_count = std::min(offset_limit, uint32_t(command.offsets.size()));

            std::array<uint32_t, offset_limit> offset_array;
            for (uint32_t i = 0; i < offset_count; ++i) { offset_array[i] = command.offsets[i] / 16; }
            std::array<uint32_t, offset_limit> offset_range;
            for (uint32_t i = 0; i < offset_count; ++i) { offset_range[i] = 256 / 16; }

            reinterpret_cast<ID3D11DeviceContext1*>(device->GetContext())->CSSetConstantBuffers1(layout->GetUBCount(),
              layout->GetSBCount(), layout->GetSBItems(), offset_array.data(), offset_range.data());
          }

          if (command.view)
          {
            device->GetContext()->DispatchIndirect((reinterpret_cast<D11Resource*>(&command.view->GetResource()))->GetBuffer(), command.view->GetCount().offset + 5 * 4);
          }
          else
          {
            device->GetContext()->Dispatch(command.argument.grid_x, command.argument.grid_y, command.argument.grid_z);
          }
        }
      }
    }

    device->GetContext()->ClearState();
  }

  void D11Pass::Discard()
  {
    //if (context)
    //{
    //  context->Release();
    //  context = nullptr;
    //}

    //if (command_list)
    //{
    //  command_list->Release();
    //  command_list = nullptr;
    //}
  }

  D11Pass::D11Pass(const std::string& name, Device& device) 
    : Pass(name, device)
  {
  }

  D11Pass::~D11Pass()
  {
    D11Pass::Discard();
  }
}