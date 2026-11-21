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


#include "mtl_dxil.h"

#include <dxc/dxcapi.h>
#if RAYGENE_HAS_METAL_IRCONVERTER
#include <metal_irconverter/metal_irconverter.h>
#endif

#include <cstring>
#include <string>

namespace RayGene3D
{
  namespace
  {
    // Releases a COM object (IUnknown) when it goes out of scope.
    template <typename T>
    struct ComPtr
    {
      T* ptr{ nullptr };
      ComPtr() = default;
      ~ComPtr() { if (ptr) ptr->Release(); }
      ComPtr(const ComPtr&) = delete;
      ComPtr& operator=(const ComPtr&) = delete;
      T** operator&() { return &ptr; }
      T* operator->() const { return ptr; }
    };

    std::wstring ToWide(const std::string& text)
    {
      return std::wstring(text.begin(), text.end());
    }

    // Root signature for a ray tracing library: CBV b0..b(n-1) as root descriptors,
    // then one SRV table over t0..(n-1) and one UAV table over u0..(n-1).
    // The ranges vector must outlive the call, so it is owned by this function.
#if RAYGENE_HAS_METAL_IRCONVERTER
    IRRootSignature* CreateRayTracingRootSignature(const MTLDXILRayTracingLayout& layout, IRError** error)
    {
      std::vector<IRDescriptorRange1> ranges;
      if (layout.srv_count > 0)
      {
        IRDescriptorRange1 range{};
        range.RangeType = IRDescriptorRangeTypeSRV;
        range.NumDescriptors = layout.srv_count;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.Flags = IRDescriptorRangeFlagDataStatic;
        range.OffsetInDescriptorsFromTableStart = 0;
        ranges.push_back(range);
      }
      if (layout.uav_count > 0)
      {
        IRDescriptorRange1 range{};
        range.RangeType = IRDescriptorRangeTypeUAV;
        range.NumDescriptors = layout.uav_count;
        range.BaseShaderRegister = 0;
        range.RegisterSpace = 0;
        range.Flags = IRDescriptorRangeFlagDataStatic;
        range.OffsetInDescriptorsFromTableStart = layout.srv_count;
        ranges.push_back(range);
      }

      std::vector<IRRootParameter1> params;
      for (uint32_t i = 0; i < layout.cbv_count; ++i)
      {
        IRRootParameter1 param{};
        param.ParameterType = IRRootParameterTypeCBV;
        param.Descriptor.ShaderRegister = i;
        param.Descriptor.RegisterSpace = 0;
        param.Descriptor.Flags = IRRootDescriptorFlagDataStaticWhileSetAtExecute;
        param.ShaderVisibility = IRShaderVisibilityAll;
        params.push_back(param);
      }
      if (!ranges.empty())
      {
        IRRootParameter1 param{};
        param.ParameterType = IRRootParameterTypeDescriptorTable;
        param.DescriptorTable.NumDescriptorRanges = uint32_t(ranges.size());
        param.DescriptorTable.pDescriptorRanges = ranges.data();
        param.ShaderVisibility = IRShaderVisibilityAll;
        params.push_back(param);
      }

      IRVersionedRootSignatureDescriptor desc{};
      desc.version = IRRootSignatureVersion_1_1;
      desc.desc_1_1.NumParameters = uint32_t(params.size());
      desc.desc_1_1.pParameters = params.data();
      desc.desc_1_1.NumStaticSamplers = 0;
      desc.desc_1_1.pStaticSamplers = nullptr;
      desc.desc_1_1.Flags = IRRootSignatureFlagNone;

      return IRRootSignatureCreateFromDescriptor(&desc, error);
    }
#endif
  }

