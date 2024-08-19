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
#include "d11_resource.h"
#include "d11_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

namespace RayGene3D
{

  void D11Resource::Initialize()
  {
    if (resource)
    {
      BLAST_LOG("Initializing existing Resource: %s", name.c_str());
      return;
    }

    D11Device* device = reinterpret_cast<D11Device*>(&this->GetDevice());

    const auto get_usage = [this]()
    {
      D3D11_USAGE usage = D3D11_USAGE_DEFAULT;
      usage = hint & HINT_DYNAMIC_BUFFER ? D3D11_USAGE_DYNAMIC : usage;
      return usage;
    };

    const auto get_access = [this]()
    {
      uint32_t access = 0;
      access = hint & HINT_DYNAMIC_BUFFER ? uint32_t(D3D11_CPU_ACCESS_WRITE) : access;
      return access;
    };

    const auto get_misc = [this]()
    {
      uint32_t misc = 0;
      misc = hint & HINT_CUBEMAP_IMAGE ? misc | D3D11_RESOURCE_MISC_TEXTURECUBE : misc;
      {
        misc = (type == TYPE_BUFFER && (usage & USAGE_SHADER_RESOURCE))  ? misc | D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : misc;
        misc = (type == TYPE_BUFFER && (usage & USAGE_UNORDERED_ACCESS)) ? misc | D3D11_RESOURCE_MISC_BUFFER_STRUCTURED : misc;
        misc = (type == TYPE_BUFFER && (usage & USAGE_ARGUMENT_LIST)) ? misc | D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS : misc;
      }
      return misc;
    };

    const auto get_bind = [this]()
    {
      uint32_t bind = 0;
      {
        bind = usage & USAGE_SHADER_RESOURCE ? bind | D3D11_BIND_SHADER_RESOURCE : bind;
        bind = usage & USAGE_RENDER_TARGET ? bind | D3D11_BIND_RENDER_TARGET : bind;
        bind = usage & USAGE_DEPTH_STENCIL ? bind | D3D11_BIND_DEPTH_STENCIL : bind;
        bind = usage & USAGE_UNORDERED_ACCESS ? bind | D3D11_BIND_UNORDERED_ACCESS : bind;
        bind = usage & USAGE_VERTEX_ARRAY ? bind | D3D11_BIND_VERTEX_BUFFER : bind;
        bind = usage & USAGE_INDEX_ARRAY ? bind | D3D11_BIND_INDEX_BUFFER : bind;
        bind = usage & USAGE_CONSTANT_DATA ? bind | D3D11_BIND_CONSTANT_BUFFER : bind;
        bind = usage & USAGE_ARGUMENT_LIST ? bind | D3D11_BIND_VIDEO_ENCODER : bind; // Hack to create command buffer
      }
      return bind;
    };

    const auto get_format = [this]()
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

    const auto populate_subresources_fn = [this](const std::vector<std::pair<const void*, uint32_t>>& interops)
    {
      BLAST_ASSERT(layers_or_stride * mipmaps_or_count == uint32_t(interops.size()));

      auto result = std::vector<D3D11_SUBRESOURCE_DATA>(layers_or_stride * mipmaps_or_count);
      for (uint32_t i = 0; i < layers_or_stride; ++i)
      {
        for (uint32_t j = 0; j < mipmaps_or_count; ++j)
        {
          const auto [data, size] = interops[i * mipmaps_or_count + j];
          BLAST_ASSERT(data != nullptr && size != 0);

          const auto mip_extent_x = std::max(1u, size_x >> j);
          const auto mip_extent_y = std::max(1u, size_y >> j);

          result[i * mipmaps_or_count + j].pSysMem = data;
          result[i * mipmaps_or_count + j].SysMemPitch = size / mip_extent_y;
          result[i * mipmaps_or_count + j].SysMemSlicePitch = size / (mip_extent_x * mip_extent_y);
        }
      }
      return result;
    };

    switch (type)
    {
    case TYPE_BUFFER:
    {
      D3D11_BUFFER_DESC buffer_desc = {};
      buffer_desc.ByteWidth = mipmaps_or_count * layers_or_stride;
      buffer_desc.Usage = get_usage();
      buffer_desc.BindFlags = get_bind();
      buffer_desc.CPUAccessFlags = get_access();
      buffer_desc.MiscFlags = get_misc();
      buffer_desc.StructureByteStride = layers_or_stride;

      std::vector<D3D11_SUBRESOURCE_DATA> arr_sd_items(interops.size());
      for (size_t i = 0; i < arr_sd_items.size(); ++i)
      {
        const auto [data, size] = interops[i];
        BLAST_ASSERT(data != nullptr && size != 0);

        arr_sd_items[i].pSysMem = data;
        arr_sd_items[i].SysMemPitch = 0;
        arr_sd_items[i].SysMemSlicePitch = 0;
      }
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateBuffer(&buffer_desc, 
        arr_sd_items.empty() ? nullptr : arr_sd_items.data(), reinterpret_cast<ID3D11Buffer**>(&resource)));
      GetBuffer()->GetDesc(&info.buffer_desc);
      break;
    }

    case TYPE_TEX1D:
    {
      D3D11_TEXTURE1D_DESC tex1d_desc = {};
      tex1d_desc.Width = size_x;
      tex1d_desc.MipLevels = mipmaps_or_count;
      tex1d_desc.ArraySize = layers_or_stride;
      tex1d_desc.Format = get_format();
      tex1d_desc.Usage = get_usage();
      tex1d_desc.BindFlags = get_bind();
      tex1d_desc.CPUAccessFlags = get_access();
      tex1d_desc.MiscFlags = get_misc();

      //std::vector<D3D11_SUBRESOURCE_DATA> arr_sd_items(interops.size());
      //for (size_t i = 0; i < arr_sd_items.size(); ++i)
      //{
      //  const auto [data, size] = interops[i];
      //  BLAST_ASSERT(data != nullptr && size != 0);

      //  const auto mip_extent_x = size_x >> j;
      //  const auto mip_extent_y = size_y >> j;

      //  arr_sd_items[i * mipmaps_or_count + j].pSysMem = data;
      //  arr_sd_items[i * mipmaps_or_count + j].SysMemPitch = size / mip_extent_y;
      //  arr_sd_items[i * mipmaps_or_count + j].SysMemSlicePitch = size / (mip_extent_x * mip_extent_y);
      //}
      if (interops.empty())
      {
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateTexture1D(&tex1d_desc, 
          nullptr, reinterpret_cast<ID3D11Texture1D**>(&resource)));
      }
      else
      {
        const auto subresources = populate_subresources_fn(interops);
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateTexture1D(&tex1d_desc, 
          subresources.data(), reinterpret_cast<ID3D11Texture1D**>(&resource)));
      }

