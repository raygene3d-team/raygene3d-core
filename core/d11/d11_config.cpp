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
#include "d11_config.h"
#include "d11_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

#include <d3dcompiler.h>
#pragma comment (lib, "d3dcompiler.lib")

namespace RayGene3D
{
  //void D11Compile(const std::string& source, const wchar_t* entry, const wchar_t* target, std::vector<char>& bytecode)
  //{
  //  IDxcLibrary* library{ nullptr };
  //  BLAST_ASSERT(S_OK == DxcCreateInstance(CLSID_DxcLibrary, __uuidof(IDxcLibrary), (void**)(&library)));

  //  IDxcBlobEncoding* input;
  //  BLAST_ASSERT(S_OK == library->CreateBlobWithEncodingFromPinned(source.c_str(), uint32_t(source.size()), CP_UTF8, &input));

  //  if (library) { library->Release(); library = nullptr; }

  //  std::vector<const wchar_t*> args =
  //  {
  //    //L"-Zpr",			//Row-major matrices
  //    //L"-WX",				//Warnings as errors
  //    //L"-Gec",
  //    //L"-HV 2016",
  //    //L"-flegacy-macro-expansion",
  //    //L"-flegacy-resource-reservation",
  //#ifdef _DEBUG
  //    L"-Zi",				//Debug info
  //    L"-Od",				//Disable optimization
  //#else
  //    L"-O3",				//Optimization level 3
  //#endif
  //  };

  ////std::vector<DxcDefine> dxcDefines(defines.size());
  ////for (size_t i = 0; i < defines.size(); ++i)
  ////{
  ////  DxcDefine& m = dxcDefines(i);
  ////  m.Name = defines[i].first.c_str();
  ////  m.Value = defines[i].second.c_str();
  ////}

  //  IDxcCompiler* compiler{ nullptr };
  //  BLAST_ASSERT(S_OK == DxcCreateInstance(CLSID_DxcCompiler, __uuidof(IDxcCompiler), (void**)(&compiler)));

  //  IDxcOperationResult* result{ nullptr };
  //  BLAST_ASSERT(S_OK == compiler->Compile(input, nullptr, entry, target, args.data(), uint32_t(args.size()), nullptr, 0, nullptr, &result));
  //  input->Release();

  //  if (compiler) { compiler->Release(); compiler = nullptr; }

  //  HRESULT hr{ S_OK };
  //  BLAST_ASSERT(S_OK == result->GetStatus(&hr));
  //  if (hr != S_OK)
  //  {
  //    IDxcBlobEncoding* error{ nullptr };
  //    BLAST_ASSERT(S_OK == result->GetErrorBuffer(&error));
  //    BLAST_LOG("DXCompiler output: \n%s", reinterpret_cast<char*>(error->GetBufferPointer()));
  //    if(error) error->Release();
  //    return;
  //  }

  //  IDxcBlob* output{ nullptr };
  //  result->GetResult(&output);

  //  bytecode.assign(reinterpret_cast<char*>(output->GetBufferPointer()), reinterpret_cast<char*>(output->GetBufferPointer()) + output->GetBufferSize());
  //  output->Release();
  //}

  class D11Includer : public ID3DInclude
  {
  private:
    std::string path;

  public:
    HRESULT Open(D3D_INCLUDE_TYPE IncludeType, LPCSTR pFileName, LPCVOID pParentData, LPCVOID* ppData, UINT* pBytes) override
    {
      const auto file_path = path + std::string(pFileName);

      std::fstream fs;
      fs.open(file_path, std::fstream::in);
      std::stringstream ss;
      ss << fs.rdbuf();

      const auto size = ss.str().size();
      const auto data = new char[size];
      BLAST_ASSERT(data);

      memcpy(data, ss.str().data(), size);

      *ppData = data;
      *pBytes = size;

      return S_OK;
    }

    HRESULT Close(LPCVOID pData) override
    {
      if (pData)
      {
        delete[] pData;
      }

      return S_OK;
    }

