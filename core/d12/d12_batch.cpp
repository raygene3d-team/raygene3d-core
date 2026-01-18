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
#include "d12_batch.h"
#include "d12_config.h"
#include "d12_pass.h"
#include "d12_device.h"
#include "d12_resource.h"
#include "d12_view.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d12.lib")

namespace RayGene3D
{
  void D12Batch::Initialize()
  {
    auto config = reinterpret_cast<D12Config*>(&this->GetConfig());
    auto pass = reinterpret_cast<D12Pass*>(&config->GetPass());
    auto device = reinterpret_cast<D12Device*>(&pass->GetDevice());

    const auto get_filter = [this](Sampler::Filtering filtering, bool compare)
      {
        switch (filtering)
        {
        default: return D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case Sampler::FILTERING_NEAREST: return compare ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_POINT : D3D12_FILTER_MIN_MAG_MIP_POINT;
        case Sampler::FILTERING_LINEAR: return compare ? D3D12_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR : D3D12_FILTER_MIN_MAG_MIP_LINEAR;
        case Sampler::FILTERING_ANISOTROPIC: return compare ? D3D12_FILTER_COMPARISON_ANISOTROPIC : D3D12_FILTER_ANISOTROPIC;
        }
      };

    const auto get_addressing = [this](Sampler::Addressing addressing)
      {
        switch (addressing)
        {
        default: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case Sampler::ADDRESSING_REPEAT: return D3D12_TEXTURE_ADDRESS_MODE_WRAP;
        case Sampler::ADDRESSING_MIRROR: return D3D12_TEXTURE_ADDRESS_MODE_MIRROR;
        case Sampler::ADDRESSING_CLAMP: return D3D12_TEXTURE_ADDRESS_MODE_CLAMP;
        case Sampler::ADDRESSING_BORDER: return D3D12_TEXTURE_ADDRESS_MODE_BORDER;
        }
      };

    const auto get_comparison = [this](Sampler::Comparison comparison)
      {
        switch (comparison)
        {
        default: return D3D12_COMPARISON_FUNC_NONE;
        case Sampler::COMPARISON_NEVER: return D3D12_COMPARISON_FUNC_NEVER;
        case Sampler::COMPARISON_LESS: return D3D12_COMPARISON_FUNC_LESS;
        case Sampler::COMPARISON_EQUAL: return D3D12_COMPARISON_FUNC_EQUAL;
        case Sampler::COMPARISON_LESS_EQUAL: return D3D12_COMPARISON_FUNC_LESS_EQUAL;
        case Sampler::COMPARISON_GREATER: return D3D12_COMPARISON_FUNC_GREATER;
        case Sampler::COMPARISON_NOT_EQUAL: return D3D12_COMPARISON_FUNC_NOT_EQUAL;
        case Sampler::COMPARISON_GREATER_EQUAL: return D3D12_COMPARISON_FUNC_GREATER_EQUAL;
        case Sampler::COMPARISON_ALWAYS: return D3D12_COMPARISON_FUNC_ALWAYS;
        }
      };

    sampler_descs.resize(samplers.size());
    for (uint32_t i = 0; i < sampler_descs.size(); ++i)
    {
      sampler_descs[i].Filter = get_filter(samplers[i].filtering, samplers[i].comparison != Sampler::COMPARISON_NEVER);
      sampler_descs[i].AddressU = get_addressing(samplers[i].addressing);
      sampler_descs[i].AddressV = get_addressing(samplers[i].addressing);
      sampler_descs[i].AddressW = get_addressing(samplers[i].addressing);
      sampler_descs[i].MipLODBias = samplers[i].bias_lod;
      sampler_descs[i].MaxAnisotropy = samplers[i].anisotropy;
      sampler_descs[i].ComparisonFunc = get_comparison(samplers[i].comparison);
      sampler_descs[i].BorderColor = D3D12_STATIC_BORDER_COLOR_OPAQUE_BLACK;
      sampler_descs[i].MinLOD = samplers[i].min_lod;
      sampler_descs[i].MaxLOD = samplers[i].max_lod;
      sampler_descs[i].ShaderRegister = i;
      sampler_descs[i].RegisterSpace = 0;
      sampler_descs[i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }

    if (pass->GetType() == Pass::TYPE_TRACING && device->GetRayTracingSupported())
    {
      ID3D12GraphicsCommandList7* command_list = nullptr;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
        device->GetCommandAllocator(), nullptr, IID_PPV_ARGS(&command_list)));
      BLAST_ASSERT(S_OK == command_list->Close());

      size_t fence_value = 0;
      ID3D12Fence* fence = nullptr;
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateFence(fence_value, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
      HANDLE fence_event = CreateEvent(nullptr, false, false, nullptr);

      {
        BLAST_ASSERT(S_OK == command_list->Reset(device->GetCommandAllocator(), nullptr));

        blas_items.resize(entities.size(), nullptr);
        for (auto i = 0u; i < uint32_t(entities.size()); ++i)
        {
          const auto& entity = entities[i];

          const auto vtx_resource = reinterpret_cast<D12Resource*>(&entity.va_views[0]->GetResource());
          const auto vtx_stride = vtx_resource->GetLayersOrStride();
          const auto vtx_count = entity.vtx_or_grid_y.length;
          const auto vtx_offset = entity.vtx_or_grid_y.offset;
          const auto vtx_address = vtx_resource->GetAddress();

          const auto idx_resource = reinterpret_cast<D12Resource*>(&entity.ia_views[0]->GetResource());
          const auto idx_stride = idx_resource->GetLayersOrStride();
          const auto idx_count = entity.idx_or_grid_z.length;
          const auto idx_offset = entity.idx_or_grid_z.offset;
          const auto idx_address = idx_resource->GetAddress();

          D3D12_RAYTRACING_GEOMETRY_DESC geometry_desc = {};
          geometry_desc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
          geometry_desc.Triangles.VertexBuffer.StartAddress = vtx_address;
          geometry_desc.Triangles.VertexBuffer.StrideInBytes = vtx_stride;
          geometry_desc.Triangles.VertexFormat = DXGI_FORMAT_R32G32B32_FLOAT;
          geometry_desc.Triangles.VertexCount = vtx_count;
          geometry_desc.Triangles.IndexBuffer = idx_address;
          geometry_desc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
          geometry_desc.Triangles.IndexCount = idx_count;
          geometry_desc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

          D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC as_desc = {};
          as_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
          as_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
          as_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
          as_desc.Inputs.NumDescs = 1;
          as_desc.Inputs.pGeometryDescs = &geometry_desc;

          D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild_info = {};
          device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_desc.Inputs, &prebuild_info);

          BLAST_ASSERT(device->GetScratchSize() >= prebuild_info.ScratchDataSizeInBytes);
          {
            D3D12_RESOURCE_DESC resource_desc = {};
            resource_desc.Alignment = 0;
            resource_desc.DepthOrArraySize = 1;
            resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
            resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
            resource_desc.Format = DXGI_FORMAT_UNKNOWN;
            resource_desc.Height = 1;
            resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
            resource_desc.MipLevels = 1;
            resource_desc.SampleDesc.Count = 1;
            resource_desc.SampleDesc.Quality = 0;
            resource_desc.Width = prebuild_info.ResultDataMaxSizeInBytes;

            D3D12_HEAP_PROPERTIES heap_prop = {};
            heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
            heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
            heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
            heap_prop.CreationNodeMask = 0;
            heap_prop.VisibleNodeMask = 0;

            BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
              &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&blas_items[i])));
          }

