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
#include "d11_device.h"
#include "d11_view.h"
#include "d11_resource.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

namespace RayGene3D
{
  void D11Device::Initialize()
  {
    if (device)
    {
      BLAST_LOG("Initializing existing Device: %s", name.c_str());
      return;
    }

    IDXGIFactory* factory = nullptr;
    BLAST_ASSERT(S_OK == CreateDXGIFactory(__uuidof(IDXGIFactory), (void**)(&factory)));

    IDXGIAdapter* adapter = nullptr;
    BLAST_ASSERT(S_OK == factory->EnumAdapters(ordinal, &adapter));

    DXGI_ADAPTER_DESC adapter_desc;
    adapter->GetDesc(&adapter_desc);

    char adapter_name[256];
    size_t adapter_size = 0;
    wcstombs_s(&adapter_size, adapter_name, adapter_desc.Description, 256);
    name = std::string(adapter_name) + " (D3D11 API)";

    const uint32_t device_flags = debug ? D3D11_CREATE_DEVICE_DEBUG | D3D11_CREATE_DEVICE_SINGLETHREADED : 0;
    const D3D_FEATURE_LEVEL feature_level = D3D_FEATURE_LEVEL_11_1;
    const D3D_DRIVER_TYPE driver_type = D3D_DRIVER_TYPE_UNKNOWN;

    BLAST_ASSERT(S_OK == D3D11CreateDevice(adapter, driver_type, nullptr, device_flags, &feature_level, 1, D3D11_SDK_VERSION, &device, nullptr, &context));

    ID3D11Device1* device1 = nullptr;
    ID3D11DeviceContext1* context1 = nullptr;
    BLAST_ASSERT(S_OK == device->QueryInterface(__uuidof(ID3D11Device1), reinterpret_cast<void**>(&device1)));
    BLAST_ASSERT(S_OK == context->QueryInterface(__uuidof(ID3D11DeviceContext1), reinterpret_cast<void**>(&context1)));
    context->Release();
    context = context1;
    device->Release();
    device = device1;


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
      swapchain_desc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT | DXGI_USAGE_UNORDERED_ACCESS;
      swapchain_desc.BufferCount = 3;
      swapchain_desc.OutputWindow = reinterpret_cast<HWND>(window);
      swapchain_desc.Windowed = true;
      swapchain_desc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
      swapchain_desc.Flags = 0;

      IDXGIDevice* dxgi_device = nullptr;
      BLAST_ASSERT(S_OK == device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgi_device));

      IDXGIAdapter* dxgi_adapter = nullptr;
      BLAST_ASSERT(S_OK == dxgi_device->GetParent(__uuidof(IDXGIAdapter), (void**)&dxgi_adapter));

      IDXGIFactory* dxgi_factory = nullptr;
      BLAST_ASSERT(S_OK == dxgi_adapter->GetParent(__uuidof(IDXGIFactory), (void**)&dxgi_factory));

      BLAST_ASSERT(S_OK == dxgi_factory->CreateSwapChain(device, &swapchain_desc, &swapchain));

      BLAST_ASSERT(S_OK == swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backbuffer));
    }

    for (auto& resource : resources)
    {
      if (resource) { resource->Initialize(); }
    }

    for (auto& layout : layouts)
    {
      if (layout) { layout->Initialize(); }
    }

    for (auto& config : configs)
    {
      if (config) { config->Initialize(); }
    }

    for (auto& pass : passes)
    {
      if (pass) { pass->Initialize(); }
    }
  }


  void D11Device::Use()
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
    //  auto spark_target_resource = &dynamic_cast<SparkBroker*>(spark_broker.get())->GetAsset()->AccessUnorderedAccesses()[0]->GetResource();
    //  //auto spark_target_resource = &dynamic_cast<SparkBroker*>(spark_broker_.get())->GetOutput()->AccessRenderTargets()[0]->GetResource();
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

    if (screen && backbuffer)
    {
      context->CopyResource(backbuffer, reinterpret_cast<D11Resource*>(screen.get())->GetResource());
    }

    if (swapchain)
    {
      swapchain->Present(0, 0);
    }
  }


  void D11Device::Discard()
  {
    for (auto& pass : passes)
    {
      if (pass) { pass->Discard(); }
    }

    for (auto& config : configs)
    {
      if (config) { config->Discard(); }
    }

    for (auto& layout : layouts)
    {
      if (layout) { layout->Discard(); }
    }

    for (auto& resource : resources)
    {
      if (resource) { resource->Discard(); }
    }

    if (backbuffer)
    {
      backbuffer->Release();
      backbuffer = nullptr;
    }

    if (swapchain)
    {
      swapchain->Release();
      swapchain = nullptr;
    }

    if (context)
    {
      context->Release();
      context = nullptr;
    }

    if (device)
    {
      device->Release();
      device = nullptr;
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

  D11Device::D11Device(const std::string& name) 
    : Device(name)
  {
  }

  D11Device::~D11Device()
  {
    Discard();
  }
}