  public:
    D11Includer(const std::string& path) : ID3DInclude(), path(path) {}
    virtual ~D11Includer() {}
  };


  static void D11Compile(const std::string& source, const char* entry, const char* target, 
    std::map<std::string, std::string> defines, const std::string& path, std::vector<char>& bytecode)
  {


    D11Includer includer(path);

    const uint32_t flags{ D3DCOMPILE_PREFER_FLOW_CONTROL | D3DCOMPILE_ENABLE_STRICTNESS | D3DCOMPILE_IEEE_STRICTNESS | D3DCOMPILE_OPTIMIZATION_LEVEL3 };

    const auto limit = 32u;
    D3D_SHADER_MACRO macros[limit] = { 0 };

    auto count = 0;
    for (const auto& define : defines)
    {
      macros[count++] = { define.first.c_str(), define.second.c_str() };
      if (count == limit) break;
    }

    ID3DBlob* errors = nullptr;
    ID3DBlob* shader = nullptr;
    HRESULT hr = D3DCompile(source.c_str(), source.size(), nullptr, macros, &includer, entry, target, flags, 0, &shader, &errors);
    if (errors)
    {
      BLAST_LOG("shader compilation output: \n%s", reinterpret_cast<char*>(errors->GetBufferPointer()));
      errors->Release();
    }

    if (shader)
    {
      bytecode.assign(reinterpret_cast<char*>(shader->GetBufferPointer()), reinterpret_cast<char*>(shader->GetBufferPointer()) + shader->GetBufferSize());
      shader->Release();
    }
  }

