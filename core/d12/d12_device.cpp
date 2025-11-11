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
#include "d12_device.h"
#include "d12_view.h"
#include "d12_resource.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

namespace RayGene3D
{
  void D12Device::Initialize()
  {
    if (device)
    {
      BLAST_LOG("Initializing existing Device: %s", name.c_str());
      return;
    }

    IDXGIFactory* factory = nullptr;
    BLAST_ASSERT(S_OK == CreateDXGIFactory(IID_PPV_ARGS(&factory)));

    IDXGIAdapter* adapter = nullptr;
    BLAST_ASSERT(S_OK == factory->EnumAdapters(ordinal, &adapter));

    DXGI_ADAPTER_DESC adapter_desc;
    adapter->GetDesc(&adapter_desc);

    char adapter_name[256];
    size_t adapter_size = 0;
    wcstombs_s(&adapter_size, adapter_name, adapter_desc.Description, 256);
    name = std::string(adapter_name) + " (D3D12 API)\n";

    if (debug)
    {
      BLAST_ASSERT(S_OK == D3D12GetDebugInterface(IID_PPV_ARGS(&debug_controller)));
      debug_controller->EnableDebugLayer();
    }

    const D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_12_0;
    const D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_UNKNOWN;

    BLAST_ASSERT(S_OK == D3D12CreateDevice(adapter, feature_level, IID_PPV_ARGS(&device)));

    D3D12_COMMAND_QUEUE_DESC queue_desc = {};
    queue_desc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queue_desc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
    BLAST_ASSERT(S_OK == device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&command_queue)));

    BLAST_ASSERT(S_OK == device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&command_allocator)));

    BLAST_ASSERT(S_OK == device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, command_allocator, nullptr, IID_PPV_ARGS(&command_list)));



    if (window)
    {
      DXGI_SWAP_CHAIN_DESC swapchain_desc = {};
      swapchain_desc.BufferDesc.Width = extent_x;
      swapchain_desc.BufferDesc.Height = extent_y;
      swapchain_desc.BufferDesc.RefreshRate = { 0, 0 };
      swapchain_desc.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
      swapchain_desc.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
      swapchain_desc.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
      swapchain_desc.SampleDesc = { 1, 0 };
      swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
      swapchain_desc.BufferCount = 3;
      swapchain_desc.OutputWindow = reinterpret_cast<HWND>(window);
      swapchain_desc.Windowed = true;
      swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
      swapchain_desc.Flags = 0;

      //IDXGIDevice* dxgi_device = nullptr;
      //BLAST_ASSERT(S_OK == device->QueryInterface(IID_PPV_ARGS(&dxgi_device)));

      //IDXGIAdapter* dxgi_adapter = nullptr;
      //BLAST_ASSERT(S_OK == dxgi_device->GetParent(IID_PPV_ARGS(&dxgi_adapter)));

      //IDXGIFactory* dxgi_factory = nullptr;
      //BLAST_ASSERT(S_OK == dxgi_adapter->GetParent(IID_PPV_ARGS(&dxgi_factory)));

      HRESULT res = factory->CreateSwapChain(command_queue, &swapchain_desc, &swapchain);
      BLAST_ASSERT(S_OK == res);

      BLAST_ASSERT(S_OK == swapchain->GetBuffer(0, IID_PPV_ARGS(&screen_buffer)));

      {
        D3D12_HEAP_PROPERTIES heap_properties = {};
        heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
        heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
        heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
        heap_properties.CreationNodeMask = 0;
        heap_properties.VisibleNodeMask = 0;

        D3D12_RESOURCE_DESC  resource_desc = {};
        resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
        resource_desc.Alignment = 0;
        resource_desc.Width = staging_size;
        resource_desc.Height = 1;
        resource_desc.DepthOrArraySize = 1;
        resource_desc.MipLevels = 1;
        resource_desc.Format = DXGI_FORMAT_UNKNOWN;
        resource_desc.SampleDesc = {1, 0};
        resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

        BLAST_ASSERT(S_OK == device->CreateCommittedResource(
          &heap_properties,
          D3D12_HEAP_FLAG_NONE,
          &resource_desc,
          D3D12_RESOURCE_STATE_COPY_SOURCE,
          nullptr,
          IID_PPV_ARGS(&staging_buffer)));
      }
    }

    //for (auto& resource : resources)
    //{
    //  if (resource) { resource->Initialize(); }
    //}

    //for (auto& pass : passes)
    //{
    //  if (pass) { pass->Initialize(); }
    //}
  }


  void D12Device::Use()
  {
    //for (auto& frame : frames)
    //{
    //  frame->Use();
    //}

    //{

    //  //const auto scene_property = spark_broker->GetProperty();
    //  //const auto delta_property = scene_property->GetObjectItem("delta_time");
    //  //delta_property->SetItem<float>(static_cast<float>(delta));

    //  auto dst_resource = back_buffer_;

    //  spark_broker->Use();
    //  auto spark_target_resource = &dynamic_cast<Render3DBrokerBroker*>(spark_broker.get())->GetAsset()->AccessUnorderedAccesses()[0]->GetResource();
    //  //auto spark_target_resource = &dynamic_cast<Render3DBrokerBroker*>(spark_broker_.get())->GetOutput()->AccessRenderTargets()[0]->GetResource();
    //  auto src_resource = dynamic_cast<D3DResource*>(spark_target_resource)->GetResource();

    //  //const bool screenshot_enabled = false;
    //  //if (screenshot_enabled)
    //  //{
    //  //  const uint32_t step_count = 10;
    //  //  if (frame_count_ % step_count == 0)
    //  //  {
    //  //    spark_target_resource->Retrieve(0);
    //  //    auto the_io_broker = dynamic_cast<IOBroker*>(io_broker_.get());
    //  //    the_io_broker->SetImage(spark_target_resource->GetPropertyItem(0));
    //  //    the_io_broker->SetSizeX(screen_width_);
    //  //    the_io_broker->SetSizeY(screen_height_);

    //  //    char filepath[256];
    //  //    sprintf_s(filepath, "D:/raygene-out/%06d.png", frame_count_ / step_count);
    //  //    the_io_broker->SetFilepath(std::string(filepath));

    //  //    the_io_broker->Use();
    //  //  }
    //  //}
    //  //++frame_count_;


    //  //imgui_broker_->Use();
    //  //auto imgui_target_resource = &dynamic_cast<ImguiBroker*>(imgui_broker_.get())->GetOutput()->AccessRenderTargets()[0]->GetResource();
    //  //auto src_resource = dynamic_cast<D3DResource*>(imgui_target_resource)->GetResource();  

    //  //d3d_main_device->GetContext()->ResolveSubresource(dst_resource, 0, src_resource, 0, DXGI_FORMAT_R8G8B8A8_UNORM);
    //  this->GetContext()->CopyResource(dst_resource, src_resource);
    //}

    for (auto& pass : passes)
    {
      pass->Use();
    }

    if (screen && back_buffer)
    {
      auto screen_buffer = reinterpret_cast<D12Resource*>(screen.get())->GetResource();
      
      {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = screen_buffer;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
        command_list->ResourceBarrier(1, &barrier);
      }

      {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = back_buffer;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
        command_list->ResourceBarrier(1, &barrier);        
      }
      
      command_list->CopyResource(back_buffer, screen_buffer);

      {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = back_buffer;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
        command_list->ResourceBarrier(1, &barrier);
      } 
      
      {
        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
        barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
        barrier.Transition.pResource = screen_buffer;
        barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
        barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
        command_list->ResourceBarrier(1, &barrier);
      }
    }

    if (swapchain)
    {
      swapchain->Present(0, 0);
    }
  }


  void D12Device::Discard()
  {
    //for (auto& pass : passes)
    //{
    //  if (pass) { pass->Discard(); }
    //}

    //for (auto& resource : resources)
    //{
    //  if (resource) { resource->Discard(); }
    //}

    if (screen_buffer)
    {
      screen_buffer->Release();
      screen_buffer = nullptr;
    }

    if (staging_buffer)
    {
      staging_buffer->Release();
      staging_buffer = nullptr;
    }

    if (swapchain)
    {
      swapchain->Release();
      swapchain = nullptr;
    }

    if (command_list)
    {
      command_list->Release();
      command_list = nullptr;
    }

    if (command_allocator)
    {
      command_allocator->Release();
      command_allocator = nullptr;
    }

    if (command_queue)
    {
      command_queue->Release();
      command_queue = nullptr;
    }

    if (device)
    {
      device->Release();
      device = nullptr;
    }

    if (debug_controller)
    {
      debug_controller->Release();
      debug_controller = nullptr;
    }
  }


  //void D3DCore::Update(std::pair<void*, size_t> src, std::pair<std::shared_ptr<Resource>, uint32_t> dst)
  //{
  //  HRESULT hr = S_OK;

  //  D3DResource* d3d_resource = reinterpret_cast<D3DResource*>(dst.first.get());
  //  switch (d3d_resource->GetType())
  //  {
  //  case Resource::TYPE_BUFFER:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), dst.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(mapped_subresource.pData, src.first, src.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), dst.second);
  //    break;
  //  }
  //  case Resource::TYPE_TEXTURE1D:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), dst.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(mapped_subresource.pData, src.first, src.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), dst.second);
  //    break;
  //  }
  //  case Resource::TYPE_IMAGE2D:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), dst.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(mapped_subresource.pData, src.first, src.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), dst.second);
  //    break;
  //  }
  //  case Resource::TYPE_TEXTURE3D:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), dst.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(mapped_subresource.pData, src.first, src.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), dst.second);
  //    break;
  //  }
  //  }
  //}

  //void D3DCore::Update(std::pair<std::shared_ptr<Resource>, uint32_t> src, std::pair<void*, size_t> dst)
  //{
  //  HRESULT hr = S_OK;

  //  D3DResource* d3d_resource = reinterpret_cast<D3DResource*>(src.first.get());
  //  switch (d3d_resource->GetType())
  //  {
  //  case Resource::TYPE_BUFFER:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), src.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(dst.first, mapped_subresource.pData, dst.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), src.second);
  //    break;
  //  }
  //  case Resource::TYPE_TEXTURE1D:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), src.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(dst.first, mapped_subresource.pData, dst.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), src.second);
  //    break;
  //  }
  //  case Resource::TYPE_IMAGE2D:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), src.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(dst.first, mapped_subresource.pData, dst.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), src.second);
  //    break;
  //  }
  //  case Resource::TYPE_TEXTURE3D:
  //  {
  //    D3D11_MAPPED_SUBRESOURCE mapped_subresource{ 0 };
  //    hr = this->GetContext()->Map(d3d_resource->GetResource(), src.second, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subresource);
  //    BLAST_ASSERT(hr == S_OK);

  //    const uint32_t size = mapped_subresource.RowPitch;
  //    //BLAST_ASSERT(size == src.second);
  //    memcpy(dst.first, mapped_subresource.pData, dst.second);

  //    this->GetContext()->Unmap(d3d_resource->GetResource(), src.second);
  //    break;
  //  }
  //  }
  //}

  //void D3DCore::Copy(std::shared_ptr<Resource>& src, std::shared_ptr<Resource>& dst)
  //{
  //  D3DResource* src_d3d_resource = reinterpret_cast<D3DResource*>(src.get());
  //  D3DResource* dst_d3d_resource = reinterpret_cast<D3DResource*>(dst.get());
  //  this->GetContext()->CopyResource(dst_d3d_resource->GetResource(), src_d3d_resource->GetResource());
  //}

  //void D3DCore::Copy(std::pair<std::shared_ptr<Resource>, uint32_t> src, uint32_t offset_x, uint32_t offset_y, uint32_t offset_z,
  //  std::pair<std::shared_ptr<Resource>, uint32_t> dst, uint32_t size_x, uint32_t size_y, uint32_t size_z)
  //{
  //  D3DResource* src_d3d_resource = reinterpret_cast<D3DResource*>(src.first.get());
  //  D3DResource* dst_d3d_resource = reinterpret_cast<D3DResource*>(dst.first.get());

  //  if (size_x == 0 || size_y == 0 || size_z == 0)
  //  {
  //    this->GetContext()->CopySubresourceRegion(dst_d3d_resource->GetResource(), dst.second, offset_x, offset_y, offset_z,
  //      src_d3d_resource->GetResource(), src.second, nullptr);
  //  }
  //  else
  //  {
  //    D3D11_BOX d3d_box = { size_x , size_x , size_x };
  //    this->GetContext()->CopySubresourceRegion(dst_d3d_resource->GetResource(), dst.second, offset_x, offset_y, offset_z,
  //      src_d3d_resource->GetResource(), src.second, &d3d_box);
  //  }
  //}

  //void D3DCore::Run(std::shared_ptr<Asset>& asset, std::shared_ptr<Output>& output,
  //  uint32_t indices_count, uint32_t indices_start, uint32_t indices_shift,
  //  uint32_t vertices_count, uint32_t vertices_start, uint32_t instances_num)
  //{
  //  asset->Use();
  //  output->Use();
  //  this->GetContext()->DrawIndexedInstanced(indices_count, instances_num, indices_start, indices_shift, 0);
  //}

  //void D3DCore::Run(std::shared_ptr<Asset>& asset, uint32_t grid_x, uint32_t grid_y, uint32_t grid_z)
  //{
  //}

  D12Device::D12Device(const std::string& name)
    : Device(name)
  {
  }

  D12Device::~D12Device()
  {
    Discard();
  }
}