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


#include "mtl_resource.h"
#include "mtl_device.h"

#include <cstdlib>
#include <algorithm>
#include <cstring>
#import <Metal/Metal.h>

namespace RayGene3D
{
  static MTLResourceOptions get_buffer_options(Resource::Hint hint)
  {
    // Dynamic buffers are CPU-writable through contents; static buffers are GPU-private-leaning managed storage.
    return (hint & Resource::HINT_DYNAMIC_BUFFER)
      ? MTLResourceStorageModeShared | MTLResourceCPUCacheModeDefaultCache
      : MTLResourceStorageModeManaged;
  }

  static MTLPixelFormat get_pixel_format(Format format)
  {
    switch (format)
    {
    default: return MTLPixelFormatInvalid;
    case FORMAT_R32G32B32A32_FLOAT: return MTLPixelFormatRGBA32Float;
    case FORMAT_R32G32B32A32_UINT: return MTLPixelFormatRGBA32Uint;
    case FORMAT_R32G32B32A32_SINT: return MTLPixelFormatRGBA32Sint;
    case FORMAT_R16G16B16A16_FLOAT: return MTLPixelFormatRGBA16Float;
    case FORMAT_R16G16B16A16_UNORM: return MTLPixelFormatRGBA16Unorm;
    case FORMAT_R16G16B16A16_UINT: return MTLPixelFormatRGBA16Uint;
    case FORMAT_R16G16B16A16_SNORM: return MTLPixelFormatRGBA16Snorm;
    case FORMAT_R16G16B16A16_SINT: return MTLPixelFormatRGBA16Sint;
    case FORMAT_R32G32_FLOAT: return MTLPixelFormatRG32Float;
    case FORMAT_R32G32_UINT: return MTLPixelFormatRG32Uint;
    case FORMAT_R32G32_SINT: return MTLPixelFormatRG32Sint;
    case FORMAT_D32_FLOAT_S8X24_UINT: return MTLPixelFormatDepth32Float_Stencil8;
    case FORMAT_D24_UNORM_S8_UINT: return MTLPixelFormatDepth24Unorm_Stencil8;
    case FORMAT_R10G10B10A2_UNORM: return MTLPixelFormatRGB10A2Unorm;
    case FORMAT_R10G10B10A2_UINT: return MTLPixelFormatRGB10A2Uint;
    case FORMAT_R11G11B10_FLOAT: return MTLPixelFormatRG11B10Float;
    case FORMAT_R8G8B8A8_UNORM: return MTLPixelFormatRGBA8Unorm;
    case FORMAT_R8G8B8A8_SRGB: return MTLPixelFormatRGBA8Unorm_sRGB;
    case FORMAT_R8G8B8A8_UINT: return MTLPixelFormatRGBA8Uint;
    case FORMAT_R8G8B8A8_SNORM: return MTLPixelFormatRGBA8Snorm;
    case FORMAT_R8G8B8A8_SINT: return MTLPixelFormatRGBA8Sint;
    case FORMAT_B8G8R8A8_UNORM: return MTLPixelFormatBGRA8Unorm;
    case FORMAT_R16G16_FLOAT: return MTLPixelFormatRG16Float;
    case FORMAT_R16G16_UNORM: return MTLPixelFormatRG16Unorm;
    case FORMAT_R16G16_UINT: return MTLPixelFormatRG16Uint;
    case FORMAT_R16G16_SNORM: return MTLPixelFormatRG16Snorm;
    case FORMAT_R16G16_SINT: return MTLPixelFormatRG16Sint;
    case FORMAT_R32_FLOAT: return MTLPixelFormatR32Float;
    case FORMAT_R32_UINT: return MTLPixelFormatR32Uint;
    case FORMAT_R32_SINT: return MTLPixelFormatR32Sint;
    case FORMAT_D32_FLOAT: return MTLPixelFormatDepth32Float;
    case FORMAT_D16_UNORM: return MTLPixelFormatDepth16Unorm;
    case FORMAT_R16_FLOAT: return MTLPixelFormatR16Float;
    case FORMAT_R16_UNORM: return MTLPixelFormatR16Unorm;
    case FORMAT_R16_UINT: return MTLPixelFormatR16Uint;
    case FORMAT_R16_SNORM: return MTLPixelFormatR16Snorm;
    case FORMAT_R16_SINT: return MTLPixelFormatR16Sint;
    case FORMAT_R8_UNORM: return MTLPixelFormatR8Unorm;
    case FORMAT_R8_UINT: return MTLPixelFormatR8Uint;
    case FORMAT_R8_SNORM: return MTLPixelFormatR8Snorm;
    case FORMAT_R8_SINT: return MTLPixelFormatR8Sint;
    }
  }

  static MTLTextureUsage get_texture_usage(Usage usage)
  {
    // The base texture must allow every use of any view created from it (Metal rejects a view whose
    // usage is not a subset of the base texture's usage), so this is the union of all view usages.
    (void)usage;
    return MTLTextureUsageShaderRead | MTLTextureUsageShaderWrite | MTLTextureUsageRenderTarget | MTLTextureUsagePixelFormatView;
  }