  void D11Config::Initialize()
  {
    auto pass = reinterpret_cast<D11Pass*>(&this->GetPass());
    auto device = reinterpret_cast<D11Device*>(&pass->GetDevice());

    const auto& path = device->GetPath();

    cs_bytecode.clear();
    vs_bytecode.clear();
    hs_bytecode.clear();
    ds_bytecode.clear();
    gs_bytecode.clear();
    ps_bytecode.clear();

    if (compilation & COMPILATION_CS) { D11Compile(source, "cs_main", "cs_5_0", defines, path, cs_bytecode); BLAST_ASSERT(!cs_bytecode.empty()); }
    if (compilation & COMPILATION_VS) { D11Compile(source, "vs_main", "vs_5_0", defines, path, vs_bytecode); BLAST_ASSERT(!vs_bytecode.empty()); }
    if (compilation & COMPILATION_HS) { D11Compile(source, "hs_main", "hs_5_0", defines, path, hs_bytecode); BLAST_ASSERT(!hs_bytecode.empty()); }
    if (compilation & COMPILATION_DS) { D11Compile(source, "ds_main", "ds_5_0", defines, path, ds_bytecode); BLAST_ASSERT(!ds_bytecode.empty()); }
    if (compilation & COMPILATION_GS) { D11Compile(source, "gs_main", "gs_5_0", defines, path, gs_bytecode); BLAST_ASSERT(!gs_bytecode.empty()); }
    if (compilation & COMPILATION_PS) { D11Compile(source, "ps_main", "ps_5_0", defines, path, ps_bytecode); BLAST_ASSERT(!ps_bytecode.empty()); }

    const auto get_format = [this](Format format)
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

    if (ia_state.topology != TOPOLOGY_UNKNOWN)
    {
      if (!ia_state.attributes.empty())
      {
        auto stride_max = 0u;
        std::map<uint32_t, uint32_t> stride_map;

        std::vector<D3D11_INPUT_ELEMENT_DESC> element_descs(ia_state.attributes.size(), { 0 });
        for (size_t i = 0; i < ia_state.attributes.size(); ++i)
        {
          element_descs[i].SemanticName = "register";
          element_descs[i].SemanticIndex = uint32_t(i);
          element_descs[i].Format = get_format(ia_state.attributes[i].format);
          element_descs[i].InputSlot = ia_state.attributes[i].slot;
          element_descs[i].AlignedByteOffset = ia_state.attributes[i].offset;
          element_descs[i].InputSlotClass = ia_state.attributes[i].instance ? D3D11_INPUT_PER_INSTANCE_DATA : D3D11_INPUT_PER_VERTEX_DATA;
          element_descs[i].InstanceDataStepRate = 0;

          stride_map[ia_state.attributes[i].slot] = ia_state.attributes[i].stride;
          stride_max = std::max(stride_max, ia_state.attributes[i].slot);
        }


        strides.resize(stride_max + 1, 0);
        for (const auto& stride_item : stride_map)
        {
          strides[stride_item.first] = stride_item.second;
        }

        BLAST_ASSERT(S_OK == device->GetDevice()->CreateInputLayout(element_descs.data(), uint32_t(element_descs.size()),
          vs_bytecode.data(), vs_bytecode.size(), &input_layout));
      }

      const auto get_fill = [](Fill fill)
      {
        switch (fill)
        {
        default:           return D3D11_FILL_SOLID;
        case FILL_POINT:   return D3D11_FILL_SOLID; // Not implemented
        case FILL_LINE:    return D3D11_FILL_WIREFRAME;
        case FILL_SOLID:   return D3D11_FILL_SOLID;
        }
      };

      const auto get_cull = [](Cull cull)
      {
        switch (cull)
        {
        default:           return D3D11_CULL_BACK;
        case CULL_NONE:    return D3D11_CULL_NONE;
        case CULL_FRONT:   return D3D11_CULL_FRONT;
        case CULL_BACK:    return D3D11_CULL_BACK;
        }

      };

      D3D11_RASTERIZER_DESC raster_desc;
      raster_desc.FillMode = get_fill(rc_state.fill_mode);
      raster_desc.CullMode = get_cull(rc_state.cull_mode);
      raster_desc.FrontCounterClockwise = false;
      raster_desc.DepthBias = this->rc_state.depth_bias;
      raster_desc.DepthBiasClamp = this->rc_state.bias_clamp;
      raster_desc.SlopeScaledDepthBias = this->rc_state.bias_slope;
      raster_desc.DepthClipEnable = this->rc_state.clip_enabled;
      raster_desc.ScissorEnable = this->rc_state.scissor_enabled;
      raster_desc.MultisampleEnable = this->rc_state.multisample_enabled;
      raster_desc.AntialiasedLineEnable = false;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateRasterizerState(&raster_desc, &raster_state));



      const auto get_comparison = [](Comparison comparison)
      {
        switch (comparison)
        {
        default:                          return D3D11_COMPARISON_ALWAYS;
        case COMPARISON_NEVER:            return D3D11_COMPARISON_NEVER;
        case COMPARISON_LESS:             return D3D11_COMPARISON_LESS;
        case COMPARISON_EQUAL:            return D3D11_COMPARISON_EQUAL;
        case COMPARISON_LESS_EQUAL:       return D3D11_COMPARISON_LESS_EQUAL;
        case COMPARISON_GREATER:          return D3D11_COMPARISON_GREATER;
        case COMPARISON_NOT_EQUAL:        return D3D11_COMPARISON_NOT_EQUAL;
        case COMPARISON_GREATER_EQUAL:    return D3D11_COMPARISON_GREATER_EQUAL;
        case COMPARISON_ALWAYS:           return D3D11_COMPARISON_ALWAYS;
        }
      };

      const auto get_action = [](Action action)
      {
        switch (action)
        {
        default:                return D3D11_STENCIL_OP_KEEP;
        case ACTION_KEEP:       return D3D11_STENCIL_OP_KEEP;
        case ACTION_ZERO:       return D3D11_STENCIL_OP_ZERO;
        case ACTION_REPLACE:    return D3D11_STENCIL_OP_REPLACE;
        case ACTION_INCR_SAT:   return D3D11_STENCIL_OP_INCR_SAT;
        case ACTION_DECR_SAT:   return D3D11_STENCIL_OP_DECR_SAT;
        case ACTION_INVERT:     return D3D11_STENCIL_OP_INVERT;
        case ACTION_INCR:       return D3D11_STENCIL_OP_INCR;
        case ACTION_DECR:       return D3D11_STENCIL_OP_DECR;
        }
      };

      D3D11_DEPTH_STENCIL_DESC depth_desc;
      depth_desc.DepthEnable = ds_state.depth_enabled;
      depth_desc.DepthWriteMask = ds_state.depth_write ? D3D11_DEPTH_WRITE_MASK_ALL : D3D11_DEPTH_WRITE_MASK_ZERO;
      depth_desc.DepthFunc = get_comparison(ds_state.depth_comparison);
      depth_desc.StencilEnable = ds_state.stencil_enabled;
      depth_desc.StencilReadMask = ds_state.stencil_rmask;
      depth_desc.StencilWriteMask = ds_state.stencil_wmask;
      depth_desc.FrontFace.StencilFunc = get_comparison(ds_state.stencil_fface_mode.comparison);
      depth_desc.FrontFace.StencilDepthFailOp = get_action(ds_state.stencil_fface_mode.depth_fail);
      depth_desc.FrontFace.StencilFailOp = get_action(ds_state.stencil_fface_mode.stencil_fail);
      depth_desc.FrontFace.StencilPassOp = get_action(ds_state.stencil_fface_mode.stencil_pass);
      depth_desc.BackFace.StencilFunc = get_comparison(ds_state.stencil_bface_mode.comparison);
      depth_desc.BackFace.StencilDepthFailOp = get_action(ds_state.stencil_bface_mode.depth_fail);
      depth_desc.BackFace.StencilFailOp = get_action(ds_state.stencil_bface_mode.stencil_fail);
      depth_desc.BackFace.StencilPassOp = get_action(ds_state.stencil_bface_mode.stencil_pass);
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateDepthStencilState(&depth_desc, &depth_state));




