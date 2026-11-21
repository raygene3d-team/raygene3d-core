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
#ifndef __OBJC__
#error "mtl_resource.h must be included only from Objective-C++ (.mm) files"
#endif
#include "../resource.h"
#import <Metal/Metal.h>
#include "mtl_view.h"

namespace RayGene3D
{
  class MTLResource : public Resource
  {
  protected:
    id<MTLBuffer> buffer{ nil };    // owned, set for TYPE_BUFFER
    id<MTLTexture> texture{ nil };  // owned, set for TYPE_TEX1D/2D/3D

  public:
    id GetNative() const
    {
      if (buffer) return buffer;
      return texture;
    }

  public:
    const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage,
      const Range& levels_or_length = Range{ 0u, size_t(-1) },
      const Range& layers_or_stride = Range{ 0u, size_t(-1) },
      View::Bind bind = View::BIND_UNKNOWN) override
    {
      return views.emplace_back(new MTLView(name, *this, usage, levels_or_length, layers_or_stride, bind));
    }

  public:
    void Commit() override;
    void Retrieve() override;
    void Blit(const std::shared_ptr<Resource>& resource) override;

  public:
    void* Map() override;
    void Unmap() override;

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    MTLResource(const std::string& name,
      Device& device,
      const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    MTLResource(const std::string& name,
      Device& device,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    MTLResource(const std::string& name,
      Device& device,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    MTLResource(const std::string& name,
      Device& device,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      std::pair<const uint8_t*, size_t> interop = {});
    virtual ~MTLResource();
    MTLResource(const MTLResource&) = delete;
    MTLResource& operator=(const MTLResource&) = delete;
  };
}
