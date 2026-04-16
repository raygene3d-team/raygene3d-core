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
#include "d12_config.h"
#include "d12_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

#include <dxc/dxcapi.h>
#pragma comment (lib, "dxcompiler.lib")

namespace RayGene3D
{
  class D12Includer : public IDxcIncludeHandler
  {
  private:
    IDxcUtils* utils;
    std::string path;
    std::unordered_set<std::string> includees;

  public:
    virtual HRESULT LoadSource(LPCWSTR w_name, IDxcBlob** blob) override
    {
      IDxcBlobEncoding* encoding{ nullptr };

      auto name = new char[256];
      wcstombs(name, w_name, 256);
      const auto includee = path + std::string(name);

      if (includees.find(includee) != includees.cend())
      {
        static const char dummy[] = " ";
        BLAST_ASSERT(S_OK == utils->CreateBlobFromPinned(dummy, std::size(dummy), DXC_CP_ACP, &encoding));
      }
      else
      {
        wchar_t* w_includee = new wchar_t[256];
        mbstowcs(w_includee, includee.c_str(), 256);

        BLAST_ASSERT(S_OK == utils->LoadFile(w_includee, DXC_CP_ACP, &encoding));
        includees.insert(includee);
      }

      *blob = encoding;
      return S_OK;
    }

  public:
    virtual ULONG AddRef() override { return 0; };
    virtual ULONG Release() override { return 0; };
    virtual HRESULT QueryInterface(REFIID riid, void** ppvObject) override { return E_NOINTERFACE; }

  public:
    D12Includer(IDxcUtils* utils, const std::string& path) : IDxcIncludeHandler(), utils(utils), path(path) {}
    virtual ~D12Includer() {}
  };

