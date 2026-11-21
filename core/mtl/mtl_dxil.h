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
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

namespace RayGene3D
{
  // Register counts used to build the root signature for ray tracing libraries.
  // Registers are contiguous from 0: CBV b0..b(cbv-1), SRV t0..t(srv-1), UAV u0..u(uav-1).
  struct MTLDXILRayTracingLayout
  {
    uint32_t cbv_count{ 0 };
    uint32_t srv_count{ 0 };
    uint32_t uav_count{ 0 };
  };

  // HLSL -> DXIL via dxcompiler (in-process). Returns an empty vector on failure and writes the reason to 'error'.
  //   profile: e.g. "ms_6_5", "ps_6_5", "lib_6_3".
  std::vector<uint8_t> MTLCompileHLSLToDXIL(const std::string& source,
    const std::string& include_dir,
    const std::string& entry,
    const std::string& profile,
    const std::vector<std::pair<std::string, std::string>>& defines,
    std::string& error);

  // HLSL -> SPIR-V via dxcompiler (-spirv, in-process). Returns an empty vector on failure and writes the reason to 'error'.
  //   profile: e.g. "vs_6_0", "ps_6_0", "cs_6_0".
  //   entry: HLSL entry point name, e.g. "vs_main".
  std::vector<uint32_t> MTLCompileHLSLToSPIRV(const std::string& source,
    const std::string& include_dir,
    const std::string& entry,
    const std::string& profile,
    const std::vector<std::pair<std::string, std::string>>& defines,
    std::string& error);

  // DXIL -> metallib bytes via libmetalirconverter. Returns an empty vector on failure and writes the reason to 'error'.
  //   layout: required for ray tracing libraries (lib_*), ignored when nullptr.
  //   entry: function to convert; empty for a single-entry shader.
  std::vector<uint8_t> MTLConvertDXILToMetalLib(const std::vector<uint8_t>& dxil,
    const std::string& entry,
    const MTLDXILRayTracingLayout* layout,
    std::string& error);
}
