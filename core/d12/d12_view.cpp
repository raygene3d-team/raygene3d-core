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
#include "d12_view.h"
#include "d12_resource.h"
#include "d12_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

namespace RayGene3D
{
  void D12View::Initialize()
  {
    auto resource = reinterpret_cast<D12Resource*>(&this->GetResource());
    auto device = reinterpret_cast<D12Device*>(&resource->GetDevice());

    const auto get_format = [](Format format)
    {
      switch (format)
      {
      default: return DXGI_FORMAT_UNKNOWN;
      case FORMAT_R32G32B32A32_FLOAT: return DXGI_FORMAT_R32G32B32A32_FLOAT;
      case FORMAT_R32G32B32A32_UINT: return DXGI_FORMAT_R32G32B32A32_UINT;
      case FORMAT_R32G32B32A32_SINT: return DXGI_FORMAT_R32G32B32A32_SINT;
      case FORMAT_R32G32B32_FLOAT: return DXGI_FORMAT_R32G32B32_FLOAT;
      case FORMAT_R32G32B32_UINT: return DXGI_FORMAT_R32G32B32_UINT;
      case FORMAT_R32G32B32_SINT: return DXGI_FORMAT_R32G32B32_SINT;
      case FORMAT_R16G16B16A16_FLOAT: return DXGI_FORMAT_R16G16B16A16_FLOAT;
      case FORMAT_R16G16B16A16_UNORM: return DXGI_FORMAT_R16G16B16A16_UNORM;
      case FORMAT_R16G16B16A16_UINT: return DXGI_FORMAT_R16G16B16A16_UINT;
      case FORMAT_R16G16B16A16_SNORM: return DXGI_FORMAT_R16G16B16A16_SNORM;
      case FORMAT_R16G16B16A16_SINT: return DXGI_FORMAT_R16G16B16A16_SINT;
      case FORMAT_R32G32_FLOAT: return DXGI_FORMAT_R32G32_FLOAT;
      case FORMAT_R32G32_UINT: return DXGI_FORMAT_R32G32_UINT;
      case FORMAT_R32G32_SINT: return DXGI_FORMAT_R32G32_SINT;
      case FORMAT_D32_FLOAT_S8X24_UINT: return DXGI_FORMAT_D32_FLOAT_S8X24_UINT;
      case FORMAT_R10G10B10A2_UNORM: return DXGI_FORMAT_R10G10B10A2_UNORM;
      case FORMAT_R10G10B10A2_UINT: return DXGI_FORMAT_R10G10B10A2_UINT;
      case FORMAT_R11G11B10_FLOAT: return DXGI_FORMAT_R11G11B10_FLOAT;
      case FORMAT_R8G8B8A8_UNORM: return DXGI_FORMAT_R8G8B8A8_UNORM;
      case FORMAT_R8G8B8A8_SRGB: return DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
      case FORMAT_R8G8B8A8_UINT: return DXGI_FORMAT_R8G8B8A8_UINT;
      case FORMAT_R8G8B8A8_SNORM: return DXGI_FORMAT_R8G8B8A8_SNORM;
      case FORMAT_R8G8B8A8_SINT: return DXGI_FORMAT_R8G8B8A8_SINT;
      case FORMAT_R16G16_FLOAT: return DXGI_FORMAT_R16G16_FLOAT;
      case FORMAT_R16G16_UNORM: return DXGI_FORMAT_R16G16_UNORM;
      case FORMAT_R16G16_UINT: return DXGI_FORMAT_R16G16_UINT;
      case FORMAT_R16G16_SNORM: return DXGI_FORMAT_R16G16_SNORM;
      case FORMAT_R16G16_SINT: return DXGI_FORMAT_R16G16_SINT;
      case FORMAT_D32_FLOAT: return DXGI_FORMAT_D32_FLOAT;
      case FORMAT_R32_FLOAT: return DXGI_FORMAT_R32_FLOAT;
      case FORMAT_R32_UINT: return DXGI_FORMAT_R32_UINT;
      case FORMAT_R32_SINT: return DXGI_FORMAT_R32_SINT;
      case FORMAT_D24_UNORM_S8_UINT: return DXGI_FORMAT_D24_UNORM_S8_UINT;
      case FORMAT_R8G8_UNORM: return DXGI_FORMAT_R8G8_UNORM;
      case FORMAT_R8G8_UINT: return DXGI_FORMAT_R8G8_UINT;
      case FORMAT_R8G8_SNORM: return DXGI_FORMAT_R8G8_SNORM;
      case FORMAT_R8G8_SINT: return DXGI_FORMAT_R8G8_SINT;
      case FORMAT_R16_FLOAT: return DXGI_FORMAT_R16_FLOAT;
      case FORMAT_D16_UNORM: return DXGI_FORMAT_D16_UNORM;
      case FORMAT_R16_UNORM: return DXGI_FORMAT_R16_UNORM;
      case FORMAT_R16_UINT: return DXGI_FORMAT_R16_UINT;
      case FORMAT_R16_SNORM: return DXGI_FORMAT_R16_SNORM;
      case FORMAT_R16_SINT: return DXGI_FORMAT_R16_SINT;
      case FORMAT_R8_UNORM: return DXGI_FORMAT_R8_UNORM;
      case FORMAT_R8_UINT: return DXGI_FORMAT_R8_UINT;
      case FORMAT_R8_SNORM: return DXGI_FORMAT_R8_SNORM;
      case FORMAT_R8_SINT: return DXGI_FORMAT_R8_SINT;
      case FORMAT_R9G9B9E5_SHAREDEXP: return DXGI_FORMAT_R9G9B9E5_SHAREDEXP;
      case FORMAT_R8G8_B8G8_UNORM: return DXGI_FORMAT_R8G8_B8G8_UNORM;
      case FORMAT_G8R8_G8B8_UNORM: return DXGI_FORMAT_G8R8_G8B8_UNORM;
      case FORMAT_BC1_UNORM: return DXGI_FORMAT_BC1_UNORM;
      case FORMAT_BC1_SRGB: return DXGI_FORMAT_BC1_UNORM_SRGB;
      case FORMAT_BC2_UNORM: return DXGI_FORMAT_BC2_UNORM;
      case FORMAT_BC2_SRGB: return DXGI_FORMAT_BC2_UNORM_SRGB;
      case FORMAT_BC3_UNORM: return DXGI_FORMAT_BC3_UNORM;
      case FORMAT_BC3_SRGB: return DXGI_FORMAT_BC3_UNORM_SRGB;
      case FORMAT_BC4_UNORM: return DXGI_FORMAT_BC4_UNORM;
      case FORMAT_BC4_SNORM: return DXGI_FORMAT_BC4_SNORM;
      case FORMAT_BC5_UNORM: return DXGI_FORMAT_BC5_UNORM;
      case FORMAT_BC5_SNORM: return DXGI_FORMAT_BC5_SNORM;
      case FORMAT_B5G6R5_UNORM: return DXGI_FORMAT_B5G6R5_UNORM;
      case FORMAT_B5G5R5A1_UNORM: return DXGI_FORMAT_B5G5R5A1_UNORM;
      case FORMAT_B8G8R8A8_UNORM: return DXGI_FORMAT_B8G8R8A8_UNORM;
      case FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
      case FORMAT_B8G8R8A8_SRGB: return DXGI_FORMAT_B8G8R8A8_UNORM_SRGB;
      case FORMAT_B8G8R8X8_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
      case FORMAT_BC6H_UF16: return DXGI_FORMAT_BC6H_UF16;
      case FORMAT_BC6H_SF16: return DXGI_FORMAT_BC6H_SF16;
      case FORMAT_BC7_UNORM: return DXGI_FORMAT_BC7_UNORM;
      case FORMAT_BC7_SRGB: return DXGI_FORMAT_BC7_UNORM_SRGB;
      }
    };

    switch (usage)
    {
    case USAGE_SHADER_RESOURCE:
    {
      D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
      switch (resource->GetType())
      {
      case Resource::TYPE_BUFFER:
      {
        const auto desc = resource->GetResource()->GetDesc();

        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
        srv_desc.Format = DXGI_FORMAT_UNKNOWN;
        srv_desc.Shader4ComponentMapping = 0;
        srv_desc.Buffer.FirstElement = levels_or_length.offset;
        srv_desc.Buffer.NumElements = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
        srv_desc.Buffer.StructureByteStride = layers_or_stride.length = -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        srv_desc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
        break;
      }
      case Resource::TYPE_TEX1D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1DARRAY;
          srv_desc.Format = get_format(resource->GetFormat());
          srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
          srv_desc.Texture1DArray.MostDetailedMip = levels_or_length.offset;
          srv_desc.Texture1DArray.MipLevels = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
          srv_desc.Texture1DArray.FirstArraySlice = layers_or_stride.offset;
          srv_desc.Texture1DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        }
        else
        {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE1D;
          srv_desc.Format = get_format(resource->GetFormat());
          srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
          srv_desc.Texture1D.MostDetailedMip = levels_or_length.offset;
          srv_desc.Texture1D.MipLevels = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
        }
        break;
      }
      case Resource::TYPE_TEX2D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          if (bind == BIND_CUBEMAP_ARRAY)
          {
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBEARRAY;
            srv_desc.Format = get_format(resource->GetFormat());
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.TextureCubeArray.MostDetailedMip = levels_or_length.offset;
            srv_desc.TextureCubeArray.MipLevels = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
            srv_desc.TextureCubeArray.First2DArrayFace = layers_or_stride.offset;
            srv_desc.TextureCubeArray.NumCubes = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
          }
          else if (bind == BIND_CUBEMAP_LAYER)
          {
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
            srv_desc.Format = get_format(resource->GetFormat());
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.TextureCube.MostDetailedMip = levels_or_length.offset;
            srv_desc.TextureCube.MipLevels = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
          }
          else
          {
            srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2DARRAY;
            srv_desc.Format = get_format(resource->GetFormat());
            srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
            srv_desc.Texture2DArray.MostDetailedMip = levels_or_length.offset;
            srv_desc.Texture2DArray.MipLevels = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
            srv_desc.Texture2DArray.FirstArraySlice = layers_or_stride.offset;
            srv_desc.Texture2DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
          }
        }
        else
        {
          srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
          srv_desc.Format = get_format(resource->GetFormat());
          srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
          srv_desc.Texture2D.MostDetailedMip = levels_or_length.offset;
          srv_desc.Texture2D.MipLevels = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
        }
        break;
      }
      case Resource::TYPE_TEX3D:
      {
        srv_desc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE3D;
        srv_desc.Format = get_format(resource->GetFormat());
        srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
        srv_desc.Texture3D.MostDetailedMip = levels_or_length.offset;
        srv_desc.Texture3D.MipLevels = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
        break;
      }
      }

      srv_desc.Format = srv_desc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT ? DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS : srv_desc.Format;
      srv_desc.Format = srv_desc.Format == DXGI_FORMAT_D32_FLOAT ? DXGI_FORMAT_R32_FLOAT : srv_desc.Format;
      srv_desc.Format = srv_desc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : srv_desc.Format;
      srv_desc.Format = srv_desc.Format == DXGI_FORMAT_D16_UNORM ? DXGI_FORMAT_R16_UNORM : srv_desc.Format;

      handle = resource->ObtainHandle();
      device->GetDevice()->CreateShaderResourceView(resource->GetResource(), &srv_desc, handle);
      break;
    }

    case USAGE_RENDER_TARGET:
    {
      D3D12_RENDER_TARGET_VIEW_DESC rtv_desc = {};
      switch (resource->GetType())
      {
      case Resource::TYPE_BUFFER:
      {
        const auto desc = resource->GetResource()->GetDesc();

        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_BUFFER;
        rtv_desc.Format = DXGI_FORMAT_UNKNOWN;
        rtv_desc.Buffer.FirstElement = levels_or_length.offset;
        rtv_desc.Buffer.NumElements = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
        break;
      }
      case Resource::TYPE_TEX1D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1DARRAY;
          rtv_desc.Format = get_format(resource->GetFormat());
          rtv_desc.Texture1DArray.MipSlice = levels_or_length.offset;
          rtv_desc.Texture1DArray.FirstArraySlice = layers_or_stride.offset;
          rtv_desc.Texture1DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        }
        else
        {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE1D;
          rtv_desc.Format = get_format(resource->GetFormat());
          rtv_desc.Texture1D.MipSlice = levels_or_length.offset;
        }
        break;
      }
      case Resource::TYPE_TEX2D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2DARRAY;
          rtv_desc.Format = get_format(resource->GetFormat());
          rtv_desc.Texture2DArray.MipSlice = levels_or_length.offset;
          rtv_desc.Texture2DArray.FirstArraySlice = layers_or_stride.offset;
          rtv_desc.Texture2DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        }
        else
        {
          rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
          rtv_desc.Format = get_format(resource->GetFormat());
          rtv_desc.Texture2D.MipSlice = levels_or_length.offset;
        }
        break;
      }
      case Resource::TYPE_TEX3D:
      {
        rtv_desc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE3D;
        rtv_desc.Format = get_format(resource->GetFormat());
        rtv_desc.Texture3D.MipSlice = levels_or_length.offset;
        rtv_desc.Texture3D.FirstWSlice = layers_or_stride.offset;
        rtv_desc.Texture3D.WSize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        break;
      }
      }

      rtv_desc.Format = rtv_desc.Format == DXGI_FORMAT_D32_FLOAT_S8X24_UINT ? DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS : rtv_desc.Format;
      rtv_desc.Format = rtv_desc.Format == DXGI_FORMAT_D32_FLOAT ? DXGI_FORMAT_R32_FLOAT : rtv_desc.Format;
      rtv_desc.Format = rtv_desc.Format == DXGI_FORMAT_D24_UNORM_S8_UINT ? DXGI_FORMAT_R24_UNORM_X8_TYPELESS : rtv_desc.Format;
      rtv_desc.Format = rtv_desc.Format == DXGI_FORMAT_D16_UNORM ? DXGI_FORMAT_R16_UNORM : rtv_desc.Format;

      handle = resource->ObtainHandle(D12Resource::HEAP_RTV);
      device->GetDevice()->CreateRenderTargetView(resource->GetResource(), &rtv_desc, handle);
      break;
    }

    case USAGE_DEPTH_STENCIL:
    {
      D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc = {};
      switch (resource->GetType())
      {
      case Resource::TYPE_TEX1D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1DARRAY;
          dsv_desc.Format = get_format(resource->GetFormat());
          dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
          dsv_desc.Texture1DArray.MipSlice = levels_or_length.offset;
          dsv_desc.Texture1DArray.FirstArraySlice = layers_or_stride.offset;
          dsv_desc.Texture1DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        }
        else
        {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE1D;
          dsv_desc.Format = get_format(resource->GetFormat());
          dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
          dsv_desc.Texture1D.MipSlice = levels_or_length.offset;
        }
        break;
      }
      case Resource::TYPE_TEX2D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2DARRAY;
          dsv_desc.Format = get_format(resource->GetFormat());
          dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
          dsv_desc.Texture2DArray.MipSlice = levels_or_length.offset;
          dsv_desc.Texture2DArray.FirstArraySlice = layers_or_stride.offset;
          dsv_desc.Texture2DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        }
        else
        {
          dsv_desc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
          dsv_desc.Format = get_format(resource->GetFormat());
          dsv_desc.Flags = D3D12_DSV_FLAG_NONE;
          dsv_desc.Texture2D.MipSlice = levels_or_length.offset;
        }
        break;
      }
      }

      dsv_desc.Format = dsv_desc.Format == DXGI_FORMAT_R32_FLOAT_X8X24_TYPELESS ? DXGI_FORMAT_D32_FLOAT_S8X24_UINT : dsv_desc.Format;
      dsv_desc.Format = dsv_desc.Format == DXGI_FORMAT_R32_FLOAT ? DXGI_FORMAT_D32_FLOAT : dsv_desc.Format;
      dsv_desc.Format = dsv_desc.Format == DXGI_FORMAT_R24_UNORM_X8_TYPELESS ? DXGI_FORMAT_D24_UNORM_S8_UINT : dsv_desc.Format;
      dsv_desc.Format = dsv_desc.Format == DXGI_FORMAT_R16_UNORM ? DXGI_FORMAT_D16_UNORM : dsv_desc.Format;

      handle = resource->ObtainHandle(D12Resource::HEAP_DSV);
      device->GetDevice()->CreateDepthStencilView(resource->GetResource(), &dsv_desc, handle);
      break;
    }

    case USAGE_UNORDERED_ACCESS:
    {
      D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc = {};
      switch (resource->GetType())
      {
      case Resource::TYPE_BUFFER:
      {
        const auto desc = resource->GetResource()->GetDesc();

        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
        uav_desc.Format = DXGI_FORMAT_UNKNOWN;
        uav_desc.Buffer.FirstElement = levels_or_length.offset;
        uav_desc.Buffer.NumElements = levels_or_length.length == -1 ? resource->GetLevelsOrLength() : levels_or_length.length;
        uav_desc.Buffer.StructureByteStride = layers_or_stride.length = -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        uav_desc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;
        break;
      }
      case Resource::TYPE_TEX1D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1DARRAY;
          uav_desc.Format = get_format(resource->GetFormat());
          uav_desc.Texture1DArray.MipSlice = levels_or_length.offset;
          uav_desc.Texture1DArray.FirstArraySlice = layers_or_stride.offset;
          uav_desc.Texture1DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        }
        else
        {
          uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE1D;
          uav_desc.Format = get_format(resource->GetFormat());
          uav_desc.Texture1D.MipSlice = levels_or_length.offset;
        }
        break;
      }
      case Resource::TYPE_TEX2D:
      {
        if (resource->GetHint() & Resource::HINT_LAYERED_IMAGE)
        {
          uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2DARRAY;
          uav_desc.Format = get_format(resource->GetFormat());
          uav_desc.Texture2DArray.MipSlice = levels_or_length.offset;
          uav_desc.Texture2DArray.FirstArraySlice = layers_or_stride.offset;
          uav_desc.Texture2DArray.ArraySize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        }
        else
        {
          uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
          uav_desc.Format = get_format(resource->GetFormat());
          uav_desc.Texture2D.MipSlice = levels_or_length.offset;
        }
        break;
      }
      case Resource::TYPE_TEX3D:
      {
        uav_desc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE3D;
        uav_desc.Format = get_format(resource->GetFormat());
        uav_desc.Texture3D.MipSlice = levels_or_length.offset;
        uav_desc.Texture3D.FirstWSlice = layers_or_stride.offset;
        uav_desc.Texture3D.WSize = layers_or_stride.length == -1 ? resource->GetLayersOrStride() : layers_or_stride.length;
        break;
      }
      }
      handle = resource->ObtainHandle();
      device->GetDevice()->CreateUnorderedAccessView(resource->GetResource(), nullptr, &uav_desc, handle);
      break;
    }

    default:
      break;
    }
  }

  void D12View::Use()
  {
  }

  void D12View::Discard()
  {
    auto resource = reinterpret_cast<D12Resource*>(&this->GetResource());
    auto device = reinterpret_cast<D12Device*>(&resource->GetDevice());

    switch (usage)
    {
    //case USAGE_CONSTANT_DATA: resource->DropHandle(handle); break;
    case USAGE_SHADER_RESOURCE: resource->DropHandle(handle); break;
    case USAGE_UNORDERED_ACCESS: resource->DropHandle(handle); break;
    case USAGE_RENDER_TARGET: resource->DropHandle(handle, D12Resource::HEAP_RTV); break;
    case USAGE_DEPTH_STENCIL: resource->DropHandle(handle, D12Resource::HEAP_DSV); break;
    }
  }

  D12View::D12View(const std::string& name,
    Resource& resource,
    Usage usage,
    const Range& levels_or_length,
    const Range& layers_or_stride,
    View::Bind bind)
    : View(name, resource, usage, levels_or_length, layers_or_stride, bind)
  {
    D12View::Initialize();
  }

  D12View::~D12View()
  {
    D12View::Discard();
  }
}