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
#include "d11_view.h"

#include <dxgi.h>
#include <d3d11_1.h>

namespace RayGene3D
{
  class D11Resource : public Resource
  {
  protected:
    ID3D11Resource* resource{ nullptr };

    union Info
    {
      D3D11_BUFFER_DESC buffer_desc;
      D3D11_TEXTURE1D_DESC tex1d_desc;
      D3D11_TEXTURE2D_DESC tex2d_desc;
      D3D11_TEXTURE3D_DESC tex3d_desc;
    } info;

  public:
    void Commit(uint32_t index) override;
    void Retrieve(uint32_t index) override;
    void Blit(const std::shared_ptr<Resource>& resource) override;

    void* Map() override;
    void Unmap() override;

  public:
    void SetResource(ID3D11Resource* resource) { this->resource = resource; }
    ID3D11Resource* GetResource() { return resource; }
    ID3D11Buffer* GetBuffer() const { return reinterpret_cast<ID3D11Buffer*>(resource); }
    ID3D11Texture1D* GetTexture1D() const { return reinterpret_cast<ID3D11Texture1D*>(resource); }
    ID3D11Texture2D* GetTexture2D() const { return reinterpret_cast<ID3D11Texture2D*>(resource); }
    ID3D11Texture3D* GetTexture3D() const { return reinterpret_cast<ID3D11Texture3D*>(resource); }

  public:
    const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, const View::Range& bytes = View::Range{ 0, uint32_t(-1) }) override
    {
      return views.emplace_back(new D11View(name, *this, usage, bytes));
    }
    const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, View::Bind bind, 
      const View::Range& mipmaps = View::Range{ 0, uint32_t(-1) },
      const View::Range& layers = View::Range{ 0, uint32_t(-1) }) override
    {
      return views.emplace_back(new D11View(name, *this, usage, bind, mipmaps, layers));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D11Resource(const std::string& name,
      Device& device,
      const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    D11Resource(const std::string& name,
      Device& device,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    D11Resource(const std::string& name,
      Device& device,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    D11Resource(const std::string& name,
      Device& device,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    virtual ~D11Resource();
  };
}
