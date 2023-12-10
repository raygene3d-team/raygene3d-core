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


#include "vlk_layout.h"
#include "vlk_device.h"

namespace RayGene3D
{
  void VLKLayout::Initialize()
  {
    auto device = reinterpret_cast<VLKDevice*>(&this->GetDevice());

    if (device->GetRTXSupported())
    {
      vkCreateAccelerationStructureKHR = reinterpret_cast<PFN_vkCreateAccelerationStructureKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkCreateAccelerationStructureKHR"));
      vkDestroyAccelerationStructureKHR = reinterpret_cast<PFN_vkDestroyAccelerationStructureKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkDestroyAccelerationStructureKHR"));
      //vkBindAccelerationStructureMemoryNV = reinterpret_cast<PFN_vkBindAccelerationStructureMemoryNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkBindAccelerationStructureMemoryNV"));
      vkGetAccelerationStructureDeviceAddressKHR = reinterpret_cast<PFN_vkGetAccelerationStructureDeviceAddressKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetAccelerationStructureDeviceAddressKHR"));
      vkGetAccelerationStructureBuildSizesKHR = reinterpret_cast<PFN_vkGetAccelerationStructureBuildSizesKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetAccelerationStructureBuildSizesKHR"));
      vkCmdBuildAccelerationStructuresKHR = reinterpret_cast<PFN_vkCmdBuildAccelerationStructuresKHR>(vkGetDeviceProcAddr(device->GetDevice(), "vkCmdBuildAccelerationStructuresKHR"));
    }

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
        create_info.compareEnable = sampler.comparison != Sampler::COMPARISON_NEVER ? VK_FALSE : VK_TRUE;
        create_info.compareOp = get_comparison(sampler.comparison);
        create_info.minLod = sampler.min_lod;
        create_info.maxLod = sampler.max_lod;
        create_info.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
        create_info.unnormalizedCoordinates = VK_FALSE;

        BLAST_ASSERT(VK_SUCCESS == vkCreateSampler(device->GetDevice(), &create_info, nullptr, &sampler_states.at(i)));
      }
    }

    if (device->GetRTXSupported() && !rtx_entities.empty())
    {
      BLAST_LOG("RTX Instances count: %d", rtx_entities.size());

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

      std::vector<VkDeviceMemory> scratch_memories(rtx_entities.size(), nullptr);
      std::vector<VkBuffer> scratch_buffers(rtx_entities.size(), nullptr);

      VkDeviceMemory scratch_memory{ nullptr };
      VkBuffer scratch_buffer{ nullptr };

      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(command_buffer, &beginInfo));

      {
        blas_memories.resize(rtx_entities.size());
        blas_buffers.resize(rtx_entities.size());
        blas_items.resize(rtx_entities.size());

        for (uint32_t i = 0; i < uint32_t(rtx_entities.size()); ++i)
        {
          auto& blas_memory = blas_memories[i];
          auto& blas_buffer = blas_buffers[i];
          auto& blas_item = blas_items[i];

          const auto& rtx_entity = rtx_entities[i];

          const auto va_resource = reinterpret_cast<VLKResource*>(&rtx_entity.va_view->GetResource());
          const auto va_stride = va_resource->GetStride();
          const auto va_count = rtx_entity.va_count;
          const auto va_offset = rtx_entity.va_offset;
          const auto va_address = device->GetAddress(va_resource->GetBuffer());

          const auto ia_resource = reinterpret_cast<VLKResource*>(&rtx_entity.ia_view->GetResource());
          const auto ia_stride = ia_resource->GetStride();
          const auto ia_count = rtx_entity.ia_count;
          const auto ia_offset = rtx_entity.ia_offset;
          const auto ia_address = device->GetAddress(ia_resource->GetBuffer());

          const auto primitive_stride = ia_stride;
          const auto primitive_count = ia_count;
          const auto primitive_offset = ia_offset;

          BLAST_LOG("Vertices and Triangles count/offset/stride: %d/%d/%d, %d/%d/%d", va_count, va_offset, va_stride, ia_count, ia_offset, ia_stride);

          VkAccelerationStructureGeometryKHR structure_geometry{};
          structure_geometry.sType                                        = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
          structure_geometry.flags                                        = VK_GEOMETRY_OPAQUE_BIT_KHR;
          structure_geometry.geometryType                                 = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
          structure_geometry.geometry.triangles.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
          structure_geometry.geometry.triangles.vertexData.deviceAddress  = va_address;
          structure_geometry.geometry.triangles.vertexStride              = va_stride;
          structure_geometry.geometry.triangles.vertexFormat              = VK_FORMAT_R32G32B32_SFLOAT;
          structure_geometry.geometry.triangles.maxVertex                 = va_count - 1;
          structure_geometry.geometry.triangles.indexData.deviceAddress   = ia_address;
          structure_geometry.geometry.triangles.indexType                 = VK_INDEX_TYPE_UINT32;

          VkAccelerationStructureBuildRangeInfoKHR range_info{};
          range_info.primitiveCount   = primitive_count;
          range_info.primitiveOffset  = primitive_offset * 4; //byte offset
          range_info.firstVertex      = va_offset;
          range_info.transformOffset  = 0;

          VkAccelerationStructureBuildGeometryInfoKHR geometry_info{};
          geometry_info.sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
          geometry_info.type            = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
          geometry_info.flags           = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
          geometry_info.mode            = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
          geometry_info.geometryCount   = 1;
          geometry_info.pGeometries     = &structure_geometry;

          VkAccelerationStructureBuildSizesInfoKHR sizes_info{};
          sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

          vkGetAccelerationStructureBuildSizesKHR(device->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &geometry_info, &primitive_count, &sizes_info);

          {
            const auto size = sizes_info.accelerationStructureSize;
            const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
            const auto buffer = device->CreateBuffer(size, usage);
            const auto requirements = device->GetRequirements(buffer);
            const auto property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            const auto index = device->GetMemoryIndex(property, requirements.memoryTypeBits);
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

          {
            const auto size = sizes_info.buildScratchSize;
            const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            const auto buffer = device->CreateBuffer(size, usage);
            const auto requirements = device->GetRequirements(buffer);
            const auto property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
            const auto index = device->GetMemoryIndex(property, requirements.memoryTypeBits);
            const auto memory = device->AllocateMemory(requirements.size, index, true);

            BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

            scratch_buffers[i] = buffer;
            scratch_memories[i] = memory;
          }
          
          geometry_info.dstAccelerationStructure = blas_item;
          geometry_info.scratchData.deviceAddress = device->GetAddress(scratch_buffers[i]);

          const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
          vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &geometry_info, &range_info_ptr);
        }
      }

      
      {
        std::vector<VkAccelerationStructureInstanceKHR> instances(rtx_entities.size());

        for (uint32_t i = 0; i < uint32_t(instances.size()); ++i)
        {
          const auto& rtx_entity = rtx_entities[i];

          VkAccelerationStructureDeviceAddressInfoKHR address_info{};
          address_info.sType                  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
          address_info.accelerationStructure  = blas_items[i];
          const auto blas_address = vkGetAccelerationStructureDeviceAddressKHR(device->GetDevice(), &address_info);

          VkTransformMatrixKHR transformMatrix = {
            rtx_entity.transform[0 + 0], rtx_entity.transform[0 + 1], rtx_entity.transform[0 + 2], rtx_entity.transform[0 + 3],
            rtx_entity.transform[4 + 0], rtx_entity.transform[4 + 1], rtx_entity.transform[4 + 2], rtx_entity.transform[4 + 3],
            rtx_entity.transform[8 + 0], rtx_entity.transform[8 + 1], rtx_entity.transform[8 + 2], rtx_entity.transform[8 + 3]
          };

          instances[i] = { transformMatrix, i, 0xFF, 0, VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, blas_address }; 
        } 

        {
          const auto size = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);
          const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
          const auto buffer = device->CreateBuffer(size, usage);
          const auto requirements = device->GetRequirements(buffer);
          const auto property = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT;
          const auto index = device->GetMemoryIndex(property, requirements.memoryTypeBits);
          const auto memory = device->AllocateMemory(requirements.size, index, true);

          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

          instances_buffer = buffer;
          instances_memory = memory;

          //BLAST_LOG("Instance calculated/allocated size (bytes): %d/%d", size, requirements.size);
        }

        void* mapped{ nullptr };
        BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), instances_memory, 0, VK_WHOLE_SIZE, 0, &mapped));
        memcpy(mapped, instances.data(), instances.size() * sizeof(VkAccelerationStructureInstanceKHR));
        vkUnmapMemory(device->GetDevice(), instances_memory);

        
        VkAccelerationStructureGeometryKHR structure_geometry{};
        structure_geometry.sType                                  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
        structure_geometry.geometryType                           = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        structure_geometry.geometry.instances.sType               = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        structure_geometry.geometry.instances.data.deviceAddress  = device->GetAddress(instances_buffer);;

        const auto instance_count = uint32_t(instances.size());
        const auto instance_offset = 0;

        VkAccelerationStructureBuildRangeInfoKHR range_info{};
        range_info.primitiveCount     = instance_count;
        range_info.primitiveOffset    = instance_offset;
        range_info.firstVertex        = 0;
        range_info.transformOffset    = 0;

        VkAccelerationStructureBuildGeometryInfoKHR geometry_info{};
        geometry_info.sType           = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        geometry_info.type            = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        geometry_info.flags           = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        geometry_info.mode            = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        geometry_info.geometryCount   = 1;
        geometry_info.pGeometries     = &structure_geometry;

        VkAccelerationStructureBuildSizesInfoKHR sizes_info{};
        sizes_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;

        vkGetAccelerationStructureBuildSizesKHR(device->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
          &geometry_info, &instance_count, &sizes_info);

        {
          const auto size = sizes_info.accelerationStructureSize;
          const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR;
          const auto buffer = device->CreateBuffer(size, usage);
          const auto requirements = device->GetRequirements(buffer);
          const auto property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
          const auto index = device->GetMemoryIndex(property, requirements.memoryTypeBits);
          const auto memory = device->AllocateMemory(requirements.size, index, true);

          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

          tlas_buffer = buffer;
          tlas_memory = memory;

          //BLAST_LOG("TLAS calculated/allocated size (bytes): %d/%d", size, requirements.size);
        }
        

        VkAccelerationStructureCreateInfoKHR create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        create_info.size = sizes_info.accelerationStructureSize;
        create_info.buffer = tlas_buffer;
        BLAST_ASSERT(VK_SUCCESS == vkCreateAccelerationStructureKHR(device->GetDevice(), &create_info, nullptr, &tlas_item));
          
        {
          const auto size = sizes_info.buildScratchSize;
          const auto usage = VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
          const auto buffer = device->CreateBuffer(size, usage);
          const auto requirements = device->GetRequirements(buffer);
          const auto property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
          const auto index = device->GetMemoryIndex(property, requirements.memoryTypeBits);
          const auto memory = device->AllocateMemory(requirements.size, index, true);

          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

          scratch_buffer = buffer;
          scratch_memory = memory;

          //BLAST_LOG("Scratch calculated/allocated size (bytes): %d/%d", size, requirements.size);
        }

        geometry_info.dstAccelerationStructure = tlas_item;
        geometry_info.scratchData.deviceAddress = device->GetAddress(scratch_buffer);

        const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
        vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &geometry_info, &range_info_ptr);
      }

      BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(command_buffer));

      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &command_buffer;

      BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE)); // fence));
      BLAST_ASSERT(VK_SUCCESS == vkQueueWaitIdle(device->GetQueue()));

      vkDestroyBuffer(device->GetDevice(), scratch_buffer, nullptr);
      vkFreeMemory(device->GetDevice(), scratch_memory, nullptr);

      for (uint32_t i = 0; i < uint32_t(rtx_entities.size()); ++i)
      {
        vkDestroyBuffer(device->GetDevice(), scratch_buffers[i], nullptr);
        vkFreeMemory(device->GetDevice(), scratch_memories[i], nullptr);
        
      }
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
      if (tlas_item) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR, 1 }); }

      VkDescriptorPoolCreateInfo poolCreateInfo = {};
      poolCreateInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
      poolCreateInfo.poolSizeCount = uint32_t(pool_sizes.size());
      poolCreateInfo.pPoolSizes = pool_sizes.data();
      poolCreateInfo.maxSets = uint32_t(sets.size());
      BLAST_ASSERT(VK_SUCCESS == vkCreateDescriptorPool(device->GetDevice(), &poolCreateInfo, nullptr, &pool));
    }

    {
      tables.resize(1, nullptr); //Currently only one descriptor set layout (table)
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
        std::vector<VkDescriptorSetLayoutBinding> descriptors(tlas_item ? 1 : 0);
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
        buffer_info.offset = ub_views.at(i)->GetCount().offset;        
        buffer_info.range = ub_views.at(i)->GetCount().length == uint32_t(-1) ? VK_WHOLE_SIZE : ub_views.at(i)->GetCount().length;

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
        buffer_info.offset = sb_views.at(i)->GetCount().offset;
        buffer_info.range = sb_views.at(i)->GetCount().length == uint32_t(-1) ? VK_WHOLE_SIZE : sb_views.at(i)->GetCount().length;

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

    if (tlas_item)
    {
      VkWriteDescriptorSetAccelerationStructureKHR descriptor_acceleration = {};
      VkWriteDescriptorSet descriptor = {};

      descriptor_acceleration.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR;
      descriptor_acceleration.accelerationStructureCount = 1;
      descriptor_acceleration.pAccelerationStructures = &tlas_item;

      descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor.pNext = &descriptor_acceleration;
      descriptor.dstSet = sets.at(0);
      descriptor.dstBinding = 0 + write_offset;
      descriptor.dstArrayElement = 0;
      descriptor.descriptorCount = 1;
      descriptor.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR;
      descriptor.pImageInfo = nullptr;
      descriptor.pBufferInfo = nullptr;
      descriptor.pTexelBufferView = nullptr;

      vkUpdateDescriptorSets(device->GetDevice(), 1, &descriptor, 0, nullptr);
      write_offset += 1;
    }

    BLAST_LOG("Layout count: %d", write_offset);


    //for (auto& command : commands)
    //{
    //  command->Initialize();
    //}

  }

  void VLKLayout::Use()
  {
  }

  void VLKLayout::Discard()
  {
    //for (auto& command : commands)
    //{
    //  command->Discard();
    //}

    auto device = reinterpret_cast<VLKDevice*>(&this->GetDevice());

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

    for (auto& table : tables)
    { 
      vkDestroyDescriptorSetLayout(device->GetDevice(), table, nullptr);
    }
    tables.clear();    

    if (pool) 
    { 
      vkDestroyDescriptorPool(device->GetDevice(), pool, nullptr); pool = nullptr;
    }

    //RTX section
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
  }

  VLKLayout::VLKLayout(const std::string& name,
    Device& device,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views,
    const std::pair<const Layout::Sampler*, uint32_t>& samplers,
    const std::pair<const Layout::RTXEntity*, uint32_t>& rtx_entities)
    : Layout(name, device, ub_views, sb_views, ri_views, wi_views, rb_views, wb_views, samplers, rtx_entities)
  {
    VLKLayout::Initialize();
  }

  VLKLayout::~VLKLayout()
  {
    VLKLayout::Discard();
  }
}
