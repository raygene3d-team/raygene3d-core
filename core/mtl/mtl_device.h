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
#include "mtl_resource.h"
#include "mtl_pass.h"

namespace RayGene3D
{
  class MTLDevice : public Device
  {
  protected:
    void* device{ nullptr };         // id<MTLDevice>
    void* queue{ nullptr };          // id<MTLCommandQueue>
    void* command_buffer{ nullptr }; // id<MTLCommandBuffer> for the current frame
    void* layer{ nullptr };          // CAMetalLayer* taken from window handle

  public:
    void* GetDevice() const { return device; }
    void* GetQueue() const { return queue; }
    void* GetCommandBuffer() const { return command_buffer; }
    void* GetLayer() const { return layer; }

  public:
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new MTLResource(name, *this, desc, hint, interop));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new MTLResource(name, *this, desc, hint, interop));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new MTLResource(name, *this, desc, hint, interop));
    }
    const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {}) override
    {
      return resources.emplace_back(new MTLResource(name, *this, desc, hint, interop));
    }

    const std::shared_ptr<Pass>& CreatePass(const std::string& name,
      Pass::Type type,
      uint32_t size_x,
      uint32_t size_y,
      size_t layers,
      const std::pair<const Pass::RTAttachment*, size_t>& rt_attachments,
      const std::pair<const Pass::DSAttachment*, size_t>& ds_attachments) override
    {
      return passes.emplace_back(new MTLPass(name, *this, type,
        size_x, size_y, layers, rt_attachments, ds_attachments));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    MTLDevice(const std::string& name);
    virtual ~MTLDevice();
  };
}
