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


#include "mtl_config.h"
#include "mtl_pass.h"
#include "mtl_device.h"
#include "mtl_dxil.h"

#include <cstdio>
#include <cstdlib>

#if RAYGENE_MSL_LEGACY_SHADERC
#include <shaderc/shaderc.hpp>
#endif
#include <spirv_cross/spirv_msl.hpp>

#import <Metal/Metal.h>

namespace RayGene3D
{
  // Maps the engine vertex format onto the Metal vertex format used by the geometry pass inputs.
  static MTLVertexFormat get_vertex_format(Format format)
  {
    switch (format)
    {
    default: return MTLVertexFormatInvalid;
    case FORMAT_R32_FLOAT: return MTLVertexFormatFloat;
    case FORMAT_R32_UINT: return MTLVertexFormatUInt;
    case FORMAT_R32G32_FLOAT: return MTLVertexFormatFloat2;
    case FORMAT_R32G32B32_FLOAT: return MTLVertexFormatFloat3;
    case FORMAT_R32G32B32A32_FLOAT: return MTLVertexFormatFloat4;
    case FORMAT_R32G32B32A32_UINT: return MTLVertexFormatUInt4;
    case FORMAT_R8G8B8A8_UNORM: return MTLVertexFormatUChar4Normalized;
    }
  }

  // Builds the Metal vertex descriptor from the same IAState the Vulkan backend uses,
  // so attribute i maps to [[attribute(i)]] (location = i) and slot maps to the buffer index.
  static MTLVertexDescriptor* build_vertex_descriptor(const Config::IAState& ia_state)
  {
    if (ia_state.attributes.empty()) return nil;

    MTLVertexDescriptor* vd = [MTLVertexDescriptor vertexDescriptor];
    for (uint32_t i = 0; i < uint32_t(ia_state.attributes.size()); ++i)
    {
      const auto& attribute = ia_state.attributes[i];
      vd.attributes[i].format = get_vertex_format(attribute.format);
      vd.attributes[i].offset = attribute.offset;
      vd.attributes[i].bufferIndex = attribute.slot;
    }

    for (const auto& attribute : ia_state.attributes)
    {
      vd.layouts[attribute.slot].stride = attribute.stride;
      vd.layouts[attribute.slot].stepFunction = attribute.instance
        ? MTLVertexStepFunctionPerInstance
        : MTLVertexStepFunctionPerVertex;
      vd.layouts[attribute.slot].stepRate = 1;
    }

    return vd;
  }

#if RAYGENE_MSL_LEGACY_SHADERC
  class MTLIncluder : public shaderc::CompileOptions::IncluderInterface
  {
    struct Includee
    {
      std::string name;
      std::string content;
    };

    shaderc_include_result* GetInclude(
      const char* requested_source,
      shaderc_include_type type,
      const char* requesting_source,
      size_t include_depth) override
    {
      const std::filesystem::path base_path = std::filesystem::path(requesting_source).parent_path();
      const std::filesystem::path full_path = (base_path / requested_source).lexically_normal();

      auto includee = new Includee;
      includee->name = full_path.string();

      std::fstream fs;
      fs.open(includee->name, std::fstream::in);
      std::stringstream ss;
      ss << fs.rdbuf();
      includee->content = std::move(ss.str());

      auto result = new shaderc_include_result;
      result->user_data = includee;
      result->source_name = includee->name.c_str();
      result->source_name_length = includee->name.size();
      result->content = includee->content.c_str();
      result->content_length = includee->content.size();

      return result;
    }

    void ReleaseInclude(shaderc_include_result* result) override
    {
      if (result)
      {
        auto includee = reinterpret_cast<Includee*>(result->user_data);
        if (includee) delete includee;
        result->user_data = nullptr;
      }
    }

  public:
    MTLIncluder() {}
    virtual ~MTLIncluder() {}
  };
#endif