  static void D12Compile(const char* name, const std::string& source, const std::string& path, const std::string& file,
    const char* entry, const char* target, std::map<std::string, std::string> defines, std::vector<char>& bytecode)
  {
    wchar_t* w_name = new wchar_t[64];
    mbstowcs(w_name, name, 64);

    wchar_t* w_filepath = new wchar_t[256];
    mbstowcs(w_filepath, (path + file).c_str(), 256);

    wchar_t* w_entry = new wchar_t[16];
    mbstowcs(w_entry, entry, 16);

    wchar_t* w_target = new wchar_t[16];
    mbstowcs(w_target, target, 16);

    IDxcUtils* utils{ nullptr };
    BLAST_ASSERT(S_OK == DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils)));

    IDxcIncludeHandler* include_handler{ nullptr };
    include_handler = new D12Includer(utils, path);
    include_handler->AddRef();

    IDxcBlobEncoding* blob_encoding{ nullptr };
     if (source.empty())
     {
       BLAST_ASSERT(S_OK == utils->LoadFile(w_filepath, DXC_CP_ACP, &blob_encoding));
     }
     else
     {
       BLAST_ASSERT(S_OK == utils->CreateBlobFromPinned(source.c_str(), uint32_t(source.size()), DXC_CP_ACP, &blob_encoding));
     }

    //auto data = encoding->GetBufferPointer();
    //auto size = encoding->GetBufferSize();

    const wchar_t* args[] =
    {
      //L"-Zpr",			//Row-major matrices
      //L"-WX",				//Warnings as errors
      //L"-Gec",
      //L"-HV 2016",
      //L"-flegacy-macro-expansion",
      //L"-flegacy-resource-reservation",
  #ifdef _DEBUG
      L"-Zi",				//Debug info
      L"-Od",				//Disable optimization
      L"-Qembed_debug",
  #else
      L"-O3",				//Optimization level 3
  #endif
    };

  //std::vector<DxcDefine> dxcDefines(defines.size());
  //for (size_t i = 0; i < defines.size(); ++i)
  //{
  //  DxcDefine& m = dxcDefines(i);
  //  m.Name = defines[i].first.c_str();
  //  m.Value = defines[i].second.c_str();
  //}

    IDxcCompiler* compiler{ nullptr };
    BLAST_ASSERT(S_OK == DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));

    IDxcOperationResult* result{ nullptr };
    BLAST_ASSERT(S_OK == compiler->Compile(blob_encoding, 
      w_name, w_entry, w_target,
      args, std::size(args), 
      nullptr, 0, 
      include_handler, &result));

    if (compiler) { compiler->Release(); compiler = nullptr; }
    if (blob_encoding) { blob_encoding->Release(); blob_encoding = nullptr; }
    if (include_handler) { include_handler->Release(); include_handler = nullptr; }

    if (utils) { utils->Release(); utils = nullptr; }


    HRESULT hr{ S_OK };
    BLAST_ASSERT(S_OK == result->GetStatus(&hr));
    if (hr != S_OK)
    {
      IDxcBlobEncoding* error{ nullptr };
      BLAST_ASSERT(S_OK == result->GetErrorBuffer(&error));
      BLAST_LOG("DXCompiler output: \n%s", reinterpret_cast<char*>(error->GetBufferPointer()));
      if(error) error->Release();
      return;
    }

    IDxcBlob* output{ nullptr };
    result->GetResult(&output);

    bytecode.assign(reinterpret_cast<char*>(output->GetBufferPointer()), reinterpret_cast<char*>(output->GetBufferPointer()) + output->GetBufferSize());
    output->Release();
  }



  //static void D12Compile(const char* name, const std::string& source, const std::string& path, const std::string& file,
  //  const char* entry, const char* target, std::map<std::string, std::string> defines, std::vector<char>& bytecode)
  //{
  //  wchar_t* w_name = new wchar_t[64];
  //  mbstowcs(w_name, name, 64);

  //  wchar_t* w_filepath = new wchar_t[256];
  //  mbstowcs(w_filepath, (path + file).c_str(), 256);

  //  IDxcLibrary* library{ nullptr };
  //  BLAST_ASSERT(S_OK == DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&library)));

  //  IDxcBlobEncoding* blob_encoding{ nullptr };
  //  if (source.empty())
  //  {
  //    BLAST_ASSERT(S_OK == library->CreateBlobFromFile(w_filepath, DXC_CP_ACP, &blob_encoding));
  //  }
  //  else
  //  {
  //    BLAST_ASSERT(S_OK == library->CreateBlobWithEncodingFromPinned(source.c_str(), uint32_t(source.size()), DXC_CP_ACP, &blob_encoding));
  //  }

  //  IDxcIncludeHandler* include_handler{ nullptr };
  //  BLAST_ASSERT(S_OK == library->CreateIncludeHandler(&include_handler));

  //  IDxcCompiler3* compiler{ nullptr };
  //  BLAST_ASSERT(S_OK == DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler)));

  //  const wchar_t* args[] =
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
  //    L"-Qembed_debug"
  //#else
  //    L"-O3",				//Optimization level 3
  //#endif
  //  };

  //  //std::vector<DxcDefine> dxcDefines(defines.size());
  //  //for (size_t i = 0; i < defines.size(); ++i)
  //  //{
  //  //  DxcDefine& m = dxcDefines(i);
  //  //  m.Name = defines[i].first.c_str();
  //  //  m.Value = defines[i].second.c_str();
  //  //}

  //  wchar_t* w_entry = new wchar_t[16];
  //  mbstowcs(w_entry, entry, 16);

  //  wchar_t* w_target = new wchar_t[16];
  //  mbstowcs(w_target, target, 16);

  //  IDxcOperationResult* operation_result{ nullptr };
  //  BLAST_ASSERT(S_OK == compiler->Compile(blob_encoding, 
  //    w_name, w_entry, w_target,
  //    args, std::size(args),
  //    nullptr, 0,
  //    include_handler, &operation_result));

  //  if (compiler) { compiler->Release(); compiler = nullptr; }
  //  if (blob_encoding) { blob_encoding->Release(); blob_encoding = nullptr; }
  //  if (include_handler) { include_handler->Release(); include_handler = nullptr; }

  //  HRESULT hr{ S_OK };
  //  BLAST_ASSERT(S_OK == operation_result->GetStatus(&hr));
  //  if (hr != S_OK)
  //  {
  //    IDxcBlobEncoding* error{ nullptr };
  //    BLAST_ASSERT(S_OK == operation_result->GetErrorBuffer(&error));
  //    BLAST_LOG("DXCompiler output: \n%s", reinterpret_cast<char*>(error->GetBufferPointer()));
  //    if (error) error->Release();
  //    return;
  //  }

  //  IDxcBlob* output{ nullptr };
  //  operation_result->GetResult(&output);

  //  bytecode.assign(reinterpret_cast<char*>(output->GetBufferPointer()), reinterpret_cast<char*>(output->GetBufferPointer()) + output->GetBufferSize());
  //  output->Release();
  //}


  void D12Config::Initialize()
  {
    auto pass = reinterpret_cast<D12Pass*>(&this->GetPass());
    auto device = reinterpret_cast<D12Device*>(&pass->GetDevice());

    const auto& path = device->GetPath();

    comp_bytecode.clear();
    vert_bytecode.clear();
    tesc_bytecode.clear();
    tese_bytecode.clear();
    geom_bytecode.clear();
    frag_bytecode.clear();
    if (compilation & COMPILATION_COMP) { D12Compile(name.c_str(), source, path, file, "cs_main", "cs_6_0", defines, comp_bytecode); BLAST_ASSERT(!comp_bytecode.empty()); }
    if (compilation & COMPILATION_VERT) { D12Compile(name.c_str(), source, path, file, "vs_main", "vs_6_0", defines, vert_bytecode); BLAST_ASSERT(!vert_bytecode.empty()); }
    if (compilation & COMPILATION_TESC) { D12Compile(name.c_str(), source, path, file, "hs_main", "hs_6_0", defines, tesc_bytecode); BLAST_ASSERT(!tesc_bytecode.empty()); }
    if (compilation & COMPILATION_TESE) { D12Compile(name.c_str(), source, path, file, "ds_main", "ds_6_0", defines, tese_bytecode); BLAST_ASSERT(!tese_bytecode.empty()); }
    if (compilation & COMPILATION_GEOM) { D12Compile(name.c_str(), source, path, file, "gs_main", "gs_6_0", defines, geom_bytecode); BLAST_ASSERT(!geom_bytecode.empty()); }
    if (compilation & COMPILATION_FRAG) { D12Compile(name.c_str(), source, path, file, "ps_main", "ps_6_0", defines, frag_bytecode); BLAST_ASSERT(!frag_bytecode.empty()); }

    task_bytecode.clear();
    mesh_bytecode.clear();
    rgen_bytecode.clear();
    call_bytecode.clear();
    isec_bytecode.clear();
    chit_bytecode.clear();
    ahit_bytecode.clear();
    miss_bytecode.clear();
    if (compilation & COMPILATION_TASK) { D12Compile(name.c_str(), source, path, file, "task_main", "as_6_5", defines, task_bytecode); BLAST_ASSERT(!task_bytecode.empty());}
    if (compilation & COMPILATION_MESH) { D12Compile(name.c_str(), source, path, file, "mesh_main", "ms_6_5", defines, mesh_bytecode); BLAST_ASSERT(!mesh_bytecode.empty()); }
    if (compilation & COMPILATION_RGEN) { D12Compile(name.c_str(), source, path, file, "rgen", "lib_6_3", defines, rgen_bytecode); BLAST_ASSERT(!rgen_bytecode.empty()); }
    if (compilation & COMPILATION_CALL) { D12Compile(name.c_str(), source, path, file, "call", "lib_6_3", defines, call_bytecode); BLAST_ASSERT(!call_bytecode.empty()); }
    if (compilation & COMPILATION_ISEC) { D12Compile(name.c_str(), source, path, file, "isec", "lib_6_3", defines, isec_bytecode); BLAST_ASSERT(!isec_bytecode.empty()); }
    if (compilation & COMPILATION_CHIT) { D12Compile(name.c_str(), source, path, file, "chit", "lib_6_3", defines, chit_bytecode); BLAST_ASSERT(!chit_bytecode.empty()); }
    if (compilation & COMPILATION_AHIT) { D12Compile(name.c_str(), source, path, file, "ahit", "lib_6_3", defines, ahit_bytecode); BLAST_ASSERT(!ahit_bytecode.empty()); }
    if (compilation & COMPILATION_MISS) { D12Compile(name.c_str(), source, path, file, "miss", "lib_6_3", defines, miss_bytecode); BLAST_ASSERT(!miss_bytecode.empty()); }

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


    auto stride_max = 0u;
    std::map<uint32_t, uint32_t> stride_map;

    element_descs.resize(ia_state.attributes.size());
    for (size_t i = 0; i < element_descs.size(); ++i)
    {
      element_descs[i].SemanticName = "register";
      element_descs[i].SemanticIndex = uint32_t(i);
      element_descs[i].Format = get_format(ia_state.attributes[i].format);
      element_descs[i].InputSlot = ia_state.attributes[i].slot;
      element_descs[i].AlignedByteOffset = ia_state.attributes[i].offset;
      element_descs[i].InputSlotClass = ia_state.attributes[i].instance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
      element_descs[i].InstanceDataStepRate = 0;

      stride_map[ia_state.attributes[i].slot] = ia_state.attributes[i].stride;
      stride_max = std::max(stride_max, ia_state.attributes[i].slot);
    }

    //strides.resize(stride_max + 1, 0);
    //for (const auto& stride_item : stride_map)
    //{
    //  strides[stride_item.first] = stride_item.second;
    //}


    const auto get_fill = [](Fill fill)
    {
      switch (fill)
      {
      default:           return (D3D12_FILL_MODE)0;
      case FILL_POINT:   return (D3D12_FILL_MODE)1; // Not implemented
      case FILL_LINE:    return D3D12_FILL_MODE_WIREFRAME;
      case FILL_SOLID:   return D3D12_FILL_MODE_SOLID;
      }
    };

    const auto get_cull = [](Cull cull)
    {
      switch (cull)
      {
      default:           return (D3D12_CULL_MODE)0;
      case CULL_NONE:    return D3D12_CULL_MODE_NONE;
      case CULL_FRONT:   return D3D12_CULL_MODE_FRONT;
      case CULL_BACK:    return D3D12_CULL_MODE_BACK;
      }

    };

      
    raster_desc.FillMode = get_fill(rc_state.fill_mode);
    raster_desc.CullMode = get_cull(rc_state.cull_mode);
    raster_desc.FrontCounterClockwise = false;
    raster_desc.DepthBias = this->rc_state.depth_bias;
    raster_desc.DepthBiasClamp = this->rc_state.bias_clamp;
    raster_desc.SlopeScaledDepthBias = this->rc_state.bias_slope;
    raster_desc.DepthClipEnable = this->rc_state.clip_enabled;
    //raster_desc.ScissorEnable = this->rc_state.scissor_enabled;
    raster_desc.MultisampleEnable = this->rc_state.multisample_enabled;
    raster_desc.AntialiasedLineEnable = false;
    //BLAST_ASSERT(S_OK == device->GetDevice()->CreateRasterizerState(&raster_desc, &raster_state));



    const auto get_comparison = [](Comparison comparison)
    {
      switch (comparison)
      {
      default:                          return D3D12_COMPARISON_FUNC_ALWAYS;
      case COMPARISON_NEVER:            return D3D12_COMPARISON_FUNC_NEVER;
      case COMPARISON_LESS:             return D3D12_COMPARISON_FUNC_LESS;
      case COMPARISON_EQUAL:            return D3D12_COMPARISON_FUNC_EQUAL;
      case COMPARISON_LESS_EQUAL:       return D3D12_COMPARISON_FUNC_LESS_EQUAL;
      case COMPARISON_GREATER:          return D3D12_COMPARISON_FUNC_GREATER;
      case COMPARISON_NOT_EQUAL:        return D3D12_COMPARISON_FUNC_NOT_EQUAL;
      case COMPARISON_GREATER_EQUAL:    return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
      case COMPARISON_ALWAYS:           return D3D12_COMPARISON_FUNC_ALWAYS;
      }
    };

    const auto get_action = [](Action action)
    {
      switch (action)
      {
      default:                return D3D12_STENCIL_OP_KEEP;
      case ACTION_KEEP:       return D3D12_STENCIL_OP_KEEP;
      case ACTION_ZERO:       return D3D12_STENCIL_OP_ZERO;
      case ACTION_REPLACE:    return D3D12_STENCIL_OP_REPLACE;
      case ACTION_INCR_SAT:   return D3D12_STENCIL_OP_INCR_SAT;
      case ACTION_DECR_SAT:   return D3D12_STENCIL_OP_DECR_SAT;
      case ACTION_INVERT:     return D3D12_STENCIL_OP_INVERT;
      case ACTION_INCR:       return D3D12_STENCIL_OP_INCR;
      case ACTION_DECR:       return D3D12_STENCIL_OP_DECR;
      }
    };

      
    depth_desc.DepthEnable = ds_state.depth_enabled;
    depth_desc.DepthWriteMask = ds_state.depth_write ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
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
    //(S_OK == device->GetDevice()->CreateDepthStencilState(&depth_desc, &depth_state));




    const auto get_operand = [](Operand operand)
    {
      switch (operand)
      {
      default:                         return (D3D12_BLEND)0;
      case OPERAND_ZERO:               return D3D12_BLEND_ZERO;
      case OPERAND_ONE:                return D3D12_BLEND_ONE;
      case OPERAND_SRC_COLOR:          return D3D12_BLEND_SRC_COLOR;
      case OPERAND_INV_SRC_COLOR:      return D3D12_BLEND_INV_SRC_COLOR;
      case OPERAND_SRC_ALPHA:          return D3D12_BLEND_SRC_ALPHA;
      case OPERAND_INV_SRC_ALPHA:      return D3D12_BLEND_INV_SRC_ALPHA;
      case OPERAND_DEST_ALPHA:         return D3D12_BLEND_DEST_ALPHA;
      case OPERAND_INV_DEST_ALPHA:     return D3D12_BLEND_INV_DEST_ALPHA;
      case OPERAND_DEST_COLOR:         return D3D12_BLEND_DEST_COLOR;
      case OPERAND_INV_DEST_COLOR:     return D3D12_BLEND_INV_DEST_COLOR;
      case OPERAND_SRC_ALPHA_SAT:      return D3D12_BLEND_SRC_ALPHA_SAT;
      case OPERAND_BLEND_FACTOR:       return D3D12_BLEND_BLEND_FACTOR;
      case OPERAND_INV_BLEND_FACTOR:   return D3D12_BLEND_INV_BLEND_FACTOR;
      }
    };


    const auto get_operation = [](Operation operation)
    {
      switch (operation)
      {
      default:                        return (D3D12_BLEND_OP)0;
      case OPERATION_ADD:             return D3D12_BLEND_OP_ADD;
      case OPERATION_SUBTRACT:        return D3D12_BLEND_OP_SUBTRACT;
      case OPERATION_REV_SUBTRACT:    return D3D12_BLEND_OP_REV_SUBTRACT;
      case OPERATION_MIN:             return D3D12_BLEND_OP_MIN;
      case OPERATION_MAX:             return D3D12_BLEND_OP_MAX;
      }
    };

     
    blend_desc.AlphaToCoverageEnable = om_state.atc_enabled;
    blend_desc.IndependentBlendEnable = false; //TODO: Implement true only
    for (size_t i = 0; i < std::min(om_state.target_blends.size(), size_t(8u)); ++i)
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
    //BLAST_ASSERT(S_OK == device->GetDevice()->CreateBlendState(&blend_desc, &blend_state));
  //}
  
    const auto get_primitive_topology = [](Topology topology)
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
    primitive_topology = get_primitive_topology(ia_state.topology);
    
    const auto get_topology_type = [](Topology topology)
    {
      switch (topology)
      {
      default:                                  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_UNDEFINED;
      case TOPOLOGY_POINTLIST:                  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
      case TOPOLOGY_LINELIST:                   return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
      case TOPOLOGY_LINESTRIP:                  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE;
      case TOPOLOGY_TRIANGLELIST:               return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      case TOPOLOGY_TRIANGLESTRIP:              return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      case TOPOLOGY_LINELIST_ADJ:               return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      case TOPOLOGY_LINESTRIP_ADJ:              return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      case TOPOLOGY_TRIANGLELIST_ADJ:           return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      case TOPOLOGY_TRIANGLESTRIP_ADJ:          return D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
      case TOPOLOGY_1_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_2_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_3_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_4_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_5_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_6_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_7_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_8_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_9_CONTROL_POINT_PATCHLIST:  return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_10_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_11_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_12_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_13_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_14_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_15_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_16_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_17_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_18_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_19_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_20_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_21_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_22_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_23_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_24_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_25_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_26_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_27_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_28_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_29_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_30_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_31_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      case TOPOLOGY_32_CONTROL_POINT_PATCHLIST: return D3D12_PRIMITIVE_TOPOLOGY_TYPE_PATCH;
      }
    };
    topology_type = get_topology_type(ia_state.topology);

    use_mesh_pipeline = ia_state.topology == TOPOLOGY_UNKNOWN;

    const auto vp_count = rc_state.viewports.size();
    vp_items.resize(std::min(vp_count, size_t(D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX)), { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f });
    for (size_t i = 0; i < vp_items.size(); ++i)
    {
      if (i < vp_count)
      {
        const auto& viewport = rc_state.viewports[i];
        vp_items[i] = { viewport.origin_x, viewport.origin_y, viewport.extent_x, viewport.extent_y, viewport.min_z, viewport.max_z };
      }
    }

    const auto sr_count = rc_state.viewports.size(); // Equal to viewport count in dx12;
    sr_items.resize(std::min(sr_count, size_t(D3D12_VIEWPORT_AND_SCISSORRECT_MAX_INDEX)), { 0, 0, 0, 0 });
    for (size_t i = 0; i < vp_items.size(); ++i)
    {
      if (i < sr_count)
      {
        sr_items[i] = { 0, 0, int32_t(vp_items[i].Width), int32_t(vp_items[i].Height) };
      }
    }

    if (ia_state.topology != TOPOLOGY_UNKNOWN)
    {
      if (!ia_state.attributes.empty())
      {
        auto stride_max = 0u;
        std::map<uint32_t, uint32_t> stride_map;

        std::vector<D3D12_INPUT_ELEMENT_DESC> element_descs(ia_state.attributes.size(), { 0 });
        for (size_t i = 0; i < ia_state.attributes.size(); ++i)
        {
          element_descs[i].SemanticName = "register";
          element_descs[i].SemanticIndex = uint32_t(i);
          element_descs[i].Format = get_format(ia_state.attributes[i].format);
          element_descs[i].InputSlot = ia_state.attributes[i].slot;
          element_descs[i].AlignedByteOffset = ia_state.attributes[i].offset;
          element_descs[i].InputSlotClass = ia_state.attributes[i].instance ? D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA : D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA;
          element_descs[i].InstanceDataStepRate = 0;

          stride_map[ia_state.attributes[i].slot] = ia_state.attributes[i].stride;
          stride_max = std::max(stride_max, ia_state.attributes[i].slot);
        }

        strides.resize(stride_max + 1, 0);
        for (const auto& stride_item : stride_map)
        {
          strides[stride_item.first] = stride_item.second;
        }
      }
    }
  }

  void D12Config::Use()
  {
    auto pass = reinterpret_cast<D12Pass*>(&this->GetPass());
    auto device = reinterpret_cast<D12Device*>(&pass->GetDevice());

    device->GetCommandList()->RSSetViewports(vp_items.size(), vp_items.data());
    device->GetCommandList()->RSSetScissorRects(sr_items.size(), sr_items.data());

    for (const auto& batch : batches)
    {
      batch->Use();
    }
  }

  void D12Config::Discard()
  {
  }

  D12Config::D12Config(const std::string& name,
    Pass& pass,
    const std::string& source,
    Config::Compilation compilation,
    const std::pair<const std::pair<std::string, std::string>*, size_t>& defines,
    const Config::IAState& ia_state,
    const Config::RCState& rc_state,
    const Config::DSState& ds_state,
    const Config::OMState& om_state)
    : Config(name, pass, source, compilation, defines, ia_state, rc_state, ds_state, om_state)
  {
    D12Config::Initialize();
  }

  D12Config::D12Config(const std::string& name,
    Pass& pass,
    const std::string& path,
    const std::string& file,
    Config::Compilation compilation,
    const std::pair<const std::pair<std::string, std::string>*, size_t>& defines,
    const Config::IAState& ia_state,
    const Config::RCState& rc_state,
    const Config::DSState& ds_state,
    const Config::OMState& om_state)
    : Config(name, pass, path, file, compilation, defines, ia_state, rc_state, ds_state, om_state)
  {
    D12Config::Initialize();
  }

  D12Config::~D12Config()
  {
    D12Config::Discard();
  }
}