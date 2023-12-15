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
#include "../../raygene3d-wrap/base.h"

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
      uint32_t length{ uint32_t(-1) };
    };
   
  protected:
    Resource& resource;

  protected:
    Usage usage{ USAGE_UNKNOWN };

  protected:
    Bind bind{ BIND_UNKNOWN };

  protected:
    Range stride;
    Range count;

  public:
    Resource& GetResource() { return resource; }
    
  public:
    Usage GetUsage() const { return usage; }
    Bind GetBind() const { return bind; }

  public:
    const Range& GetStride() const { return stride; }
    const Range& GetCount() const { return count; }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    View(const std::string& name,
      Resource& resource,
      Usage usage,
      const View::Range& bytes = Range{0, uint32_t(-1)});
    View(const std::string& name,
      Resource& resource,
      Usage usage,
      View::Bind bind,
      const View::Range& mipmaps = Range{ 0, uint32_t(-1) },
      const View::Range& layers = Range{ 0, uint32_t(-1) });
    virtual ~View();
  };

  typedef std::shared_ptr<View> SPtrView;
  typedef std::weak_ptr<View> WPtrView;
  typedef std::unique_ptr<View> UPtrView;
}