  static std::string CompileMSL(const char* name, std::string& source, const std::string& path, const std::string& file,
    const char* target, const std::map<std::string, std::string>& defines, std::vector<uint32_t>& spirv_out)
  {
    if (source.empty())
    {
      std::fstream fs;
      fs.open(path + file, std::fstream::in);
      std::stringstream ss;
      ss << fs.rdbuf();
      fs.close();
      source = ss.str();
    }

    // "#version 460" is GLSL. Anything else is HLSL with per-stage entry points.
    const bool is_glsl = source.find("#version 460") != source.npos;

    // Per-stage entry point, DXC profile, and stage define. GLSL sources always use "main".
    const char* entry = "main";
    const char* profile = "";
    const char* stage_define = "";
    if (strcmp(target, "comp") == 0) { entry = is_glsl ? "main" : "cs_main"; profile = "cs_6_0"; stage_define = "COMP"; } else
    if (strcmp(target, "vert") == 0) { entry = is_glsl ? "main" : "vs_main"; profile = "vs_6_0"; stage_define = "VERT"; } else
    if (strcmp(target, "frag") == 0) { entry = is_glsl ? "main" : "ps_main"; profile = "ps_6_0"; stage_define = "FRAG"; }

#if RAYGENE_MSL_LEGACY_SHADERC
    // Previous approach: shaderc (glslang) compiles both HLSL and GLSL to SPIR-V.
    {
      shaderc::CompileOptions options;
      options.AddMacroDefinition("USE_SPIRV");
      options.AddMacroDefinition("USE_MSL");
      if (*stage_define) options.AddMacroDefinition(stage_define);
      options.SetOptimizationLevel(shaderc_optimization_level_performance);
      options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
      options.SetTargetSpirv(shaderc_spirv_version_1_3);
      options.SetIncluder(std::make_unique<MTLIncluder>());
      options.SetSourceLanguage(is_glsl ? shaderc_source_language_glsl : shaderc_source_language_hlsl);

      auto kind = shaderc_glsl_infer_from_source;
      if (strcmp(target, "comp") == 0) kind = shaderc_compute_shader;
      else if (strcmp(target, "vert") == 0) kind = shaderc_vertex_shader;
      else if (strcmp(target, "frag") == 0) kind = shaderc_fragment_shader;

      for (const auto& define : defines) options.AddMacroDefinition(define.first, define.second);

      shaderc::Compiler compiler;
      const auto module = compiler.CompileGlslToSpv(source, (shaderc_shader_kind)kind, file.empty() ? "unknown" : (path + file).c_str(), entry, options);

      if (module.GetCompilationStatus() != shaderc_compilation_status_success)
      {
        BLAST_LOG(module.GetErrorMessage().c_str());
        return std::string();
      }

      spirv_out.assign(module.cbegin(), module.cend());
    }
#else
    // Current approach: GLSL still uses shaderc; HLSL uses in-process DXC -spirv
    // (same settings as tools/msl/build_msl.sh: Vulkan 1.2 target, USE_SPIRV/USE_MSL defines).
    if (is_glsl)
    {
      BLAST_LOG("MSL: GLSL input requires RAYGENE_MSL_LEGACY_SHADERC (-DUSE_DXC_SPIRV_MSL=OFF)");
      return std::string();
    }

    // The stage define selects the per-stage branch in the shared shader headers.
    std::vector<std::pair<std::string, std::string>> dxc_defines(defines.begin(), defines.end());
    dxc_defines.emplace_back(stage_define, std::string());

    std::string error;
    spirv_out = MTLCompileHLSLToSPIRV(source, path, entry, profile, dxc_defines, error);
    if (spirv_out.empty())
    {
      BLAST_LOG("%s", error.c_str());
      return std::string();
    }
#endif

    // SPIR-V -> Metal Shading Language.
    // Binding model: descriptor set 0 binding N is mapped to Metal buffer/texture/sampler index N
    // (see enable_decoration_binding), so existing layout(set=0, binding=N) shaders keep working.
    spirv_cross::CompilerMSL msl(spirv_out.data(), spirv_out.size());

    // Normalize the stage entry point to "main0" so Metal lookups are deterministic
    // regardless of the HLSL (vs_main/ps_main/cs_main) or GLSL (main) source name.
    const auto model = strcmp(target, "comp") == 0 ? spv::ExecutionModelGLCompute
      : strcmp(target, "vert") == 0 ? spv::ExecutionModelVertex
      : spv::ExecutionModelFragment;
    msl.rename_entry_point(entry, "main0", model);

    spirv_cross::CompilerMSL::Options msl_options;
    msl_options.set_msl_version(2, 3);
    msl_options.platform = spirv_cross::CompilerMSL::Options::Platform::macOS;
    msl_options.enable_decoration_binding = true;
    msl_options.pad_fragment_output_components = true;
    msl.set_msl_options(msl_options);

    return msl.compile();
  }

