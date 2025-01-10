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


#include "vlk_config.h"
#include "vlk_device.h"

#define ENABLE_HLSL
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>


namespace RayGene3D
{
  void CompileVLK(const std::string& source, const char* entry, const char* target, 
    std::map<std::string, std::string> defines, const std::string& path, std::vector<char>& bytecode)
  {
    class VLKIncluder : public glslang::TShader::Includer
    {
    private:
      std::string path;

    public:
      // For the "system" or <>-style includes; search the "system" paths.
      IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override
      {
        const auto file_path = path + std::string(headerName);

        std::fstream fs;
        fs.open(file_path, std::fstream::in);

        std::stringstream ss;
        ss << fs.rdbuf();
        const std::string text = ss.str();

        const auto size = text.size();
        const auto data = new char[size];
        //assert(data);

        memcpy(data, text.data(), size);

        auto result = new IncludeResult(file_path, data, size, nullptr);
        //assert(result);

        return result;
      }

      // For the "local"-only aspect of a "" include. Should not search in the
      // "system" paths, because on returning a failure, the parser will
      // call includeSystem() to look in the "system" locations.
      IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override
      {
        const auto file_path = path + std::string(headerName);

        std::fstream fs;
        fs.open(file_path, std::fstream::in);

        std::stringstream ss;
        ss << fs.rdbuf();
        const std::string text = ss.str();

        const auto size = text.size();
        const auto data = new char[size];
        //assert(data);

        memcpy(data, text.data(), size);

        auto result = new IncludeResult(file_path, data, size, nullptr);
        //assert(result);

        return result;
      }

      // Signals that the parser will no longer use the contents of the
      // specified IncludeResult.
      void releaseInclude(IncludeResult* result) override
      {
        if (result)
        {
          if (result->headerData)
          {
            delete[] result->headerData;
          }
          delete result;
        }
      }

    public:
      VLKIncluder(const std::string& path) : glslang::TShader::Includer(), path(path) {}
      virtual ~VLKIncluder() {}
    };


    glslang::InitializeProcess();
    
    glslang::EShTargetLanguageVersion version = glslang::EShTargetSpv_1_0;
    glslang::EShSource language = glslang::EShSourceCount;
    std::string preamble = "#define USE_SPIRV\n";

    EShLanguage stage = EShLangCount;
    if (strcmp(target, "cs_5_0") == 0) { language = glslang::EShSourceHlsl; stage = EShLangCompute; } else
    if (strcmp(target, "vs_5_0") == 0) { language = glslang::EShSourceHlsl; stage = EShLangVertex; } else
    if (strcmp(target, "ds_5_0") == 0) { language = glslang::EShSourceHlsl; stage = EShLangTessEvaluation; } else
    if (strcmp(target, "hs_5_0") == 0) { language = glslang::EShSourceHlsl; stage = EShLangTessControl; } else
    if (strcmp(target, "gs_5_0") == 0) { language = glslang::EShSourceHlsl; stage = EShLangGeometry; } else
    if (strcmp(target, "ps_5_0") == 0) { language = glslang::EShSourceHlsl; stage = EShLangFragment; } else
    if (strcmp(target, "task") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangTask;       preamble.append("#define TASK\n"); } else
    if (strcmp(target, "mesh") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangMesh;       preamble.append("#define MESH\n"); } else
    if (strcmp(target, "rgen") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangRayGen;     preamble.append("#define RGEN\n"); } else
    if (strcmp(target, "isec") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangIntersect;  preamble.append("#define ISEC\n"); } else
    if (strcmp(target, "chit") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangClosestHit; preamble.append("#define CHIT\n"); } else
    if (strcmp(target, "ahit") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangAnyHit;     preamble.append("#define AHIT\n"); } else
    if (strcmp(target, "miss") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangMiss;       preamble.append("#define MISS\n"); } else
    if (strcmp(target, "call") == 0) { language = glslang::EShSourceGlsl; version = glslang::EShTargetSpv_1_4; stage = EShLangCallable;   preamble.append("#define CALL\n"); }

    for (const auto& define : defines)
    {
      preamble.append("#define " + define.first + " " + define.second + "\n");
    }

    glslang::TShader shader(stage);
    shader.setEnvInput(language, stage, glslang::EShClientVulkan, 100);
    shader.setEnvClient(glslang::EShClientVulkan, glslang::EShTargetVulkan_1_0);
    shader.setEnvTarget(glslang::EShTargetSpv, version);
    shader.setPreamble(preamble.c_str());
    shader.setInvertY(true);
    shader.setEntryPoint(entry);
    //shader.setSourceEntryPoint(entry);
    //shader.setNoStorageFormat(false);
    //shader.setNanMinMaxClamp(false);

    VLKIncluder includer(path);
   
    const auto source_str = source.c_str();
    shader.setStrings(&source_str, 1);

    TBuiltInResource resources{ 0 };
    EShMessages messages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules /*| EShMsgHlslOffsets | EShMsgReadHlsl*/);
    if (!shader.parse(&resources, 100, false, messages, includer))
    {
      BLAST_LOG(shader.getInfoLog());
      return;
    }

