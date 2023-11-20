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
      vkCreateAccelerationStructureNV = reinterpret_cast<PFN_vkCreateAccelerationStructureNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkCreateAccelerationStructureNV"));
      vkDestroyAccelerationStructureNV = reinterpret_cast<PFN_vkDestroyAccelerationStructureNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkDestroyAccelerationStructureNV"));
      vkBindAccelerationStructureMemoryNV = reinterpret_cast<PFN_vkBindAccelerationStructureMemoryNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkBindAccelerationStructureMemoryNV"));
      vkGetAccelerationStructureHandleNV = reinterpret_cast<PFN_vkGetAccelerationStructureHandleNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetAccelerationStructureHandleNV"));
      vkGetAccelerationStructureMemoryRequirementsNV = reinterpret_cast<PFN_vkGetAccelerationStructureMemoryRequirementsNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkGetAccelerationStructureMemoryRequirementsNV"));
      vkCmdBuildAccelerationStructureNV = reinterpret_cast<PFN_vkCmdBuildAccelerationStructureNV>(vkGetDeviceProcAddr(device->GetDevice(), "vkCmdBuildAccelerationStructureNV"));
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

      const auto get_device_address_fn = [](VkDevice device, VkBuffer buffer)
      {
        if (buffer == VK_NULL_HANDLE)
          return 0ULL;

        VkBufferDeviceAddressInfo info = {};
        info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
        info.buffer = buffer;
        return vkGetBufferDeviceAddress(device, &info);
      };

      //std::vector<VkGeometryNV> geometriesNV(rtx_entities.size());


      {
        bottom_geometries.resize(rtx_entities.size());
        bottom_range_infos.resize(rtx_entities.size());
        bottom_sizes_infos.resize(rtx_entities.size());
        bottom_sizes.resize(rtx_entities.size());
        bottom_accelerations.resize(rtx_entities.size());
        bottom_memories.resize(rtx_entities.size());
        bottom_handles.resize(rtx_entities.size());

        for (uint32_t i = 0; i < uint32_t(rtx_entities.size()); ++i)
        {
          const auto& rtx_entity = rtx_entities[i];

          const auto& va_view = rtx_entity.va_view;
          const auto& ia_view = rtx_entity.ia_view;

          auto& bottom_geometry = bottom_geometries[i];
          auto& bottom_range_info = bottom_range_infos[i];
          auto& bottom_sizes_info = bottom_sizes_infos[i];
          auto& bottom_size = bottom_sizes[i];
          auto& bottom_acceleration = bottom_accelerations[i];
          auto& bottom_memory = bottom_memories[i];
          auto& bottom_buffer = bottom_buffers[i];
          auto& bottom_handle = bottom_handles[i];

          const auto va_resource = reinterpret_cast<VLKResource*>(&va_view->GetResource());
          const auto va_stride = va_resource->GetStride();
          const auto va_count = rtx_entity.va_count;
          const auto va_offset = rtx_entity.va_offset * va_stride;

          const auto ia_resource = reinterpret_cast<VLKResource*>(&ia_view->GetResource());
          const auto ia_stride = 4;
          const auto ia_count = rtx_entity.ia_count;
          const auto ia_offset = rtx_entity.ia_offset * ia_stride;

          BLAST_LOG("Vertices/Triangles count and offset: %d/%d, %d/%d", va_count, ia_count / 3, va_offset, ia_offset / 3);

          bottom_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
          bottom_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
          bottom_geometry.geometryType = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
          bottom_geometry.geometry.triangles.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
          bottom_geometry.geometry.triangles.vertexData.deviceAddress = get_device_address_fn(device->GetDevice(), va_resource->GetBuffer());
          bottom_geometry.geometry.triangles.vertexStride = va_stride;
          bottom_geometry.geometry.triangles.vertexFormat = VK_FORMAT_R32G32B32_SFLOAT;
          bottom_geometry.geometry.triangles.maxVertex = va_count - 1;
          bottom_geometry.geometry.triangles.indexData.deviceAddress = get_device_address_fn(device->GetDevice(), ia_resource->GetBuffer());
          bottom_geometry.geometry.triangles.indexType = ia_stride == 4 ? VK_INDEX_TYPE_UINT32 : VK_INDEX_TYPE_UINT16;
          bottom_geometry.geometry.triangles.transformData = {};
          //geometry.geometry.aabbs.sType = VK_ACCELERATION_STRUCTURE_TYPE_GEOMETRY_AABB_KHR;

          const auto primitive_count = ia_count / 3;

          VkAccelerationStructureBuildGeometryInfoKHR geometry_info = {};
          geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
          geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
          geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
          geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
          geometry_info.geometryCount = 1;
          geometry_info.pGeometries = &bottom_geometry;

          vkGetAccelerationStructureBuildSizesKHR(device->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &geometry_info, &primitive_count, &bottom_sizes_info);

          VkBufferCreateInfo info{ VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
          info.size = bottom_sizes_info.accelerationStructureSize;
          info.usage = VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR
            | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT
            | VK_BUFFER_USAGE_TRANSFER_DST_BIT;
          BLAST_ASSERT(VK_SUCCESS == vkCreateBuffer(device->GetDevice(), &info, nullptr, &bottom_buffer));

          VkBufferMemoryRequirementsInfo2 requirements_info = {};
          requirements_info.sType = VK_STRUCTURE_TYPE_BUFFER_MEMORY_REQUIREMENTS_INFO_2;
          requirements_info.buffer = bottom_buffer;
          VkMemoryRequirements2 requirements = {};
          requirements.sType = VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2;
          vkGetBufferMemoryRequirements2(device->GetDevice(), &requirements_info, &requirements);
          VkMemoryAllocateInfo allocate_info = {};
          allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
          allocate_info.allocationSize = requirements.memoryRequirements.size;
          allocate_info.memoryTypeIndex = device->GetMemoryIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requirements.memoryRequirements.memoryTypeBits);
          BLAST_ASSERT(VK_SUCCESS == vkAllocateMemory(device->GetDevice(), &allocate_info, nullptr, &bottom_memory));
          bottom_size = requirements.memoryRequirements.size;
          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), bottom_buffer, bottom_memory, 0));

          VkAccelerationStructureCreateInfoKHR create_info{};
          create_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
          create_info.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
          create_info.size = bottom_sizes_info.accelerationStructureSize;
          create_info.buffer = bottom_buffer;
          BLAST_ASSERT(VK_SUCCESS == vkCreateAccelerationStructureKHR(device->GetDevice(), &create_info, nullptr, &bottom_acceleration));

          VkAccelerationStructureDeviceAddressInfoKHR address_info = {};
          address_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
          address_info.accelerationStructure = bottom_acceleration;
          bottom_handle = vkGetAccelerationStructureDeviceAddressKHR(device->GetDevice(), &address_info);
       
          BLAST_LOG("Accelearion/memory/size/handle: %d, %d, %d, %d", bottom_acceleration, bottom_memory, bottom_size, bottom_handle);
        }
      }

      std::vector<VkAccelerationStructureInstanceKHR> instances(rtx_entities.size());
      {
        for (uint32_t i = 0; i < uint32_t(instances.size()); ++i)
        {
          const auto& rtx_entity = rtx_entities[i];

          //glm::mat3x4 transform = glm::mat3x4(1.0f);
          instances[i] = { 
            rtx_entity.transform[0 + 0], rtx_entity.transform[0 + 1], rtx_entity.transform[0 + 2], rtx_entity.transform[0 + 3],
            rtx_entity.transform[4 + 0], rtx_entity.transform[4 + 1], rtx_entity.transform[4 + 2], rtx_entity.transform[4 + 3],
            rtx_entity.transform[8 + 0], rtx_entity.transform[8 + 1], rtx_entity.transform[8 + 2], rtx_entity.transform[8 + 3],
            i, 0xFF, 0, VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR, bottom_handles[i] };
        }

        {
          VkBufferCreateInfo buffer_info = {};
          buffer_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
          buffer_info.flags = 0;
          buffer_info.size = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);
          buffer_info.usage = /*VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT | */ VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
          buffer_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
          buffer_info.queueFamilyIndexCount = 0;
          buffer_info.pQueueFamilyIndices = nullptr;
          BLAST_ASSERT(VK_SUCCESS == vkCreateBuffer(device->GetDevice(), &buffer_info, nullptr, &buffer_instances));

          VkMemoryRequirements requirements;
          vkGetBufferMemoryRequirements(device->GetDevice(), buffer_instances, &requirements);
          VkMemoryAllocateInfo allocate_info = {};
          allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
          allocate_info.allocationSize = requirements.size;
          allocate_info.memoryTypeIndex = device->GetMemoryIndex(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, requirements.memoryTypeBits);
          BLAST_ASSERT(VK_SUCCESS == vkAllocateMemory(device->GetDevice(), &allocate_info, nullptr, &memory_instances));

          void* mapped = nullptr;
          BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), memory_instances, 0, VK_WHOLE_SIZE, 0, &mapped));
          memcpy(mapped, instances.data(), instances.size() * sizeof(VkAccelerationStructureInstanceKHR));
          vkUnmapMemory(device->GetDevice(), memory_instances);

          BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer_instances, memory_instances, 0));
        }

        {
          VkBufferDeviceAddressInfo address_info = {};
          address_info.sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO;
          address_info.buffer = buffer_instances;

          // Wraps a device pointer to the above uploaded instances.
          VkAccelerationStructureGeometryInstancesDataKHR instances_data = {};
          instances_data.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
          instances_data.data.deviceAddress = vkGetBufferDeviceAddress(device->GetDevice(), &address_info);

          // Put the above into a VkAccelerationStructureGeometryKHR. We need to put the instances struct in a union and label it as instance data.
          top_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
          top_geometry.geometryType = VK_GEOMETRY_TYPE_INSTANCES_KHR;
          top_geometry.geometry.instances = instances_data;

          VkAccelerationStructureBuildGeometryInfoKHR geometry_info = {};
          geometry_info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
          geometry_info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
          geometry_info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
          geometry_info.mode = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
          geometry_info.geometryCount = 1;
          geometry_info.pGeometries = &top_geometry;

          const auto instances_count = (uint32_t)instances.size();

          vkGetAccelerationStructureBuildSizesKHR(device->GetDevice(), VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
            &geometry_info, &instances_count, &top_sizes_info);

          VkAccelerationStructureCreateInfoNV accelerationInfo = {};
          accelerationInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_NV;
          accelerationInfo.info.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
          accelerationInfo.info.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
          accelerationInfo.info.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
          accelerationInfo.info.instanceCount = uint32_t(instancesNV.size());
          accelerationInfo.info.geometryCount = 0;
          BLAST_ASSERT(VK_SUCCESS == vkCreateAccelerationStructureNV(device->GetDevice(), &accelerationInfo, nullptr, &top_acceleration));

          VkAccelerationStructureMemoryRequirementsInfoNV requirementsInfo = {};
          requirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
          requirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_NV;
          requirementsInfo.accelerationStructure = top_acceleration;
          VkMemoryRequirements2 requirements = {};
          vkGetAccelerationStructureMemoryRequirementsNV(device->GetDevice(), &requirementsInfo, &requirements);
          VkMemoryAllocateInfo allocateInfo = {};
          allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
          allocateInfo.allocationSize = requirements.memoryRequirements.size;
          allocateInfo.memoryTypeIndex = device->GetMemoryIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requirements.memoryRequirements.memoryTypeBits);
          BLAST_ASSERT(VK_SUCCESS == vkAllocateMemory(device->GetDevice(), &allocateInfo, nullptr, &top_memory));
          top_size = requirements.memoryRequirements.size;

          VkBindAccelerationStructureMemoryInfoNV bindInfo = {};
          bindInfo.sType = VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_NV;
          bindInfo.accelerationStructure = top_acceleration;
          bindInfo.memory = top_memory;
          BLAST_ASSERT(VK_SUCCESS == vkBindAccelerationStructureMemoryNV(device->GetDevice(), 1, &bindInfo));

          BLAST_ASSERT(VK_SUCCESS == vkGetAccelerationStructureHandleNV(device->GetDevice(), top_acceleration, sizeof(uint64_t), &top_handle));

          BLAST_LOG("Accelearion/memory/size/handle: %d, %d, %d, %d", top_acceleration, top_memory, top_size, top_handle);
        }
      }


      {
        VkDeviceSize scratchSize = 0u;

        for(uint32_t i = 0; i < uint32_t(bottom_accelerations.size()); ++i)
        {
          VkAccelerationStructureMemoryRequirementsInfoNV requirementsInfo = {};
          requirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
          requirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
          requirementsInfo.accelerationStructure = bottom_accelerations[i];
          VkMemoryRequirements2 requirements = {};
          vkGetAccelerationStructureMemoryRequirementsNV(device->GetDevice(), &requirementsInfo, &requirements);
          scratchSize = std::max(scratchSize, requirements.memoryRequirements.size);

          BLAST_LOG("Original/scratch size bottom: %d/%d", bottom_sizes[i], requirements.memoryRequirements.size);
        }

        {
          VkAccelerationStructureMemoryRequirementsInfoNV requirementsInfo = {};
          requirementsInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_NV;
          requirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_NV;
          requirementsInfo.accelerationStructure = top_acceleration;
          VkMemoryRequirements2 requirements = {};
          vkGetAccelerationStructureMemoryRequirementsNV(device->GetDevice(), &requirementsInfo, &requirements);
          scratchSize = std::max(scratchSize, requirements.memoryRequirements.size);

          BLAST_LOG("Original/scratch size top: %d/%d", top_size, requirements.memoryRequirements.size);
        }

        VkBufferCreateInfo bufferInfo = {};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.flags = 0;
        bufferInfo.size = scratchSize;
        bufferInfo.usage = VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        bufferInfo.queueFamilyIndexCount = 0;
        bufferInfo.pQueueFamilyIndices = nullptr;
        BLAST_ASSERT(VK_SUCCESS == vkCreateBuffer(device->GetDevice(), &bufferInfo, nullptr, &bufferScratch));

        VkMemoryRequirements requirements = {};
        vkGetBufferMemoryRequirements(device->GetDevice(), bufferScratch, &requirements);
        VkMemoryAllocateInfo allocateInfo = {};
        allocateInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocateInfo.allocationSize = requirements.size;
        allocateInfo.memoryTypeIndex = device->GetMemoryIndex(VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, requirements.memoryTypeBits);
        BLAST_ASSERT(VK_SUCCESS == vkAllocateMemory(device->GetDevice(), &allocateInfo, nullptr, &memoryScratch));

        BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), bufferScratch, memoryScratch, 0));
      }

      {
        VkCommandBufferAllocateInfo allocate_info = {};
        allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocate_info.commandPool = device->GetCommandPool();
        allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocate_info.commandBufferCount = 1;
        BLAST_ASSERT(VK_SUCCESS == vkAllocateCommandBuffers(device->GetDevice(), &allocate_info, &commandBuffer));

        VkFenceCreateInfo create_info = {};
        create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        create_info.flags = VK_FENCE_CREATE_SIGNALED_BIT;
        BLAST_ASSERT(VK_SUCCESS == vkCreateFence(device->GetDevice(), &create_info, nullptr, &fence));
      }

      VkCommandBufferBeginInfo beginInfo = {};
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(commandBuffer, &beginInfo));

      for (uint32_t i = 0; i < uint32_t(bottom_accelerations.size()); ++i)
      {
        VkAccelerationStructureInfoNV buildInfo = {};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_NV;
        buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
        buildInfo.instanceCount = 0;
        buildInfo.geometryCount = 1;
        buildInfo.pGeometries = &geometriesNV[i];
        vkCmdBuildAccelerationStructureNV(commandBuffer, &buildInfo, VK_NULL_HANDLE, 0, VK_FALSE, bottom_accelerations[i], VK_NULL_HANDLE, bufferScratch, 0);

        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
          0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
      }

      {
        VkAccelerationStructureInfoNV buildInfo = {};
        buildInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_INFO_NV;
        buildInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_NV;
        buildInfo.flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_NV;
        buildInfo.instanceCount = rtx_entities.size();
        buildInfo.geometryCount = 0;
        buildInfo.pGeometries = nullptr;
        vkCmdBuildAccelerationStructureNV(commandBuffer, &buildInfo, bufferInstances, 0, VK_FALSE, top_acceleration, VK_NULL_HANDLE, bufferScratch, 0);

        VkMemoryBarrier memoryBarrier = {};
        memoryBarrier.sType = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memoryBarrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        memoryBarrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_NV | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_NV;
        vkCmdPipelineBarrier(commandBuffer, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_NV,
          0, 1, &memoryBarrier, 0, nullptr, 0, nullptr);
      }

      BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(commandBuffer));


      VkSubmitInfo submitInfo = {};
      submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submitInfo.commandBufferCount = 1;
      submitInfo.pCommandBuffers = &commandBuffer;
      BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE)); // fence));
      BLAST_ASSERT(VK_SUCCESS == vkQueueWaitIdle(device->GetQueue()));
      
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
      if (top_acceleration) { pool_sizes.push_back({ VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV, 1 }); }

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
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
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
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
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
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
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
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
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
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
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
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
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
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
        }
        bindings.insert(bindings.end(), descriptors.begin(), descriptors.end());
      }
      {
        std::vector<VkDescriptorSetLayoutBinding> descriptors(top_acceleration ? 1 : 0);
        for (uint32_t i = 0; i < uint32_t(descriptors.size()); ++i)
        {
          auto& descriptor = descriptors.at(i);
          descriptor.binding = i + uint32_t(bindings.size());
          descriptor.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
          descriptor.descriptorCount = 1;
          descriptor.pImmutableSamplers = nullptr;
          descriptor.stageFlags = VK_SHADER_STAGE_ALL_GRAPHICS | VK_SHADER_STAGE_COMPUTE_BIT
            | VK_SHADER_STAGE_RAYGEN_BIT_NV | VK_SHADER_STAGE_ANY_HIT_BIT_NV | VK_SHADER_STAGE_CLOSEST_HIT_BIT_NV | VK_SHADER_STAGE_MISS_BIT_NV;
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

    if (top_acceleration)
    {
      VkWriteDescriptorSetAccelerationStructureNV descriptor_acceleration = {};
      VkWriteDescriptorSet descriptor = {};

      descriptor_acceleration.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_NV;
      descriptor_acceleration.accelerationStructureCount = 1;
      descriptor_acceleration.pAccelerationStructures = &top_acceleration;

      descriptor.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
      descriptor.pNext = &descriptor_acceleration;
      descriptor.dstSet = sets.at(0);
      descriptor.dstBinding = 0 + write_offset;
      descriptor.dstArrayElement = 0;
      descriptor.descriptorCount = 1;
      descriptor.descriptorType = VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_NV;
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

    if (commandBuffer)
    {
      vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &commandBuffer); commandBuffer = nullptr;
    }

    //if (commandPool)
    //{
    //  vkDestroyCommandPool(device->GetDevice(), commandPool, nullptr); commandPool = nullptr;
    //}

    for (auto& bottom_acceleration : bottom_accelerations)
    {
      if (bottom_acceleration)
      {
        vkDestroyAccelerationStructureNV(device->GetDevice(), bottom_acceleration, nullptr); bottom_acceleration = nullptr;
      }
    }
    bottom_accelerations.clear();

    for (auto& bottom_memory : bottom_memories)
    {
      if (bottom_memory)
      {
        vkFreeMemory(device->GetDevice(), bottom_memory, nullptr); bottom_memory = nullptr;
      }
    }
    bottom_memories.clear();

    if (top_acceleration)
    {
      vkDestroyAccelerationStructureNV(device->GetDevice(), top_acceleration, nullptr); top_acceleration = nullptr;
    }

    if (top_memory)
    {
      vkFreeMemory(device->GetDevice(), top_memory, nullptr); top_memory = nullptr;
    }

    if (bufferInstances)
    {
      vkDestroyBuffer(device->GetDevice(), bufferInstances, nullptr); bufferInstances = nullptr;
    }

    if (memoryInstances)
    {
      vkFreeMemory(device->GetDevice(), memoryInstances, nullptr); memoryInstances = nullptr;
    }

    if (bufferScratch)
    {
      vkDestroyBuffer(device->GetDevice(), bufferScratch, nullptr); bufferScratch = nullptr;
    }

    if (memoryScratch)
    {
      vkFreeMemory(device->GetDevice(), memoryScratch, nullptr); memoryScratch = nullptr;
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
