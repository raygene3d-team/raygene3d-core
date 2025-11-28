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
#include "d12_resource.h"
#include "d12_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

namespace RayGene3D
{
  D3D12_CPU_DESCRIPTOR_HANDLE D12Resource::ObtainGeneral()
  {
    const auto slot = std::distance(general_slots.cbegin(), std::find(general_slots.cbegin(), general_slots.cend(), false));
    if (slot == general_limit) return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

    const auto size = reinterpret_cast<const D12Device*>(&this->GetDevice())->GetGeneralSize();
    const auto handle = general_heap->GetCPUDescriptorHandleForHeapStart().ptr + slot * size;

    general_slots[slot] = true;

    return D3D12_CPU_DESCRIPTOR_HANDLE{ handle };
  }

  D3D12_CPU_DESCRIPTOR_HANDLE D12Resource::ObtainRTV()
  {
    const auto slot = std::distance(rtv_slots.cbegin(), std::find(rtv_slots.cbegin(), rtv_slots.cend(), false));
    if (slot == rtv_limit) return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

    const auto size = reinterpret_cast<const D12Device*>(&this->GetDevice())->GetRTVSize();
    const auto handle = rtv_heap->GetCPUDescriptorHandleForHeapStart().ptr + slot * size;

    rtv_slots[slot] = true;

    return D3D12_CPU_DESCRIPTOR_HANDLE{ handle };
  }

  D3D12_CPU_DESCRIPTOR_HANDLE D12Resource::ObtainDSV()
  {
    const auto slot = std::distance(dsv_slots.cbegin(), std::find(dsv_slots.cbegin(), dsv_slots.cend(), false));
    if (slot == dsv_limit) return D3D12_CPU_DESCRIPTOR_HANDLE{ 0 };

    const auto size = reinterpret_cast<const D12Device*>(&this->GetDevice())->GetDSVSize();
    const auto handle = dsv_heap->GetCPUDescriptorHandleForHeapStart().ptr + slot * size;

    dsv_slots[slot] = true;

    return D3D12_CPU_DESCRIPTOR_HANDLE{ handle };
  }

  void D12Resource::DropGeneral(D3D12_CPU_DESCRIPTOR_HANDLE handle)
  { 
    if (handle.ptr == 0) return;

    const auto size = reinterpret_cast<const D12Device*>(&this->GetDevice())->GetGeneralSize();
    const auto slot = (handle.ptr - general_heap->GetCPUDescriptorHandleForHeapStart().ptr) / size;

    general_slots[slot] = false;
  }

  void D12Resource::DropRTV(D3D12_CPU_DESCRIPTOR_HANDLE handle)
  { 
    if (handle.ptr == 0) return;

    const auto size = reinterpret_cast<const D12Device*>(&this->GetDevice())->GetRTVSize();
    const auto slot = (handle.ptr - rtv_heap->GetCPUDescriptorHandleForHeapStart().ptr) / size;

    rtv_slots[slot] = false;
  }

  void D12Resource::DropDSV(D3D12_CPU_DESCRIPTOR_HANDLE handle)
  {
    if (handle.ptr == 0) return;

    const auto size = reinterpret_cast<const D12Device*>(&this->GetDevice())->GetDSVSize();
    const auto slot = (handle.ptr - dsv_heap->GetCPUDescriptorHandleForHeapStart().ptr) / size;

    dsv_slots[slot] = false;
  }