      const auto get_operand = [](Operand operand)
      {
        switch (operand)
        {
        default:                         return D3D11_BLEND_ZERO;
        case OPERAND_ZERO:               return D3D11_BLEND_ZERO;
        case OPERAND_ONE:                return D3D11_BLEND_ONE;
        case OPERAND_SRC_COLOR:          return D3D11_BLEND_SRC_COLOR;
        case OPERAND_INV_SRC_COLOR:      return D3D11_BLEND_INV_SRC_COLOR;
        case OPERAND_SRC_ALPHA:          return D3D11_BLEND_SRC_ALPHA;
        case OPERAND_INV_SRC_ALPHA:      return D3D11_BLEND_INV_SRC_ALPHA;
        case OPERAND_DEST_ALPHA:         return D3D11_BLEND_DEST_ALPHA;
        case OPERAND_INV_DEST_ALPHA:     return D3D11_BLEND_INV_DEST_ALPHA;
        case OPERAND_DEST_COLOR:         return D3D11_BLEND_DEST_COLOR;
        case OPERAND_INV_DEST_COLOR:     return D3D11_BLEND_INV_DEST_COLOR;
        case OPERAND_SRC_ALPHA_SAT:      return D3D11_BLEND_SRC_ALPHA_SAT;
        case OPERAND_BLEND_FACTOR:       return D3D11_BLEND_BLEND_FACTOR;
        case OPERAND_INV_BLEND_FACTOR:   return D3D11_BLEND_INV_BLEND_FACTOR;
        }
      };


      const auto get_operation = [](Operation operation)
      {
        switch (operation)
        {
        default:                        return D3D11_BLEND_OP_ADD;
        case OPERATION_ADD:             return D3D11_BLEND_OP_ADD;
        case OPERATION_SUBTRACT:        return D3D11_BLEND_OP_SUBTRACT;
        case OPERATION_REV_SUBTRACT:    return D3D11_BLEND_OP_REV_SUBTRACT;
        case OPERATION_MIN:             return D3D11_BLEND_OP_MIN;
        case OPERATION_MAX:             return D3D11_BLEND_OP_MAX;
        }
      };

