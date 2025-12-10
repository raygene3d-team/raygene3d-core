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
  public:
    enum Heap
    {
      HEAP_SAMPLER = 0,
      HEAP_GENERAL = 1,
      HEAP_RTV = 2,
      HEAP_DSV = 3,
    };

    union Handle
    {
      D3D12_CPU_DESCRIPTOR_HANDLE cpu;
      D3D12_GPU_DESCRIPTOR_HANDLE gpu;
    } handle;

  protected:
    static constexpr size_t sampler_limit{ 4 * 1024 };
    static constexpr size_t general_limit{ 16 * 1024 };
    static constexpr size_t rt_limit{ 128 * 1024 };
    static constexpr size_t ds_limit{ 128 * 1024 };


  protected:
    ID3D12Debug* debug_controller{ nullptr };
    ID3D12Device* device{ nullptr };
    ID3D12CommandQueue* command_queue{ nullptr };
    ID3D12CommandAllocator* command_allocator{ nullptr };
    ID3D12GraphicsCommandList* command_list{ nullptr };

    IDXGISwapChain* swapchain{ nullptr };
    std::vector<ID3D12Resource*> back_buffers;
    //ID3D12CommandQueue* present_command_queue{ nullptr };
    //ID3D12CommandAllocator* present_command_allocator{ nullptr };
    //ID3D12GraphicsCommandList* present_command_list{ nullptr };    

    ID3D12Resource* screen_buffer{ nullptr };
    ID3D12Resource* staging_buffer{ nullptr };

    uint32_t current_index = 0;

  protected:
    ID3D12DescriptorHeap* sampler_heap{ nullptr };
    std::array<bool, sampler_limit> sampler_slots{};

    ID3D12DescriptorHeap* general_heap{ nullptr };
    std::array<bool, general_limit> general_slots{};

    ID3D12DescriptorHeap* rt_heap{ nullptr };
    std::array<bool, rt_limit> rt_slots{};

    ID3D12DescriptorHeap* ds_heap{ nullptr };
    std::array<bool, ds_limit> ds_slots{};

  protected:
    size_t fence_value{ 0 };
    ID3D12Fence* fence{ nullptr };
    HANDLE fence_event{ nullptr };

  protected:
    uint32_t sampler_size{ 0 };
    uint32_t general_size{ 0 };
    uint32_t rt_size{ 0 };
    uint32_t ds_size{ 0 };

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
    ID3D12DescriptorHeap* GetSamplerHeap() const { return sampler_heap; }
    ID3D12DescriptorHeap* GetGeneralHeap() const { return general_heap; }
    ID3D12DescriptorHeap* GetRTHeap() const { return rt_heap; }
    ID3D12DescriptorHeap* GetDSHeap() const { return ds_heap; }

    Handle GetSamplerHandle(uint32_t slot, bool gpu = false) const { return { slot * sampler_size + 
      (gpu ? sampler_heap->GetGPUDescriptorHandleForHeapStart().ptr : sampler_heap->GetCPUDescriptorHandleForHeapStart().ptr) };
    }
    Handle GetGeneralHandle(uint32_t slot, bool gpu = false) const { return { slot * general_size + 
      (gpu ? general_heap->GetGPUDescriptorHandleForHeapStart().ptr : general_heap->GetCPUDescriptorHandleForHeapStart().ptr) };
    }
    Handle GetRTHandle(uint32_t slot, bool gpu = false) const { return { slot * rt_size + 
      (gpu ? rt_heap->GetGPUDescriptorHandleForHeapStart().ptr : rt_heap->GetCPUDescriptorHandleForHeapStart().ptr) };
    }
    Handle GetDSHandle(uint32_t slot, bool gpu = false) const { return { slot * ds_size + 
      (gpu ? ds_heap->GetGPUDescriptorHandleForHeapStart().ptr : ds_heap->GetCPUDescriptorHandleForHeapStart().ptr) };
    }

    uint32_t ObtainSamplerSlot() {
      const auto slot = std::distance(sampler_slots.cbegin(), std::find(sampler_slots.cbegin(), sampler_slots.cend(), false));
      if (slot == sampler_limit) return -1; sampler_slots[slot] = true; return slot;
    }
    uint32_t ObtainGeneralSlot() {
      const auto slot = std::distance(general_slots.cbegin(), std::find(general_slots.cbegin(), general_slots.cend(), false));
      if (slot == general_limit) return -1; general_slots[slot] = true; return slot;
    }
    uint32_t ObtainRTSlot() {
      const auto slot = std::distance(rt_slots.cbegin(), std::find(rt_slots.cbegin(), rt_slots.cend(), false));
      if (slot == rt_limit) return -1; rt_slots[slot] = true; return slot;
    }
    uint32_t ObtainDSSlot() {
      const auto slot = std::distance(ds_slots.cbegin(), std::find(ds_slots.cbegin(), ds_slots.cend(), false));
      if (slot == ds_limit) return -1; ds_slots[slot] = true; return slot;
    }

    void DropSamplerSlot(uint32_t slot) { 
      if (slot == -1) return; sampler_slots[slot] = false;
    }
    void DropGeneralSlot(uint32_t slot) { 
      if (slot == -1) return; general_slots[slot] = false;
    }
    void DropRTSlot(uint32_t slot) {
      if (slot == -1) return; rt_slots[slot] = false;
    }
    void DropDSSlot(uint32_t slot) {
      if (slot == -1) return; ds_slots[slot] = false;
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D12Device(const std::string& name);
    virtual ~D12Device();
  };
}