  void D12Resource::Initialize()
  {
    if (resource)
    {
      BLAST_LOG("Initializing existing Resource: %s", name.c_str());
      return;
    }

    auto device = reinterpret_cast<D12Device*>(&this->GetDevice());

    const auto get_state = [this]()
    {
      auto state = D3D12_RESOURCE_STATE_COMMON | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
      {
        state |= usage & USAGE_SHADER_RESOURCE  ? state | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE             : state;
        state |= usage & USAGE_RENDER_TARGET    ? state | D3D12_RESOURCE_STATE_RENDER_TARGET                     : state;
        state |= usage & USAGE_DEPTH_STENCIL    ? state | D3D12_RESOURCE_STATE_DEPTH_WRITE                       : state;
        state |= usage & USAGE_UNORDERED_ACCESS ? state | D3D12_RESOURCE_STATE_UNORDERED_ACCESS                  : state;
        state |= usage & USAGE_VERTEX_ARRAY     ? state | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER        : state;
        state |= usage & USAGE_INDEX_ARRAY      ? state | D3D12_RESOURCE_STATE_INDEX_BUFFER                      : state;
        state |= usage & USAGE_CONSTANT_DATA    ? state | D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER        : state;
        state |= usage & USAGE_ARGUMENT_LIST    ? state | D3D12_RESOURCE_STATE_INDIRECT_ARGUMENT                 : state;
        state |= usage & USAGE_RAYTRACING_INPUT ? state | D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE : state;
      }
    };

    const auto get_flags = [this]()
    {
      auto flags = D3D12_RESOURCE_FLAG_NONE;
      {
        //flags = usage &~USAGE_SHADER_RESOURCE  ? flags | D3D12_RESOURCE_FLAG_DENY_SHADER_RESOURCE   : flags;
        flags = usage & USAGE_RENDER_TARGET    ? flags | D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET    : flags;
        flags = usage & USAGE_DEPTH_STENCIL    ? flags | D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL    : flags;
        flags = usage & USAGE_UNORDERED_ACCESS ? flags | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS : flags;
      }
      return flags;
    };

    const auto get_format = [this]()
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
    };

    D3D12_RESOURCE_DESC desc{};

    switch (type)
    {
    case TYPE_BUFFER:
    {
      desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      desc.Alignment = 0;
      desc.Width = levels_or_length * layers_or_stride;
      desc.Height = 1;
      desc.DepthOrArraySize = 1;
      desc.MipLevels = 1;
      desc.Format = DXGI_FORMAT_UNKNOWN;
      desc.SampleDesc = { 1, 0 };
      desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      desc.Flags = get_flags();
      break;
    }

    case TYPE_TEX1D:
    {
      desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE1D;
      desc.Alignment = 0;
      desc.Width = size_x;
      desc.Height = 1;
      desc.DepthOrArraySize = layers_or_stride;
      desc.MipLevels = levels_or_length;
      desc.Format = get_format();
      desc.SampleDesc = { 1, 0 };
      desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      desc.Flags = get_flags();
      break;
    }

    case TYPE_TEX2D:
    {
      desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      desc.Alignment = 0;
      desc.Width = size_x;
      desc.Height = size_y;
      desc.DepthOrArraySize = layers_or_stride;
      desc.MipLevels = levels_or_length;
      desc.Format = get_format();
      desc.SampleDesc = { 1, 0 };
      desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      desc.Flags = get_flags();

      //if (tex2d_desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
      //{
      //  switch (tex2d_desc.Format)
      //  {
      //  case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: tex2d_desc.Format = DXGI_FORMAT_R32G8X24_TYPELESS; break;
      //  case DXGI_FORMAT_D32_FLOAT: tex2d_desc.Format = DXGI_FORMAT_R32_TYPELESS; break;
      //  case DXGI_FORMAT_D24_UNORM_S8_UINT: tex2d_desc.Format = DXGI_FORMAT_R24G8_TYPELESS; break;
      //  case DXGI_FORMAT_D16_UNORM: tex2d_desc.Format = DXGI_FORMAT_R16_TYPELESS; break;
      //  }
      //}
      break;
    }

    case TYPE_TEX3D:
    {
      desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
      desc.Alignment = 0;
      desc.Width = size_x;
      desc.Height = size_y;
      desc.DepthOrArraySize = size_z;
      desc.MipLevels = levels_or_length;
      desc.Format = get_format();
      desc.SampleDesc = { 1, 0 };
      desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
      desc.Flags = get_flags();
      break;
    }

    default:
      break;
    }

    D3D12_HEAP_PROPERTIES heap_prop = {};
    heap_prop.Type = hint & HINT_DYNAMIC_BUFFER ? D3D12_HEAP_TYPE_UPLOAD : D3D12_HEAP_TYPE_DEFAULT;
    heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
    heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
    heap_prop.CreationNodeMask = 0;
    heap_prop.VisibleNodeMask = 0;

    BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommittedResource(
      &heap_prop, 
      D3D12_HEAP_FLAG_NONE,
      &desc, 
      D3D12_RESOURCE_STATE_COMMON, 
      nullptr, 
      IID_PPV_ARGS(&resource)));

    if (type == TYPE_BUFFER)
    {
      address = resource->GetGPUVirtualAddress();
    }


    //const auto populate_texture_subresources_fn =
    //  [this](std::pair<const uint8_t*, size_t> interop)
    //  {
    //    BLAST_ASSERT(layers_or_stride * Size(format, size_x, size_y, size_z, { 0, levels_or_length }) == interop.second);

    //    auto offset = 0ull;
    //    auto result = std::vector<D3D12_SUBRESOURCE_DATA>(layers_or_stride * levels_or_length);
    //    for (size_t i = 0; i < layers_or_stride; ++i)
    //    {
    //      for (size_t j = 0; j < levels_or_length; ++j)
    //      {
    //        const auto size = Size(format, size_x, size_y, size_z, { j, 1 });
    //        const auto data = interop.first + offset;

    //        result[i * levels_or_length + j].pData = data;
    //        result[i * levels_or_length + j].RowPitch = size / Mip(size_x, j);
    //        result[i * levels_or_length + j].SlicePitch = size / size_t(Mip(size_x, j) * Mip(size_y, j));

    //        offset += size;
    //      }
    //    }
    //    return result;
    //  };

    //const auto populate_buffer_subresources_fn =
    //  [this](std::pair<const uint8_t*, size_t> interop)
    //  {
    //    D3D12_SUBRESOURCE_DATA subres_data = {};
    //    subres_data.pData = interop.first;
    //    subres_data.RowPitch = 0;
    //    subres_data.SlicePitch = 0;

    //    return subres_data;
    //  };

    {
      D3D12_DESCRIPTOR_HEAP_DESC rtv_heap_desc = {};
      rtv_heap_desc.NumDescriptors = rtv_limit;
      rtv_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
      rtv_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateDescriptorHeap(&rtv_heap_desc, IID_PPV_ARGS(&rtv_heap)));
    }

    {
      D3D12_DESCRIPTOR_HEAP_DESC dsv_heap_esc = {};
      dsv_heap_esc.NumDescriptors = dsv_limit;
      dsv_heap_esc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
      dsv_heap_esc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateDescriptorHeap(&dsv_heap_esc, IID_PPV_ARGS(&dsv_heap)));
    }

    {
      D3D12_DESCRIPTOR_HEAP_DESC general_heap_desc = {};
      general_heap_desc.NumDescriptors = general_limit;
      general_heap_desc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
      general_heap_desc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateDescriptorHeap(&general_heap_desc, IID_PPV_ARGS(&general_heap)));
    }

    if (interop != std::pair(nullptr, 0))
    {
      switch (type)
      {
      case TYPE_BUFFER:
      {
        const auto interop_data = interop.first;
        BLAST_ASSERT(interop_data != nullptr);

        const auto interop_size = interop.second;
        BLAST_ASSERT(interop_size == levels_or_length * layers_or_stride);

        const auto staging_size = device->GetStagingSize();
        const auto staging_buffer = device->GetStagingBuffer();

        ID3D12GraphicsCommandList* command_list = nullptr;
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, 
          device->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&command_list)));

        size_t fence_value = 0;
        ID3D12Fence* fence = nullptr;
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateFence(fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        HANDLE fence_event = CreateEvent(nullptr, false, false, nullptr);

        for (auto i = 0u; i < (interop_size + staging_size - 1) / staging_size; ++i)
        {
          const auto data = interop_data + i * staging_size;
          const auto size = std::min(staging_size, interop_size - i * staging_size);

          uint8_t* mapped = nullptr;
          BLAST_ASSERT(S_OK == staging_buffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped)));
          memcpy(mapped, data, size);
          staging_buffer->Unmap(0, nullptr);

          command_list->CopyBufferRegion(resource, i * staging_size, staging_buffer, 0, size);

          BLAST_ASSERT(S_OK == command_list->Close());
          device->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&command_list));

          ++fence_value;
          BLAST_ASSERT(S_OK == device->GetCommandQueue()->Signal(fence, fence_value));
          BLAST_ASSERT(S_OK == fence->SetEventOnCompletion(fence_value, fence_event));
          WaitForSingleObject(fence_event, INFINITE);
        }
        command_list->Release();
        fence->Release();
        CloseHandle(fence_event);
      }
      break;
      case TYPE_TEX1D:
      case TYPE_TEX2D:
      case TYPE_TEX3D:
      {
        const auto interop_data = interop.first;
        BLAST_ASSERT(interop_data != nullptr);

        const auto interop_size = interop.second;
        BLAST_ASSERT(interop_size == Size(format, size_x, size_y, size_z, { 0, levels_or_length }) * layers_or_stride);

        const auto staging_buffer = device->GetStagingBuffer();
        const auto staging_size = device->GetStagingSize();

        BLAST_ASSERT(Size(format, size_x, size_y, size_z, { 0, 1 }) <= staging_size);

        //constexpr auto subres_limit = 16;
        //D3D12_PLACED_SUBRESOURCE_FOOTPRINT layouts[subres_limit] = {};
        //uint32_t counts[subres_limit] = {};
        //size_t strides[subres_limit] = {};
        //size_t lenghts[subres_limit] = {};
        //device->GetDevice()->GetCopyableFootprints(&desc, 0, 1, 0, layouts, counts, strides, lenghts);

        ID3D12GraphicsCommandList* command_list = nullptr;
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
          device->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&command_list)));
        BLAST_ASSERT(S_OK == command_list->Close());

        size_t fence_value = 0;
        ID3D12Fence* fence = nullptr;
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateFence(fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
        HANDLE fence_event = CreateEvent(nullptr, false, false, nullptr);

        {
          BLAST_ASSERT(S_OK == command_list->Reset(device->GetCommandAllocator(), nullptr));

          D3D12_RESOURCE_BARRIER barrier{};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
          barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
          barrier.Transition.pResource = resource;
          barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COMMON;
          barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
          barrier.Transition.Subresource = 0;
          command_list->ResourceBarrier(1, &barrier);

          BLAST_ASSERT(S_OK == command_list->Close());
          device->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&command_list));

          ++fence_value;
          BLAST_ASSERT(S_OK == device->GetCommandQueue()->Signal(fence, fence_value));
          BLAST_ASSERT(S_OK == fence->SetEventOnCompletion(fence_value, fence_event));
          WaitForSingleObject(fence_event, INFINITE);
        }

        auto offset = 0ull;
        for (size_t i = 0; i < layers_or_stride; ++i)
        {
          for (size_t j = 0; j < levels_or_length; ++j)
          {
            const auto data = interop.first + offset;
            const auto size = Size(format, size_x, size_y, size_z, { j, 1 });

            offset += size;

            uint8_t* mapped = nullptr;
            BLAST_ASSERT(S_OK == staging_buffer->Map(0, nullptr, reinterpret_cast<void**>(&mapped)));
            memcpy(mapped, data, size);
            staging_buffer->Unmap(0, nullptr);

            const auto width = Mip(size_x, j);
            const auto height = Mip(size_y, j);
            const auto depth = Mip(size_z, j);

           

            D3D12_TEXTURE_COPY_LOCATION src{};
            src.pResource = staging_buffer;
            src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
            src.PlacedFootprint.Offset = 0;
            //src.PlacedFootprint.Footprint = layouts[j].Footprint;
            src.PlacedFootprint.Footprint.Format = get_format();
            src.PlacedFootprint.Footprint.Width = width;
            src.PlacedFootprint.Footprint.Height = height;
            src.PlacedFootprint.Footprint.Depth = depth;
            src.PlacedFootprint.Footprint.RowPitch = (size / height * depth + 255u) & ~255u;

            BLAST_ASSERT(S_OK == command_list->Reset(device->GetCommandAllocator(), nullptr));

            D3D12_TEXTURE_COPY_LOCATION dst{};
            dst.pResource = resource;
            dst.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
            dst.SubresourceIndex = i * levels_or_length + j;
            command_list->CopyTextureRegion(&dst, 0, 0, 0, &src, nullptr);

            BLAST_ASSERT(S_OK == command_list->Close());
            device->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&command_list));

            ++fence_value;
            BLAST_ASSERT(S_OK == device->GetCommandQueue()->Signal(fence, fence_value));
            BLAST_ASSERT(S_OK == fence->SetEventOnCompletion(fence_value, fence_event));
            WaitForSingleObject(fence_event, INFINITE);
          }
        }

        {
          BLAST_ASSERT(S_OK == command_list->Reset(device->GetCommandAllocator(), nullptr));

          D3D12_RESOURCE_BARRIER barrier{};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
          barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
          barrier.Transition.pResource = resource;
          barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
          barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COMMON;
          barrier.Transition.Subresource = 0;
          command_list->ResourceBarrier(1, &barrier);

          BLAST_ASSERT(S_OK == command_list->Close());
          device->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&command_list));

          ++fence_value;
          BLAST_ASSERT(S_OK == device->GetCommandQueue()->Signal(fence, fence_value));
          BLAST_ASSERT(S_OK == fence->SetEventOnCompletion(fence_value, fence_event));
          WaitForSingleObject(fence_event, INFINITE);
        }

        command_list->Release();
        fence->Release();
        CloseHandle(fence_event);
      }
      break;
      }
    }
    for (auto& view : views)
    {
      view->Initialize();
    }
  }

  void D12Resource::Use()
  {
  }

  void D12Resource::Discard()
  {
    for (auto& view : views)
    {
      view->Discard();
    }

    if (general_heap)
    {
      general_heap->Release();
      general_heap = nullptr;
    }

    if (rtv_heap)
    {
      rtv_heap->Release();
      rtv_heap = nullptr;
    }

    if (dsv_heap)
    {
      dsv_heap->Release();
      dsv_heap = nullptr;
    }

    if (resource)
    {
      resource->Release();
      resource = nullptr;
    }
  }


  void D12Resource::Commit()
  {
    D12Device* device = reinterpret_cast<D12Device*>(&this->GetDevice());


    //const auto [data, size] = interops[index];
    ////const auto [stride, count] = properties[index]->GetRawSize();

    //switch (type)
    //{
    //case TYPE_BUFFER:
    //  if (size == levels_or_length * layers_or_stride)
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, 0, 0);
    //  }
    //  break;

    //case TYPE_TEX1D:
    //  if (size * 8 == size_x * BitCount(format))
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, 0, 0);
    //  }
    //  break;

    //case TYPE_TEX2D:
    //  if (size * 8 == size_x * size_y * BitCount(format))
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, size_x, 0);
    //  }
    //  break;

    //case TYPE_TEX3D:
    //  if (size * 8 == size_x * size_y * size_z * BitCount(format))
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, size_x, size_x * size_y);
    //  }
    //  break;
    //}
  }


  void D12Resource::Retrieve()
  {
    D12Device* device = reinterpret_cast<D12Device*>(&this->GetDevice());


    //ID3D11Resource* temp_resource = nullptr;
    //switch (type)
    //{
    //case TYPE_BUFFER:
    //{
    //  D3D11_BUFFER_DESC temp_buffer_desc{ 0 };
    //  temp_buffer_desc.ByteWidth = stride * count;
    //  temp_buffer_desc.Usage = D3D11_USAGE_STAGING;
    //  temp_buffer_desc.BindFlags = 0;
    //  temp_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    //  temp_buffer_desc.MiscFlags = 0;
    //  temp_buffer_desc.StructureByteStride = 0;
    //  BLAST_ASSERT(S_OK == device->GetDevice()->CreateBuffer(&temp_buffer_desc, nullptr, (ID3D11Buffer**)&temp_resource));
    //  break;
    //}
    //case TYPE_IMAGE2D:
    //{
    //  D3D11_TEXTURE2D_DESC temp_tex2d_desc{ 0 };
    //  temp_tex2d_desc.Width = extent_x;
    //  temp_tex2d_desc.Height = extent_y;
    //  temp_tex2d_desc.MipLevels = 1;
    //  temp_tex2d_desc.ArraySize = mipmaps;
    //  temp_tex2d_desc.Format = get_format();
    //  temp_tex2d_desc.SampleDesc = tex2d_desc.SampleDesc;
    //  temp_tex2d_desc.Usage = D3D11_USAGE_STAGING;
    //  temp_tex2d_desc.BindFlags = 0;
    //  temp_tex2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    //  temp_tex2d_desc.MiscFlags = 0;
    //  hr = core->GetDevice()->CreateTexture2D(&temp_texture2d_desc, nullptr, (ID3D11Texture2D**)&temp_resource);
    //  break;
    //}
    //default: break;
    //}
    //BLAST_ASSERT(hr == S_OK);

    //core->GetContext()->CopyResource(temp_resource, resource_);

    //D3D11_MAPPED_SUBRESOURCE temp_mapped_subresource{ 0 };
    //hr = core->GetContext()->Map(temp_resource, index, D3D11_MAP_READ_WRITE, 0, &temp_mapped_subresource);
    //BLAST_ASSERT(hr == S_OK);

    //switch (type_)
    //{
    //case TYPE_BUFFER:
    //{
    //  properties_[index]->SetRawBytes({ temp_mapped_subresource.pData, buffer_desc_.ByteWidth }, 0);
    //  break;
    //}
    //case TYPE_IMAGE2D:
    //{
    //  properties_[index]->SetRawBytes({ temp_mapped_subresource.pData, 4 * texture2d_desc_.Width * texture2d_desc_.Height }, 0); // WARNING!!! Texture2D is assumed to be power-of-two
    //  break;
    //}
    //default: break;
    //}

    //core->GetContext()->Unmap(temp_resource, index);
    //temp_resource->Release();
  }

  void D12Resource::Blit(const std::shared_ptr<Resource>& resource)
  {
    D12Device* device = reinterpret_cast<D12Device*>(&this->GetDevice());

    if (resource)
    {
      ID3D12GraphicsCommandList* command_list = nullptr;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        device->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&command_list)));

      size_t fence_value = 0;
      ID3D12Fence* fence = nullptr;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateFence(fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
      HANDLE fence_event = CreateEvent(nullptr, false, false, nullptr);
      
      command_list->CopyResource(this->resource, reinterpret_cast<D12Resource*>(resource.get())->resource);

      BLAST_ASSERT(S_OK == command_list->Close());
      device->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&command_list));

      ++fence_value;
      BLAST_ASSERT(S_OK == device->GetCommandQueue()->Signal(fence, fence_value));
      BLAST_ASSERT(S_OK == fence->SetEventOnCompletion(fence_value, fence_event));
      WaitForSingleObject(fence_event, INFINITE);

      command_list->Release();
      fence->Release();
      CloseHandle(fence_event);
    }   
  }


  void* D12Resource::Map()
  {
    D12Device* device = reinterpret_cast<D12Device*>(&this->GetDevice());

    if (type != Resource::TYPE_BUFFER || ~hint & HINT_DYNAMIC_BUFFER)
    {
      return nullptr;
    }

    void* mapped = nullptr;
    BLAST_ASSERT(S_OK == resource->Map(0, nullptr, reinterpret_cast<void**>(&mapped)));

    return mapped;
  }

  void D12Resource::Unmap()
  {
    D12Device* device = reinterpret_cast<D12Device*>(&this->GetDevice());

    if (type != Resource::TYPE_BUFFER || ~hint & HINT_DYNAMIC_BUFFER)
    {
      return;
    }

    resource->Unmap(0, nullptr);
  }

  D12Resource::D12Resource(const std::string& name,
    Device& device,
    const Resource::BufferDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    D12Resource::Initialize();
  }

  D12Resource::D12Resource(const std::string& name,
    Device& device,
    const Resource::Tex1DDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    D12Resource::Initialize();
  }

  D12Resource::D12Resource(const std::string& name,
    Device& device,
    const Resource::Tex2DDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    D12Resource::Initialize();
  }

  D12Resource::D12Resource(const std::string& name,
    Device& device,
    const Resource::Tex3DDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    D12Resource::Initialize();
  }

  D12Resource::~D12Resource()
  {
    D12Resource::Discard();
  }
}