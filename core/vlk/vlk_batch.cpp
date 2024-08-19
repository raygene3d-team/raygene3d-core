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


#include "vlk_batch.h"
#include "vlk_config.h"
#include "vlk_pass.h"
#include "vlk_device.h"
#include "vlk_view.h"

namespace RayGene3D
{
  void VLKBatch::Initialize()
  {
    auto config = reinterpret_cast<VLKConfig*>(&this->GetConfig());
    auto pass = reinterpret_cast<VLKPass*>(&config->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

    {
      const auto get_image_filter = [this](Sampler::Filtering filtering)
      {
        switch (filtering)
        {
        default: return VK_FILTER_LINEAR;
        case Sampler::FILTERING_NEAREST: return VK_FILTER_NEAREST;
        case Sampler::FILTERING_LINEAR: return VK_FILTER_LINEAR;
        }
      };

      const auto get_mipmap_filter = [this](Sampler::Filtering filtering)
      {
        switch (filtering)
        {
        default: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        case Sampler::FILTERING_NEAREST: return VK_SAMPLER_MIPMAP_MODE_NEAREST;
        case Sampler::FILTERING_LINEAR: return VK_SAMPLER_MIPMAP_MODE_LINEAR;
        }
      };

      const auto get_addressing = [this](Sampler::Addressing addressing)
      {
        switch (addressing)
        {
        default: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case Sampler::ADDRESSING_REPEAT: return VK_SAMPLER_ADDRESS_MODE_REPEAT;
        case Sampler::ADDRESSING_MIRROR: return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
        case Sampler::ADDRESSING_CLAMP: return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
        case Sampler::ADDRESSING_BORDER: return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
        }
      };

      const auto get_comparison = [this](Sampler::Comparison comparison)
      {
        switch (comparison)
        {
        default: return VK_COMPARE_OP_MAX_ENUM;
        case Sampler::COMPARISON_NEVER: return VK_COMPARE_OP_NEVER;
        case Sampler::COMPARISON_LESS: return VK_COMPARE_OP_LESS;
        case Sampler::COMPARISON_EQUAL: return VK_COMPARE_OP_EQUAL;
        case Sampler::COMPARISON_LESS_EQUAL: return VK_COMPARE_OP_LESS_OR_EQUAL;
        case Sampler::COMPARISON_GREATER: return VK_COMPARE_OP_GREATER;
        case Sampler::COMPARISON_NOT_EQUAL: return VK_COMPARE_OP_NOT_EQUAL;
        case Sampler::COMPARISON_GREATER_EQUAL: return VK_COMPARE_OP_GREATER_OR_EQUAL;
        case Sampler::COMPARISON_ALWAYS: return VK_COMPARE_OP_ALWAYS;
        }
      };

      sampler_states.resize(samplers.size());
      for (uint32_t i = 0; i < uint32_t(sampler_states.size()); ++i)
      {
        const auto& sampler = samplers.at(i);

        VkSamplerCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
        create_info.pNext = nullptr;
        create_info.flags = 0;
        create_info.magFilter = get_image_filter(sampler.filtering);
        create_info.minFilter = get_image_filter(sampler.filtering);
        create_info.mipmapMode = get_mipmap_filter(sampler.filtering);
        create_info.addressModeU = get_addressing(sampler.addressing);
        create_info.addressModeV = get_addressing(sampler.addressing);
        create_info.addressModeW = get_addressing(sampler.addressing);
        create_info.mipLodBias = sampler.bias_lod;
        create_info.anisotropyEnable = sampler.filtering != Sampler::FILTERING_ANISOTROPIC ? VK_FALSE : VK_TRUE;
        create_info.maxAnisotropy = float(sampler.anisotropy);
        create_info.compareEnable = sampler.comparison == Sampler::COMPARISON_NEVER ? VK_FALSE : VK_TRUE;
        create_info.compareOp = get_comparison(sampler.comparison);
        create_info.minLod = sampler.min_lod;
        create_info.maxLod = sampler.max_lod;
        create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        create_info.unnormalizedCoordinates = VK_FALSE;

        BLAST_ASSERT(VK_SUCCESS == vkCreateSampler(device->GetDevice(), &create_info, nullptr, &sampler_states.at(i)));
      }
    }


    if (pass->GetType() == Pass::TYPE_RAYTRACING && device->GetRTXSupported())
    {
      {
        vkCreateRayTracingPipelinesKHR = reinterpret_cast<PFN_vkCreateRayTracingPipelinesKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkCreateRayTracingPipelinesKHR"));
        vkGetRayTracingShaderGroupHandlesKHR = reinterpret_cast<PFN_vkGetRayTracingShaderGroupHandlesKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetRayTracingShaderGroupHandlesKHR"));
        vkCmdTraceRaysKHR = reinterpret_cast<PFN_vkCmdTraceRaysKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkCmdTraceRaysKHR"));
        vkCmdTraceRaysIndirectKHR = reinterpret_cast<PFN_vkCmdTraceRaysIndirectKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkCmdTraceRaysIndirectKHR"));
      }

      {
        vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkCreateAccelerationStructureKHR"));
        vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetAccelerationStructureDeviceAddressKHR"));
        vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
        vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkCmdBuildAccelerationStructuresKHR"));
        vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkDestroyAccelerationStructureKHR"));
      }

      {
        VkCommandBufferAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = device->GetCommandPool();
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;
        BLAST_ASSERT(VK_SUCCESS == vkAllocateCommandBuffers(device->GetDevice(), &allocate_info, &command_buffer));

        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        BLAST_ASSERT(VK_SUCCESS == vkCreateFence(device->GetDevice(), &create_info, nullptr, &fence));
      }

      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(command_buffer, &beginInfo));

      {
        blas_memories.resize(entities.size());
        blas_buffers.resize(entities.size());
        blas_items.resize(entities.size());

        for (auto i = 0u; i < uint32_t(entities.size()); ++i)
        {
          auto& blas_memory = blas_memories[i];
          auto& blas_buffer = blas_buffers[i];
          auto& blas_item = blas_items[i];

          const auto& chunk = entities[i];

          const auto vtx_resource = reinterpret_cast<VLKResource*>(&chunk.va_views[0]->GetResource());
          const auto vtx_stride = vtx_resource->GetLayersOrStride();
          const auto vtx_count = chunk.vtx_or_grid_y.length;
          const auto vtx_offset = chunk.vtx_or_grid_y.offset;
          const auto vtx_address = device->GetAddress(vtx_resource->GetBuffer());

          const auto idx_resource = reinterpret_cast<VLKResource*>(&chunk.ia_views[0]->GetResource());
          const auto idx_stride = idx_resource->GetLayersOrStride();
          const auto idx_count = chunk.idx_or_grid_z.length;
          const auto idx_offset = chunk.idx_or_grid_z.offset;
          const auto idx_address = device->GetAddress(idx_resource->GetBuffer());

          //BLAST_LOG("Vertices and Triangles count/offset: %d/%d, %d/%d", va_count, va_offset, ia_count, ia_offset);

          VkAccelerationStructureGeometryKHR structure_geometry{};
          structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
          structure_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
          structure_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
          structure_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
          structure_geometry.geometry.triangles.vertexData.deviceAddress = vtx_address;
          structure_geometry.geometry.triangles.vertexStride = vtx_stride;
          structure_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
          structure_geometry.geometry.triangles.maxVertex = vtx_count - 1;
          structure_geometry.geometry.triangles.indexData.deviceAddress = idx_address;
          structure_geometry.geometry.triangles.indexType = VK_INDEX_TYPE_UINT32;

          VkAccelerationStructureBuildRangeInfoKHR range_info{};
          range_info.primitiveCount = idx_count / 3;
          range_info.primitiveOffset = idx_offset * idx_stride / 3; //byte offset
          range_info.firstVertex = vtx_offset;
          range_info.transformOffset = 0;

          VkAccelerationStructureBuildGeometryInfoKHR geometry_info{};
          geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
          geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
          geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
          geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
          geometry_info.geometryCount = 1;
          geometry_info.pGeometries = &structure_geometry;
          geometry_info.scratchData.deviceAddress = device->GetScratchAddress();;

          VkAccelerationStructureBuildSizesInfoKHR sizes_info{};
          sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
          vkGetAccelerationStructureBuildSizesKHR(device->GetDevice(),
            VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &geometry_info,
            &range_info.primitiveCount,
            &sizes_info);
          BLAST_ASSERT(device->GetScratchSize() >= sizes_info.buildScratchSize);

          {
            const auto size = sizes_info.accelerationStructureSize;
            const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
            const auto buffer = device->CreateBuffer(size, usage);
            const auto requirements = device->GetRequirements(buffer);
            const auto flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            const auto index = device->GetMemoryIndex(flags, requirements.memoryTypeBits);
            const auto memory = device->AllocateMemory(requirements.size, index, true);

            BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

            blas_buffer = buffer;
            blas_memory = memory;
          }

          VkAccelerationStructureCreateInfoKHR create_info{};
          create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
          create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
          create_info.size = sizes_info.accelerationStructureSize;
          create_info.buffer = blas_buffer;
          BLAST_ASSERT(VK_SUCCESS == vkCreateAccelerationStructureKHR(device->GetDevice(), &create_info, nullptr, &blas_item));

          geometry_info.dstAccelerationStructure = blas_item;

          const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
          vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &geometry_info, &range_info_ptr);

          VkMemoryBarrier memory_barrier = {};
          memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
          memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
          memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
          vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
        }
      }

      {
        std::vector<VkAccelerationStructureInstanceKHR> instances(entities.size());

        for (auto i = 0u; i < uint32_t(entities.size()); ++i)
        {
          VkAccelerationStructureDeviceAddressInfoKHR address_info{};
          address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
          address_info.accelerationStructure = blas_items[i];
          const auto blas_address = vkGetAccelerationStructureDeviceAddressKHR(device->GetDevice(), &address_info);

          VkTransformMatrixKHR transformMatrix = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f
          };

          instances[i] = { transformMatrix, i, 0xFF, 0, VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, blas_address };
        }

        {
          const auto size = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);
          const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
          const auto buffer = device->CreateBuffer(size, usage);
          const auto requirements = device->GetRequirements(buffer);
          const auto flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
          const auto index = device->GetMemoryIndex(flags, requirements.memoryTypeBits);
          const auto memory = device->AllocateMemory(requirements.size, index, true);

          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

          instances_buffer = buffer;
          instances_memory = memory;
        }