          as_desc.DestAccelerationStructureData = blas_items[i]->GetGPUVirtualAddress();
          as_desc.ScratchAccelerationStructureData = device->GetScratchBuffer()->GetGPUVirtualAddress();

          command_list->BuildRaytracingAccelerationStructure(&as_desc, 0, nullptr);

          D3D12_RESOURCE_BARRIER barrier = {};
          barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
          barrier.UAV.pResource = blas_items[i];
          command_list->ResourceBarrier(1, &barrier);
        }

        {
          D3D12_RESOURCE_DESC resource_desc = {};
          resource_desc.Alignment = 0;
          resource_desc.DepthOrArraySize = 1;
          resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
          resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;
          resource_desc.Format = DXGI_FORMAT_UNKNOWN;
          resource_desc.Height = 1;
          resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
          resource_desc.MipLevels = 1;
          resource_desc.SampleDesc.Count = 1;
          resource_desc.SampleDesc.Quality = 0;
          resource_desc.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * entities.size();

          D3D12_HEAP_PROPERTIES heap_prop = {};
          heap_prop.Type = D3D12_HEAP_TYPE_UPLOAD;
          heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
          heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
          heap_prop.CreationNodeMask = 0;
          heap_prop.VisibleNodeMask = 0;

          BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
            &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&instances_item)));
        }

        D3D12_RAYTRACING_INSTANCE_DESC* instance_descs = nullptr;
        BLAST_ASSERT(S_OK == instances_item->Map(0, nullptr, (void**)&instance_descs));
        for (auto i = 0u; i < uint32_t(entities.size()); ++i)
        {
          instance_descs[i] = { {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
          }, i, 0xFF, 0, D3D12_RAYTRACING_INSTANCE_FLAG_TRIANGLE_CULL_DISABLE, blas_items[i]->GetGPUVirtualAddress() };
        }
        instances_item->Unmap(0, nullptr);

        D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC as_desc = {};
        as_desc.Inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;
        as_desc.Inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
        as_desc.Inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_NONE;
        as_desc.Inputs.NumDescs = 1;
        as_desc.Inputs.InstanceDescs = instances_item->GetGPUVirtualAddress();

        D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO prebuild_info;
        device->GetDevice()->GetRaytracingAccelerationStructurePrebuildInfo(&as_desc.Inputs, &prebuild_info);

        BLAST_ASSERT(device->GetScratchSize() >= prebuild_info.ScratchDataSizeInBytes);
        {
          D3D12_RESOURCE_DESC resource_desc = {};
          resource_desc.Alignment = 0;
          resource_desc.DepthOrArraySize = 1;
          resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
          resource_desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
          resource_desc.Format = DXGI_FORMAT_UNKNOWN;
          resource_desc.Height = 1;
          resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
          resource_desc.MipLevels = 1;
          resource_desc.SampleDesc.Count = 1;
          resource_desc.SampleDesc.Quality = 0;
          resource_desc.Width = prebuild_info.ResultDataMaxSizeInBytes;

          D3D12_HEAP_PROPERTIES heap_prop = {};
          heap_prop.Type = D3D12_HEAP_TYPE_DEFAULT;
          heap_prop.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
          heap_prop.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
          heap_prop.CreationNodeMask = 0;
          heap_prop.VisibleNodeMask = 0;

          BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommittedResource(&heap_prop, D3D12_HEAP_FLAG_NONE,
            &resource_desc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, nullptr, IID_PPV_ARGS(&tlas_item)));
        }

        as_desc.DestAccelerationStructureData = tlas_item->GetGPUVirtualAddress();
        as_desc.ScratchAccelerationStructureData = device->GetScratchBuffer()->GetGPUVirtualAddress();

        command_list->BuildRaytracingAccelerationStructure(&as_desc, 0, nullptr);

        D3D12_RESOURCE_BARRIER barrier = {};
        barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
        barrier.UAV.pResource = tlas_item;
        command_list->ResourceBarrier(1, &barrier);

        BLAST_ASSERT(S_OK == command_list->Close());
        device->GetCommandQueue()->ExecuteCommandLists(1, reinterpret_cast<ID3D12CommandList**>(&command_list));

        ++fence_value;
        BLAST_ASSERT(S_OK == device->GetCommandQueue()->Signal(fence, fence_value));
        BLAST_ASSERT(S_OK == fence->SetEventOnCompletion(fence_value, fence_event));
        WaitForSingleObject(fence_event, INFINITE);
      }
      command_list->Release();
      fence->Release();
      CloseHandle(fence_event);

      D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc = {};
      srv_desc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
      srv_desc.Format = DXGI_FORMAT_UNKNOWN;
      srv_desc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
      srv_desc.RaytracingAccelerationStructure.Location = tlas_item->GetGPUVirtualAddress();

      tlas_slot = device->ObtainGeneralSlot(); const auto handle = device->GetGeneralHandle(tlas_slot);
      device->GetDevice()->CreateShaderResourceView(nullptr, &srv_desc, handle.cpu);
      

      //as_items.push_back(tlas_item);
    }


    const auto ub_count = ub_views.size();
    ub_items.resize(std::min(ub_count, size_t(D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)));
    for (size_t i = 0; i < ub_items.size(); ++i)
    {
      if (i < ub_count)
      {
        const auto& ub_view = ub_views[i];
        if (ub_view)
        {
          ub_items[i] = (reinterpret_cast<D12View*>(ub_view.get()))->GetAddress();
        }
      }
    }

    const auto sb_count = sb_views.size();
    sb_items.resize(std::min(sb_count, size_t(D3D12_COMMONSHADER_CONSTANT_BUFFER_API_SLOT_COUNT - 1)));
    for (size_t i = 0; i < sb_items.size(); ++i)
    {
      if (i < sb_count)
      {
        const auto& sb_view = sb_views[i];
        if (sb_view)
        {
          sb_items[i] = (reinterpret_cast<D12View*>(sb_view.get()))->GetAddress();
        }
      }
    }

    const auto rr_count = rb_views.size() + ri_views.size();
    rr_items.resize(std::min(rr_count, size_t(D3D12_COMMONSHADER_INPUT_RESOURCE_SLOT_COUNT)));
    for (size_t i = 0; i < rr_items.size(); ++i)
    {
      if (i < rr_count)
      {
        size_t offset = 0u;
        if (i - offset < rb_views.size() && rb_views[i - offset])
        {
          const auto slot = (reinterpret_cast<D12View*>(rb_views[i - offset].get()))->GetSlot();
          rr_items[i] = device->GetGeneralHandle(slot, true).gpu; continue;
        }

        offset += rb_views.size();
        if (i - offset < ri_views.size() && ri_views[i - offset])
        {
          const auto slot = (reinterpret_cast<D12View*>(ri_views[i - offset].get()))->GetSlot();
          rr_items[i] = device->GetGeneralHandle(slot, true).gpu; continue;
        }
      }
    }

    if (tlas_slot != uint32_t(-1))
    {
      size_t offset = rr_items.size();
      rr_items.resize(offset + 1);
      rr_items[offset] = device->GetGeneralHandle(tlas_slot, true).gpu;
    }

    const auto wr_count = wb_views.size() + wi_views.size();
    wr_items.resize(std::min(wr_count, size_t(D3D12_PS_CS_UAV_REGISTER_COUNT)));
    for (size_t i = 0; i < wr_items.size(); ++i)
    {
      if (i < wr_count)
      {
        size_t offset = 0u;
        if (i - offset < wb_views.size() && wb_views[i - offset])
        {
          const auto slot = (reinterpret_cast<D12View*>(wb_views[i - offset].get()))->GetSlot();
          wr_items[i] = device->GetGeneralHandle(slot, true).gpu; continue;
        }

        offset += wb_views.size();
        if (i - offset < wi_views.size() && wi_views[i - offset])
        {
          const auto slot = (reinterpret_cast<D12View*>(wi_views[i - offset].get()))->GetSlot();
          wr_items[i] = device->GetGeneralHandle(slot, true).gpu; continue;
        }
      }
    }

    auto parameter_offset = 0ull;

    root_parameters.resize(parameter_offset + ub_items.size());
    for (size_t i = 0; i < ub_items.size(); ++i)
    {
      root_parameters[parameter_offset + i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
      root_parameters[parameter_offset + i].Descriptor = { uint32_t(i + parameter_offset), 0 };
      root_parameters[parameter_offset + i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }
    parameter_offset += ub_items.size();

    root_parameters.resize(parameter_offset + sb_items.size());
    for (size_t i = 0; i < sb_items.size(); ++i)
    {
      root_parameters[parameter_offset + i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
      root_parameters[parameter_offset + i].Descriptor = { uint32_t(i + parameter_offset), 0 };
      root_parameters[parameter_offset + i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }
    parameter_offset += sb_items.size();


    std::vector<D3D12_DESCRIPTOR_RANGE> rr_ranges(rr_items.size());
    root_parameters.resize(parameter_offset + rr_ranges.size());
    for (size_t i = 0; i < rr_ranges.size(); ++i)
    {
      rr_ranges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
      rr_ranges[i].NumDescriptors = 1;
      rr_ranges[i].BaseShaderRegister = i;
      rr_ranges[i].RegisterSpace = 0;
      rr_ranges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

      root_parameters[parameter_offset + i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
      root_parameters[parameter_offset + i].DescriptorTable = { 1u, &rr_ranges[i] };
      root_parameters[parameter_offset + i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
    }
    parameter_offset += rr_items.size();


    std::vector<D3D12_DESCRIPTOR_RANGE> wr_ranges(wr_items.size());
    root_parameters.resize(parameter_offset + wr_ranges.size());
    for (size_t i = 0; i < wr_ranges.size(); ++i)
    {
      wr_ranges[i].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
      wr_ranges[i].NumDescriptors = 1;
      wr_ranges[i].BaseShaderRegister = i;
      wr_ranges[i].RegisterSpace = 0;
      wr_ranges[i].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

      root_parameters[parameter_offset + i].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
      root_parameters[parameter_offset + i].DescriptorTable = { 1u, &wr_ranges[i] };
      root_parameters[parameter_offset + i].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
      parameter_offset += 1;
    }

    D3D12_ROOT_SIGNATURE_DESC signature_desc = {};
    signature_desc.NumParameters = root_parameters.size();
    signature_desc.pParameters = root_parameters.data();
    signature_desc.NumStaticSamplers = sampler_descs.size();
    signature_desc.pStaticSamplers = sampler_descs.data();
    signature_desc.Flags =
      D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
      D3D12_ROOT_SIGNATURE_FLAG_CBV_SRV_UAV_HEAP_DIRECTLY_INDEXED;

    ID3DBlob* signature{ nullptr };
    ID3DBlob* errors{ nullptr };
    BLAST_ASSERT(S_OK == D3D12SerializeRootSignature(&signature_desc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &errors));

    if (errors)
    {
      BLAST_LOG("signature serialization output: \n%s", reinterpret_cast<char*>(errors->GetBufferPointer()));
      errors->Release();
    }

    if (signature)
    {
      BLAST_ASSERT(S_OK == device->GetDevice()->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(),
        IID_PPV_ARGS(&root_signature)));
      signature->Release();
    }

    switch (pass->GetType())
    {
    case Pass::TYPE_GRAPHIC:
    {
      {
        D3D12_GRAPHICS_PIPELINE_STATE_DESC pso_desc = {};
        pso_desc.pRootSignature = root_signature;
        pso_desc.VS = config->GetVSBytecode();
        pso_desc.PS = config->GetPSBytecode();
        pso_desc.HS = config->GetHSBytecode();
        pso_desc.DS = config->GetDSBytecode();
        pso_desc.GS = config->GetGSBytecode();
        pso_desc.StreamOutput = {};
        pso_desc.BlendState = config->GetBlendDesc();
        pso_desc.SampleMask = UINT_MAX;
        pso_desc.RasterizerState = config->GetRasterDesc();
        pso_desc.DepthStencilState = config->GetDepthDesc();
        pso_desc.InputLayout = config->GetLayoutDesc();
        pso_desc.IBStripCutValue = D3D12_INDEX_BUFFER_STRIP_CUT_VALUE_DISABLED;
        pso_desc.PrimitiveTopologyType = config->GetTopologyType();
        pso_desc.NumRenderTargets = pass->GetRTCount();
        pso_desc.RTVFormats[0] = pass->GetRTFormat(0);
        pso_desc.RTVFormats[1] = pass->GetRTFormat(1);
        pso_desc.RTVFormats[2] = pass->GetRTFormat(2);
        pso_desc.RTVFormats[3] = pass->GetRTFormat(3);
        pso_desc.RTVFormats[4] = pass->GetRTFormat(4);
        pso_desc.RTVFormats[5] = pass->GetRTFormat(5);
        pso_desc.RTVFormats[6] = pass->GetRTFormat(6);
        pso_desc.RTVFormats[7] = pass->GetRTFormat(7);
        pso_desc.DSVFormat = pass->GetDSFormat(0);
        pso_desc.SampleDesc = { 1, 0 };
        pso_desc.NodeMask = 0;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateGraphicsPipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state)));
      }

      {
        D3D12_INDIRECT_ARGUMENT_DESC argument_desc = {};
        argument_desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DRAW_INDEXED;

        D3D12_COMMAND_SIGNATURE_DESC signature_desc = {};
        signature_desc.ByteStride = uint32_t(sizeof(Graphic));
        signature_desc.NumArgumentDescs = 1;
        signature_desc.pArgumentDescs = &argument_desc;

        BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommandSignature(&signature_desc, nullptr, IID_PPV_ARGS(&command_signature)));
      }
    } break;
    case Pass::TYPE_COMPUTE:
    {
      {
        D3D12_COMPUTE_PIPELINE_STATE_DESC pso_desc = {};
        pso_desc.pRootSignature = root_signature;
        pso_desc.CS = config->GetCSBytecode();
        pso_desc.NodeMask = 0;
        pso_desc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateComputePipelineState(&pso_desc, IID_PPV_ARGS(&pipeline_state)));

        //D3D12_PIPELINE_STATE_STREAM_DESC state_desc = {};
        //device->GetDevice()->CreatePipelineState(&state_desc, IID_PPV_ARGS(&pipeline_state));
      }

      {
        D3D12_INDIRECT_ARGUMENT_DESC argument_desc = {};
        argument_desc.Type = D3D12_INDIRECT_ARGUMENT_TYPE_DISPATCH;

        D3D12_COMMAND_SIGNATURE_DESC signature_desc = {};
        signature_desc.ByteStride = uint32_t(sizeof(Compute));
        signature_desc.NumArgumentDescs = 1;
        signature_desc.pArgumentDescs = &argument_desc;

        BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommandSignature(&signature_desc, nullptr, IID_PPV_ARGS(&command_signature)));
      }
    } break;


    case Pass::TYPE_TRACING:
    {
      {
        std::vector<D3D12_STATE_SUBOBJECT> state_subobjects;

        D3D12_GLOBAL_ROOT_SIGNATURE global_root_signature = {};
        global_root_signature.pGlobalRootSignature = root_signature;
        state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE, (const void*)&global_root_signature });

        //D3D12_LOCAL_ROOT_SIGNATURE local_root_signature = {};
        //local_root_signature.pLocalRootSignature = root_signature;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_LOCAL_ROOT_SIGNATURE, (const void*)&local_root_signature });

        D3D12_EXPORT_DESC rgen_export_desc = { rgen_name, nullptr, D3D12_EXPORT_FLAG_NONE };
        D3D12_DXIL_LIBRARY_DESC rgen_library_desc = {};
        rgen_library_desc.DXILLibrary = config->GetRGenBytecode();
        rgen_library_desc.pExports = &rgen_export_desc;
        rgen_library_desc.NumExports = 1;
        state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &rgen_library_desc });

        //D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION rgen_export_association = {};
        //const wchar_t* rgen_export_name[] = {rgen_name};
        //rgen_export_association.pSubobjectToAssociate = &state_subobjects[0];
        //rgen_export_association.pExports = rgen_export_name;
        //rgen_export_association.NumExports = 1;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, (const void*)&rgen_export_association });

        //D3D12_EXPORT_DESC ahit_export_desc = { ahit_name, nullptr, D3D12_EXPORT_FLAG_NONE };
        //D3D12_DXIL_LIBRARY_DESC ahit_library_desc = {};
        //ahit_library_desc.DXILLibrary = config->GetAHitBytecode();
        //ahit_library_desc.pExports = &ahit_export_desc;
        //ahit_library_desc.NumExports = 1;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &ahit_library_desc });

        //D3D12_EXPORT_DESC chit_export_desc = { chit_name, nullptr, D3D12_EXPORT_FLAG_NONE };
        //D3D12_DXIL_LIBRARY_DESC chit_library_desc = {};
        //chit_library_desc.DXILLibrary = config->GetCHitBytecode();
        //chit_library_desc.pExports = &chit_export_desc;
        //chit_library_desc.NumExports = 1;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &chit_library_desc });

        //D3D12_EXPORT_DESC isec_export_desc = { isec_name, nullptr, D3D12_EXPORT_FLAG_NONE };
        //D3D12_DXIL_LIBRARY_DESC isec_library_desc = {};
        //isec_library_desc.DXILLibrary = config->GetISecBytecode();
        //isec_library_desc.pExports = &isec_export_desc;
        //isec_library_desc.NumExports = 1;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &isec_library_desc });
        //
        //D3D12_HIT_GROUP_DESC hit_group_desc = {};
        //hit_group_desc.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;
        //hit_group_desc.HitGroupExport = xhit_name;
        //hit_group_desc.AnyHitShaderImport = ahit_name;
        //hit_group_desc.ClosestHitShaderImport = chit_name;
        //hit_group_desc.IntersectionShaderImport = isec_name;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP, (const void*)&hit_group_desc }); 

        //D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION xhit_export_association = {};
        //const wchar_t* xhit_export_name[] = { xhit_name };
        //xhit_export_association.pSubobjectToAssociate = &state_subobjects[0];
        //xhit_export_association.pExports = xhit_export_name;
        //xhit_export_association.NumExports = 1;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, (const void*)&xhit_export_association });

        D3D12_EXPORT_DESC miss_export_desc = { miss_name, nullptr, D3D12_EXPORT_FLAG_NONE };
        D3D12_DXIL_LIBRARY_DESC miss_library_desc = {};
        miss_library_desc.DXILLibrary = config->GetMissBytecode();
        miss_library_desc.pExports = &miss_export_desc;
        miss_library_desc.NumExports = 1;
        state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY, &miss_library_desc });

        //D3D12_SUBOBJECT_TO_EXPORTS_ASSOCIATION miss_export_association = {};
        //const wchar_t* miss_export_name[] = { miss_name };
        //miss_export_association.pSubobjectToAssociate = &state_subobjects[0];
        //miss_export_association.pExports = miss_export_name;
        //miss_export_association.NumExports = 1;
        //state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_SUBOBJECT_TO_EXPORTS_ASSOCIATION, (const void*)&miss_export_association });

        D3D12_RAYTRACING_SHADER_CONFIG raytracing_shader_config = {};
        raytracing_shader_config.MaxAttributeSizeInBytes = 2 * sizeof(float);
        raytracing_shader_config.MaxPayloadSizeInBytes = 4 * sizeof(float);
        state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG, (const void*)&raytracing_shader_config });

        D3D12_RAYTRACING_PIPELINE_CONFIG raytracing_pipeline_config = {};
        raytracing_pipeline_config.MaxTraceRecursionDepth = 1;
        state_subobjects.push_back({ D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG, (const void*)&raytracing_pipeline_config });

        D3D12_STATE_OBJECT_DESC state_desc = {};
        state_desc.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;
        state_desc.pSubobjects = state_subobjects.data();
        state_desc.NumSubobjects = state_subobjects.size();
        BLAST_ASSERT(S_OK == device->GetDevice()->CreateStateObject(&state_desc, IID_PPV_ARGS(&state_object)));
      }

      {
        uint32_t identifier_size = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
        uint32_t align_size = D3D12_RAYTRACING_SHADER_TABLE_BYTE_ALIGNMENT;
        uint32_t signature_size = (ub_items.size() + sb_items.size() + rr_items.size() + wr_items.size()) * 8;
        uint32_t entry_size = ((identifier_size + signature_size + align_size - 1) / align_size) * align_size;
        uint32_t entry_count = 2; //hardcoded now for 2 shader records 
        uint32_t table_size = entry_size * entry_count;

        {
          D3D12_HEAP_PROPERTIES heap_properties = {};
          heap_properties.Type = D3D12_HEAP_TYPE_UPLOAD;
          heap_properties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
          heap_properties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
          heap_properties.CreationNodeMask = 0;
          heap_properties.VisibleNodeMask = 0;

          D3D12_RESOURCE_DESC  resource_desc = {};
          resource_desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
          resource_desc.Alignment = 0;
          resource_desc.Width = table_size;
          resource_desc.Height = 1;
          resource_desc.DepthOrArraySize = 1;
          resource_desc.MipLevels = 1;
          resource_desc.Format = DXGI_FORMAT_UNKNOWN;
          resource_desc.SampleDesc = { 1, 0 };
          resource_desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
          resource_desc.Flags = D3D12_RESOURCE_FLAG_NONE;

          BLAST_ASSERT(S_OK == device->GetDevice()->CreateCommittedResource(&heap_properties, D3D12_HEAP_FLAG_NONE,
            &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&table_buffer)));
        }

        ID3D12StateObjectProperties* so_properties;
        BLAST_ASSERT(S_OK == state_object->QueryInterface(IID_PPV_ARGS(&so_properties)));

        uint8_t* mapped = nullptr;
        BLAST_ASSERT(S_OK == table_buffer->Map(0, nullptr, (void**)&mapped));
        {
          {
            auto entry_data = mapped + entry_size * 0;
            memcpy(entry_data, so_properties->GetShaderIdentifier(rgen_name), identifier_size);
            entry_data += identifier_size;
            memcpy(entry_data, ub_items.data(), ub_items.size() * 8);
            entry_data += ub_items.size() * 8;
            memcpy(entry_data, sb_items.data(), sb_items.size() * 8);
            entry_data += sb_items.size() * 8;
            memcpy(entry_data, rr_items.data(), rr_items.size() * 8);
            entry_data += rr_items.size() * 8;
            memcpy(entry_data, wr_items.data(), wr_items.size() * 8);
            entry_data += wr_items.size() * 8;
          }
          {
            auto entry_data = mapped + entry_size * 1;
            memcpy(entry_data, so_properties->GetShaderIdentifier(miss_name), identifier_size);
            entry_data += identifier_size;
            memcpy(entry_data, ub_items.data(), ub_items.size() * 8);
            entry_data += ub_items.size() * 8;
            memcpy(entry_data, sb_items.data(), sb_items.size() * 8);
            entry_data += sb_items.size() * 8;
            memcpy(entry_data, rr_items.data(), rr_items.size() * 8);
            entry_data += rr_items.size() * 8;
            memcpy(entry_data, wr_items.data(), wr_items.size() * 8);
            entry_data += wr_items.size() * 8;
          }
          {
            //auto entry_data = mapped + entry_size * 2;
            //memcpy(entry_data, so_properties->GetShaderIdentifier(xhit_name), identifier_size);
            //entry_data += identifier_size;
            //memcpy(entry_data, ub_items.data(), ub_items.size() * 8);
            //entry_data += ub_items.size() * 8;
            //memcpy(entry_data, sb_items.data(), sb_items.size() * 8);
            //entry_data += sb_items.size() * 8;
            //memcpy(entry_data, rr_items.data(), rr_items.size() * 8);
            //entry_data += rr_items.size() * 8;
            //memcpy(entry_data, wr_items.data(), wr_items.size() * 8);
            //entry_data += wr_items.size() * 8;
          }
        }
        table_buffer->Unmap(0, nullptr);

        so_properties->Release();

        rgen_region = { table_buffer->GetGPUVirtualAddress() + entry_size * 0, entry_size };
        miss_region = { table_buffer->GetGPUVirtualAddress() + entry_size * 1, entry_size, entry_size };
      }
    }
    break;
    }
  }

  void D12Batch::Use()
  {
    auto config = reinterpret_cast<D12Config*>(&this->GetConfig());
    auto pass = reinterpret_cast<D12Pass*>(&config->GetPass());
    auto device = reinterpret_cast<D12Device*>(&pass->GetDevice());

    

    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      device->GetCommandList()->SetPipelineState(pipeline_state);
      device->GetCommandList()->SetGraphicsRootSignature(root_signature);

      auto parameter_offset = 0ull;

      for (size_t i = 0; i < ub_items.size(); ++i)
      {
        device->GetCommandList()->SetGraphicsRootConstantBufferView(parameter_offset + i, ub_items[i]);
      }
      parameter_offset += ub_items.size();

      for (size_t i = 0; i < sb_items.size(); ++i)
      {
        device->GetCommandList()->SetGraphicsRootConstantBufferView(parameter_offset + i, sb_items[i]);
      }
      parameter_offset += sb_items.size();

      for (size_t i = 0; i < rr_items.size(); ++i)
      {
        device->GetCommandList()->SetGraphicsRootDescriptorTable(parameter_offset + i, rr_items[i]);
      }
      parameter_offset += rr_items.size();

      for (size_t i = 0; i < wr_items.size(); ++i)
      {
        device->GetCommandList()->SetGraphicsRootDescriptorTable(parameter_offset + i, wr_items[i]);
      }
      parameter_offset += wr_items.size();

      for (size_t i = 0; i < entities.size(); ++i)
      {
        const auto& entity = entities[i];

        if (!sb_views.empty())
        {
          const auto sb_limit = size_t(4u);
          const auto sb_count = std::min(sb_limit, sb_views.size());

          uint32_t sb_offsets[sb_limit] = {};
          uint32_t sb_strides[sb_limit] = {};

          for (size_t j = 0; j < sb_count; ++j)
          {
            const auto& sb_view = sb_views[j];
            if (sb_view)
            {
              sb_offsets[j] = entity.sb_offset ? entity.sb_offset.value()[j] : 0u;
              device->GetCommandList()->SetGraphicsRootConstantBufferView(ub_items.size() + j, sb_items[j] + sb_offsets[j]);
            }
          }
        }

        {

          const size_t va_limit = D3D12_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
          uint32_t va_strides[va_limit]{ 0 };
          uint32_t va_offsets[va_limit]{ 0 };
          D3D12_VERTEX_BUFFER_VIEW va_items[va_limit] = {};
          const auto va_count = std::min(va_limit, entity.va_views.size());
          for (size_t i = 0; i < va_count; ++i)
          {
            const auto& va_view = entity.va_views.at(i);
            if (va_view)
            {
              const auto& resource = reinterpret_cast<const D12Resource*>(&va_view->GetResource());
              const auto desc = resource->GetResource()->GetDesc();
              va_items[i].BufferLocation = resource->GetAddress() + va_view->GetLevelsOrLength().offset;
              va_items[i].SizeInBytes = va_view->GetLevelsOrLength().length == -1 ? size_t(resource->GetLevelsOrLength() * resource->GetLayersOrStride()) : va_view->GetLevelsOrLength().length;
              va_items[i].StrideInBytes = config->GetStrides().at(i);
            }
          }
          device->GetCommandList()->IASetVertexBuffers(0, va_count, va_items);

          const size_t ia_limit = 1u;
          uint32_t ia_offsets[ia_limit]{ 0 };
          DXGI_FORMAT ia_formats[ia_limit]{ DXGI_FORMAT_UNKNOWN };
          D3D12_INDEX_BUFFER_VIEW ia_items[ia_limit] = {};
          const auto ia_count = std::min(ia_limit, entity.ia_views.size());
          for (size_t i = 0; i < ia_count; ++i)
          {
            const auto& ia_view = entity.ia_views.at(i);
            if (ia_view)
            {
              const auto& resource = (reinterpret_cast<const D12Resource*>(&ia_view->GetResource()));
              ia_items[i].BufferLocation = resource->GetAddress() + ia_view->GetLevelsOrLength().offset;
              ia_items[i].SizeInBytes = ia_view->GetLevelsOrLength().length == -1 ? size_t(resource->GetLevelsOrLength() * resource->GetLayersOrStride()) : ia_view->GetLevelsOrLength().length;
              ia_items[i].Format = config->GetIAState().indexer
                == Config::INDEXER_32_BIT ? DXGI_FORMAT_R32_UINT
                : Config::INDEXER_16_BIT ? DXGI_FORMAT_R16_UINT
                : DXGI_FORMAT_UNKNOWN;
            }
          }
          device->GetCommandList()->IASetIndexBuffer(&ia_items[0]);

          device->GetCommandList()->IASetPrimitiveTopology(config->GetPrimitiveTopology());


          if (entity.arg_view)
          {
            const auto aa_buffer = (reinterpret_cast<D12Resource*>(&entity.arg_view->GetResource()))->GetResource();
            //const auto aa_stride = uint32_t(sizeof(Graphic));
            //const auto aa_draws = 1u;
            //const auto aa_offset = chunk.arg_view->GetLevelsOrLength().offset;
            device->GetCommandList()->ExecuteIndirect(
              command_signature,
              1,
              aa_buffer,
              0,
              nullptr,
              0);
          }
          else
          {
            const auto ins_count = entity.ins_or_grid_x.length;
            const auto ins_offset = entity.ins_or_grid_x.offset;
            const auto vtx_count = entity.vtx_or_grid_y.length;
            const auto vtx_offset = entity.vtx_or_grid_y.offset;
            const auto idx_count = entity.idx_or_grid_z.length;
            const auto idx_offset = entity.idx_or_grid_z.offset;
            device->GetCommandList()->DrawIndexedInstanced(idx_count, ins_count, idx_offset, vtx_offset, ins_offset);
          }
        }
      }
    }


    if (pass->GetType() == Pass::TYPE_COMPUTE)
    {
      device->GetCommandList()->SetPipelineState(pipeline_state);
      device->GetCommandList()->SetComputeRootSignature(root_signature);

      auto parameter_offset = 0ull;

      for (size_t i = 0; i < ub_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootConstantBufferView(parameter_offset + i, ub_items[i]);
      }
      parameter_offset += ub_items.size();

      for (size_t i = 0; i < sb_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootConstantBufferView(parameter_offset + i, sb_items[i]);
      }
      parameter_offset += sb_items.size();

      for (size_t i = 0; i < rr_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootDescriptorTable(parameter_offset + i, rr_items[i]);
      }
      parameter_offset += rr_items.size();

      for (size_t i = 0; i < wr_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootDescriptorTable(parameter_offset + i, wr_items[i]);
      }
      parameter_offset += wr_items.size();


      for (size_t i = 0; i < entities.size(); ++i)
      {
        const auto& entity = entities[i];

        if (!sb_views.empty())
        {
          const auto sb_limit = size_t(4u);
          const auto sb_count = std::min(sb_limit, sb_views.size());

          uint32_t sb_offsets[sb_limit] = {};
          uint32_t sb_strides[sb_limit] = {};

          for (size_t j = 0; j < sb_count; ++j)
          {
            const auto& sb_view = sb_views[j];
            if (sb_view)
            {
              sb_offsets[j] = entity.sb_offset ? entity.sb_offset.value()[j] : 0u;
              device->GetCommandList()->SetComputeRootConstantBufferView(ub_items.size() + j, sb_items[j] + sb_offsets[j]);
            }
          }
        }


        if (entity.arg_view)
        {
          const auto aa_buffer = (reinterpret_cast<D12Resource*>(&entity.arg_view->GetResource()))->GetResource();
          //const auto aa_stride = uint32_t(sizeof(Compute));
          //const auto aa_offset = entity.arg_view->GetLevelsOrLength().offset;

          device->GetCommandList()->ExecuteIndirect(
            command_signature,
            1,
            aa_buffer,
            0,
            nullptr,
            0);
        }
        else
        {
          const auto grid_x = entity.ins_or_grid_x.length;
          const auto grid_y = entity.vtx_or_grid_y.length;
          const auto grid_z = entity.idx_or_grid_z.length;
          device->GetCommandList()->Dispatch(grid_x, grid_y, grid_z);
        }
      }
    }

    if (pass->GetType() == Pass::TYPE_TRACING && device->GetRayTracingSupported())
    {
      device->GetCommandList()->SetComputeRootSignature(root_signature);

      auto parameter_offset = 0ull;

      for (size_t i = 0; i < ub_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootConstantBufferView(parameter_offset + i, ub_items[i]);
      }
      parameter_offset += ub_items.size();

      for (size_t i = 0; i < sb_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootConstantBufferView(parameter_offset + i, sb_items[i]);
      }
      parameter_offset += sb_items.size();

      for (size_t i = 0; i < rr_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootDescriptorTable(parameter_offset + i, rr_items[i]);
      }
      parameter_offset += rr_items.size();

      for (size_t i = 0; i < wr_items.size(); ++i)
      {
        device->GetCommandList()->SetComputeRootDescriptorTable(parameter_offset + i, wr_items[i]);
      }
      parameter_offset += wr_items.size();

      //const auto grid_x = subset.vtx_or_grid_x.length;
      //const auto grid_y = subset.idx_or_grid_y.length;
      //const auto grid_z = subset.ins_or_grid_z.length;

      const auto extent_x = device->GetExtentX();
      const auto extent_y = device->GetExtentY();
      const auto extent_z = 1;

      D3D12_DISPATCH_RAYS_DESC dispatch_desc = {};
      dispatch_desc.RayGenerationShaderRecord = rgen_region;
      dispatch_desc.MissShaderTable = miss_region;
      //dispatch_desc.HitGroupTable = xhit_region;
      //dispatch_desc.CallableShaderTable = call_region;     
      dispatch_desc.Width = extent_x;
      dispatch_desc.Height = extent_y;
      dispatch_desc.Depth = extent_z;

      device->GetCommandList()->SetPipelineState1(state_object);
      device->GetCommandList()->DispatchRays(&dispatch_desc);
    }
  }

  void D12Batch::Discard()
  {
    if (tlas_item)
    {
      tlas_item->Release();
      tlas_item = nullptr;
    }

    if (tlas_slot != uint32_t(-1))
    {
      auto config = reinterpret_cast<D12Config*>(&this->GetConfig());
      auto pass = reinterpret_cast<D12Pass*>(&config->GetPass());
      auto device = reinterpret_cast<D12Device*>(&pass->GetDevice());

      device->DropGeneralSlot(tlas_slot);
    }

    for (auto& blas_item : blas_items)
    {
      blas_item->Release();
      blas_item = nullptr;
    }
    blas_items.clear();

    if (instances_item)
    {
      instances_item->Release();
      instances_item = nullptr;
    }

    if (table_buffer)
    {
      table_buffer->Release();
      table_buffer = nullptr;
    }

    if (root_signature)
    {
      root_signature->Release();
      root_signature = nullptr;
    }

    if (pipeline_state)
    {
      pipeline_state->Release();
      pipeline_state = nullptr;
    }


    if (command_signature)
    {
      command_signature->Release();
      command_signature = nullptr;
    }
  }

  D12Batch::D12Batch(const std::string& name,
    Config& config,
    const std::pair<const Entity*, size_t>& entities,
    const std::pair<const Sampler*, size_t>& samplers,
    const std::pair<const std::shared_ptr<View>*, size_t>& ub_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& sb_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& ri_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& wi_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& rb_views,
    const std::pair<const std::shared_ptr<View>*, size_t>& wb_views
  )
    : Batch(name, config, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views)
  {
    D12Batch::Initialize();
  }

  D12Batch::~D12Batch()
  {
    D12Batch::Discard();
  }
}