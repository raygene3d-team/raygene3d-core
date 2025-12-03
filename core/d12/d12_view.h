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
#include "../view.h"

#include <dxgi.h>
#include <d3d12.h>
#include <wrl.h>

namespace RayGene3D
{
  class D12View : public View
  {
  protected:
    D3D12_CPU_DESCRIPTOR_HANDLE handle;
    D3D12_GPU_VIRTUAL_ADDRESS address;

    union Info
    {
      D3D12_CONSTANT_BUFFER_VIEW_DESC cbv_desc;
      D3D12_SHADER_RESOURCE_VIEW_DESC srv_desc;
      D3D12_UNORDERED_ACCESS_VIEW_DESC uav_desc;
      D3D12_RENDER_TARGET_VIEW_DESC rtv_desc;
      D3D12_DEPTH_STENCIL_VIEW_DESC dsv_desc;      
    } info;

  public:
  //  void SetView(D3D12_CPU_DESCRIPTOR_HANDLE view) { this->view = view; }
    D3D12_CPU_DESCRIPTOR_HANDLE GetHandle() const { return handle; }
    D3D12_GPU_VIRTUAL_ADDRESS GetAddress() const { return address; }
  //  ID3D11ShaderResourceView* GetSRView() const { return reinterpret_cast<ID3D11ShaderResourceView*>(view); }
  //  ID3D11RenderTargetView* GetRTView() const { return reinterpret_cast<ID3D11RenderTargetView*>(view); }
  //  ID3D11DepthStencilView* GetDSView() const { return reinterpret_cast<ID3D11DepthStencilView*>(view); }
  //  ID3D11UnorderedAccessView* GetUAView() const { return reinterpret_cast<ID3D11UnorderedAccessView*>(view); }

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    D12View(const std::string& name,
      Resource& resource,
      Usage usage,
      const Range& levels_or_length = Range{ 0u, size_t(-1) },
      const Range& layers_or_stride = Range{ 0u, size_t(-1) },
      View::Bind bind = View::BIND_UNKNOWN);
    virtual ~D12View();
  };
}