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


#include "vlk_resource.h"
#include "vlk_device.h"

namespace RayGene3D
{
  void VLKResource::Initialize()
  {
    const auto& device = reinterpret_cast<VLKDevice*>(&this->GetDevice());

    const auto get_flags = [this]()
    {
      uint32_t flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
      flags = hint & HINT_DYNAMIC_BUFFER ? VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT : flags;
      return flags;
    };

    switch (type)
    {
    case TYPE_BUFFER:
    {
      const auto get_bind = [this]()
      {
        uint32_t bind = 0;
        bind = usage & USAGE_SHADER_RESOURCE  ? bind | (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) : bind;
        bind = usage & USAGE_UNORDERED_ACCESS ? bind | (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_STORAGE_BUFFER_BIT) : bind;
        bind = usage & USAGE_VERTEX_ARRAY     ? bind | (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT) : bind;
        bind = usage & USAGE_INDEX_ARRAY      ? bind | (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT) : bind;
        bind = usage & USAGE_CONSTANT_DATA    ? bind | (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT) : bind;
        bind = usage & USAGE_COMMAND_INDIRECT ? bind | (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT) : bind;
        bind = usage & USAGE_RAYTRACING_INPUT ? bind | (VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR) : bind;

        return bind;
      };

      {
        const auto addressable = hint & HINT_ADDRESS_BUFFER && device->GetRTXSupported();
        const auto size = stride * count;
        const auto usage = get_bind() | (addressable ? VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT : 0);
        const auto buffer = device->CreateBuffer(size, usage);
        const auto requirements = device->GetRequirements(buffer);
        const auto flags = get_flags();
        const auto index = device->GetMemoryIndex(flags, requirements.memoryTypeBits);
        
        BLAST_LOG("Allocating %d bytes [%s]", requirements.size, name.c_str());
        const auto memory = device->AllocateMemory(requirements.size, index, addressable);

        BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

        this->buffer = buffer;
        this->memory = memory;
      }

      //auto create_info = VkBufferCreateInfo{};
      //create_info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
      //create_info.flags = 0;
      //create_info.size = stride * count;
      //create_info.usage = get_bind();
      //create_info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
      //create_info.queueFamilyIndexCount = 0;
      //create_info.pQueueFamilyIndices = nullptr;
      //BLAST_ASSERT(VK_SUCCESS == vkCreateBuffer(device->GetDevice(), &create_info, nullptr, &buffer));

      ////device->AllocateMemory(memory, buffer, get_flags(hint));

      //VkMemoryAllocateFlagsInfo flags_info{};
      //flags_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_FLAGS_INFO;
      //flags_info.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

      //auto requirements = VkMemoryRequirements{};
      //vkGetBufferMemoryRequirements(device->GetDevice(), buffer, &requirements);
      //auto allocate_info = VkMemoryAllocateInfo{};
      //allocate_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
      //allocate_info.pNext = hint & HINT_ADDRESS_BUFFER ? &flags_info : nullptr;
      //allocate_info.allocationSize = requirements.size;
      //allocate_info.memoryTypeIndex = device->GetMemoryIndex(get_flags(hint), requirements.memoryTypeBits);
      //BLAST_LOG("Allocating %d bytes [%s]", requirements.size, name.c_str());
      //BLAST_ASSERT(VK_SUCCESS == vkAllocateMemory(device->GetDevice(), &allocate_info, nullptr, &memory));

      //BLAST_ASSERT(VK_SUCCESS == vkBindBufferMemory(device->GetDevice(), buffer, memory, 0));

      if (!interops.empty())
      {
        auto size = 0u;
        for (uint32_t i = 0; i < uint32_t(interops.size()); ++i)
        {
          const auto [interop_data, interop_size] = interops[i];
          BLAST_ASSERT(interop_data != nullptr && interop_size != 0);

          size += interop_size;
        }

        BLAST_ASSERT(size == stride * count);

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device->GetCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        BLAST_ASSERT(VK_SUCCESS == vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer));

        VkDeviceSize staging_size = device->GetStagingSize();
        VkBuffer staging_buffer = device->GetStagingBuffer();
        VkDeviceMemory staging_memory = device->GetStagingMemory();

        const auto src_count = uint32_t(interops.size());
        const auto dst_count = (size - 1) / uint32_t(staging_size) + 1;

        auto dst_index = 0u;
        auto src_index = 0u;

        auto dst_offset = 0u;
        auto src_offset = 0u;

        while (dst_index < dst_count)
        {
          void* mapped = nullptr;
          BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), staging_memory, 0, VK_WHOLE_SIZE, 0, &mapped));

          const auto dst_data = reinterpret_cast<uint8_t*>(mapped);
          const auto dst_size = uint32_t(staging_size);

          while(src_index < src_count)
          {
            const auto [interop_data, interop_size] = interops[src_index];

            const auto src_data = reinterpret_cast<const uint8_t*>(interop_data);
            const auto src_size = interop_size;

            const auto src_range = src_size - src_offset;
            const auto dst_range = dst_size - dst_offset;

            const auto range = std::min(src_range, dst_range);
            memcpy(dst_data + dst_offset, src_data + src_offset, range);

            src_offset += range;
            dst_offset += range;

            if (src_offset == src_size) { src_offset = 0; src_index += 1; }
            if (dst_offset == dst_size) break;
          }

          vkUnmapMemory(device->GetDevice(), staging_memory);

          VkCommandBufferBeginInfo beginInfo{};
          beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
          beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

          BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(commandBuffer, &beginInfo));

