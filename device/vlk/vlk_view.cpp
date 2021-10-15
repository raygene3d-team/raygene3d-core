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


#include "vlk_view.h"
#include "vlk_resource.h"
#include "vlk_device.h"


namespace RayGene3D
{
  void VLKView::Initialize()
  {
    auto* resource = reinterpret_cast<VLKResource*>(&this->GetResource());
    auto* device = reinterpret_cast<VLKDevice*>(&resource->GetDevice());

    const auto get_type = [this]()
    {
      auto* resource = reinterpret_cast<VLKResource*>(&this->GetResource());

      if (resource->GetType() == Resource::TYPE_IMAGE3D)
      {
        return resource->GetHint() & Resource::HINT_LAYERED_IMAGE ? VK_IMAGE_VIEW_TYPE_MAX_ENUM : VK_IMAGE_VIEW_TYPE_3D;
      }

      if (resource->GetType() == Resource::TYPE_IMAGE2D)
      {
        if (bind == BIND_SHADER_RESOURCE)
        {
          if(usage == USAGE_CUBEMAP_LAYER) return VK_IMAGE_VIEW_TYPE_CUBE;
          if(usage == USAGE_CUBEMAP_ARRAY) return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
        }
        return resource->GetHint() & Resource::HINT_LAYERED_IMAGE ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
      }

      if (resource->GetType() == Resource::TYPE_IMAGE1D)
      {
        return resource->GetHint() & Resource::HINT_LAYERED_IMAGE ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
      }

      return VK_IMAGE_VIEW_TYPE_2D; //Hack for comatibility
    };


    const auto get_format = [this]()
    {
      auto* resource = reinterpret_cast<VLKResource*>(&this->GetResource());

      switch (resource->GetFormat())
      {
      default: return VK_FORMAT_UNDEFINED;
      case FORMAT_R32G32B32A32_FLOAT: return VK_FORMAT_R32G32B32A32_SFLOAT;
      case FORMAT_R32G32B32A32_UINT: return VK_FORMAT_R32G32B32A32_UINT;
      case FORMAT_R32G32B32A32_SINT: return VK_FORMAT_R32G32B32A32_SINT;
      case FORMAT_R32G32B32_FLOAT: return VK_FORMAT_R32G32B32_SFLOAT;
      case FORMAT_R32G32B32_UINT: return VK_FORMAT_R32G32B32_UINT;
      case FORMAT_R32G32B32_SINT: return VK_FORMAT_R32G32B32_SINT;
      case FORMAT_R16G16B16A16_FLOAT: return VK_FORMAT_R16G16B16A16_SFLOAT;
      case FORMAT_R16G16B16A16_UNORM: return VK_FORMAT_R16G16B16A16_UNORM;
      case FORMAT_R16G16B16A16_UINT: return VK_FORMAT_R16G16B16A16_UINT;
      case FORMAT_R16G16B16A16_SNORM: return VK_FORMAT_R16G16B16A16_SNORM;
      case FORMAT_R16G16B16A16_SINT: return VK_FORMAT_R16G16B16A16_SINT;
      case FORMAT_R32G32_FLOAT: return VK_FORMAT_R32G32_SFLOAT;
      case FORMAT_R32G32_UINT: return VK_FORMAT_R32G32_UINT;
      case FORMAT_R32G32_SINT: return VK_FORMAT_R32G32_SINT;
      case FORMAT_D32_FLOAT_S8X24_UINT: return VK_FORMAT_D32_SFLOAT_S8_UINT;
      case FORMAT_R10G10B10A2_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
      case FORMAT_R10G10B10A2_UINT: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
      case FORMAT_R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
      case FORMAT_R8G8B8A8_UNORM: return VK_FORMAT_R8G8B8A8_UNORM;
      case FORMAT_R8G8B8A8_SRGB: return VK_FORMAT_R8G8B8A8_SRGB;
      case FORMAT_R8G8B8A8_UINT: return VK_FORMAT_R8G8B8A8_UINT;
      case FORMAT_R8G8B8A8_SNORM: return VK_FORMAT_R8G8B8A8_SNORM;
      case FORMAT_R8G8B8A8_SINT: return VK_FORMAT_R8G8B8A8_SINT;
      case FORMAT_R16G16_FLOAT: return VK_FORMAT_R16G16_SFLOAT;
      case FORMAT_R16G16_UNORM: return VK_FORMAT_R16G16_UNORM;
      case FORMAT_R16G16_UINT: return VK_FORMAT_R16G16_UINT;
      case FORMAT_R16G16_SNORM: return VK_FORMAT_R16G16_SNORM;
      case FORMAT_R16G16_SINT: return VK_FORMAT_R16G16_SINT;
      case FORMAT_D32_FLOAT: return VK_FORMAT_D32_SFLOAT;
      case FORMAT_R32_FLOAT: return VK_FORMAT_R32_SFLOAT;
      case FORMAT_R32_UINT: return VK_FORMAT_R32_UINT;
      case FORMAT_R32_SINT: return VK_FORMAT_R32_SINT;
      case FORMAT_D24_UNORM_S8_UINT: return VK_FORMAT_D24_UNORM_S8_UINT;
      case FORMAT_R8G8_UNORM: return VK_FORMAT_R8G8_UNORM;
      case FORMAT_R8G8_UINT: return VK_FORMAT_R8G8_UINT;
      case FORMAT_R8G8_SNORM: return VK_FORMAT_R8G8_SNORM;
      case FORMAT_R8G8_SINT: return VK_FORMAT_R8G8_SINT;
      case FORMAT_R16_FLOAT: return VK_FORMAT_R16_SFLOAT;
      case FORMAT_D16_UNORM: return VK_FORMAT_D16_UNORM;
      case FORMAT_R16_UNORM: return VK_FORMAT_R16_UNORM;
      case FORMAT_R16_UINT: return VK_FORMAT_R16_UINT;
      case FORMAT_R16_SNORM: return VK_FORMAT_R16_SNORM;
      case FORMAT_R16_SINT: return VK_FORMAT_R16_SINT;
      case FORMAT_R8_UNORM: return VK_FORMAT_R8_UNORM;
      case FORMAT_R8_UINT: return VK_FORMAT_R8_UINT;
      case FORMAT_R8_SNORM: return VK_FORMAT_R8_SNORM;
      case FORMAT_R8_SINT: return VK_FORMAT_R8_SINT;
      case FORMAT_R9G9B9E5_SHAREDEXP: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
      case FORMAT_R8G8_B8G8_UNORM: return VK_FORMAT_G8B8G8R8_422_UNORM;
      case FORMAT_G8R8_G8B8_UNORM: return VK_FORMAT_B8G8R8G8_422_UNORM;
      case FORMAT_BC1_UNORM: return VK_FORMAT_BC1_RGB_UNORM_BLOCK;
      case FORMAT_BC1_SRGB: return VK_FORMAT_BC1_RGB_SRGB_BLOCK;
      case FORMAT_BC2_UNORM: return VK_FORMAT_BC2_UNORM_BLOCK;
      case FORMAT_BC2_SRGB: return VK_FORMAT_BC2_SRGB_BLOCK;
      case FORMAT_BC3_UNORM: return VK_FORMAT_BC3_UNORM_BLOCK;
      case FORMAT_BC3_SRGB: return VK_FORMAT_BC3_SRGB_BLOCK;
      case FORMAT_BC4_UNORM: return VK_FORMAT_BC4_UNORM_BLOCK;
      case FORMAT_BC4_SNORM: return VK_FORMAT_BC4_SNORM_BLOCK;
      case FORMAT_BC5_UNORM: return VK_FORMAT_BC5_UNORM_BLOCK;
      case FORMAT_BC5_SNORM: return VK_FORMAT_BC5_SNORM_BLOCK;
      case FORMAT_B5G6R5_UNORM: return VK_FORMAT_B5G6R5_UNORM_PACK16;
      case FORMAT_B5G5R5A1_UNORM: return VK_FORMAT_B5G5R5A1_UNORM_PACK16;
      case FORMAT_B8G8R8A8_UNORM: return VK_FORMAT_B8G8R8A8_UNORM;
        // case FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
      case FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
        // case FORMAT_B8G8R8X8_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
      case FORMAT_BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
      case FORMAT_BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
      case FORMAT_BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
      case FORMAT_BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
      }
    };

    const auto get_aspect = [this]()
    {
      const auto* resource = reinterpret_cast<VLKResource*>(&this->GetResource());
      const auto format = resource->GetFormat();

      switch (bind)
      {
      case BIND_UNORDERED_ACCESS: return format == FORMAT_D32_FLOAT || format == FORMAT_D16_UNORM ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
      case BIND_SHADER_RESOURCE: return format == FORMAT_D32_FLOAT || format == FORMAT_D16_UNORM ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;
      case BIND_RENDER_TARGET: return VK_IMAGE_ASPECT_COLOR_BIT;
      case BIND_DEPTH_STENCIL: return VK_IMAGE_ASPECT_DEPTH_BIT;
      }
      return VK_IMAGE_ASPECT_FLAG_BITS_MAX_ENUM;
    };

    auto image = resource->GetImage();
    if (image)
    {
      BLAST_LOG("Creating image view: [%s]", name.c_str());

      auto create_info = VkImageViewCreateInfo{};
      create_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
      create_info.image = image;
      create_info.viewType = get_type();
      create_info.format = get_format();
      create_info.subresourceRange.aspectMask = get_aspect();
      create_info.subresourceRange.baseMipLevel = mipmap_offset;
      create_info.subresourceRange.levelCount = mipmap_count == -1 ? resource->GetMipmaps() : mipmap_count;
      create_info.subresourceRange.baseArrayLayer = layer_offset;
      create_info.subresourceRange.layerCount = layer_count == -1 ? resource->GetLayers() : layer_count;

      //if (create_info.format == VK_FORMAT_D32_SFLOAT && bind != BIND_DEPTH_STENCIL)
      //{
      //  create_info.format = VK_FORMAT_R32_SFLOAT;
      //  //create_info.subresourceRange.layerCount = 1;
      //}

      BLAST_ASSERT(VK_SUCCESS == vkCreateImageView(device->GetDevice(), &create_info, nullptr, &view));
    }

    //auto buffer = resource->GetBuffer();
    //if (buffer)
    //{
    //  auto create_info = VkBufferViewCreateInfo{};
    //  create_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    //  create_info.flags = 0;
    //  create_info.buffer = buffer;
    //  create_info.format = VK_FORMAT_UNDEFINED;
    //  create_info.offset = 0;
    //  create_info.range = VK_WHOLE_SIZE;
    //  BLAST_ASSERT(VK_SUCCESS == vkCreateBufferView(device->GetDevice(), &create_info, nullptr, &buffer_view));
    //}


  }

  void VLKView::Use()
  {
  }

  void VLKView::Discard()
  {
    auto* resource = reinterpret_cast<VLKResource*>(&this->GetResource());
    auto* device = reinterpret_cast<VLKDevice*>(&resource->GetDevice());

    //if (buffer_view)
    //{
    //  vkDestroyBufferView(device->GetDevice(), buffer_view, nullptr);
    //  buffer_view = nullptr;
    //}

    if (view)
    {
      vkDestroyImageView(device->GetDevice(), view, nullptr);
      view = nullptr;
    }
  }

  VLKView::VLKView(const std::string& name, Resource& resource) 
    : View(name, resource)
  {
  }

  VLKView::~VLKView()
  {
    VLKView::Discard();
  }
}