      GetTexture1D()->GetDesc(&info.tex1d_desc);
      break;
    }

    case TYPE_TEX2D:
    {
      D3D11_TEXTURE2D_DESC tex2d_desc = {};
      tex2d_desc.Width = size_x;
      tex2d_desc.Height = size_y;
      tex2d_desc.MipLevels = mipmaps_or_count;
      tex2d_desc.ArraySize = layers_or_stride;
      tex2d_desc.Format = get_format();
      tex2d_desc.SampleDesc = { 1, 0 };
      tex2d_desc.Usage = get_usage();
      tex2d_desc.BindFlags = get_bind();
      tex2d_desc.CPUAccessFlags = get_access();
      tex2d_desc.MiscFlags = get_misc();

      if (tex2d_desc.BindFlags & D3D11_BIND_DEPTH_STENCIL)
      {
        switch (tex2d_desc.Format)
        {
        case DXGI_FORMAT_D32_FLOAT_S8X24_UINT: tex2d_desc.Format = DXGI_FORMAT_R32G8X24_TYPELESS; break;
        case DXGI_FORMAT_D32_FLOAT: tex2d_desc.Format = DXGI_FORMAT_R32_TYPELESS; break;
        case DXGI_FORMAT_D24_UNORM_S8_UINT: tex2d_desc.Format = DXGI_FORMAT_R24G8_TYPELESS; break;
        case DXGI_FORMAT_D16_UNORM: tex2d_desc.Format = DXGI_FORMAT_R16_TYPELESS; break;
        }
      }

      //std::vector<D3D11_SUBRESOURCE_DATA> arr_sd_items(mipmaps_or_count * layers_or_stride);
      //for (uint32_t i = 0; i < layers_or_stride; ++i)
      //{
      //  for (uint32_t j = 0; j < mipmaps_or_count; ++j)
      //  {
      //    const auto [data, size] = interops[i];
      //    BLAST_ASSERT(data != nullptr && size != 0);

      //    const auto mip_extent_x = size_x >> j;
      //    const auto mip_extent_y = size_y >> j;

      //    arr_sd_items[i * mipmaps_or_count + j].pSysMem = data;
      //    arr_sd_items[i * mipmaps_or_count + j].SysMemPitch = size / mip_extent_y;
      //    arr_sd_items[i * mipmaps_or_count + j].SysMemSlicePitch = size / (mip_extent_x * mip_extent_y);
      //  }
      //}

      if (interops.empty())
      {
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateTexture2D(&tex2d_desc,
          nullptr, reinterpret_cast<ID3D11Texture2D**>(&resource)));
      }
      else
      {
        const auto subresources = populate_subresources_fn(interops);
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateTexture2D(&tex2d_desc,
          subresources.data(), reinterpret_cast<ID3D11Texture2D**>(&resource)));
      }
      
      GetTexture2D()->GetDesc(&info.tex2d_desc);
      break;
    }

    case TYPE_TEX3D:
    {
      D3D11_TEXTURE3D_DESC tex3d_desc = {};
      tex3d_desc.Width = size_x;
      tex3d_desc.Height = size_x;
      tex3d_desc.Depth = size_x;
      tex3d_desc.MipLevels = mipmaps_or_count;
      tex3d_desc.Format = get_format();
      tex3d_desc.Usage = get_usage();
      tex3d_desc.BindFlags = get_bind();
      tex3d_desc.CPUAccessFlags = get_access();
      tex3d_desc.MiscFlags = get_misc();

      //std::vector<D3D11_SUBRESOURCE_DATA> arr_sd_items(interops.size());
      //for (size_t i = 0; i < arr_sd_items.size(); ++i)
      //{
      //  const auto [data, size] = interops[i];
      //  BLAST_ASSERT(data != nullptr && size != 0);

      //  const auto mip_extent_x = size_x >> j;
      //  const auto mip_extent_y = size_y >> j;

      //  arr_sd_items[i * mipmaps_or_count + j].pSysMem = data;
      //  arr_sd_items[i * mipmaps_or_count + j].SysMemPitch = size / mip_extent_y;
      //  arr_sd_items[i * mipmaps_or_count + j].SysMemSlicePitch = size / (mip_extent_x * mip_extent_y);
      //}

      if (interops.empty())
      {
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateTexture3D(&tex3d_desc,
          nullptr, reinterpret_cast<ID3D11Texture3D**>(&resource)));
      }
      else
      {
        const auto subresources = populate_subresources_fn(interops);
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateTexture3D(&tex3d_desc,
          subresources.data(), reinterpret_cast<ID3D11Texture3D**>(&resource)));
      }

      GetTexture3D()->GetDesc(&info.tex3d_desc);
      break;
    }

    default:
      break;
    }

    for (auto& view : views)
    {
      view->Initialize();
    }
  }

  void D11Resource::Use()
  {
  }

  void D11Resource::Discard()
  {
    for (auto& view : views)
    {
      view->Discard();
    }

    if (resource)
    {
      resource->Release();
      resource = nullptr;
    }
  }


  void D11Resource::Commit(uint32_t index)
  {
    D11Device* device = reinterpret_cast<D11Device*>(&this->GetDevice());

    if (index >= interops.size())
    {
      return;
    }

    //const auto [data, size] = interops[index];
    ////const auto [stride, count] = properties[index]->GetRawSize();

    //switch (type)
    //{
    //case TYPE_BUFFER:
    //  if (size == mipmaps_or_count * layers_or_stride)
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, 0, 0);
    //  }
    //  break;

    //case TYPE_TEX1D:
    //  if (size * 8 == size_x * BitCount(format))
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, 0, 0);
    //  }
    //  break;

    //case TYPE_TEX2D:
    //  if (size * 8 == size_x * size_y * BitCount(format))
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, size_x, 0);
    //  }
    //  break;

    //case TYPE_TEX3D:
    //  if (size * 8 == size_x * size_y * size_z * BitCount(format))
    //  {
    //    device->GetContext()->UpdateSubresource(resource, index, nullptr, data, size_x, size_x * size_y);
    //  }
    //  break;
    //}
  }


  void D11Resource::Retrieve(uint32_t index)
  {
    D11Device* device = reinterpret_cast<D11Device*>(&this->GetDevice());

    if (index >= interops.size())
    {
      return;
    }

    //ID3D11Resource* temp_resource = nullptr;
    //switch (type)
    //{
    //case TYPE_BUFFER:
    //{
    //  D3D11_BUFFER_DESC temp_buffer_desc{ 0 };
    //  temp_buffer_desc.ByteWidth = stride * count;
    //  temp_buffer_desc.Usage = D3D11_USAGE_STAGING;
    //  temp_buffer_desc.BindFlags = 0;
    //  temp_buffer_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    //  temp_buffer_desc.MiscFlags = 0;
    //  temp_buffer_desc.StructureByteStride = 0;
    //  BLAST_ASSERT(S_OK == device->GetDevice()->CreateBuffer(&temp_buffer_desc, nullptr, (ID3D11Buffer**)&temp_resource));
    //  break;
    //}
    //case TYPE_IMAGE2D:
    //{
    //  D3D11_TEXTURE2D_DESC temp_tex2d_desc{ 0 };
    //  temp_tex2d_desc.Width = extent_x;
    //  temp_tex2d_desc.Height = extent_y;
    //  temp_tex2d_desc.MipLevels = 1;
    //  temp_tex2d_desc.ArraySize = mipmaps;
    //  temp_tex2d_desc.Format = get_format();
    //  temp_tex2d_desc.SampleDesc = tex2d_desc.SampleDesc;
    //  temp_tex2d_desc.Usage = D3D11_USAGE_STAGING;
    //  temp_tex2d_desc.BindFlags = 0;
    //  temp_tex2d_desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
    //  temp_tex2d_desc.MiscFlags = 0;
    //  hr = core->GetDevice()->CreateTexture2D(&temp_texture2d_desc, nullptr, (ID3D11Texture2D**)&temp_resource);
    //  break;
    //}
    //default: break;
    //}
    //BLAST_ASSERT(hr == S_OK);

    //core->GetContext()->CopyResource(temp_resource, resource_);

    //D3D11_MAPPED_SUBRESOURCE temp_mapped_subresource{ 0 };
    //hr = core->GetContext()->Map(temp_resource, index, D3D11_MAP_READ_WRITE, 0, &temp_mapped_subresource);
    //BLAST_ASSERT(hr == S_OK);

    //switch (type_)
    //{
    //case TYPE_BUFFER:
    //{
    //  properties_[index]->SetRawBytes({ temp_mapped_subresource.pData, buffer_desc_.ByteWidth }, 0);
    //  break;
    //}
    //case TYPE_IMAGE2D:
    //{
    //  properties_[index]->SetRawBytes({ temp_mapped_subresource.pData, 4 * texture2d_desc_.Width * texture2d_desc_.Height }, 0); // WARNING!!! Texture2D is assumed to be power-of-two
    //  break;
    //}
    //default: break;
    //}

    //core->GetContext()->Unmap(temp_resource, index);
    //temp_resource->Release();
  }

  void D11Resource::Blit(const std::shared_ptr<Resource>& resource)
  {
    D11Device* device = reinterpret_cast<D11Device*>(&this->GetDevice());

    if (resource)
    {
      device->GetContext()->CopyResource(this->resource, reinterpret_cast<D11Resource*>(resource.get())->GetResource());
    }   
  }


  void* D11Resource::Map()
  {
    D11Device* device = reinterpret_cast<D11Device*>(&this->GetDevice());

    if (type != Resource::TYPE_BUFFER || ~hint & HINT_DYNAMIC_BUFFER)
    {
      return nullptr;
    }

    D3D11_MAPPED_SUBRESOURCE mapped_subres{ 0 };
    BLAST_ASSERT(S_OK == device->GetContext()->Map(this->resource, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped_subres));

    return mapped_subres.pData;
  }

  void D11Resource::Unmap()
  {
    D11Device* device = reinterpret_cast<D11Device*>(&this->GetDevice());

    if (type != Resource::TYPE_BUFFER || ~hint & HINT_DYNAMIC_BUFFER)
    {
      return;
    }

    device->GetContext()->Unmap(this->resource, 0);
  }

  D11Resource::D11Resource(const std::string& name,
    Device& device,
    const Resource::BufferDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    D11Resource::Initialize();
  }

  D11Resource::D11Resource(const std::string& name,
    Device& device,
    const Resource::Tex1DDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    D11Resource::Initialize();
  }

  D11Resource::D11Resource(const std::string& name,
    Device& device,
    const Resource::Tex2DDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    D11Resource::Initialize();
  }

  D11Resource::D11Resource(const std::string& name,
    Device& device,
    const Resource::Tex3DDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    D11Resource::Initialize();
  }

  D11Resource::~D11Resource()
  {
    D11Resource::Discard();
  }
}