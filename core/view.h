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
#include "../../raygene3d-root/base.h"

namespace RayGene3D
{
  class Resource;

  class View : public Usable
  {
  public:

    enum Bind
    {
      BIND_UNKNOWN = 0x0L,
      BIND_DEPTH_ONLY = 0x1L,
      BIND_STENCIL_ONLY = 0x2L,
      BIND_CUBEMAP_LAYER = 0x4L,
      BIND_CUBEMAP_ARRAY = 0x8L,
      BIND_FORCE_UINT = 0xffffffff
    };

  public:
    struct Range
    {
      uint32_t offset{ 0 };
      uint32_t count{ uint32_t(-1) };
    };
   
  protected:
    Resource& resource;

  protected:
    Usage usage{ USAGE_UNKNOWN };

  //protected:
  //  Format format{ FORMAT_UNKNOWN };

  protected:
    Bind bind{ BIND_UNKNOWN };

  protected:
    Range byte_range;
    Range layer_range;
    Range mipmap_range;

  public:
    Resource& GetResource() { return resource; }
    
  public:
    Bind GetBind() const { return bind; }
    void SetBind(Bind bind) { this->bind = bind; }
    Usage GetUsage() const { return usage; }
    void SetUsage(Usage usage) { this->usage = usage; }

  public:
    //void SetFormat(Format format) { this->format = format; }
    //Format GetFormat() const { return format; }

  public:
    void SetByteOffset(uint32_t byte_offset) { this->byte_range.offset = byte_offset; }
    uint32_t GetByteOffset() const { return byte_range.offset; }
    void SetByteCount(uint32_t byte_count) { this->byte_range.count = byte_count; }
    uint32_t GetByteCount() const { return byte_range.count; }

  public:
    void SetLayerOffset(uint32_t layer_offset) { this->layer_range.offset = layer_offset; }
    uint32_t GetLayerOffset() const { return layer_range.offset; }
    void SetLayerCount(uint32_t layer_count) { this->layer_range.count = layer_count; }
    uint32_t GetLayerCount() const { return layer_range.count; }
    void SetMipmapOffset(uint32_t mipmap_offset) { this->mipmap_range.offset = mipmap_offset; }
    uint32_t GetMipmapOffset() const { return mipmap_range.offset; }
    void SetMipmapCount(uint32_t mipmap_count) { this->mipmap_range.count = mipmap_count; }
    uint32_t GetMipmapCount() const { return mipmap_range.count; }

  public:
    void SetByteRange(const Range& range) { byte_range = range; }
    const Range& GetByteRange() const { return byte_range; }

  public:
    void SetLayerRange(const Range& range) { layer_range = range; }
    const Range& GetLayerRange() const { return layer_range; }
    void SetMipmapRange(const Range& range) { mipmap_range = range; }
    const Range& GetMipmapRange() const { return mipmap_range; }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    View(const std::string& name, Resource& resource);
    virtual ~View();
  };
}