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
#include "../batch.h"
#include "d11_mesh.h"

#include <dxgi.h>
#include <d3d11_1.h>

namespace RayGene3D
{
  class D11Batch : public Batch
  {
  protected:
    std::vector<ID3D11SamplerState*> sampler_states;

  protected:
    std::vector<ID3D11Buffer*> ub_items;
    std::vector<ID3D11Buffer*> sb_items;
    std::vector<ID3D11ShaderResourceView*> rr_items;
    std::vector<ID3D11UnorderedAccessView*> wr_items;

  public:
    const std::shared_ptr<Mesh>& CreateMesh(const std::string& name,
      uint32_t va_count,
      uint32_t va_offset,
      uint32_t ia_count,
      uint32_t ia_offset) override
    {
      return meshes.emplace_back(new D11Mesh(name, *this, va_count, va_offset, ia_count, ia_offset));
    }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D11Batch(const std::string& name,
      Technique& technique,
      const std::pair<const std::shared_ptr<View>*, uint32_t> ce_views,
      const std::pair<const Batch::Command*, uint32_t>& commands,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views = {},
      const std::pair<const Batch::Sampler*, uint32_t>& samplers = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views = {}
    );
    virtual ~D11Batch();
  };
}