  std::vector<uint32_t> MTLCompileHLSLToSPIRV(const std::string& source,
    const std::string& include_dir,
    const std::string& entry,
    const std::string& profile,
    const std::vector<std::pair<std::string, std::string>>& defines,
    std::string& error)
  {
    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcCompiler3> compiler;
    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))) ||
      FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
    {
      error = "dxcompiler: failed to create DXC instances";
      return {};
    }

    ComPtr<IDxcIncludeHandler> include_handler;
    utils->CreateDefaultIncludeHandler(&include_handler);

    // Mirrors the scripted pipeline (tools/msl/build_msl.sh): -spirv, Vulkan 1.2 target,
    // USE_SPIRV/USE_MSL so the shared shader headers select the Vulkan-compatible branches.
    std::vector<std::wstring> args_storage{
      L"-spirv",
      L"-fspv-target-env=vulkan1.2",
      L"-E", ToWide(entry),
      L"-T", ToWide(profile),
      L"-HV", L"2021",
      L"-D", L"USE_SPIRV=1",
      L"-D", L"USE_MSL=1",
      L"-I", ToWide(include_dir),
    };
    for (const auto& define : defines)
    {
      const std::string value = define.second.empty() ? define.first : define.first + "=" + define.second;
      args_storage.push_back(L"-D");
      args_storage.push_back(ToWide(value));
    }

    std::vector<LPCWSTR> args;
    args.reserve(args_storage.size());
    for (const auto& arg : args_storage) args.push_back(arg.c_str());

    DxcBuffer source_buffer{};
    source_buffer.Ptr = source.data();
    source_buffer.Size = source.size();
    source_buffer.Encoding = DXC_CP_UTF8;

    ComPtr<IDxcResult> result;
    if (FAILED(compiler->Compile(&source_buffer, args.data(), UINT32(args.size()),
      include_handler.ptr, IID_PPV_ARGS(&result))) || !result.ptr)
    {
      error = "dxcompiler: Compile call failed";
      return {};
    }

    HRESULT status = S_OK;
    result->GetStatus(&status);
    if (FAILED(status))
    {
      ComPtr<IDxcBlobUtf8> errors;
      if (result->HasOutput(DXC_OUT_ERRORS) && SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors.ptr)
      {
        error.assign(errors->GetStringPointer(), errors->GetStringLength());
      }
      if (error.empty()) error = "dxcompiler: compilation failed";
      return {};
    }

    ComPtr<IDxcBlob> object;
    if (!result->HasOutput(DXC_OUT_OBJECT) ||
      FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&object), nullptr)) || !object.ptr)
    {
      error = "dxcompiler: no SPIR-V object produced";
      return {};
    }

    // SPIR-V is a stream of 32-bit words; the DXC blob is always 4-byte aligned.
    const auto* words = static_cast<const uint32_t*>(object->GetBufferPointer());
    return std::vector<uint32_t>(words, words + object->GetBufferSize() / sizeof(uint32_t));
  }


  std::vector<uint8_t> MTLCompileHLSLToDXIL(const std::string& source,
    const std::string& include_dir,
    const std::string& entry,
    const std::string& profile,
    const std::vector<std::pair<std::string, std::string>>& defines,
    std::string& error)
  {
    ComPtr<IDxcUtils> utils;
    ComPtr<IDxcCompiler3> compiler;
    if (FAILED(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&utils))) ||
      FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler))))
    {
      error = "dxcompiler: failed to create DXC instances";
      return {};
    }

    ComPtr<IDxcIncludeHandler> include_handler;
    utils->CreateDefaultIncludeHandler(&include_handler);

    // Validation stays enabled (no -Vd), so invalid DXIL is rejected here instead of at Metal load time.
    // Library targets (lib_6_3) take no -E; shader stages (ms_6_5, ps_6_5) name their entry point.
    std::vector<std::wstring> args_storage;
    if (!entry.empty())
    {
      args_storage.push_back(L"-E");
      args_storage.push_back(ToWide(entry));
    }
    args_storage.insert(args_storage.end(), {
      L"-T", ToWide(profile),
      L"-HV", L"2021",
      L"-D", L"USE_MSL=1",
      L"-I", ToWide(include_dir),
    });
    for (const auto& define : defines)
    {
      const std::string value = define.second.empty() ? define.first : define.first + "=" + define.second;
      args_storage.push_back(L"-D");
      args_storage.push_back(ToWide(value));
    }

    std::vector<LPCWSTR> args;
    args.reserve(args_storage.size());
    for (const auto& arg : args_storage) args.push_back(arg.c_str());

    DxcBuffer source_buffer{};
    source_buffer.Ptr = source.data();
    source_buffer.Size = source.size();
    source_buffer.Encoding = DXC_CP_UTF8;

    ComPtr<IDxcResult> result;
    if (FAILED(compiler->Compile(&source_buffer, args.data(), UINT32(args.size()),
      include_handler.ptr, IID_PPV_ARGS(&result))) || !result.ptr)
    {
      error = "dxcompiler: Compile call failed";
      return {};
    }

    HRESULT status = S_OK;
    result->GetStatus(&status);
    if (FAILED(status))
    {
      ComPtr<IDxcBlobUtf8> errors;
      if (result->HasOutput(DXC_OUT_ERRORS) && SUCCEEDED(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr)) && errors.ptr)
      {
        error.assign(errors->GetStringPointer(), errors->GetStringLength());
      }
      if (error.empty()) error = "dxcompiler: compilation failed";
      return {};
    }

    ComPtr<IDxcBlob> object;
    if (!result->HasOutput(DXC_OUT_OBJECT) ||
      FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&object), nullptr)) || !object.ptr)
    {
      error = "dxcompiler: no DXIL object produced";
      return {};
    }

    const auto* bytes = static_cast<const uint8_t*>(object->GetBufferPointer());
    return std::vector<uint8_t>(bytes, bytes + object->GetBufferSize());
  }

  std::vector<uint8_t> MTLConvertDXILToMetalLib(const std::vector<uint8_t>& dxil,
    const std::string& entry,
    const MTLDXILRayTracingLayout* layout,
    std::string& error)
  {
#if !RAYGENE_HAS_METAL_IRCONVERTER
    (void)dxil; (void)entry; (void)layout;
    error = "libmetalirconverter is not linked (configure with -DUSE_METAL_IRCONVERTER=ON)";
    return {};
#else
    IRCompiler* compiler = IRCompilerCreate();
    IRError* ir_error = nullptr;
    IRRayTracingPipelineConfiguration* rt_config = nullptr;
    IRRootSignature* root_signature = nullptr;

    if (layout)
    {
      rt_config = IRRayTracingPipelineConfigurationCreate();
      IRRayTracingPipelineConfigurationSetMaxRecursiveDepth(rt_config, 1);
      IRRayTracingPipelineConfigurationSetRayGenerationCompilationMode(rt_config, IRRayGenerationCompilationKernel);
      IRRayTracingPipelineConfigurationSetIntersectionFunctionCompilationMode(rt_config, IRIntersectionFunctionCompilationVisibleFunction);
      IRCompilerSetRayTracingPipelineConfiguration(compiler, rt_config);

      root_signature = CreateRayTracingRootSignature(*layout, &ir_error);
      if (!root_signature)
      {
        error = "libmetalirconverter: root signature creation failed";
        if (ir_error) IRErrorDestroy(ir_error);
        IRRayTracingPipelineConfigurationDestroy(rt_config);
        IRCompilerDestroy(compiler);
        return {};
      }
      IRCompilerSetGlobalRootSignature(compiler, root_signature);
    }

    if (!entry.empty())
    {
      IRCompilerSetEntryPointName(compiler, entry.c_str());
    }

    IRObject* dxil_object = IRObjectCreateFromDXIL(dxil.data(), dxil.size(), IRBytecodeOwnershipNone);
    IRObject* ir_object = IRCompilerAllocCompileAndLink(compiler, nullptr, dxil_object, &ir_error);

    std::vector<uint8_t> metallib;
    if (!ir_object)
    {
      error = "libmetalirconverter: compile failed, error code " + std::to_string(ir_error ? IRErrorGetCode(ir_error) : 0u);
    }
    else
    {
      const IRShaderStage stage = IRObjectGetMetalIRShaderStage(ir_object);
      IRMetalLibBinary* binary = IRMetalLibBinaryCreate();
      if (IRObjectGetMetalLibBinary(ir_object, stage, binary))
      {
        metallib.resize(IRMetalLibGetBytecodeSize(binary));
        IRMetalLibGetBytecode(binary, metallib.data());
      }
      else
      {
        error = "libmetalirconverter: metallib extraction failed";
      }
      IRMetalLibBinaryDestroy(binary);
      IRObjectDestroy(ir_object);
    }

    if (ir_error) IRErrorDestroy(ir_error);
    if (dxil_object) IRObjectDestroy(dxil_object);
    if (root_signature) IRRootSignatureDestroy(root_signature);
    if (rt_config) IRRayTracingPipelineConfigurationDestroy(rt_config);
    IRCompilerDestroy(compiler);
    return metallib;
#endif
  }


}
