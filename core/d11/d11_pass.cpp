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
      const uint32_t rt_count = std::min(rt_limit, uint32_t(rt_attachments.size()));
      for (uint32_t i = 0; i < rt_count; ++i)
      {
        const auto& rt_view = rt_attachments[i].view;
        const auto& rt_value = rt_attachments[i].value;

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
      const uint32_t ds_count = std::min(ds_limit, uint32_t(ds_attachments.size()));
      for (uint32_t i = 0; i < ds_count; ++i)
      {
        const auto& ds_view = ds_attachments[i].view;
        const auto& ds_value = ds_attachments[i].value;

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
    }

    for (const auto& effect : effects)
    {
      effect->Use();
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

  D11Pass::D11Pass(const std::string& name,
    Device& device,
    Pass::Type type,
    uint32_t size_x,
    uint32_t size_y,
    uint32_t layers,
    const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments,
    const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments)
    : Pass(name, device, type, size_x, size_y, layers, rt_attachments, ds_attachments)
  {
    D11Pass::Initialize();
  }

  D11Pass::~D11Pass()
  {
    D11Pass::Discard();
  }
}