  void MTLResource::Initialize()
  {
    auto device = reinterpret_cast<MTLDevice*>(&this->GetDevice());
    auto mtl_device = (__bridge id<MTLDevice>)device->GetDevice();

    switch (type)
    {
    case TYPE_BUFFER:
    {
      const auto size = levels_or_length * layers_or_stride;
      id<MTLBuffer> buffer = [mtl_device newBufferWithLength:size options:get_buffer_options(hint)];
      BLAST_LOG("Allocating %llu bytes [%s]", (unsigned long long)buffer.allocatedSize, name.c_str());

      if (interop.first != nullptr && interop.second > 0)
      {
        BLAST_ASSERT(interop.second == size);
        memcpy([buffer contents], interop.first, interop.second);
#if TARGET_OS_OSX
        if (buffer.storageMode == MTLStorageModeManaged)
        {
          [buffer didModifyRange:NSMakeRange(0, buffer.length)];
        }
#endif
      }

      native = (__bridge_retained void*)buffer;
      break;
    }
    case TYPE_TEX1D:
    case TYPE_TEX2D:
    case TYPE_TEX3D:
    {
      // Metal requires textureType and arrayLength to agree:
      // non-array types must use arrayLength == 1, array types must use arrayLength > 1.
      // Cubemap images are allocated as plain 2D arrays (one slice per face), like Vulkan's
      // CUBE_COMPATIBLE image: the cube type is applied per view (MTLView, BIND_CUBEMAP_*), so UAV
      // views (RWTexture2DArray) and SRV cube views can both reference the same storage.
      const bool is_array = layers_or_stride > 1;

      MTLTextureType texture_type = MTLTextureType2D;
      NSUInteger array_length = 1;

      if (type == TYPE_TEX3D)
      {
        texture_type = MTLTextureType3D;
      }
      else if (type == TYPE_TEX1D)
      {
        texture_type = is_array ? MTLTextureType1DArray : MTLTextureType1D;
        array_length = is_array ? static_cast<NSUInteger>(layers_or_stride) : 1;
      }
      else
      {
        texture_type = is_array ? MTLTextureType2DArray : MTLTextureType2D;
        array_length = is_array ? static_cast<NSUInteger>(layers_or_stride) : 1;
      }

      MTLTextureDescriptor* desc = [[MTLTextureDescriptor alloc] init];
      desc.pixelFormat = get_pixel_format(format);
      desc.width = size_x;
      desc.height = type == TYPE_TEX1D ? 1 : size_y;
      desc.depth = type == TYPE_TEX3D ? size_z : 1;
      desc.mipmapLevelCount = levels_or_length;
      desc.arrayLength = array_length;
      desc.textureType = texture_type;
      // Initial data is uploaded with replaceRegion, which requires CPU-writable storage.
      // Textures without initial data stay private for GPU-only access.
      const bool has_interop = interop.first != nullptr && interop.second > 0;
      desc.storageMode = has_interop ? MTLStorageModeShared : MTLStorageModePrivate;
      desc.usage = get_texture_usage(usage);
      id<MTLTexture> texture = [mtl_device newTextureWithDescriptor:desc];
      BLAST_LOG("Allocating %llu bytes [%s]", (unsigned long long)texture.allocatedSize, name.c_str());

      // Upload the initial data in the same layout Vulkan uses: layer-major, then mip level,
      // each mip tightly packed (Size(format, ..., { level, 1 })).
      if (has_interop && type == TYPE_TEX2D)
      {
        BLAST_ASSERT(Size(format, size_x, size_y, size_z, { 0, levels_or_length }) * layers_or_stride == interop.second);

        auto offset = 0ull;
        for (size_t i = 0; i < layers_or_stride; ++i)
        {
          for (size_t j = 0; j < levels_or_length; ++j)
          {
            const auto mip_w = std::max<NSUInteger>(1, size_x >> j);
            const auto mip_h = std::max<NSUInteger>(1, size_y >> j);
            const auto size = Size(format, size_x, size_y, size_z, { j, 1 });
            const auto row = size / mip_h;

            [texture replaceRegion:MTLRegionMake2D(0, 0, mip_w, mip_h)
                       mipmapLevel:NSUInteger(j)
                             slice:NSUInteger(i)
                         withBytes:interop.first + offset
                       bytesPerRow:NSUInteger(row)
                     bytesPerImage:NSUInteger(size)];
            offset += size;
          }
        }
      }

      native = (__bridge_retained void*)texture;
      break;
    }
    default:
      break;
    }
  }

  void MTLResource::Commit()
  {
  }

  void MTLResource::Retrieve()
  {
  }

  void MTLResource::Blit(const std::shared_ptr<Resource>& resource)
  {
  }

  void* MTLResource::Map()
  {
    if (type != TYPE_BUFFER || !native) return nullptr;

    id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)native;
    return [buffer contents];
  }

  void MTLResource::Unmap()
  {
    if (type != TYPE_BUFFER || !native) return;

    id<MTLBuffer> buffer = (__bridge id<MTLBuffer>)native;
#if TARGET_OS_OSX
    if (buffer.storageMode == MTLStorageModeManaged)
    {
      [buffer didModifyRange:NSMakeRange(0, buffer.length)];
    }
#endif
  }

  void MTLResource::Use()
  {
  }

  void MTLResource::Discard()
  {
    for (const auto& view : views)
    {
      reinterpret_cast<MTLView*>(view.get())->Discard();
    }

    if (native)
    {
      // Balance the __bridge_retained taken in Initialize().
      id object = (__bridge_transfer id)native;
      (void)object;
      native = nullptr;
    }
  }

  MTLResource::MTLResource(const std::string& name,
    Device& device,
    const Resource::BufferDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    MTLResource::Initialize();
  }

  MTLResource::MTLResource(const std::string& name,
    Device& device,
    const Resource::Tex1DDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    MTLResource::Initialize();
  }

  MTLResource::MTLResource(const std::string& name,
    Device& device,
    const Resource::Tex2DDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    MTLResource::Initialize();
  }

  MTLResource::MTLResource(const std::string& name,
    Device& device,
    const Resource::Tex3DDesc& desc,
    Resource::Hint hint,
    std::pair<const uint8_t*, size_t> interop)
    : Resource(name, device, desc, hint, interop)
  {
    MTLResource::Initialize();
  }

  MTLResource::~MTLResource()
  {
    MTLResource::Discard();
  }
}