        {
          void* mapped{ nullptr };
          BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), instances_memory, 0, VK_WHOLE_SIZE, 0, &mapped));
          memcpy(mapped, instances.data(), instances.size() * sizeof(VkAccelerationStructureInstanceKHR));
          vkUnmapMemory(device->GetDevice(), instances_memory);
        }

        VkAccelerationStructureGeometryKHR structure_geometry{};
        structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        structure_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        structure_geometry.geometry.instances.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        structure_geometry.geometry.instances.data.deviceAddress = device->GetAddress(instances_buffer);

        VkAccelerationStructureBuildRangeInfoKHR range_info{};
        range_info.primitiveCount = uint32_t(instances.size());
        range_info.primitiveOffset = 0;
        range_info.firstVertex = 0;
        range_info.transformOffset = 0;

        VkAccelerationStructureBuildGeometryInfoKHR geometry_info{};
        geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        geometry_info.geometryCount = 1;
        geometry_info.pGeometries = &structure_geometry;
        geometry_info.scratchData.deviceAddress = device->GetScratchAddress();

        VkAccelerationStructureBuildSizesInfoKHR sizes_info{};
        sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
        vkGetAccelerationStructureBuildSizesKHR(device->GetDevice(),
          VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
          &geometry_info,
          &range_info.primitiveCount,
          &sizes_info);
        BLAST_ASSERT(device->GetScratchSize() >= sizes_info.buildScratchSize);

        {
          const auto size = sizes_info.accelerationStructureSize;
          const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
          const auto buffer = device->CreateBuffer(size, usage);
          const auto requirements = device->GetRequirements(buffer);
          const auto flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
          const auto index = device->GetMemoryIndex(flags, requirements.memoryTypeBits);
          const auto memory = device->AllocateMemory(requirements.size, index, true);

          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

          tlas_buffer = buffer;
          tlas_memory = memory;
        }

        VkAccelerationStructureCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        create_info.size = sizes_info.accelerationStructureSize;
        create_info.buffer = tlas_buffer;
        BLAST_ASSERT(VK_SUCCESS == vkCreateAccelerationStructureKHR(device->GetDevice(), &create_info, nullptr, &tlas_item));

        geometry_info.dstAccelerationStructure = tlas_item;

        const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
        vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &geometry_info, &range_info_ptr);

        VkMemoryBarrier memory_barrier = {};
        memory_barrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memory_barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        memory_barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vkCmdPipelineBarrier(command_buffer,
          VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
          VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
          0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
      }

      BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(command_buffer));

      VkSubmitInfo submit_info = {};
      submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info.commandBufferCount = 1;
      submit_info.pCommandBuffers = &command_buffer;

      BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(device->GetQueue(), 1, &submit_info, VK_NULL_HANDLE)); // fence));
      BLAST_ASSERT(VK_SUCCESS == vkQueueWaitIdle(device->GetQueue()));

      as_items.push_back(tlas_item);
    }
    

    //ub_views = std::move(cd_views); //TODO: remove this dirty hack
    //ri_views = std::move(sr_views); //TODO: remove this dirty hack

    {
      std::vector<VkDescriptorPoolSize> pool_sizes;
      if (!samplers.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLER, uint32_t(samplers.size()) }); }
      if (!ub_views.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, uint32_t(ub_views.size()) }); }
      if (!sb_views.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, uint32_t(sb_views.size()) }); }
      if (!ri_views.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, uint32_t(ri_views.size()) }); }
      if (!rb_views.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, uint32_t(rb_views.size()) }); }
      if (!wi_views.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, uint32_t(wi_views.size()) }); }
      if (!wb_views.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, uint32_t(wb_views.size()) }); }
      if (!as_items.empty()) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, uint32_t(as_items.size()) }); }

      VkDescriptorPoolCreateInfo poolCreateInfo = {};
      poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolCreateInfo.poolSizeCount = uint32_t(pool_sizes.size());
      poolCreateInfo.pPoolSizes = pool_sizes.data();
      poolCreateInfo.maxSets = uint32_t(sets.size());
      BLAST_ASSERT(VK_SUCCESS == vkCreateDescriptorPool(device->GetDevice(), &poolCreateInfo, nullptr, &pool));
    }

    {
      tables.resize(1, nullptr); //Currently only one descriptor set batch (table)
      auto& table = tables[0];
      table = {};

      std::vector<VkDescriptorSetLayoutBinding> bindings;
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(samplers.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT 
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(ub_views.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(sb_views.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(rb_views.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(ri_views.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(wb_views.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(wi_views.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(as_items.size());
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_KHR | VK_SHADER_STAGE_ANY_HIT_BIT_KHR | VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR | VK_SHADER_STAGE_MISS_BIT_KHR;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }

      VkDescriptorSetLayoutCreateInfo create_info = {};
      create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
      create_info.bindingCount = uint32_t(bindings.size());
      create_info.pBindings = bindings.data();
      BLAST_ASSERT(VK_SUCCESS == vkCreateDescriptorSetLayout(device->GetDevice(), &create_info, nullptr, &table));
    }


    {
      for (uint32_t i = 0; i < uint32_t(sets.size()); ++i)
      {
        VkDescriptorSetAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocate_info.descriptorPool = pool;
        allocate_info.descriptorSetCount = uint32_t(tables.size());
        allocate_info.pSetLayouts = tables.data();
        BLAST_ASSERT(VK_SUCCESS == vkAllocateDescriptorSets(device->GetDevice(), &allocate_info, &sets[i]));
      }

      {
        VkPipelineLayoutCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        create_info.pushConstantRangeCount = uint32_t(constants.size());
        create_info.pPushConstantRanges = constants.data();
        create_info.setLayoutCount = uint32_t(tables.size());
        create_info.pSetLayouts = tables.data();
        BLAST_ASSERT(VK_SUCCESS == vkCreatePipelineLayout(device->GetDevice(), &create_info, nullptr, &layout));
      }
    }


    uint32_t write_offset = 0;

    if(samplers.size() > 0)
    {
      std::vector<VkDescriptorImageInfo> image_infos(samplers.size());
      std::vector<VkWriteDescriptorSet> descriptors(samplers.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& image_info = image_infos.at(i);
        image_info.sampler = sampler_states.at(i);
        image_info.imageView = nullptr;
        image_info.imageLayout = VK_IMAGE_LAYOUT_UNDEFINED;

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = i + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLER;
        descriptor.pImageInfo = &image_info;
        descriptor.pBufferInfo = nullptr;
        descriptor.pTexelBufferView = nullptr;
      }
      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(samplers.size());
    }    
    
    if(ub_views.size() > 0)
    {
      std::vector<VkDescriptorBufferInfo> buffer_infos(ub_views.size());
      std::vector<VkWriteDescriptorSet> descriptors(ub_views.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& buffer_info = buffer_infos.at(i);
        buffer_info.buffer = (reinterpret_cast<VLKResource*>(&ub_views.at(i)->GetResource()))->GetBuffer();
        buffer_info.offset = ub_views.at(i)->GetMipmapsOrCount().offset;        
        buffer_info.range = ub_views.at(i)->GetMipmapsOrCount().length == uint32_t(-1) ? VK_WHOLE_SIZE : ub_views.at(i)->GetMipmapsOrCount().length;

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = i + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptor.pImageInfo = nullptr;
        descriptor.pBufferInfo = &buffer_info;
        descriptor.pTexelBufferView = nullptr;
      }
      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(ub_views.size());
    }

    if (sb_views.size() > 0)
    {
      std::vector<VkDescriptorBufferInfo> buffer_infos(sb_views.size());
      std::vector<VkWriteDescriptorSet> descriptors(sb_views.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& buffer_info = buffer_infos.at(i);
        buffer_info.buffer = (reinterpret_cast<VLKResource*>(&sb_views.at(i)->GetResource()))->GetBuffer();
        buffer_info.offset = sb_views.at(i)->GetMipmapsOrCount().offset;
        buffer_info.range = sb_views.at(i)->GetMipmapsOrCount().length == uint32_t(-1) ? VK_WHOLE_SIZE : sb_views.at(i)->GetMipmapsOrCount().length;

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = i + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptor.pImageInfo = nullptr;
        descriptor.pBufferInfo = &buffer_info;
        descriptor.pTexelBufferView = nullptr;
      }
      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(sb_views.size());
    }
    
    if (rb_views.size() > 0)
    {
      std::vector<VkDescriptorBufferInfo> buffer_infos(rb_views.size());
      std::vector<VkWriteDescriptorSet> descriptors(rb_views.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& buffer_info = buffer_infos.at(i);
        buffer_info.offset = 0;
        buffer_info.buffer = (reinterpret_cast<VLKResource*>(&rb_views.at(i)->GetResource()))->GetBuffer();
        buffer_info.range = VK_WHOLE_SIZE;

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = i + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor.pImageInfo = nullptr;
        descriptor.pBufferInfo = &buffer_info;
        descriptor.pTexelBufferView = nullptr;
      }
      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(rb_views.size());
    }

    if(ri_views.size() > 0)
    {
      std::vector<VkDescriptorImageInfo> image_infos(ri_views.size());
      std::vector<VkWriteDescriptorSet> descriptors(ri_views.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& image_info = image_infos.at(i);
        image_info.sampler = nullptr;
        image_info.imageView = (reinterpret_cast<VLKView*>(ri_views.at(i).get()))->GetView();
        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = i + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        descriptor.pImageInfo = &image_info;
        descriptor.pBufferInfo = nullptr;
        descriptor.pTexelBufferView = nullptr;
      }
      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(ri_views.size());
    }

    if (wb_views.size() > 0)
    {
      std::vector<VkDescriptorBufferInfo> buffer_infos(wb_views.size());
      std::vector<VkWriteDescriptorSet> descriptors(wb_views.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& buffer_info = buffer_infos.at(i);
        buffer_info.offset = 0;
        buffer_info.buffer = (reinterpret_cast<VLKResource*>(&wb_views.at(i)->GetResource()))->GetBuffer();
        buffer_info.range = VK_WHOLE_SIZE;

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = i + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        descriptor.pImageInfo = nullptr;
        descriptor.pBufferInfo = &buffer_info;
        descriptor.pTexelBufferView = nullptr;
      }
      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(wb_views.size());
    }

    if(wi_views.size() > 0)
    {
      std::vector<VkDescriptorImageInfo> image_infos(wi_views.size());
      std::vector<VkWriteDescriptorSet> descriptors(wi_views.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& image_info = image_infos.at(i);
        image_info.sampler = nullptr;
        image_info.imageView = (reinterpret_cast<VLKView*>(wi_views.at(i).get()))->GetView();
        image_info.imageLayout = VK_IMAGE_LAYOUT_GENERAL;

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = i + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
        descriptor.pImageInfo = &image_info;
        descriptor.pBufferInfo = nullptr;
        descriptor.pTexelBufferView = nullptr;
      }
      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(wi_views.size());
    }

    if (as_items.size() > 0)
    {
      std::vector<VkWriteDescriptorSetAccelerationStructureKHR> acceleration_infos(as_items.size());
      std::vector<VkWriteDescriptorSet> descriptors(as_items.size());
      for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
      {
        auto& acceleration_info = acceleration_infos.at(i);
        acceleration_info.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
        acceleration_info.accelerationStructureCount = 1;
        acceleration_info.pAccelerationStructures = &as_items[i];

        auto& descriptor = descriptors.at(i);
        descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor.pNext = &acceleration_info;
        descriptor.dstSet = sets.at(0);
        descriptor.dstBinding = 0 + write_offset;
        descriptor.dstArrayElement = 0;
        descriptor.descriptorCount = 1;
        descriptor.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
        descriptor.pImageInfo = nullptr;
        descriptor.pBufferInfo = nullptr;
        descriptor.pTexelBufferView = nullptr;
      }

      vkUpdateDescriptorSets(device->GetDevice(), uint32_t(descriptors.size()), descriptors.data(), 0, nullptr);
      write_offset += uint32_t(wi_views.size());
    }

    BLAST_LOG("Binding count: %d [%s]", write_offset, name.c_str());

    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      VkGraphicsPipelineCreateInfo create_info = {};
      create_info.sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
      create_info.flags               = 0;
      create_info.stageCount          = config->GetStageCount();
      create_info.pStages             = config->GetStageArray();
      create_info.pVertexInputState   = &config->GetInputState();
      create_info.pInputAssemblyState = &config->GetAssemblyState();
      create_info.pViewportState      = &config->GetViewportState();
      create_info.pRasterizationState = &config->GetRasterizationState();
      create_info.pMultisampleState   = &config->GetMultisampleState();
      create_info.pDepthStencilState  = &config->GetDepthstencilState();
      create_info.pTessellationState  = &config->GetTessellationState();
      create_info.pColorBlendState    = &config->GetColorblendState();
      create_info.pDynamicState       = nullptr;
      create_info.layout              = layout;
      create_info.renderPass          = pass->GetRenderPass();
      create_info.subpass             = 0;
      create_info.basePipelineHandle  = VK_NULL_HANDLE;
      create_info.basePipelineIndex   = -1;
      BLAST_ASSERT(VK_SUCCESS == vkCreateGraphicsPipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));
    }

    if (pass->GetType() == Pass::TYPE_COMPUTE)
    {
      VkComputePipelineCreateInfo create_info = {};
      create_info.sType               = VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO;
      create_info.flags               = 0;
      create_info.stage               = config->GetStageArray()[0];
      create_info.layout              = layout;
      create_info.basePipelineHandle  = VK_NULL_HANDLE;
      create_info.basePipelineIndex   = -1;
      BLAST_ASSERT(VK_SUCCESS == vkCreateComputePipelines(device->GetDevice(), VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));
    }

    if (pass->GetType() == Pass::TYPE_RAYTRACING && device->GetRTXSupported())
    {
      VkRayTracingPipelineCreateInfoKHR create_info = {};
      create_info.sType                         = VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR;
      create_info.flags                         = 0;
      create_info.stageCount                    = config->GetStageCount();
      create_info.pStages                       = config->GetStageArray();
      create_info.groupCount                    = config->GetGroupCount();
      create_info.pGroups                       = config->GetGroupArray();
      create_info.maxPipelineRayRecursionDepth  = 1;
      create_info.layout                        = layout;
      create_info.basePipelineHandle            = VK_NULL_HANDLE;
      BLAST_ASSERT(VK_SUCCESS == vkCreateRayTracingPipelinesKHR(device->GetDevice(), {}, VK_NULL_HANDLE, 1, &create_info, nullptr, &pipeline));

      const auto binding_size = device->GetRTXProperties().shaderGroupHandleSize;
      const auto binding_align = device->GetRTXProperties().shaderGroupBaseAlignment;

      {
        const auto size = config->GetGroupCount() * binding_align;
        const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR;
        const auto buffer = device->CreateBuffer(size, usage);
        const auto requirements = device->GetRequirements(buffer);
        const auto property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
        const auto index = device->GetMemoryIndex(property, requirements.memoryTypeBits);
        const auto memory = device->AllocateMemory(requirements.size, index, true);

        BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

        table_buffer = buffer;
        table_memory = memory;
      }

      auto* binding_data = new uint8_t[config->GetGroupCount() * binding_align];
      BLAST_ASSERT(VK_SUCCESS == vkGetRayTracingShaderGroupHandlesKHR(device->GetDevice(), pipeline, 0, 
        config->GetGroupCount(), config->GetGroupCount() * binding_align, binding_data));

      uint8_t* mapped = nullptr;
      BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), table_memory, 0, VK_WHOLE_SIZE, 0, (void**)&mapped));
      for (uint32_t i = 0; i < config->GetGroupCount(); ++i)
      {
        memcpy(mapped + i * binding_align, binding_data + i * binding_size, binding_size);

        const auto temp = reinterpret_cast<const uint64_t*>(mapped + i * binding_align);
        BLAST_LOG("RTX shader handle %d: %ld %ld %ld %ld", i, temp[0], temp[1], temp[2], temp[3]);
      }
      vkUnmapMemory(device->GetDevice(), table_memory);
      delete[] binding_data;

      if (config->GetGroupCount() > 0)
        rgen_region = { device->GetAddress(table_buffer) + 0 * binding_align, binding_align, binding_align };
      if (config->GetGroupCount() > 1)
        miss_region = { device->GetAddress(table_buffer) + 1 * binding_align, binding_align, binding_align };
      if (config->GetGroupCount() > 2)
        xhit_region = { device->GetAddress(table_buffer) + 2 * binding_align, binding_align, binding_align };
    }
  }

  void VLKBatch::Use()
  {
    auto config = reinterpret_cast<VLKConfig*>(&this->GetConfig());
    auto pass = reinterpret_cast<VLKPass*>(&config->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

    const auto command_buffer = device->GetCommadBuffer();

    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

      if (sb_views.empty())
      {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, sets.size(), sets.data(), 0, nullptr);
      }

      for (const auto& chunk : entities)
      {
        if (!sb_views.empty())
        {
          const auto sb_limit = 4u;
          const auto sb_count = std::min(sb_limit, uint32_t(sb_views.size()));

          uint32_t sb_offsets[sb_limit] = {};
          for (uint32_t i = 0; i < sb_count; ++i)
          {
            sb_offsets[i] = chunk.sb_offset ? chunk.sb_offset.value()[i] : 0u;
          }
          vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, sets.size(), sets.data(), sb_count, sb_offsets);
        }

        {
          const auto va_limit = 16u;
          std::array<uint32_t, va_limit> va_strides;
          std::array<VkDeviceSize, va_limit> va_offsets;
          std::array<VkBuffer, va_limit> va_items;

          const auto va_count = std::min(va_limit, uint32_t(chunk.va_views.size()));
          for (uint32_t i = 0; i < va_count; ++i)
          {
            const auto& va_view = chunk.va_views[i];
            if (va_view)
            {
              va_items[i] = (reinterpret_cast<VLKResource*>(&va_view->GetResource()))->GetBuffer();
              va_offsets[i] = va_view->GetMipmapsOrCount().offset;
            }
          }

          if (va_count > 0)
          {
            vkCmdBindVertexBuffers(command_buffer, 0, va_count, va_items.data(), va_offsets.data());
          }
        }

        {
          const auto ia_limit = 1u;
          std::array<VkIndexType, ia_limit> ia_formats;
          std::array<VkDeviceSize, ia_limit> ia_offsets;
          std::array<VkBuffer, ia_limit> ia_items;

          const auto ia_count = std::min(ia_limit, uint32_t(chunk.ia_views.size()));
          for (uint32_t i = 0; i < ia_count; ++i)
          {
            const auto& ia_view = chunk.ia_views[i];
            if (ia_view)
            {
              ia_items[i] = (reinterpret_cast<VLKResource*>(&ia_view->GetResource()))->GetBuffer();
              ia_offsets[i] = ia_view->GetMipmapsOrCount().offset;
              ia_formats[i] = config->GetIAState().indexer
                == Config::INDEXER_32_BIT ? VK_INDEX_TYPE_UINT32
                : Config::INDEXER_16_BIT ? VK_INDEX_TYPE_UINT16
                : VK_INDEX_TYPE_MAX_ENUM;
            }
          }

          if (ia_count > 0)
          {
            vkCmdBindIndexBuffer(command_buffer, ia_items[0], ia_offsets[0], ia_formats[0]);
          }
        }

        if (chunk.arg_view)
        {
          const auto aa_buffer = (reinterpret_cast<VLKResource*>(&chunk.arg_view->GetResource()))->GetBuffer();
          const auto aa_stride = uint32_t(sizeof(Graphic));
          const auto aa_draws = 1u;
          const auto aa_offset = chunk.arg_view->GetMipmapsOrCount().offset;
          vkCmdDrawIndexedIndirect(command_buffer, aa_buffer, aa_offset, aa_draws, aa_stride);
        }
        else
        {
          const auto ins_count = chunk.ins_or_grid_x.length;
          const auto ins_offset = chunk.ins_or_grid_x.offset;
          const auto vtx_count = chunk.vtx_or_grid_y.length;
          const auto vtx_offset = chunk.vtx_or_grid_y.offset;
          const auto idx_count = chunk.idx_or_grid_z.length;
          const auto idx_offset = chunk.idx_or_grid_z.offset;
          vkCmdDrawIndexed(command_buffer, idx_count, ins_count, idx_offset, vtx_offset, ins_offset);
        }
      }
    }


    if (pass->GetType() == Pass::TYPE_COMPUTE)
    {
      vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, pipeline);

      if (sb_views.empty())
      {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, sets.size(), sets.data(), 0, nullptr);
      }

      for (const auto& chunk : entities)
      {
        if (!sb_views.empty())
        {
          const auto sb_limit = 4u;
          const auto sb_count = std::min(sb_limit, uint32_t(sb_views.size()));

          uint32_t sb_offsets[sb_limit] = {};
          for (uint32_t i = 0; i < sb_count; ++i)
          {
            sb_offsets[i] = chunk.sb_offset ? chunk.sb_offset.value()[i] : 0u;
          }
          vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_COMPUTE, layout, 0, sets.size(), sets.data(), sb_count, sb_offsets);
        }

        if (chunk.arg_view)
        {
          const auto aa_buffer = (reinterpret_cast<VLKResource*>(&chunk.arg_view->GetResource()))->GetBuffer();
          const auto aa_stride = uint32_t(sizeof(Compute));
          const auto aa_offset = chunk.arg_view->GetMipmapsOrCount().offset;
          vkCmdDispatchIndirect(command_buffer, aa_buffer, aa_offset);
        }
        else
        {
          const auto grid_x = chunk.ins_or_grid_x.length;
          const auto grid_y = chunk.vtx_or_grid_y.length;
          const auto grid_z = chunk.idx_or_grid_z.length;
          vkCmdDispatch(command_buffer, grid_x, grid_y, grid_z);
        }
      }
    }


    if (pass->GetType() == Pass::TYPE_RAYTRACING && device->GetRTXSupported())
    {
      vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, pipeline);

      if (sb_views.empty())
      {
        vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR, layout, 0, sets.size(), sets.data(), 0, nullptr);
      }

      //const auto grid_x = subset.vtx_or_grid_x.length;
      //const auto grid_y = subset.idx_or_grid_y.length;
      //const auto grid_z = subset.ins_or_grid_z.length;

      const auto extent_x = device->GetExtentX();
      const auto extent_y = device->GetExtentY();
      vkCmdTraceRaysKHR(command_buffer, &rgen_region, &miss_region, &xhit_region, &call_region, extent_x, extent_y, 1);
    }
  }

  void VLKBatch::Discard()
  {
    //for (auto& command : commands)
    //{
    //  command->Discard();
    //}

    auto config = reinterpret_cast<VLKConfig*>(&this->GetConfig());
    auto pass = reinterpret_cast<VLKPass*>(&config->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

    //RTX section
    if (table_buffer)
    {
      vkDestroyBuffer(device->GetDevice(), table_buffer, nullptr); table_buffer = nullptr;
    }

    if (table_memory)
    {
      vkFreeMemory(device->GetDevice(), table_memory, nullptr); table_memory = nullptr;
    }

    if (fence)
    {
      vkDestroyFence(device->GetDevice(), fence, nullptr); fence = nullptr;
    }

    if (command_buffer)
    {
      vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &command_buffer); command_buffer = nullptr;
    }

    for (auto& blas_item : blas_items)
    {
      if (blas_item)
      {
        vkDestroyAccelerationStructureKHR(device->GetDevice(), blas_item, nullptr); blas_item = nullptr;
      }
    }
    blas_items.clear();

    for (auto& blas_buffer : blas_buffers)
    {
      if (blas_buffer)
      {
        vkDestroyBuffer(device->GetDevice(), blas_buffer, nullptr); blas_buffer = nullptr;
      }
    }
    blas_buffers.clear();

    for (auto& blas_memory : blas_memories)
    {
      if (blas_memory)
      {
        vkFreeMemory(device->GetDevice(), blas_memory, nullptr); blas_memory = nullptr;
      }
    }
    blas_memories.clear();

    if (tlas_item)
    {
      vkDestroyAccelerationStructureKHR(device->GetDevice(), tlas_item, nullptr); tlas_item = nullptr;
    }

    if (tlas_buffer)
    {
      vkDestroyBuffer(device->GetDevice(), tlas_buffer, nullptr); tlas_buffer = nullptr;
    }

    if (tlas_memory)
    {
      vkFreeMemory(device->GetDevice(), tlas_memory, nullptr); tlas_memory = nullptr;
    }

    if (instances_buffer)
    {
      vkDestroyBuffer(device->GetDevice(), instances_buffer, nullptr); instances_buffer = nullptr;
    }

    if (instances_memory)
    {
      vkFreeMemory(device->GetDevice(), instances_memory, nullptr); instances_memory = nullptr;
    }

    for (auto& sampler_state : sampler_states)
    {
      vkDestroySampler(device->GetDevice(), sampler_state, nullptr);
    }
    sampler_states.clear();

    if (layout) 
    { 
      vkDestroyPipelineLayout(device->GetDevice(), layout, nullptr);
      layout = nullptr;
    }

    if (pipeline)
    {
      vkDestroyPipeline(device->GetDevice(), pipeline, nullptr);
      pipeline = nullptr;
    }

    for (auto& table : tables)
    { 
      vkDestroyDescriptorSetLayout(device->GetDevice(), table, nullptr);
    }
    tables.clear();    

    if (pool) 
    { 
      vkDestroyDescriptorPool(device->GetDevice(), pool, nullptr); pool = nullptr;
    }
  }

  VLKBatch::VLKBatch(const std::string& name,
    Config& config,
    const std::pair<const Entity*, uint32_t>& entities,
    const std::pair<const Sampler*, uint32_t>& samplers,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views
  )
    : Batch(name, config, entities, samplers, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views)
  {
    VLKBatch::Initialize();
  }

  VLKBatch::~VLKBatch()
  {
    VLKBatch::Discard();
  }
}
