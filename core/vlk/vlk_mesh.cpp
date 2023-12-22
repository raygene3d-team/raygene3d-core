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


#include "vlk_mesh.h"
#include "vlk_batch.h"
#include "vlk_technique.h"
#include "vlk_pass.h"
#include "vlk_device.h"
#include "vlk_view.h"


namespace RayGene3D
{
  void VLKMesh::Initialize()
  {
    auto batch = reinterpret_cast<VLKBatch*>(&this->GetBatch());
    auto technique = reinterpret_cast<VLKTechnique*>(&batch->GetTechnique());
    auto pass = reinterpret_cast<VLKPass*>(&technique->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());


    if (pass->GetType() == Pass::TYPE_RAYTRACING && device->GetRTXSupported())
    {
      BLAST_LOG("RTX Instances count: %d", subsets.size());

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
        blas_memories.resize(subsets.size());
        blas_buffers.resize(subsets.size());
        blas_items.resize(subsets.size());

        for (auto i = 0u; i < uint32_t(subsets.size()); ++i)
        {
          auto& blas_memory = blas_memories[i];
          auto& blas_buffer = blas_buffers[i];
          auto& blas_item = blas_items[i];

          const auto vtx_resource = reinterpret_cast<VLKResource*>(&vtx_views[0]->GetResource());
          const auto vtx_stride = vtx_resource->GetStride();
          const auto vtx_count = subsets[i].argument.graphic.vtx_count;
          const auto vtx_offset = subsets[i].argument.graphic.vtx_offset;
          const auto vtx_address = device->GetAddress(vtx_resource->GetBuffer());

          const auto idx_resource = reinterpret_cast<VLKResource*>(&idx_views[0]->GetResource());
          const auto idx_stride = idx_resource->GetStride();
          const auto idx_count = subsets[i].argument.graphic.idx_count;
          const auto idx_offset = subsets[i].argument.graphic.vtx_offset;
          const auto idx_address = device->GetAddress(idx_resource->GetBuffer());

          //BLAST_LOG("Vertices and Triangles count/offset: %d/%d, %d/%d", va_count, va_offset, ia_count, ia_offset);

          VkAccelerationStructureGeometryKHR structure_geometry{};
          structure_geometry.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR;
          structure_geometry.flags = VK_GEOMETRY_OPAQUE_BIT_KHR;
          structure_geometry.geometryType                                 = VK_GEOMETRY_TYPE_TRIANGLES_KHR;
          structure_geometry.geometry.triangles.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR;
          structure_geometry.geometry.triangles.vertexData.deviceAddress  = vtx_address;
          structure_geometry.geometry.triangles.vertexStride              = vtx_stride;
          structure_geometry.geometry.triangles.vertexFormat              = VK_FORMAT_R32G32B32_SFLOAT;
          structure_geometry.geometry.triangles.maxVertex                 = vtx_count - 1;
          structure_geometry.geometry.triangles.indexData.deviceAddress   = idx_address;
          structure_geometry.geometry.triangles.indexType                 = VK_INDEX_TYPE_UINT32;

          VkAccelerationStructureBuildRangeInfoKHR range_info{};
          range_info.primitiveCount   = idx_count;
          range_info.primitiveOffset  = idx_offset * idx_stride; //byte offset
          range_info.firstVertex      = vtx_offset;
          range_info.transformOffset  = 0;

          VkAccelerationStructureBuildGeometryInfoKHR geometry_info{};
          geometry_info.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
          geometry_info.type                      = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
          geometry_info.flags                     = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
          geometry_info.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
          geometry_info.geometryCount             = 1;
          geometry_info.pGeometries               = &structure_geometry;
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
          create_info.sType   = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
          create_info.type    = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
          create_info.size    = sizes_info.accelerationStructureSize;
          create_info.buffer  = blas_buffer;
          BLAST_ASSERT(VK_SUCCESS == vkCreateAccelerationStructureKHR(device->GetDevice(), &create_info, nullptr, &blas_item));

          geometry_info.dstAccelerationStructure = blas_item;

          const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
          vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &geometry_info, &range_info_ptr);

          VkMemoryBarrier memory_barrier = {};
          memory_barrier.sType          = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
          memory_barrier.srcAccessMask  = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
          memory_barrier.dstAccessMask  = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
          vkCmdPipelineBarrier(command_buffer,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
            0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
        }
      }


      {
        std::vector<VkAccelerationStructureInstanceKHR> instances(subsets.size());

        for (auto i = 0u; i < uint32_t(subsets.size()); ++i)
        {
          VkAccelerationStructureDeviceAddressInfoKHR address_info{};
          address_info.sType                  = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR;
          address_info.accelerationStructure  = blas_items[i];
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
        structure_geometry.geometryType                           = VK_GEOMETRY_TYPE_INSTANCES_KHR;
        structure_geometry.geometry.instances.sType               = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR;
        structure_geometry.geometry.instances.data.deviceAddress  = device->GetAddress(instances_buffer);

        VkAccelerationStructureBuildRangeInfoKHR range_info{};
        range_info.primitiveCount     = uint32_t(instances.size());
        range_info.primitiveOffset    = 0;
        range_info.firstVertex        = 0;
        range_info.transformOffset    = 0;

        VkAccelerationStructureBuildGeometryInfoKHR geometry_info{};
        geometry_info.sType                     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR;
        geometry_info.type                      = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        geometry_info.flags                     = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR;
        geometry_info.mode                      = VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR;
        geometry_info.geometryCount             = 1;
        geometry_info.pGeometries               = &structure_geometry;
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
        create_info.sType     = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR;
        create_info.type      = VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR;
        create_info.size      = sizes_info.accelerationStructureSize;
        create_info.buffer    = tlas_buffer;
        BLAST_ASSERT(VK_SUCCESS == vkCreateAccelerationStructureKHR(device->GetDevice(), &create_info, nullptr, &tlas_item));

        geometry_info.dstAccelerationStructure = tlas_item;

        const VkAccelerationStructureBuildRangeInfoKHR* range_info_ptr = &range_info;
        vkCmdBuildAccelerationStructuresKHR(command_buffer, 1, &geometry_info, &range_info_ptr);

        VkMemoryBarrier memory_barrier = {};
        memory_barrier.sType          = VK_STRUCTURE_TYPE_MEMORY_BARRIER;
        memory_barrier.srcAccessMask  = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        memory_barrier.dstAccessMask  = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR | VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
        vkCmdPipelineBarrier(command_buffer,
          VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
          VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
          0, 1, &memory_barrier, 0, nullptr, 0, nullptr);
      }

      BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(command_buffer));

      VkSubmitInfo submit_info = {};
      submit_info.sType               = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      submit_info.commandBufferCount  = 1;
      submit_info.pCommandBuffers     = &command_buffer;

      BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(device->GetQueue(), 1, &submit_info, VK_NULL_HANDLE)); // fence));
      BLAST_ASSERT(VK_SUCCESS == vkQueueWaitIdle(device->GetQueue()));
    }
  }

  void VLKMesh::Use()
  {
    auto batch = reinterpret_cast<VLKBatch*>(&this->GetBatch());
    auto technique = reinterpret_cast<VLKTechnique*>(&batch->GetTechnique());
    auto pass = reinterpret_cast<VLKPass*>(&technique->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

    const auto command_buffer = device->GetCommadBuffer();

    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      {
        const auto va_limit = 16u;
        std::array<uint32_t, va_limit> va_strides;
        std::array<VkDeviceSize, va_limit> va_offsets;
        std::array<VkBuffer, va_limit> va_items;

        const auto va_count = std::min(va_limit, uint32_t(vtx_views.size()));
        for (uint32_t i = 0; i < va_count; ++i)
        {
          const auto& va_view = vtx_views[i];
          if (va_view)
          {
            va_items[i] = (reinterpret_cast<VLKResource*>(&va_view->GetResource()))->GetBuffer();
            va_offsets[i] = va_view->GetCount().offset;
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

        const auto ia_count = std::min(ia_limit, uint32_t(idx_views.size()));
        for (uint32_t i = 0; i < ia_count; ++i)
        {
          const auto& ia_view = idx_views[i];
          if (ia_view)
          {
            ia_items[i] = (reinterpret_cast<VLKResource*>(&ia_view->GetResource()))->GetBuffer();
            ia_offsets[i] = ia_view->GetCount().offset;
            ia_formats[i] = technique->GetIAState().indexer
              == Technique::INDEXER_32_BIT ? VK_INDEX_TYPE_UINT32
              : Technique::INDEXER_16_BIT ? VK_INDEX_TYPE_UINT16
              : VK_INDEX_TYPE_MAX_ENUM;
          }
        }

        if (ia_count > 0)
        {
          vkCmdBindIndexBuffer(command_buffer, ia_items[0], ia_offsets[0], ia_formats[0]);
        }
      }
    }
  }

  void VLKMesh::Discard()
  {
    auto batch = reinterpret_cast<VLKBatch*>(&this->GetBatch());
    auto technique = reinterpret_cast<VLKTechnique*>(&batch->GetTechnique());
    auto pass = reinterpret_cast<VLKPass*>(&technique->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

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

  VLKMesh::VLKMesh(const std::string& name,
    Batch& batch,
    const std::pair<const Mesh::Subset*, uint32_t>& subsets,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& vtx_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& idx_views)
    : Mesh(name, batch, subsets, vtx_views, idx_views)
  {
    VLKMesh::Initialize();
  }

  VLKMesh::~VLKMesh()
  {
    VLKMesh::Discard();
  }
}
