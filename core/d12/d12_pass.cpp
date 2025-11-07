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
#include "d12_pass.h"
#include "d12_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

namespace RayGene3D
{
  void D12Pass::Initialize()
  {
  }

  void D12Pass::Use()
  {
    if (!enabled) return;

    auto device = reinterpret_cast<D12Device*>(&this->GetDevice());

    if (type == TYPE_GRAPHIC)
    {
      constexpr auto rt_limit = size_t(D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
      D3D12_CPU_DESCRIPTOR_HANDLE rt_items[rt_limit] = {};
      const auto rt_count = std::min(rt_limit, rt_attachments.size());
      for (auto i = 0; i < rt_count; ++i)
      {
        const auto& rt_view = rt_attachments[i].view;
        const auto& rt_value = rt_attachments[i].value;

        if (rt_view)
        {
          rt_items[i] = (reinterpret_cast<D12View*>(rt_view.get()))->GetView();

          if (rt_value)
          {
            const auto& clear_target = rt_value.value();
            device->GetCommandList()->ClearRenderTargetView(rt_items[i], clear_target.data(), 0, nullptr);
          }
        }
      }

      constexpr auto ds_limit = size_t(1);
      D3D12_CPU_DESCRIPTOR_HANDLE ds_items[ds_limit] = {};
      const auto ds_count = std::min(ds_limit, ds_attachments.size());
      for (size_t i = 0; i < ds_count; ++i)
      {
        const auto& ds_view = ds_attachments[i].view;
        const auto& ds_value = ds_attachments[i].value;

        if (ds_view)
        {
          ds_items[i] = (reinterpret_cast<D12View*>(ds_view.get()))->GetView();

          D3D12_CLEAR_FLAGS clear_flags = {};
          float clear_depth = 0.0f;
          uint8_t clear_stencil = 0;

          if (ds_value.first)
          {
            clear_flags |= D3D12_CLEAR_FLAG_DEPTH;
            clear_depth = ds_value.first.value();
          }

          if (ds_value.second)
          {
            clear_flags |= D3D12_CLEAR_FLAG_STENCIL;
            clear_stencil = ds_value.second.value();
          }

          if (clear_flags)
          {
            device->GetCommandList()->ClearDepthStencilView(ds_items[i], clear_flags, clear_depth, clear_stencil, 0, nullptr);
          }
        }
      }

      device->GetCommandList()->OMSetRenderTargets(rt_count, rt_items, false, &ds_items[0]);
    }

    for (const auto& config : configs)
    {
      config->Use();
    }

    //device->GetCommandList()->ClearState();
  }

  void D12Pass::Discard()
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

  D12Pass::D12Pass(const std::string& name,
    Device& device,
    Pass::Type type,
    uint32_t size_x,
    uint32_t size_y,
    size_t layers,
    const std::pair<const Pass::RTAttachment*, size_t>& rt_attachments,
    const std::pair<const Pass::DSAttachment*, size_t>& ds_attachments)
    : Pass(name, device, type, size_x, size_y, layers, rt_attachments, ds_attachments)
  {
    D12Pass::Initialize();
  }

  D12Pass::~D12Pass()
  {
    D12Pass::Discard();
  }
}