    glslang::TProgram program;
    program.addShader(&shader);
    if (!program.link(messages))
    {
      BLAST_LOG(program.getInfoLog());
      return;
    }

    //if (program.buildReflection())
    //{
    //  program.dumpReflection();
    //}

    std::vector<unsigned int> spv;
    spv::SpvBuildLogger logger;
    glslang::SpvOptions options;
    options.generateDebugInfo = false;
    options.stripDebugInfo = false;
    options.disableOptimizer = false;
    options.optimizeSize = false;
    options.disassemble = false;
    options.validate = false;
    glslang::GlslangToSpv(*program.getIntermediate(stage), spv, &logger, &options);


    const auto data = spv.data();
    const auto size = spv.cend() - spv.cbegin();
    bytecode.resize(size * sizeof(uint32_t));
    memcpy(bytecode.data(), data, bytecode.size());

    glslang::FinalizeProcess();
  }

  void VLKConfig::Initialize()
  {
    auto pass = reinterpret_cast<VLKPass*>(&this->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

    const auto& path = device->GetPath();

    cs_bytecode.clear();
    vs_bytecode.clear();
    hs_bytecode.clear();
    ds_bytecode.clear();
    gs_bytecode.clear();
    ps_bytecode.clear();
    task_bytecode.clear();
    mesh_bytecode.clear();
    rgen_bytecode.clear();
    isec_bytecode.clear();
    miss_bytecode.clear();
    chit_bytecode.clear();
    ahit_bytecode.clear();
    call_bytecode.clear();

    if (compilation & COMPILATION_CS) { CompileVLK(source, "cs_main", "cs_5_0", defines, path, cs_bytecode); BLAST_ASSERT(!cs_bytecode.empty()); }
    if (compilation & COMPILATION_VS) { CompileVLK(source, "vs_main", "vs_5_0", defines, path, vs_bytecode); BLAST_ASSERT(!vs_bytecode.empty()); }
    if (compilation & COMPILATION_HS) { CompileVLK(source, "hs_main", "hs_5_0", defines, path, hs_bytecode); BLAST_ASSERT(!hs_bytecode.empty()); }
    if (compilation & COMPILATION_DS) { CompileVLK(source, "ds_main", "ds_5_0", defines, path, ds_bytecode); BLAST_ASSERT(!ds_bytecode.empty()); }
    if (compilation & COMPILATION_GS) { CompileVLK(source, "gs_main", "gs_5_0", defines, path, gs_bytecode); BLAST_ASSERT(!gs_bytecode.empty()); }
    if (compilation & COMPILATION_PS) { CompileVLK(source, "ps_main", "ps_5_0", defines, path, ps_bytecode); BLAST_ASSERT(!ps_bytecode.empty()); }
    if (compilation & COMPILATION_TASK) { CompileVLK(source, "main", "task", defines, path, task_bytecode); BLAST_ASSERT(!rgen_bytecode.empty()); }
    if (compilation & COMPILATION_MESH) { CompileVLK(source, "main", "mesh", defines, path, mesh_bytecode); BLAST_ASSERT(!mesh_bytecode.empty()); }
    if (compilation & COMPILATION_RGEN) { CompileVLK(source, "main", "rgen", defines, path, rgen_bytecode); BLAST_ASSERT(!rgen_bytecode.empty()); }
    if (compilation & COMPILATION_ISEC) { CompileVLK(source, "main", "isec", defines, path, isec_bytecode); BLAST_ASSERT(!isec_bytecode.empty()); }
    if (compilation & COMPILATION_MISS) { CompileVLK(source, "main", "miss", defines, path, miss_bytecode); BLAST_ASSERT(!miss_bytecode.empty()); }
    if (compilation & COMPILATION_CHIT) { CompileVLK(source, "main", "chit", defines, path, chit_bytecode); BLAST_ASSERT(!chit_bytecode.empty()); }
    if (compilation & COMPILATION_AHIT) { CompileVLK(source, "main", "ahit", defines, path, ahit_bytecode); BLAST_ASSERT(!ahit_bytecode.empty()); }
    if (compilation & COMPILATION_CALL) { CompileVLK(source, "main", "call", defines, path, call_bytecode); BLAST_ASSERT(!call_bytecode.empty()); }

    {
      const auto create_shader_module = [device](const std::vector<char>& bytecode)
      {
        auto create_info = VkShaderModuleCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        create_info.codeSize = bytecode.size();
        create_info.pCode = reinterpret_cast<const uint32_t*>(bytecode.data());

        VkShaderModule shader_module;
        BLAST_ASSERT(VK_SUCCESS == vkCreateShaderModule(device->GetDevice(), &create_info, nullptr, &shader_module));

        return shader_module;
      };

      if (!cs_bytecode.empty())
      {
        cs_module = create_shader_module(cs_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_COMPUTE_BIT;
        create_info.module = cs_module;
        create_info.pName = "cs_main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!vs_bytecode.empty())
      {
        vs_module = create_shader_module(vs_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_VERTEX_BIT;
        create_info.module = vs_module;
        create_info.pName = "vs_main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!hs_bytecode.empty())
      {
        hs_module = create_shader_module(hs_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
        create_info.module = hs_module;
        create_info.pName = "hs_main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!ds_bytecode.empty())
      {
        ds_module = create_shader_module(ds_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
        create_info.module = ds_module;
        create_info.pName = "ds_main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!gs_bytecode.empty())
      {
        gs_module = create_shader_module(gs_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        create_info.module = gs_module;
        create_info.pName = "gs_main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!ps_bytecode.empty())
      {
        ps_module = create_shader_module(ps_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        create_info.module = ps_module;
        create_info.pName = "ps_main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      // RTX shaders
      if (!rgen_bytecode.empty())
      {
        auto shader_group = VkRayTracingShaderGroupCreateInfoKHR{};
        shader_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shader_group.generalShader = uint32_t(stages.size()); // rgen shader;
        shader_group.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group.closestHitShader = VK_SHADER_UNUSED_KHR;
        shader_group.anyHitShader = VK_SHADER_UNUSED_KHR;
        groups.push_back(shader_group);

        rgen_module = create_shader_module(rgen_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
        create_info.module = rgen_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!isec_bytecode.empty())
      {
        auto shader_group = VkRayTracingShaderGroupCreateInfoKHR{};
        shader_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_PROCEDURAL_HIT_GROUP_KHR;
        shader_group.generalShader = VK_SHADER_UNUSED_KHR;
        shader_group.intersectionShader = uint32_t(stages.size());
        shader_group.closestHitShader = VK_SHADER_UNUSED_KHR;
        shader_group.anyHitShader = VK_SHADER_UNUSED_KHR;
        groups.push_back(shader_group);

        isec_module = create_shader_module(isec_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
        create_info.module = isec_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!miss_bytecode.empty())
      {
        auto shader_group = VkRayTracingShaderGroupCreateInfoKHR{};
        shader_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shader_group.generalShader = uint32_t(stages.size()); // miss shader;
        shader_group.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group.closestHitShader = VK_SHADER_UNUSED_KHR;
        shader_group.anyHitShader = VK_SHADER_UNUSED_KHR;
        groups.push_back(shader_group);

        miss_module = create_shader_module(miss_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_MISS_BIT_KHR;
        create_info.module = miss_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!chit_bytecode.empty())
      {
        auto shader_group = VkRayTracingShaderGroupCreateInfoKHR{};
        shader_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shader_group.generalShader = VK_SHADER_UNUSED_KHR;
        shader_group.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group.closestHitShader = uint32_t(stages.size()); // chit shader;
        shader_group.anyHitShader = VK_SHADER_UNUSED_KHR;
        groups.push_back(shader_group);

        chit_module = create_shader_module(chit_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
        create_info.module = chit_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!ahit_bytecode.empty())
      {
        auto shader_group = VkRayTracingShaderGroupCreateInfoKHR{};
        shader_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
        shader_group.generalShader = VK_SHADER_UNUSED_KHR;
        shader_group.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group.closestHitShader = VK_SHADER_UNUSED_KHR;
        shader_group.anyHitShader = uint32_t(stages.size()); // ahit shader;
        groups.push_back(shader_group);

        ahit_module = create_shader_module(ahit_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
        create_info.module = ahit_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!call_bytecode.empty())
      {
        auto shader_group = VkRayTracingShaderGroupCreateInfoKHR{};
        shader_group.sType = VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR;
        shader_group.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR;
        shader_group.generalShader = uint32_t(stages.size());
        shader_group.intersectionShader = VK_SHADER_UNUSED_KHR;
        shader_group.closestHitShader = VK_SHADER_UNUSED_KHR;
        shader_group.anyHitShader = VK_SHADER_UNUSED_KHR; // ahit shader;
        groups.push_back(shader_group);

        call_module = create_shader_module(call_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_CALLABLE_BIT_KHR;
        create_info.module = call_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!task_bytecode.empty())
      {
        task_module = create_shader_module(task_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_TASK_BIT_EXT;
        create_info.module = task_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }

      if (!mesh_bytecode.empty())
      {
        mesh_module = create_shader_module(mesh_bytecode);

        auto create_info = VkPipelineShaderStageCreateInfo{};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        create_info.stage = VK_SHADER_STAGE_MESH_BIT_EXT;
        create_info.module = mesh_module;
        create_info.pName = "main";
        create_info.pSpecializationInfo = nullptr;
        stages.push_back(create_info);
      }
    }


    {

    // vertex input info
    const auto get_format = [this](Format format)
    {
      switch (format)
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
      case FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
      case FORMAT_BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
      case FORMAT_BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
      case FORMAT_BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
      case FORMAT_BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
      }
    };

    auto desc_max = 0u;
    std::map<uint32_t, VkVertexInputBindingDescription> desc_map;

    input_attributes.resize(ia_state.attributes.size());
    for (uint32_t i = 0; i < uint32_t(input_attributes.size()); ++i)
    {
      input_attributes[i].location = i;
      input_attributes[i].binding = ia_state.attributes[i].slot;
      input_attributes[i].format = get_format(ia_state.attributes[i].format);
      input_attributes[i].offset = ia_state.attributes[i].offset;

      const auto& attribute = ia_state.attributes[i];
      desc_map[attribute.slot] = { attribute.slot, attribute.stride, attribute.instance ? VK_VERTEX_INPUT_RATE_INSTANCE : VK_VERTEX_INPUT_RATE_VERTEX };
      desc_max = std::max(desc_max, attribute.slot);
    }

    input_bindings.resize(desc_max + 1, VkVertexInputBindingDescription{ 0, 0, VK_VERTEX_INPUT_RATE_VERTEX });
    for (const auto& desc_item : desc_map)
    {
      input_bindings[desc_item.first] = desc_item.second;
    }

    input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    input_state.vertexBindingDescriptionCount = uint32_t(input_bindings.size());
    input_state.pVertexBindingDescriptions = input_bindings.data();
    input_state.vertexAttributeDescriptionCount = uint32_t(input_attributes.size());
    input_state.pVertexAttributeDescriptions = input_attributes.data();

    // input assembly
    const auto get_topology = [](Topology topology)
    {
      switch (topology)
      {
      default:                          return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      case TOPOLOGY_POINTLIST:          return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
      case TOPOLOGY_LINELIST:           return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
      case TOPOLOGY_LINESTRIP:          return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
      case TOPOLOGY_TRIANGLELIST:       return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
      case TOPOLOGY_TRIANGLESTRIP:      return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
      case TOPOLOGY_LINELIST_ADJ:       return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
      case TOPOLOGY_LINESTRIP_ADJ:      return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
      case TOPOLOGY_TRIANGLELIST_ADJ:   return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
      case TOPOLOGY_TRIANGLESTRIP_ADJ:  return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
      //case TOPOLOGY_PATCH_LIST: return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
      }
    };

    assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    assembly_state.topology = get_topology(ia_state.topology);
    assembly_state.primitiveRestartEnable = VK_FALSE;

    // viewport state
    raster_viewports.resize(rc_state.viewports.size());
    for (uint32_t i = 0; i < uint32_t(raster_viewports.size()); ++i)
    {
      const auto& viewport = rc_state.viewports[i];
      raster_viewports[i].x = viewport.origin_x;
      raster_viewports[i].y = viewport.origin_y;
      raster_viewports[i].width = viewport.extent_x;
      raster_viewports[i].height = viewport.extent_y;
      raster_viewports[i].minDepth = viewport.min_z;
      raster_viewports[i].maxDepth = viewport.max_z;
    }

    raster_scissors.resize(rc_state.viewports.size());
    for (uint32_t i = 0; i < uint32_t(rc_state.viewports.size()); ++i)
    {
      raster_scissors[i].offset = { (int32_t)rc_state.viewports[i].origin_x, (int32_t)rc_state.viewports[i].origin_y };
      raster_scissors[i].extent = { (uint32_t)rc_state.viewports[i].extent_x, (uint32_t)rc_state.viewports[i].extent_y };
    }

    viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewport_state.viewportCount = uint32_t(raster_viewports.size());
    viewport_state.pViewports = raster_viewports.data();
    viewport_state.scissorCount = uint32_t(raster_scissors.size());
    viewport_state.pScissors = raster_scissors.data();

    // rasterization state
    const auto get_fill = [](Fill fill)
    {
      switch (fill)
      {
      default:           return VK_POLYGON_MODE_FILL;
      case FILL_POINT:   return VK_POLYGON_MODE_POINT;
      case FILL_LINE:    return VK_POLYGON_MODE_LINE;
      case FILL_SOLID:   return VK_POLYGON_MODE_FILL;
      }
    };

    const auto get_cull = [](Cull cull)
    {
      switch (cull)
      {
      default:          return VK_CULL_MODE_NONE;
      case CULL_NONE:   return VK_CULL_MODE_NONE;
      case CULL_FRONT:  return VK_CULL_MODE_FRONT_BIT;
      case CULL_BACK:   return VK_CULL_MODE_BACK_BIT;
      }
    };

    rasterization_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterization_state.depthClampEnable = VK_FALSE;
    rasterization_state.rasterizerDiscardEnable = VK_FALSE;
    rasterization_state.polygonMode = get_fill(rc_state.fill_mode);
    rasterization_state.lineWidth = rc_state.line_width;
    rasterization_state.cullMode = get_cull(rc_state.cull_mode);
    rasterization_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterization_state.depthBiasEnable = VK_FALSE;
    rasterization_state.depthBiasConstantFactor = 0.0f;
    rasterization_state.depthBiasClamp = 0.0f;
    rasterization_state.depthBiasSlopeFactor = 0.0f;

    // multisample state
    multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisample_state.sampleShadingEnable = VK_FALSE;
    multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisample_state.minSampleShading = 1.0f;
    multisample_state.pSampleMask = nullptr;
    multisample_state.alphaToCoverageEnable = om_state.atc_enabled ? VK_TRUE : VK_FALSE;
    multisample_state.alphaToOneEnable = VK_FALSE;

    // color blend
    const auto get_operand = [](Operand operand)
    {
      switch (operand)
      {
      default:                         return VK_BLEND_FACTOR_ZERO;
      case OPERAND_ZERO:               return VK_BLEND_FACTOR_ZERO;
      case OPERAND_ONE:                return VK_BLEND_FACTOR_ONE;
      case OPERAND_SRC_COLOR:          return VK_BLEND_FACTOR_SRC_COLOR;
      case OPERAND_INV_SRC_COLOR:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
      case OPERAND_SRC_ALPHA:          return VK_BLEND_FACTOR_SRC_ALPHA;
      case OPERAND_INV_SRC_ALPHA:      return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
      case OPERAND_DEST_ALPHA:         return VK_BLEND_FACTOR_DST_ALPHA;
      case OPERAND_INV_DEST_ALPHA:     return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
      case OPERAND_DEST_COLOR:         return VK_BLEND_FACTOR_DST_COLOR;
      case OPERAND_INV_DEST_COLOR:     return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
      case OPERAND_SRC_ALPHA_SAT:      return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
      }
    };

    const auto get_operation = [](Operation operation)
    {
      switch (operation)
      {
      default:                        return VK_BLEND_OP_ADD;
      case OPERATION_ADD:             return VK_BLEND_OP_ADD;
      case OPERATION_SUBTRACT:        return VK_BLEND_OP_SUBTRACT;
      case OPERATION_REV_SUBTRACT:    return VK_BLEND_OP_REVERSE_SUBTRACT;
      case OPERATION_MIN:             return VK_BLEND_OP_MIN;
      case OPERATION_MAX:             return VK_BLEND_OP_MAX;
      }
    };


    colorblend_attachments.resize(om_state.target_blends.size());
    for (size_t i = 0; i < om_state.target_blends.size(); ++i)
    {
      colorblend_attachments[i].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
      colorblend_attachments[i].blendEnable = om_state.target_blends[i].blend_enabled ? VK_TRUE : VK_FALSE;
      colorblend_attachments[i].srcColorBlendFactor = get_operand(om_state.target_blends[i].src_color);
      colorblend_attachments[i].dstColorBlendFactor = get_operand(om_state.target_blends[i].dst_color);
      colorblend_attachments[i].colorBlendOp = get_operation(om_state.target_blends[i].blend_color);
      colorblend_attachments[i].srcAlphaBlendFactor = get_operand(om_state.target_blends[i].src_alpha);
      colorblend_attachments[i].dstAlphaBlendFactor = get_operand(om_state.target_blends[i].dst_alpha);
      colorblend_attachments[i].alphaBlendOp = get_operation(om_state.target_blends[i].blend_alpha);
    }

    colorblend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorblend_state.logicOpEnable = VK_FALSE;
    colorblend_state.logicOp = VK_LOGIC_OP_COPY;
    colorblend_state.attachmentCount = uint32_t(colorblend_attachments.size());
    colorblend_state.pAttachments = colorblend_attachments.data();
    colorblend_state.blendConstants[0] = 0.0f;
    colorblend_state.blendConstants[1] = 0.0f;
    colorblend_state.blendConstants[2] = 0.0f;
    colorblend_state.blendConstants[3] = 0.0f;

    // depth stencil state
    const auto get_comparison = [](Comparison comparison)
    {
      switch (comparison)
      {
      default:                          return VK_COMPARE_OP_ALWAYS;
      case COMPARISON_NEVER:            return VK_COMPARE_OP_NEVER;
      case COMPARISON_LESS:             return VK_COMPARE_OP_LESS;
      case COMPARISON_EQUAL:            return VK_COMPARE_OP_EQUAL;
      case COMPARISON_LESS_EQUAL:       return VK_COMPARE_OP_LESS_OR_EQUAL;
      case COMPARISON_GREATER:          return VK_COMPARE_OP_GREATER;
      case COMPARISON_NOT_EQUAL:        return VK_COMPARE_OP_NOT_EQUAL;
      case COMPARISON_GREATER_EQUAL:    return VK_COMPARE_OP_GREATER_OR_EQUAL;
      case COMPARISON_ALWAYS:           return VK_COMPARE_OP_ALWAYS;
      }
    };

    const auto get_action = [](Action action)
    {
      switch (action)
      {
      default:                return VK_STENCIL_OP_KEEP;
      case ACTION_UNKNOWN:    return VK_STENCIL_OP_KEEP;
      case ACTION_KEEP:       return VK_STENCIL_OP_KEEP;
      case ACTION_ZERO:       return VK_STENCIL_OP_ZERO;
      case ACTION_REPLACE:    return VK_STENCIL_OP_REPLACE;
      case ACTION_INCR_SAT:   return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
      case ACTION_DECR_SAT:   return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
      case ACTION_INVERT:     return VK_STENCIL_OP_INVERT;
      case ACTION_INCR:       return VK_STENCIL_OP_INCREMENT_AND_WRAP;
      case ACTION_DECR:       return VK_STENCIL_OP_DECREMENT_AND_WRAP;
      }
    };


    const auto get_control_points = [](Topology topology)
    {
      switch (topology)
      {
      default: return 0;
      case TOPOLOGY_UNKNOWN: return 0;
      case TOPOLOGY_POINTLIST: return 1;
      case TOPOLOGY_LINELIST: return 1;
      case TOPOLOGY_LINESTRIP: return 1;
      case TOPOLOGY_TRIANGLELIST: return 1;
      case TOPOLOGY_TRIANGLESTRIP: return 1;
      case TOPOLOGY_LINELIST_ADJ: return 1;
      case TOPOLOGY_LINESTRIP_ADJ: return 1;
      case TOPOLOGY_TRIANGLELIST_ADJ: return 1;
      case TOPOLOGY_TRIANGLESTRIP_ADJ: return 1;
      case TOPOLOGY_1_CONTROL_POINT_PATCHLIST: return 1;
      case TOPOLOGY_2_CONTROL_POINT_PATCHLIST: return 2;
      case TOPOLOGY_3_CONTROL_POINT_PATCHLIST: return 3;
      case TOPOLOGY_4_CONTROL_POINT_PATCHLIST: return 4;
      case TOPOLOGY_5_CONTROL_POINT_PATCHLIST: return 5;
      case TOPOLOGY_6_CONTROL_POINT_PATCHLIST: return 6;
      case TOPOLOGY_7_CONTROL_POINT_PATCHLIST: return 7;
      case TOPOLOGY_8_CONTROL_POINT_PATCHLIST: return 8;
      case TOPOLOGY_9_CONTROL_POINT_PATCHLIST: return 9;
      case TOPOLOGY_10_CONTROL_POINT_PATCHLIST: return 10;
      case TOPOLOGY_11_CONTROL_POINT_PATCHLIST: return 11;
      case TOPOLOGY_12_CONTROL_POINT_PATCHLIST: return 12;
      case TOPOLOGY_13_CONTROL_POINT_PATCHLIST: return 13;
      case TOPOLOGY_14_CONTROL_POINT_PATCHLIST: return 14;
      case TOPOLOGY_15_CONTROL_POINT_PATCHLIST: return 15;
      case TOPOLOGY_16_CONTROL_POINT_PATCHLIST: return 16;
      case TOPOLOGY_17_CONTROL_POINT_PATCHLIST: return 17;
      case TOPOLOGY_18_CONTROL_POINT_PATCHLIST: return 18;
      case TOPOLOGY_19_CONTROL_POINT_PATCHLIST: return 19;
      case TOPOLOGY_20_CONTROL_POINT_PATCHLIST: return 20;
      case TOPOLOGY_21_CONTROL_POINT_PATCHLIST: return 21;
      case TOPOLOGY_22_CONTROL_POINT_PATCHLIST: return 22;
      case TOPOLOGY_23_CONTROL_POINT_PATCHLIST: return 23;
      case TOPOLOGY_24_CONTROL_POINT_PATCHLIST: return 24;
      case TOPOLOGY_25_CONTROL_POINT_PATCHLIST: return 25;
      case TOPOLOGY_26_CONTROL_POINT_PATCHLIST: return 26;
      case TOPOLOGY_27_CONTROL_POINT_PATCHLIST: return 27;
      case TOPOLOGY_28_CONTROL_POINT_PATCHLIST: return 28;
      case TOPOLOGY_29_CONTROL_POINT_PATCHLIST: return 29;
      case TOPOLOGY_30_CONTROL_POINT_PATCHLIST: return 30;
      case TOPOLOGY_31_CONTROL_POINT_PATCHLIST: return 31;
      case TOPOLOGY_32_CONTROL_POINT_PATCHLIST: return 32;
      }
    };

    VkStencilOpState front_op_state;
    front_op_state.failOp = get_action(ds_state.stencil_fface_mode.stencil_fail);
    front_op_state.passOp = get_action(ds_state.stencil_fface_mode.stencil_pass);
    front_op_state.depthFailOp = get_action(ds_state.stencil_fface_mode.depth_fail);
    front_op_state.compareOp = get_comparison(ds_state.stencil_fface_mode.comparison);
    front_op_state.compareMask = ds_state.stencil_rmask;
    front_op_state.writeMask = ds_state.stencil_wmask;
    front_op_state.reference = ds_state.stencil_reference;

    VkStencilOpState back_op_state;
    back_op_state.failOp = get_action(ds_state.stencil_bface_mode.stencil_fail);
    back_op_state.passOp = get_action(ds_state.stencil_bface_mode.stencil_pass);
    back_op_state.depthFailOp = get_action(ds_state.stencil_bface_mode.depth_fail);
    back_op_state.compareOp = get_comparison(ds_state.stencil_bface_mode.comparison);
    back_op_state.compareMask = ds_state.stencil_rmask;
    back_op_state.writeMask = ds_state.stencil_wmask;
    back_op_state.reference = ds_state.stencil_reference;

    depthstencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthstencil_state.depthTestEnable = ds_state.depth_enabled ? VK_TRUE : VK_FALSE;
    depthstencil_state.depthWriteEnable = ds_state.depth_write ? VK_TRUE : VK_FALSE;
    depthstencil_state.depthCompareOp = get_comparison(ds_state.depth_comparison);
    depthstencil_state.depthBoundsTestEnable = VK_FALSE;
    depthstencil_state.stencilTestEnable = ds_state.stencil_enabled ? VK_TRUE : VK_FALSE;
    depthstencil_state.front = front_op_state;
    depthstencil_state.back = back_op_state;
    depthstencil_state.minDepthBounds = 0.0f;
    depthstencil_state.maxDepthBounds = 0.0f;

    tessellation_state.sType = VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_STATE_CREATE_INFO;
    tessellation_state.patchControlPoints = get_control_points(ia_state.topology);
    }
  }

  void VLKConfig::Use()
  {
    for (const auto& batch : batches)
    {
      batch->Use();
    }
  }

  void VLKConfig::Discard()
  {
    auto pass = reinterpret_cast<VLKPass*>(&this->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

    stages.clear();
    
    if (vs_module)
    {
      vkDestroyShaderModule(device->GetDevice(), vs_module, nullptr);
      vs_module = nullptr;
    }

    if (hs_module)
    {
      vkDestroyShaderModule(device->GetDevice(), hs_module, nullptr);
      hs_module = nullptr;
    }

    if (ds_module)
    {
      vkDestroyShaderModule(device->GetDevice(), ds_module, nullptr);
      ds_module = nullptr;
    }

    if (gs_module)
    {
      vkDestroyShaderModule(device->GetDevice(), gs_module, nullptr);
      gs_module = nullptr;
    }

    if (ps_module)
    {
      vkDestroyShaderModule(device->GetDevice(), ps_module, nullptr);
      ps_module = nullptr;
    }

    if (cs_module)
    {
      vkDestroyShaderModule(device->GetDevice(), cs_module, nullptr);
      cs_module = nullptr;
    }

    //RTX shaders
    if (rgen_module)
    {
      vkDestroyShaderModule(device->GetDevice(), rgen_module, nullptr);
      rgen_module = nullptr;
    }

    if (chit_module)
    {
      vkDestroyShaderModule(device->GetDevice(), chit_module, nullptr);
      chit_module = nullptr;
    }

    if (ahit_module)
    {
      vkDestroyShaderModule(device->GetDevice(), ahit_module, nullptr);
      ahit_module = nullptr;
    }

    if (miss_module)
    {
      vkDestroyShaderModule(device->GetDevice(), miss_module, nullptr);
      miss_module = nullptr;
    }
  }

  VLKConfig::VLKConfig(const std::string& name,
    Pass& pass,
    const std::string& source,
    Config::Compilation compilation,
    const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
    const Config::IAState& ia_state,
    const Config::RCState& rc_state,
    const Config::DSState& ds_state,
    const Config::OMState& om_state)
    : Config(name, pass, source, compilation, defines, ia_state, rc_state, ds_state, om_state)
  {
    VLKConfig::Initialize();
  }

  VLKConfig::~VLKConfig()
  {
    VLKConfig::Discard();
  }
}
