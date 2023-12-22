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
#include "d11_mesh.h"
#include "d11_batch.h"
#include "d11_technique.h"
#include "d11_pass.h"
#include "d11_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

namespace RayGene3D
{
  void D11Mesh::Initialize()
  {
  }

  void D11Mesh::Use()
  {
    auto batch = reinterpret_cast<D11Batch*>(&this->GetBatch());
    auto technique = reinterpret_cast<D11Technique*>(&batch->GetTechnique());
    auto pass = reinterpret_cast<D11Pass*>(&technique->GetPass());
    auto device = reinterpret_cast<D11Device*>(&pass->GetDevice());


    if (pass->GetType() == Pass::TYPE_GRAPHIC)
    {
      const uint32_t va_limit = D3D11_IA_VERTEX_INPUT_RESOURCE_SLOT_COUNT;
      uint32_t va_strides[va_limit]{ 0 };
      uint32_t va_offsets[va_limit]{ 0 };
      ID3D11Buffer* va_items[va_limit]{ nullptr };
      const uint32_t va_count = std::min(va_limit, uint32_t(vtx_views.size()));
      for (uint32_t i = 0; i < va_count; ++i)
      {
        const auto& va_view = vtx_views[i];
        if (va_view)
        {
          va_items[i] = (reinterpret_cast<D11Resource*>(&va_view->GetResource()))->GetBuffer();
          va_offsets[i] = va_view->GetCount().offset;
          va_strides[i] = technique->GetStrides().at(i);
        }
      }
      device->GetContext()->IASetVertexBuffers(0, va_count, va_items, va_strides, va_offsets);

      const uint32_t ia_limit = 1;
      uint32_t ia_offsets[ia_limit]{ 0 };
      DXGI_FORMAT ia_formats[ia_limit]{ DXGI_FORMAT_UNKNOWN };
      ID3D11Buffer* ia_items[ia_limit]{ nullptr };
      const uint32_t ia_count = std::min(ia_limit, uint32_t(idx_views.size()));
      for (uint32_t i = 0; i < ia_count; ++i)
      {
        const auto& ia_view = idx_views[i];
        if (ia_view)
        {
          ia_items[i] = (reinterpret_cast<D11Resource*>(&ia_view->GetResource()))->GetBuffer();
          ia_offsets[i] = ia_view->GetCount().offset;
          ia_formats[i] = technique->GetIAState().indexer
            == Technique::INDEXER_32_BIT ? DXGI_FORMAT_R32_UINT
            : Technique::INDEXER_16_BIT ? DXGI_FORMAT_R16_UINT
            : DXGI_FORMAT_UNKNOWN;
        }
      }
      device->GetContext()->IASetIndexBuffer(ia_items[0], ia_formats[0], ia_offsets[0]);
    }
  }

  void D11Mesh::Discard()
  {
  }

  D11Mesh::D11Mesh(const std::string& name,
    Batch& batch,
    const std::pair<const Mesh::Subset*, uint32_t>& subsets,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& vtx_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& idx_views)
    : Mesh(name, batch, subsets, vtx_views, idx_views)
  {
    D11Mesh::Initialize();
  }

  D11Mesh::~D11Mesh()
  {
    D11Mesh::Discard();
  }
}