  void MTLConfig::Initialize()
  {
    auto pass = reinterpret_cast<MTLPass*>(&this->GetPass());
    auto device = reinterpret_cast<MTLDevice*>(&pass->GetDevice());
    id<MTLDevice> mtl_device = (__bridge id<MTLDevice>)device->GetDevice();

    const auto& path = device->GetPath();

    // Metal has no tessellation, geometry, mesh/task, or ray tracing stages in this backend;
    // those compilation flags are ignored (the pass falls back to VS/PS or CS only).
    const auto create_library = [&](const char* target, std::vector<uint32_t>& spirv) -> id<MTLLibrary>
    {
      const auto msl = CompileMSL(name.c_str(), source, path, file, target, defines, spirv);
      BLAST_ASSERT(!msl.empty());
      NSError* error = nil;
      id<MTLLibrary> lib = [mtl_device newLibraryWithSource:[NSString stringWithUTF8String:msl.c_str()]
                                                    options:nil
                                                      error:&error];
      if (lib == nil)
      {
        BLAST_LOG("%s", [[error localizedDescription] UTF8String]);
        BLAST_ASSERT(false);
      }
      return lib;
    };

    if (compilation & COMPILATION_COMP)
    {
      std::vector<uint32_t> spirv;
      id<MTLLibrary> comp_library = create_library("comp", spirv);
      library = (__bridge_retained void*)comp_library;

      // Threadgroup size must match the kernel's local_size (Vulkan workgroup size).
      spirv_cross::CompilerMSL reflect(spirv.data(), spirv.size());
      threads_per_group = {
        reflect.get_execution_mode_argument(spv::ExecutionModeLocalSize, 0),
        reflect.get_execution_mode_argument(spv::ExecutionModeLocalSize, 1),
        reflect.get_execution_mode_argument(spv::ExecutionModeLocalSize, 2) };

      id<MTLFunction> function = [comp_library newFunctionWithName:@"main0"];
      BLAST_ASSERT(function != nil);
      kernel_function = (__bridge_retained void*)function;

      NSError* pipeline_error = nil;
      id<MTLComputePipelineState> state = [mtl_device newComputePipelineStateWithFunction:function error:&pipeline_error];
      BLAST_ASSERT(state != nil);
      pipeline = (__bridge_retained void*)state;
    }

    if ((compilation & COMPILATION_VERT) && (compilation & COMPILATION_FRAG) && pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      std::vector<uint32_t> vert_spirv;
      std::vector<uint32_t> frag_spirv;
      id<MTLLibrary> vert_library = create_library("vert", vert_spirv);
      id<MTLLibrary> frag_library = create_library("frag", frag_spirv);
      library = (__bridge_retained void*)vert_library;

      id<MTLFunction> vs = [vert_library newFunctionWithName:@"main0"];
      id<MTLFunction> ps = [frag_library newFunctionWithName:@"main0"];
      BLAST_ASSERT(vs != nil && ps != nil);
      vertex_function = (__bridge_retained void*)vs;
      fragment_function = (__bridge_retained void*)ps;

      MTLRenderPipelineDescriptor* desc = [[MTLRenderPipelineDescriptor alloc] init];
      desc.vertexFunction = vs;
      desc.fragmentFunction = ps;

      // Pipeline attachment formats must match the render pass framebuffer exactly (Metal validation),
      // so take them from the pass descriptor's real textures, as the Vulkan backend does from the pass.
      MTLRenderPassDescriptor* pass_descriptor = (__bridge MTLRenderPassDescriptor*)pass->GetRenderPassDescriptor();
      for (NSUInteger i = 0; i < 8; ++i)
      {
        id<MTLTexture> target = pass_descriptor.colorAttachments[i].texture;
        if (target == nil) break;
        desc.colorAttachments[i].pixelFormat = target.pixelFormat;
      }
      if (pass_descriptor.depthAttachment.texture != nil)
      {
        desc.depthAttachmentPixelFormat = pass_descriptor.depthAttachment.texture.pixelFormat;
      }
      // Metal requires the primitive topology class on the pipeline; without it the class stays Unspecified.
      switch (ia_state.topology)
      {
      case Config::TOPOLOGY_POINTLIST:
        desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassPoint;
        break;
      case Config::TOPOLOGY_LINELIST:
      case Config::TOPOLOGY_LINESTRIP:
        desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassLine;
        break;
      case Config::TOPOLOGY_TRIANGLELIST:
      case Config::TOPOLOGY_TRIANGLESTRIP:
        desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassTriangle;
        break;
      default:
        desc.inputPrimitiveTopology = MTLPrimitiveTopologyClassUnspecified;
        break;
      }

      desc.vertexDescriptor = build_vertex_descriptor(ia_state);

      NSError* pipeline_error = nil;
      id<MTLRenderPipelineState> state = [mtl_device newRenderPipelineStateWithDescriptor:desc error:&pipeline_error];
      if (state == nil)
      {
        BLAST_LOG("MTL render pipeline failed (%s): %s", name.c_str(), [[pipeline_error localizedDescription] UTF8String]);
      }
      BLAST_ASSERT(state != nil);
      pipeline = (__bridge_retained void*)state;

    }

    // Mesh and ray tracing stages: SPIRV-Cross cannot translate them, so they go HLSL -> DXIL -> metallib.
    if (compilation & (COMPILATION_MESH | COMPILATION_RGEN))
    {
      BuildDXILLibrary((__bridge void*)mtl_device, path, file);
    }
  }