      D3D11_BLEND_DESC blend_desc;
      blend_desc.AlphaToCoverageEnable = om_state.atc_enabled;
      blend_desc.IndependentBlendEnable = false; //TODO: Implement true only
      for (uint32_t i = 0; i < std::min(uint32_t(om_state.target_blends.size()), 8u); ++i)
      {
        blend_desc.RenderTarget[i].BlendEnable = om_state.target_blends[i].blend_enabled;
        blend_desc.RenderTarget[i].SrcBlend = get_operand(om_state.target_blends[i].src_color);
        blend_desc.RenderTarget[i].DestBlend = get_operand(om_state.target_blends[i].dst_color);
        blend_desc.RenderTarget[i].BlendOp = get_operation(om_state.target_blends[i].blend_color);
        blend_desc.RenderTarget[i].SrcBlendAlpha = get_operand(om_state.target_blends[i].src_alpha);
        blend_desc.RenderTarget[i].DestBlendAlpha = get_operand(om_state.target_blends[i].dst_alpha);
        blend_desc.RenderTarget[i].BlendOpAlpha = get_operation(om_state.target_blends[i].blend_alpha);
        blend_desc.RenderTarget[i].RenderTargetWriteMask = om_state.target_blends[i].write_mask;
      }
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateBlendState(&blend_desc, &blend_state));
    }

    const auto get_topology = [](Topology topology)
    {
      switch (topology)
      {
      default:                          return D3D_PRIMITIVE_TOPOLOGY_UNDEFINED;
      case TOPOLOGY_POINTLIST:          return D3D_PRIMITIVE_TOPOLOGY_POINTLIST;
      case TOPOLOGY_LINELIST:           return D3D_PRIMITIVE_TOPOLOGY_LINELIST;
      case TOPOLOGY_LINESTRIP:          return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP;
      case TOPOLOGY_TRIANGLELIST:       return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
      case TOPOLOGY_TRIANGLESTRIP:      return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP;
      case TOPOLOGY_LINELIST_ADJ:       return D3D_PRIMITIVE_TOPOLOGY_LINELIST_ADJ;
      case TOPOLOGY_LINESTRIP_ADJ:      return D3D_PRIMITIVE_TOPOLOGY_LINESTRIP_ADJ;
      case TOPOLOGY_TRIANGLELIST_ADJ:   return D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ;
      case TOPOLOGY_TRIANGLESTRIP_ADJ:  return D3D_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP_ADJ;
      }
    };

    primitive_topology = get_topology(ia_state.topology);

    const auto vp_count = uint32_t(rc_state.viewports.size());
    vp_items.resize(std::min(vp_count, uint32_t(D3D11_VIEWPORT_AND_SCISSORRECT_MAX_INDEX)), { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
    for (uint32_t i = 0; i < uint32_t(vp_items.size()); ++i)
    {
      if (i < vp_count)
      {
        const auto& viewport = rc_state.viewports[i];
        vp_items[i] = { viewport.origin_x, viewport.origin_y, viewport.extent_x, viewport.extent_y, viewport.min_z, viewport.max_z };
      }
    }

    if (!vs_bytecode.empty())
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateVertexShader(vs_bytecode.data(), vs_bytecode.size(), nullptr, &vs_shader));
    }

    if (!hs_bytecode.empty())
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateHullShader(hs_bytecode.data(), hs_bytecode.size(), nullptr, &hs_shader));
    }

    if (!ds_bytecode.empty())
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateDomainShader(ds_bytecode.data(), ds_bytecode.size(), nullptr, &ds_shader));
    }

    if (!gs_bytecode.empty())
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateGeometryShader(gs_bytecode.data(), gs_bytecode.size(), nullptr, &gs_shader));
    }

    if (!ps_bytecode.empty())
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreatePixelShader(ps_bytecode.data(), ps_bytecode.size(), nullptr, &ps_shader));
    }

    if (!cs_bytecode.empty())
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateComputeShader(cs_bytecode.data(), cs_bytecode.size(), nullptr, &cs_shader));
    }
  }

  void D11Config::Use()
  {
    auto pass = reinterpret_cast<D11Pass*>(&this->GetPass());
    auto device = reinterpret_cast<D11Device*>(&pass->GetDevice());
    
     
    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {

      device->GetContext()->VSSetShader(vs_shader, nullptr, 0);
      device->GetContext()->HSSetShader(hs_shader, nullptr, 0);
      device->GetContext()->DSSetShader(ds_shader, nullptr, 0);
      device->GetContext()->GSSetShader(gs_shader, nullptr, 0);
      device->GetContext()->PSSetShader(ps_shader, nullptr, 0);

      device->GetContext()->IASetPrimitiveTopology(primitive_topology);
      device->GetContext()->IASetInputLayout(input_layout);

      device->GetContext()->RSSetState(raster_state);

      uint32_t stencil_reference = 0xFFFFFFFF;
      device->GetContext()->OMSetDepthStencilState(depth_state, stencil_reference);

      float blend_factors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
      uint32_t sample_mask = 0xFFFFFFFF;
      device->GetContext()->OMSetBlendState(blend_state, blend_factors, sample_mask);

      device->GetContext()->RSSetViewports(vp_items.size(), vp_items.data());
    }

    if (pass->GetType() == Pass::TYPE_COMPUTE)
    {
      device->GetContext()->CSSetShader(cs_shader, nullptr, 0);
    }

    for (const auto& batch : batches)
    {
      batch->Use();
    }
  }

  void D11Config::Discard()
  {
    if (raster_state)
    {
      raster_state->Release();
      raster_state = nullptr;
    }

    if (depth_state)
    {
      depth_state->Release();
      depth_state = nullptr;
    }

    if (blend_state)
    {
      blend_state->Release();
      blend_state = nullptr;
    }

    if (input_layout)
    {
      input_layout->Release();
      input_layout = nullptr;
    }

    if (cs_shader)
    {
      cs_shader->Release();
      cs_shader = nullptr;
    }

    if (ps_shader)
    {
      ps_shader->Release();
      ps_shader = nullptr;
    }

    if (gs_shader)
    {
      gs_shader->Release();
      gs_shader = nullptr;
    }

    if (ds_shader)
    {
      ds_shader->Release();
      ds_shader = nullptr;
    }

    if (hs_shader)
    {
      hs_shader->Release();
      hs_shader = nullptr;
    }

    if (vs_shader)
    {
      vs_shader->Release();
      vs_shader = nullptr;
    }
  }

  D11Config::D11Config(const std::string& name,
    Pass& pass,
    const std::string& source, Config::Compilation compilation,
    const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
    const Config::IAState& ia_state,
    const Config::RCState& rc_state,
    const Config::DSState& ds_state,
    const Config::OMState& om_state)
    : Config(name, pass, source, compilation, defines, ia_state, rc_state, ds_state, om_state)
  {
    D11Config::Initialize();
  }

  D11Config::D11Config(const std::string& name,
    Pass& pass,
    const std::string& source, Config::Compilation compilation,
    const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
    const Config::RCState& rc_state,
    const Config::DSState& ds_state,
    const Config::OMState& om_state)
    : Config(name, pass, source, compilation, defines, rc_state, ds_state, om_state)
  {
    D11Config::Initialize();
  }

  D11Config::D11Config(const std::string& name,
    Pass& pass,
    const std::string& source, Config::Compilation compilation,
    const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines)
    : Config(name, pass, source, compilation, defines)
  {
    D11Config::Initialize();
  }

  D11Config::~D11Config()
  {
    D11Config::Discard();
  }
}