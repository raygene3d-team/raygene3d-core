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


#include "mtl_view.h"
#include "mtl_resource.h"
#include "mtl_device.h"

#import <Metal/Metal.h>

namespace RayGene3D
{
  void MTLView::Initialize()
  {
    auto* resource = reinterpret_cast<MTLResource*>(&this->GetResource());
    id native = resource->GetNative();
    if (!native) return;

    // Buffers are bound directly (the view only carries an offset), so the base object is used.
    if (resource->GetType() == Resource::TYPE_BUFFER)
    {
      view = native;
      return;
    }

    // Image views mirror VLKView: texture type, format, mip range and slice range come from the view.
    // Metal requires a texture view to reinterpret the base texture (e.g. a cube as a 2D array for UAV).
    id<MTLTexture> base = native;

    const auto get_type = [&]() -> MTLTextureType
    {
      if (resource->GetType() == Resource::TYPE_TEX3D) return MTLTextureType3D;
      if (resource->GetType() == Resource::TYPE_TEX1D)
      {
        return resource->GetHint() & Resource::HINT_LAYERED_IMAGE ? MTLTextureType1DArray : MTLTextureType1D;
      }
      if (usage & USAGE_SHADER_RESOURCE)
      {
        if (bind == BIND_CUBEMAP_LAYER) return MTLTextureTypeCube;
        if (bind == BIND_CUBEMAP_ARRAY) return MTLTextureTypeCubeArray;
      }
      return resource->GetHint() & Resource::HINT_LAYERED_IMAGE ? MTLTextureType2DArray : MTLTextureType2D;
    };

    const auto levels = NSRange{
      levels_or_length.offset,
      levels_or_length.length == size_t(-1) ? base.mipmapLevelCount - levels_or_length.offset : levels_or_length.length };

    const auto slices = NSRange{
      layers_or_stride.offset,
      layers_or_stride.length == size_t(-1) ? base.arrayLength - layers_or_stride.offset : layers_or_stride.length };

    const auto type = get_type();
    const bool same_as_base = base.textureType == type
      && levels.location == 0 && levels.length == base.mipmapLevelCount
      && slices.location == 0 && slices.length == base.arrayLength;

    if (same_as_base)
    {
      view = native;
    }
    else
    {
      id<MTLTexture> texture_view = [base newTextureViewWithPixelFormat:base.pixelFormat
                                                            textureType:type
                                                                 levels:levels
                                                                 slices:slices];
      BLAST_ASSERT(texture_view != nil);
      // newTextureViewWithPixelFormat returns +1; the member takes ownership.
      owned_view = texture_view;
      view = owned_view;
    }

    BLAST_LOG("Creating image view: [%s]", name.c_str());
  }

  void MTLView::Use()
  {
  }

  void MTLView::Discard()
  {
    // Only texture views created in Initialize() are owned here; the base resource is released by MTLResource.
    [owned_view release];
    owned_view = nil;
    view = nil;
  }

  MTLView::MTLView(const std::string& name,
    Resource& resource,
    Usage usage,
    const Range& levels_or_length,
    const Range& layers_or_stride,
    View::Bind bind)
    : View(name, resource, usage, levels_or_length, layers_or_stride, bind)
  {
    MTLView::Initialize();
  }

  MTLView::~MTLView()
  {
    MTLView::Discard();
  }
}
