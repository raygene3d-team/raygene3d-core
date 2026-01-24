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
  static DXGI_FORMAT DXGIFormat(Format format)
  {
    switch (format)
    {
    default: return DXGI_FORMAT_UNKNOWN;
    case FORMAT_R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
    case FORMAT_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
    case FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
    case FORMAT_R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
    case FORMAT_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
    case FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
    case FORMAT_R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
    case FORMAT_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
    case FORMAT_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
    case FORMAT_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
    case FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
    case FORMAT_R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
    case FORMAT_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
    case FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
    case FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
    case FORMAT_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
    case FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
    case FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
    case FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
    case FORMAT_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
    case FORMAT_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
    case FORMAT_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
    case FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
    case FORMAT_R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
    case FORMAT_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
    case FORMAT_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
    case FORMAT_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
    case FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
    case FORMAT_D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
    case FORMAT_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
    case FORMAT_R32_UINT: return DXGI_FORMAT_R32_UINT;
    case FORMAT_R32_SINT: return DXGI_FORMAT_R32_SINT;
    case FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
    case FORMAT_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
    case FORMAT_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
    case FORMAT_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
    case FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
    case FORMAT_R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
    case FORMAT_D16_UNORM: return DXGI_FORMAT_D16_UNORM;
    case FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
    case FORMAT_R16_UINT: return DXGI_FORMAT_R16_UINT;
    case FORMAT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
    case FORMAT_R16_SINT: return DXGI_FORMAT_R16_SINT;
    case FORMAT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
    case FORMAT_R8_UINT: return DXGI_FORMAT_R8_UINT;
    case FORMAT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
    case FORMAT_R8_SINT: return DXGI_FORMAT_R8_SINT;
    case FORMAT_R9G9B9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
    case FORMAT_R8G8_B8G8_UNORM: return DXGI_FORMAT_R8G8_B8G8_UNORM;
    case FORMAT_G8R8_G8B8_UNORM: return DXGI_FORMAT_G8R8_G8B8_UNORM;
    case FORMAT_BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
    case FORMAT_BC1_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
    case FORMAT_BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
    case FORMAT_BC2_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
    case FORMAT_BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
    case FORMAT_BC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
    case FORMAT_BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
    case FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
    case FORMAT_BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
    case FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
    case FORMAT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
    case FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
    case FORMAT_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
    case FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
    case FORMAT_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
    case FORMAT_B8G8R8X8_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
    case FORMAT_BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
    case FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
    case FORMAT_BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
    case FORMAT_BC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
    }
  }


  DXGI_FORMAT D12Pass::GetRTFormat(size_t index) const
  {
    return index < rt_attachments.size()
      ? rt_attachments[index].view
      ? DXGIFormat(rt_attachments[index].view->GetResource().GetFormat())
      : DXGI_FORMAT_UNKNOWN
      : DXGI_FORMAT_UNKNOWN;
  }

  DXGI_FORMAT D12Pass::GetDSFormat(size_t index) const
  {
    return index < ds_attachments.size()
      ? ds_attachments[index].view
      ? DXGIFormat(ds_attachments[index].view->GetResource().GetFormat())
      : DXGI_FORMAT_UNKNOWN
      : DXGI_FORMAT_UNKNOWN;
  }

  void D12Pass::Initialize()
  {
    auto device = reinterpret_cast<D12Device*>(&this->GetDevice());
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
          const auto slot = (reinterpret_cast<D12View*>(rt_view.get()))->GetSlot();
          rt_items[i] = device->GetRTHandle(slot).cpu;
          
          const auto resource = (reinterpret_cast<D12Resource*>(&rt_view->GetResource()));

          D3D12_RESOURCE_BARRIER barrier = {};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
          barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
          barrier.Transition.pResource = resource->GetResource();
          barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
          barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
          device->GetCommandList()->ResourceBarrier(1, &barrier);

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
          const auto slot = (reinterpret_cast<D12View*>(ds_view.get()))->GetSlot();
          ds_items[i] = device->GetDSHandle(slot).cpu;
          
          const auto resource = (reinterpret_cast<D12Resource*>(&ds_view->GetResource()));

          D3D12_RESOURCE_BARRIER barrier = {};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
          barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
          barrier.Transition.pResource = resource->GetResource();
          barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
          barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_DEPTH_WRITE;
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
          device->GetCommandList()->ResourceBarrier(1, &barrier);

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

      const auto ds_item = ds_items[0].ptr != 0 ? &ds_items[0] : nullptr;

      device->GetCommandList()->OMSetRenderTargets(rt_count, rt_items, false, ds_item);
    }

    if (type == TYPE_COMPUTE)
    {
    }


    for (const auto& config : configs)
    {
      config->Use();
    }

    if (type == TYPE_GRAPHIC)
    {
      constexpr auto rt_limit = size_t(D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT);
      const auto rt_count = std::min(rt_limit, rt_attachments.size());
      for (auto i = 0; i < rt_count; ++i)
      {
        const auto& rt_view = rt_attachments[i].view;

        if (rt_view)
        {
          const auto resource = (reinterpret_cast<D12Resource*>(&rt_view->GetResource()));

          D3D12_RESOURCE_BARRIER barrier = {};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
          barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
          barrier.Transition.pResource = resource->GetResource();
          barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
          barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
          device->GetCommandList()->ResourceBarrier(1, &barrier);
        }
      }

      constexpr auto ds_limit = size_t(1);
      const auto ds_count = std::min(ds_limit, ds_attachments.size());
      for (size_t i = 0; i < ds_count; ++i)
      {
        const auto& ds_view = ds_attachments[i].view;
        const auto& ds_value = ds_attachments[i].value;

        if (ds_view)
        {
          auto resource = (reinterpret_cast<D12Resource*>(&ds_view->GetResource()));

          D3D12_RESOURCE_BARRIER barrier = {};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
          barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
          barrier.Transition.pResource = resource->GetResource();
          barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_DEPTH_WRITE;
          barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
          barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
          device->GetCommandList()->ResourceBarrier(1, &barrier);
        }
      }
    }

    //D3D12_GLOBAL_BARRIER global_barrier = {};
    //global_barrier.SyncBefore = D3D12_BARRIER_SYNC_ALL;
    //global_barrier.SyncAfter = D3D12_BARRIER_SYNC_ALL;
    //global_barrier.AccessBefore = D3D12_BARRIER_ACCESS_COMMON;
    //global_barrier.AccessAfter = D3D12_BARRIER_ACCESS_COMMON;

    //D3D12_BARRIER_GROUP barrier_group = {};
    //barrier_group.Type = D3D12_BARRIER_TYPE_GLOBAL;
    //barrier_group.pGlobalBarriers = &global_barrier;
    //barrier_group.NumBarriers = 1;

    //device->GetCommandList()->Barrier(1, &barrier_group);

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