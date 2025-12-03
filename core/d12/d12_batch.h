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
//#include "d11_mesh.h"

#include <dxgi.h>
#include <d3d12.h>

namespace RayGene3D
{
  constexpr size_t sampler_limit{ 8 };

  class D12Batch : public Batch
  {
  protected:
    ID3D12RootSignature* root_signature{ nullptr };
    ID3D12PipelineState* pipeline_state{ nullptr };
    ID3D12CommandSignature* command_signature{ nullptr };

  protected:
    std::vector<D3D12_ROOT_PARAMETER> root_parameters;

  protected:
    std::vector<D3D12_STATIC_SAMPLER_DESC> sampler_descs;

  protected:
    std::vector<D3D12_GPU_VIRTUAL_ADDRESS> ub_items;
    std::vector<D3D12_GPU_VIRTUAL_ADDRESS> sb_items;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> rr_items;
    std::vector<D3D12_CPU_DESCRIPTOR_HANDLE> wr_items;

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D12Batch(const std::string& name,
      Config& config,
      const std::pair<const Entity*, size_t>& entities,
      const std::pair<const Sampler*, size_t>& samplers = {},
      const std::pair<const std::shared_ptr<View>*, size_t>& ub_views = {},
      const std::pair<const std::shared_ptr<View>*, size_t>& sb_views = {},
      const std::pair<const std::shared_ptr<View>*, size_t>& ri_views = {},
      const std::pair<const std::shared_ptr<View>*, size_t>& wi_views = {},
      const std::pair<const std::shared_ptr<View>*, size_t>& rb_views = {},
      const std::pair<const std::shared_ptr<View>*, size_t>& wb_views = {}
    );
    virtual ~D12Batch();
  };
}