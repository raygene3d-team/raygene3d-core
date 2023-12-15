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
  }

  void VLKMesh::Use()
  {
    if (!enabled) return;

    auto batch = reinterpret_cast<VLKBatch*>(&this->GetBatch());
    auto technique = reinterpret_cast<VLKTechnique*>(&batch->GetTechnique());
    auto pass = reinterpret_cast<VLKPass*>(&technique->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());

    const auto command_buffer = device->GetCommadBuffer();

    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      const auto va_limit = 16u;
      std::array<uint32_t, va_limit> va_strides;
      std::array<VkDeviceSize, va_limit> va_offsets;
      std::array<VkBuffer, va_limit> va_items;

      const auto va_count = std::min(va_limit, uint32_t(va_views.size()));
      for (uint32_t k = 0; k < va_count; ++k)
      {
        const auto& va_view = va_views[k];
        if (va_view)
        {
          va_items[k] = (reinterpret_cast<VLKResource*>(&va_view->GetResource()))->GetBuffer();
          va_offsets[k] = va_view->GetCount().offset;
        }
      }

      if (va_count > 0)
      {
        vkCmdBindVertexBuffers(command_buffer, 0, va_count, va_items.data(), va_offsets.data());
      }

      const auto ia_limit = 1u;
      std::array<VkIndexType, ia_limit> ia_formats;
      std::array<VkDeviceSize, ia_limit> ia_offsets;
      std::array<VkBuffer, ia_limit> ia_items;

      const auto ia_count = std::min(ia_limit, uint32_t(ia_views.size()));
      for (uint32_t k = 0; k < ia_count; ++k)
      {
        const auto& ia_view = ia_views[k];
        if (ia_view)
        {
          ia_items[k] = (reinterpret_cast<VLKResource*>(&ia_view->GetResource()))->GetBuffer();
          ia_offsets[k] = ia_view->GetCount().offset;
          ia_formats[k] = technique->GetIAState().indexer
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

  void VLKMesh::Discard()
  {
    auto batch = reinterpret_cast<VLKBatch*>(&this->GetBatch());
    auto technique = reinterpret_cast<VLKTechnique*>(&batch->GetTechnique());
    auto pass = reinterpret_cast<VLKPass*>(&technique->GetPass());
    auto device = reinterpret_cast<VLKDevice*>(&pass->GetDevice());
  }

  VLKMesh::VLKMesh(const std::string& name,
    Batch& batch,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
    uint32_t va_count, uint32_t va_offset,
    uint32_t ia_count, uint32_t ia_offset)
    : Mesh(name, batch, va_views, ia_views, va_count, va_offset, ia_count, ia_offset)
  {
    VLKMesh::Initialize();
  }

  VLKMesh::VLKMesh(const std::string& name,
    Batch& batch,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views)
    : Mesh(name, batch, va_views, ia_views)
  {
    VLKMesh::Initialize();
  }

  VLKMesh::~VLKMesh()
  {
    VLKMesh::Discard();
  }
}
