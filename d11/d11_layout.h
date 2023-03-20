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
#include "../layout.h"

#include <dxgi.h>
#include <d3d11_1.h>

namespace RayGene3D
{
  class D11Layout : public Layout
  {
  protected:
    std::vector<ID3D11SamplerState*> sampler_states;

  protected:
    std::vector<ID3D11Buffer*> ub_items;
    std::vector<ID3D11Buffer*> sb_items;
    std::vector<ID3D11ShaderResourceView*> rr_items;
    std::vector<ID3D11UnorderedAccessView*> wr_items;

  public:
    ID3D11Buffer* const* GetUBItems() const {return ub_items.data(); }
    uint32_t GetUBCount() const { return uint32_t(ub_items.size()); }
    ID3D11Buffer* const* GetSBItems() const { return sb_items.data(); }
    uint32_t GetSBCount() const { return uint32_t(sb_items.size()); }
    ID3D11ShaderResourceView* const* GetRRItems() const { return rr_items.data(); }
    uint32_t GetRRCount() const { return uint32_t(rr_items.size()); }
    ID3D11UnorderedAccessView* const* GetWRItems() const { return wr_items.data(); }
    uint32_t GetWRCount() const { return uint32_t(wr_items.size()); }
    ID3D11SamplerState* const* GetSamplerItems() const { return sampler_states.data(); }
    uint32_t GetSamplerCount() const { return uint32_t(sampler_states.size()); }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D11Layout(const std::string& name, Device& device);
    virtual ~D11Layout();
  };
}