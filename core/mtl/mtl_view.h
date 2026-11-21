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
#error "mtl_view.h must be included only from Objective-C++ (.mm) files"
#endif
#include "../view.h"
#import <Metal/Metal.h>

namespace RayGene3D
{
  class MTLView : public View
  {
  protected:
    id view{ nil };                    // id<MTLTexture> or id<MTLBuffer>, borrowed from the base resource or owned_view
    id<MTLTexture> owned_view{ nil };  // texture view created by this object (nil when view is the base object)

  public:
    id GetView() const { return view; }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    MTLView(const std::string& name,
      Resource& resource,
      Usage usage,
      const Range& levels_or_length = Range{ 0u, size_t(-1) },
      const Range& layers_or_stride = Range{ 0u, size_t(-1) },
      View::Bind bind = View::BIND_UNKNOWN);
    virtual ~MTLView();
    MTLView(const MTLView&) = delete;
    MTLView& operator=(const MTLView&) = delete;
  };
}
