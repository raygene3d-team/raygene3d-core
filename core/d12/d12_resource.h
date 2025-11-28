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
#include "../resource.h"
#include "d12_view.h"

#include <dxgi.h>
#include <d3d12.h>

namespace RayGene3D
{
  constexpr size_t general_limit{ 256 };
  constexpr size_t rtv_limit{ 32 };
  constexpr size_t dsv_limit{ 32 };  

  class D12Resource : public Resource
  {
  protected:
    ID3D12Resource* resource{ nullptr };
    D3D12_GPU_VIRTUAL_ADDRESS address{ 0 };

  protected:
    ID3D12DescriptorHeap* general_heap{ nullptr };
    std::array<bool, general_limit> general_slots{};

    ID3D12DescriptorHeap* rtv_heap{ nullptr };
    std::array<bool, rtv_limit> rtv_slots{};

    ID3D12DescriptorHeap* dsv_heap{ nullptr };
    std::array<bool, dsv_limit> dsv_slots{};

  public:
    enum Heap
    {
      HEAP_GENERAL = 0,
      HEAP_RTV = 1,
      HEAP_DSV = 2,      
    };

  public:
    void Commit() override;
    void Retrieve() override;
    void Blit(const std::shared_ptr<Resource>& resource) override;

    void* Map() override;
    void Unmap() override;

  public:
    //void SetResource(ID3D12Resource* resource) { this->resource = resource; }
    ID3D12Resource* GetResource() { return resource; }

  protected:
    D3D12_CPU_DESCRIPTOR_HANDLE ObtainGeneral();
    D3D12_CPU_DESCRIPTOR_HANDLE ObtainRTV();
    D3D12_CPU_DESCRIPTOR_HANDLE ObtainDSV();

    void DropGeneral(D3D12_CPU_DESCRIPTOR_HANDLE handle);
    void DropRTV(D3D12_CPU_DESCRIPTOR_HANDLE handle);
    void DropDSV(D3D12_CPU_DESCRIPTOR_HANDLE handle);

  public:
    D3D12_CPU_DESCRIPTOR_HANDLE ObtainHandle(Heap heap = HEAP_GENERAL)
    {
      switch (heap)
      {
      case HEAP_GENERAL: return ObtainGeneral();
      case HEAP_RTV: return ObtainRTV();
      case HEAP_DSV: return ObtainDSV();
      };
      return D3D12_CPU_DESCRIPTOR_HANDLE{};
    }

    void DropHandle(D3D12_CPU_DESCRIPTOR_HANDLE handle, Heap heap = HEAP_GENERAL)
    {
      switch (heap)
      {
      case HEAP_GENERAL: return DropGeneral(handle);
      case HEAP_RTV: return DropRTV(handle);
      case HEAP_DSV: return DropDSV(handle);
      };
    }


  public:
    const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, 
      const Range& levels_or_length = Range{ 0u, size_t(-1) },
      const Range& layers_or_stride = Range{ 0u, size_t(-1) },
      View::Bind bind = View::BIND_UNKNOWN) override
    {
      return views.emplace_back(new D12View(name, *this, usage, levels_or_length, layers_or_stride, bind));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D12Resource(const std::string& name,
      Device& device,
      const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    D12Resource(const std::string& name,
      Device& device,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    D12Resource(const std::string& name,
      Device& device,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    D12Resource(const std::string& name,
      Device& device,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    virtual ~D12Resource();
  };
}