  std::vector<std::pair<std::string, std::string>> MTLConfig::defines_list() const
  {
    return std::vector<std::pair<std::string, std::string>>(defines.begin(), defines.end());
  }

  void MTLConfig::BuildDXILLibrary(void* device_ptr, const std::string& path, const std::string& file)
  {
    id<MTLDevice> mtl_device = (__bridge id<MTLDevice>)device_ptr;

    // File-path configs leave `source` empty; load it here, as CompileMSL's callers do for shaderc.
    if (source.empty())
    {
      std::fstream fs;
      fs.open(path + file, std::fstream::in);
      std::stringstream ss;
      ss << fs.rdbuf();
      fs.close();
      source = ss.str();
    }

    // The DXIL path only accepts HLSL; a GLSL source would be rejected by dxc with a confusing error.
    BLAST_ASSERT(source.find("#version 460") == source.npos);

    // Compile one HLSL entry point to DXIL, convert it to a metallib, and load it with newLibraryWithData.
    const auto build = [&](const char* entry, const char* profile, const MTLDXILRayTracingLayout* layout) -> id<MTLLibrary>
    {
      std::string error;
      const auto dxil = MTLCompileHLSLToDXIL(source, path, entry, profile, defines_list(), error);
      if (dxil.empty())
      {
        BLAST_LOG("%s", error.c_str());
        BLAST_ASSERT(false);
        return nil;
      }

      // Empty entry selects the whole library (lib_6_3); a named entry converts only that function.
      const std::string metal_entry = (strlen(entry) > 0) ? std::string(entry) : std::string();
      const auto metallib = MTLConvertDXILToMetalLib(dxil, metal_entry, layout, error);
      if (metallib.empty())
      {
        BLAST_LOG("%s", error.c_str());
        BLAST_ASSERT(false);
        return nil;
      }

      dispatch_data_t data = dispatch_data_create(metallib.data(), metallib.size(), nullptr, DISPATCH_DATA_DESTRUCTOR_DEFAULT);
      NSError* load_error = nil;
      id<MTLLibrary> lib = [mtl_device newLibraryWithData:data error:&load_error];
      if (lib == nil)
      {
        BLAST_LOG("%s", [[load_error localizedDescription] UTF8String]);
        BLAST_ASSERT(false);
      }
      return lib;
    };

    // A mesh config is one mesh stage plus an optional pixel stage; each stage gets its own library.
    if (compilation & COMPILATION_MESH)
    {
      id<MTLLibrary> mesh_library = build("mesh_main", "ms_6_5", nullptr);
      BLAST_ASSERT(mesh_library != nil);
      library = (__bridge_retained void*)mesh_library;
      mesh_function = (__bridge_retained void*)[mesh_library newFunctionWithName:@"mesh_main"];
    }

    if ((compilation & COMPILATION_FRAG) && (compilation & COMPILATION_MESH))
    {
      id<MTLLibrary> frag_library = build("ps_main", "ps_6_5", nullptr);
      BLAST_ASSERT(frag_library != nil);
      fragment_function = (__bridge_retained void*)[frag_library newFunctionWithName:@"ps_main"];
    }

    // Ray tracing library: lib_6_3 compiles every [shader] entry, so no single entry point is selected.
    if (compilation & COMPILATION_RGEN)
    {
      const MTLDXILRayTracingLayout layout{ 3, 4, 1 };
      id<MTLLibrary> rt_library = build("", "lib_6_3", &layout);
      BLAST_ASSERT(rt_library != nil);
      library = (__bridge_retained void*)rt_library;
    }
  }

  void MTLConfig::Use()
  {
    for (const auto& batch : batches)
    {
      batch->Use();
    }
  }

  void MTLConfig::Discard()
  {
    batches.clear();

    if (pipeline) { (void)(__bridge_transfer id)pipeline; pipeline = nullptr; }
    if (kernel_function) { (void)(__bridge_transfer id)kernel_function; kernel_function = nullptr; }
    if (fragment_function) { (void)(__bridge_transfer id)fragment_function; fragment_function = nullptr; }
    if (vertex_function) { (void)(__bridge_transfer id)vertex_function; vertex_function = nullptr; }
    if (library) { (void)(__bridge_transfer id)library; library = nullptr; }
  }

  MTLConfig::MTLConfig(const std::string& name,
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
    MTLConfig::Initialize();
  }

  MTLConfig::MTLConfig(const std::string& name,
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
    MTLConfig::Initialize();
  }

  MTLConfig::~MTLConfig()
  {
    MTLConfig::Discard();
  }
}
