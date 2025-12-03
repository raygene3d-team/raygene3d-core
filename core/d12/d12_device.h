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
#include "../device.h"
#include "d12_resource.h"
#include "d12_pass.h"

#include <dxgi.h>
#include <d3d12.h>

namespace RayGene3D
{
  class D12Device : public Device
  {
  protected:
    static constexpr size_t general_limit{ 1024 };
    static constexpr size_t sampler_limit{ 1024 };

  protected:
    ID3D12Debug* debug_controller{ nullptr };
    ID3D12Device* device{ nullptr };
    ID3D12CommandQueue* command_queue{ nullptr };
    ID3D12CommandAllocator* command_allocator{ nullptr };
    ID3D12GraphicsCommandList* command_list{ nullptr };
    IDXGISwapChain* swapchain{ nullptr };
    ID3D12Resource* back_buffer{ nullptr };
    ID3D12Resource* screen_buffer{ nullptr };
    ID3D12Resource* staging_buffer{ nullptr };

  protected:
    ID3D12DescriptorHeap* general_heap{ nullptr };
    ID3D12DescriptorHeap* sampler_heap{ nullptr };

  protected:
    size_t fence_value{ 0 };
    ID3D12Fence* fence{ nullptr };
    HANDLE fence_event{ nullptr };

  protected:
    uint32_t general_size{ 0 };
    uint32_t sampler_size{ 0 };
    uint32_t rtv_size{ 0 };
    uint32_t dsv_size{ 0 };

  protected:
    size_t staging_size{ 64 * 1024 * 1024 };

  public:
    void Update(std::pair<void*, size_t> src, std::pair<std::shared_ptr<Resource>, uint32_t> dst);
    void Update(std::pair<std::shared_ptr<Resource>, uint32_t> src, std::pair<void*, size_t> dst);
    void Copy(std::shared_ptr<Resource>& src, std::shared_ptr<Resource>& dst);
    void Copy(std::pair<std::shared_ptr<Resource>, uint32_t> src, uint32_t offset_x, uint32_t offset_y, uint32_t offset_z,
      std::pair<std::shared_ptr<Resource>, uint32_t> dst, uint32_t size_x, uint32_t size_y, uint32_t size_z);

  public:
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new D12Resource(name, *this, desc, hint, interop));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new D12Resource(name, *this, desc, hint, interop));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new D12Resource(name, *this, desc, hint, interop));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new D12Resource(name, *this, desc, hint, interop));
    }

    const std::shared_ptr<Pass>& CreatePass(const std::string& name,
      Pass::Type type,
      uint32_t size_x,
      uint32_t size_y,
      size_t layers,
      const std::pair<const Pass::RTAttachment*, size_t>& rt_attachments,
      const std::pair<const Pass::DSAttachment*, size_t>& ds_attachments) override
    {
      return passes.emplace_back(new D12Pass(name, *this, type, 
        size_x, size_y, layers, rt_attachments, ds_attachments));
    }

  public:
    ID3D12Device* GetDevice() const { return device; }
    ID3D12CommandQueue* GetCommandQueue() const { return command_queue; }
    ID3D12CommandAllocator* GetCommandAllocator() const { return command_allocator; }
    ID3D12GraphicsCommandList* GetCommandList() const { return command_list; }

  public:
    size_t GetStagingSize() const { return staging_size; }
    ID3D12Resource* GetStagingBuffer() const { return staging_buffer; }

  public:
    ID3D12DescriptorHeap* GetGeneralHeap() const { return general_heap; }
    ID3D12DescriptorHeap* GetSamplerHeap() const { return sampler_heap; }

  public:
    uint32_t GetGeneralSize() const { return general_size; }
    uint32_t GetSamplerSize() const { return sampler_size; }
    uint32_t GetRTVSize() const { return rtv_size; }
    uint32_t GetDSVSize() const { return dsv_size; }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D12Device(const std::string& name);
    virtual ~D12Device();
  };
}