          VkBufferCopy copyRegion{};
          copyRegion.srcOffset = 0; // Optional
          copyRegion.dstOffset = dst_index * staging_size; // Optional
          copyRegion.size = std::min(staging_size, size - dst_index * staging_size);
          vkCmdCopyBuffer(commandBuffer, staging_buffer, buffer, 1, &copyRegion);

          BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(commandBuffer));

          VkSubmitInfo submitInfo{};
          submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
          submitInfo.commandBufferCount = 1;
          submitInfo.pCommandBuffers = &commandBuffer;

          BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE));
          BLAST_ASSERT(VK_SUCCESS == vkQueueWaitIdle(device->GetQueue()));

          if (dst_offset == dst_size) { dst_offset = 0; dst_index += 1; }
          if (src_index == src_count) break;
        }
        vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &commandBuffer);
      }

      //if(interops.size() == 1) // TODO: Implement combining multiple properties
      //{
      //  const auto [raw_data, raw_size] = interops.at(0);
      //  BLAST_ASSERT(raw_data != nullptr && raw_size != 0);
      //  
      //  const auto size = stride * count;
      //  BLAST_ASSERT(size == raw_size);

      //  VkCommandBufferAllocateInfo allocInfo{};
      //  allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      //  allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      //  allocInfo.commandPool = device->GetCommandPool();
      //  allocInfo.commandBufferCount = 1;

      //  VkCommandBuffer commandBuffer;
      //  BLAST_ASSERT(VK_SUCCESS == vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer));

      //  VkDeviceSize staging_size = device->GetStagingSize();        
      //  VkBuffer staging_buffer = device->GetStagingBuffer();
      //  VkDeviceMemory staging_memory = device->GetStagingMemory();

      //  const auto chunk_count = (size - 1) / uint32_t(staging_size) + 1;
      //  for (uint32_t i = 0; i < chunk_count; ++i)
      //  {
      //    const auto chunk_data = reinterpret_cast<const uint8_t*>(raw_data) + i * uint32_t(staging_size);
      //    const auto chunk_size = std::min(uint32_t(staging_size), raw_size - i * uint32_t(staging_size));

      //    void* mapped = nullptr;
      //    BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), staging_memory, 0, VK_WHOLE_SIZE, 0, &mapped));
      //    memcpy(mapped, chunk_data, chunk_size);
      //    vkUnmapMemory(device->GetDevice(), staging_memory);

      //    VkCommandBufferBeginInfo beginInfo{};
      //    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      //    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

      //    BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(commandBuffer, &beginInfo));

      //    VkBufferCopy copyRegion{};
      //    copyRegion.srcOffset = 0; // Optional
      //    copyRegion.dstOffset = i * staging_size; // Optional
      //    copyRegion.size = std::min(staging_size, raw_size - i * staging_size);
      //    vkCmdCopyBuffer(commandBuffer, staging_buffer, buffer, 1, &copyRegion);

      //    BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(commandBuffer));

      //    VkSubmitInfo submitInfo{};
      //    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
      //    submitInfo.commandBufferCount = 1;
      //    submitInfo.pCommandBuffers = &commandBuffer;

      //    BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE));
      //    BLAST_ASSERT(VK_SUCCESS == vkQueueWaitIdle(device->GetQueue()));
      //  }
      //  vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &commandBuffer);
      //}
      break;
    }
    case TYPE_TEX1D:
    case TYPE_TEX2D:
    case TYPE_TEX3D:
    {
      const auto get_bind = [this]()
      {
        uint32_t bind = 0;
        bind = usage & USAGE_SHADER_RESOURCE ? bind | (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_SAMPLED_BIT) : bind;
        bind = usage & USAGE_UNORDERED_ACCESS ? bind | (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_STORAGE_BIT) : bind;
        bind = usage & USAGE_RENDER_TARGET ? bind | (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) : bind;
        bind = usage & USAGE_DEPTH_STENCIL ? bind | (VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) : bind;
        return bind;
      };

      const auto get_flags = [this]()
      {
        uint32_t flags = 0;
        flags = hint & HINT_CUBEMAP_IMAGE ? flags | VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : flags;
        return flags;
      };

      const auto get_type = [this]()
      {
        if (type == TYPE_TEX1D) return VK_IMAGE_TYPE_1D;
        if (type == TYPE_TEX2D) return VK_IMAGE_TYPE_2D;
        if (type == TYPE_TEX3D) return VK_IMAGE_TYPE_3D;
        return VK_IMAGE_TYPE_MAX_ENUM;
      };

      const auto get_extent = [this]()
      {
        if (type == TYPE_TEX1D) return VkExtent3D{ size_x, 1, 1 };
        if (type == TYPE_TEX2D) return VkExtent3D{ size_x, size_y, 1 };
        if (type == TYPE_TEX3D) return VkExtent3D{ size_x, size_y, size_z };
        return VkExtent3D{};
      };

      const auto get_format = [this]()
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
        case FORMAT_R10G10B10A2_UNORM: return VK_FORMAT_A2R10G10B10_UNORM_PACK32;
        case FORMAT_R10G10B10A2_UINT: return VK_FORMAT_A2R10G10B10_UINT_PACK32;
        case FORMAT_R11G11B10_FLOAT: return VK_FORMAT_B10G11R11_UFLOAT_PACK32;
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
        case FORMAT_R9G9B9E5_SHAREDEXP: return VK_FORMAT_E5B9G9R9_UFLOAT_PACK32;
        case FORMAT_R8G8_B8G8_UNORM: return VK_FORMAT_G8B8G8R8_422_UNORM;
        case FORMAT_G8R8_G8B8_UNORM: return VK_FORMAT_B8G8R8G8_422_UNORM;
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
          // case FORMAT_B8G8R8X8_UNORM: return DXGI_FORMAT_B8G8R8X8_UNORM;
        case FORMAT_B8G8R8A8_SRGB: return VK_FORMAT_B8G8R8A8_SRGB;
          // case FORMAT_B8G8R8X8_SRGB: return DXGI_FORMAT_B8G8R8X8_UNORM_SRGB;
        case FORMAT_BC6H_UF16: return VK_FORMAT_BC6H_UFLOAT_BLOCK;
        case FORMAT_BC6H_SF16: return VK_FORMAT_BC6H_SFLOAT_BLOCK;
        case FORMAT_BC7_UNORM: return VK_FORMAT_BC7_UNORM_BLOCK;
        case FORMAT_BC7_SRGB: return VK_FORMAT_BC7_SRGB_BLOCK;
        }
      };

      {
        const auto flags = get_flags();
        const auto type = get_type();
        const auto format = get_format();
        const auto extent = get_extent();
        const auto usage = get_bind();
        const auto image = device->CreateImage(type, format, extent, stride, count, usage, flags);
        const auto requirements = device->GetRequirements(image);
        const auto property = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT;
        const auto index = device->GetMemoryIndex(property, requirements.memoryTypeBits);
        BLAST_LOG("Allocating %d bytes [%s]", requirements.size, name.c_str());
        const auto memory = device->AllocateMemory(requirements.size, index, false);

        BLAST_ASSERT(VK_SUCCESS == vkBindImageMemory(device->GetDevice(), image, memory, 0));

        this->image = image;
        this->memory = memory;
      }

      if (interops.size() == count)
      {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = device->GetCommandPool();
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        BLAST_ASSERT(VK_SUCCESS == vkAllocateCommandBuffers(device->GetDevice(), &allocInfo, &commandBuffer));

        for (uint32_t i = 0; i < count; ++i)
        {
          const auto [raw_data, raw_size] = interops.at(i);
          BLAST_ASSERT(raw_data != nullptr && raw_size != 0);

          VkBuffer staging_buffer = device->GetStagingBuffer();
          VkDeviceMemory staging_memory = device->GetStagingMemory();
          VkDeviceSize staging_size = device->GetStagingSize();
          BLAST_ASSERT(raw_size <= staging_size);

          uint32_t layer_size = 0;
          for (uint32_t j = 0; j < stride; ++j)
          {
            const uint32_t extent_x = size_x > 1 ? size_x >> j : 1;
            const uint32_t extent_y = size_y > 1 ? size_y >> j : 1;
            const uint32_t extent_z = size_z > 1 ? size_z >> j : 1;
            const uint32_t mipmap_size = extent_x * extent_y * extent_z * BitCount(format) / 8;
            layer_size += mipmap_size;
          }
          BLAST_ASSERT(raw_size == layer_size);

          void* mapped = nullptr;
          BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device->GetDevice(), staging_memory, 0, VK_WHOLE_SIZE, 0, &mapped));
          memcpy(mapped, raw_data, raw_size);
          vkUnmapMemory(device->GetDevice(), staging_memory);


          VkCommandBufferBeginInfo beginInfo{};
          beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
          beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT; // VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
          BLAST_ASSERT(VK_SUCCESS == vkBeginCommandBuffer(commandBuffer, &beginInfo));


          uint32_t offset = 0;
          for (uint32_t j = 0; j < stride; ++j)
          {
            const uint32_t layer = i;
            const uint32_t mipmap = j;
            const uint32_t extent_x = size_x > 1 ? size_x >> mipmap : 1;
            const uint32_t extent_y = size_y > 1 ? size_y >> mipmap : 1;
            const uint32_t extent_z = size_z > 1 ? size_z >> mipmap : 1;

            {
              VkImageMemoryBarrier barrier = {};
              barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
              barrier.srcAccessMask = 0;
              barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
              barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
              barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
              barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
              barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
              barrier.image = image;
              barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              barrier.subresourceRange.baseMipLevel = mipmap;
              barrier.subresourceRange.levelCount = 1;
              barrier.subresourceRange.baseArrayLayer = layer;
              barrier.subresourceRange.layerCount = 1;
              vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
            }

            VkBufferImageCopy region = {};
            region.bufferOffset = offset;
            region.bufferRowLength = 0;
            region.bufferImageHeight = 0;
            region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
            region.imageSubresource.mipLevel = mipmap;
            region.imageSubresource.baseArrayLayer = layer;
            region.imageSubresource.layerCount = 1;
            region.imageOffset = { 0, 0, 0 };
            region.imageExtent = { extent_x, extent_y, extent_z };
            vkCmdCopyBufferToImage(commandBuffer, staging_buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);


            {
              VkImageMemoryBarrier barrier = {};
              barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
              barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
              barrier.dstAccessMask = 0;
              barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
              barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
              barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
              barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
              barrier.image = image;
              barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
              barrier.subresourceRange.baseMipLevel = mipmap;
              barrier.subresourceRange.levelCount = 1;
              barrier.subresourceRange.baseArrayLayer = layer;
              barrier.subresourceRange.layerCount = 1;
              vkCmdPipelineBarrier(commandBuffer,
                VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
                0, nullptr,
                0, nullptr,
                1, &barrier);
            }

            offset += extent_x * extent_y * extent_z * BitCount(format) / 8;
          }

          BLAST_ASSERT(VK_SUCCESS == vkEndCommandBuffer(commandBuffer));

          VkSubmitInfo submitInfo{};
          submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
          submitInfo.commandBufferCount = 1;
          submitInfo.pCommandBuffers = &commandBuffer;

          BLAST_ASSERT(VK_SUCCESS == vkQueueSubmit(device->GetQueue(), 1, &submitInfo, VK_NULL_HANDLE));
          BLAST_ASSERT(VK_SUCCESS == vkQueueWaitIdle(device->GetQueue()));          
        }

        vkFreeCommandBuffers(device->GetDevice(), device->GetCommandPool(), 1, &commandBuffer);
      }
      break;
    }
    }

    for (const auto& view : views)
    {
      view->Initialize();
    }
  }

  void VLKResource::Use()
  {
  }

  void VLKResource::Discard()
  {
    for (const auto& view : views)
    {
      view->Discard();
    }

    const auto& device = reinterpret_cast<VLKDevice*>(&this->GetDevice());

    if (device)
    {
      switch (type)
      {
      case TYPE_BUFFER:
      {
        if (buffer)
        {
          vkDestroyBuffer(device->GetDevice(), buffer, nullptr);
          buffer = nullptr;
        }
        break;
      }
      case TYPE_TEX1D:
      case TYPE_TEX2D:
      case TYPE_TEX3D:
      {
        if (image)
        {
          vkDestroyImage(device->GetDevice(), image, nullptr);
          image = nullptr;
        }
        break;
      }
      }

      if (memory)
      {
        vkFreeMemory(device->GetDevice(), memory, nullptr);
        memory = nullptr;
      }
    }
  }


 //void VLKResource::Commit(uint32_t index)
 //{
 //}

 //void VLKResource::Retrieve(uint32_t index)
 //{
 //}

 //void VLKResource::Blit(const std::shared_ptr<Resource>& resource)
 //{
 //}

  void* VLKResource::Map()
  {
    const auto& device = reinterpret_cast<VLKDevice*>(&this->GetDevice())->GetDevice();

    void* data = nullptr;
    BLAST_ASSERT(VK_SUCCESS == vkMapMemory(device, memory, 0, VK_WHOLE_SIZE, 0, &data));
    return data;
  }

  void VLKResource::Unmap() 
  {
    const auto& device = reinterpret_cast<VLKDevice*>(&this->GetDevice())->GetDevice();

    vkUnmapMemory(device, memory);
  }

  void VLKResource::Commit(uint32_t index) 
  {
  }

  void VLKResource::Retrieve(uint32_t index) 
  {
  }

  void VLKResource::Blit(const std::shared_ptr<Resource>& resource) 
  {
  }

  VLKResource::VLKResource(const std::string& name,
    Device& device,
    const Resource::BufferDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    VLKResource::Initialize();
  }

  VLKResource::VLKResource(const std::string& name,
    Device& device,
    const Resource::Tex1DDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    VLKResource::Initialize();
  }

  VLKResource::VLKResource(const std::string& name,
    Device& device,
    const Resource::Tex2DDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    VLKResource::Initialize();
  }

  VLKResource::VLKResource(const std::string& name,
    Device& device,
    const Resource::Tex3DDesc& desc,
    Resource::Hint hint,
    const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Resource(name, device, desc, hint, interops)
  {
    VLKResource::Initialize();
  }

  VLKResource::~VLKResource()
  {
    VLKResource::Discard();
  }
}
