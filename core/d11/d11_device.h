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
#include "d11_resource.h"
#include "d11_pass.h"

#include <dxgi.h>
#include <d3d11_1.h>

namespace RayGene3D
{
  class D11Device : public Device
  {
  protected:
    ID3D11Device* device{ nullptr };
    ID3D11DeviceContext* context{ nullptr };
    IDXGISwapChain* swapchain{ nullptr };
    ID3D11Texture2D* backbuffer{ nullptr };

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
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new D11Resource(name, *this, desc, hint, interops));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new D11Resource(name, *this, desc, hint, interops));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new D11Resource(name, *this, desc, hint, interops));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) override
    {
      return resources.emplace_back(new D11Resource(name, *this, desc, hint, interops));
    }

    const std::shared_ptr<Pass>& CreatePass(const std::string& name,
      Pass::Type type,
      uint32_t size_x,
      uint32_t size_y,
      uint32_t layers,
      const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments,
      const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments) override
    {
      return passes.emplace_back(new D11Pass(name, *this, type, 
        size_x, size_y, layers, rt_attachments, ds_attachments));
    }

  public:
    ID3D11Device* GetDevice() const { return device; }
    ID3D11DeviceContext* GetContext() const { return context; }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D11Device(const std::string& name);
    virtual ~D11Device();
  };
}