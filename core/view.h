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
#include "base.h"

namespace RayGene3D
{
  class Resource;

  class View : public Usable
  {
  public:

    enum Bind
    {
      BIND_UNKNOWN = 0,
      BIND_SHADER_RESOURCE = 0x1L,
      BIND_RENDER_TARGET = 0x2L,
      BIND_DEPTH_STENCIL = 0x4L,
      BIND_UNORDERED_ACCESS = 0x8L,
      BIND_VERTEX_ARRAY = 0x10L,
      BIND_INDEX_ARRAY = 0x20L,
      BIND_CONSTANT_DATA = 0x40L,
      BIND_COMMAND_INDIRECT = 0x80L, //BIND_ARGUMENT_ARRAY =  = 0x80L,
      BIND_FORCE_UINT = 0xffffffff
    };

    enum Usage
    {
      USAGE_UNKNOWN = 0x0L,
      USAGE_DEPTH_ONLY = 0x1L,
      USAGE_STENCIL_ONLY = 0x2L,
      USAGE_CUBEMAP_LAYER = 0x4L,
      USAGE_CUBEMAP_ARRAY = 0x8L,
      USAGE_FORCE_UINT = 0xffffffff
    };
   
  protected:
    Resource& resource;

  protected:
    Bind bind{ BIND_UNKNOWN };
    Usage usage{ USAGE_UNKNOWN };

  protected:
    Format format{ FORMAT_UNKNOWN };

  protected:
    uint32_t byte_offset{ 0 };
    uint32_t byte_count{ uint32_t(-1) };

  protected:
    uint32_t layer_offset{ 0 };
    uint32_t layer_count{ uint32_t(-1) };
    uint32_t mipmap_offset{ 0 };
    uint32_t mipmap_count{ uint32_t(-1) };

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
    void SetByteOffset(uint32_t byte_offset) { this->byte_offset = byte_offset; }
    uint32_t GetByteOffset() const { return byte_offset; }
    void SetByteCount(uint32_t byte_count) { this->byte_count = byte_count; }
    uint32_t GetByteCount() const { return byte_count; }

  public:
    void SetLayerOffset(uint32_t layer_offset) { this->layer_offset = layer_offset; }
    uint32_t GetLayerOffset() const { return layer_offset; }
    void SetLayerCount(uint32_t layer_count) { this->layer_count = layer_count; }
    uint32_t GetLayerCount() const { return layer_count; }
    void SetMipmapOffset(uint32_t mipmap_offset) { this->mipmap_offset = mipmap_offset; }
    uint32_t GetMipmapOffset() const { return mipmap_offset; }
    void SetMipmapCount(uint32_t mipmap_count) { this->mipmap_count = mipmap_count; }
    uint32_t GetMipmapCount() const { return mipmap_count; }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    View(const std::string& name, Resource& resource);
    virtual ~View();
  };
}