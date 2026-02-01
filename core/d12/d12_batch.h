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
    template <typename Data, D3D12_PIPELINE_STATE_SUBOBJECT_TYPE Type, typename Arg = Data>
    class alignas(void*) PIPELINE_STATE_SUBOBJECT
    {
    private:
      D3D12_PIPELINE_STATE_SUBOBJECT_TYPE type;
      Data data;
    public:
      PIPELINE_STATE_SUBOBJECT() noexcept : type(Type), data(Arg()) {}
      PIPELINE_STATE_SUBOBJECT(Data const& data) noexcept : type(Type), data(data) {}
    };

    typedef PIPELINE_STATE_SUBOBJECT<D3D12_PIPELINE_STATE_FLAGS,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_FLAGS>                  PIPELINE_STATE_SUBOBJECT_FLAGS;
    typedef PIPELINE_STATE_SUBOBJECT<UINT,                                D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_NODE_MASK>              PIPELINE_STATE_SUBOBJECT_NODE_MASK;
    typedef PIPELINE_STATE_SUBOBJECT<ID3D12RootSignature*,                D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_ROOT_SIGNATURE>         PIPELINE_STATE_SUBOBJECT_ROOT_SIGNATURE;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_INPUT_LAYOUT_DESC,             D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_INPUT_LAYOUT>           PIPELINE_STATE_SUBOBJECT_INPUT_LAYOUT;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_INDEX_BUFFER_STRIP_CUT_VALUE,  D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_IB_STRIP_CUT_VALUE>     PIPELINE_STATE_SUBOBJECT_IB_STRIP_CUT_VALUE;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_PRIMITIVE_TOPOLOGY_TYPE,       D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PRIMITIVE_TOPOLOGY>     PIPELINE_STATE_SUBOBJECT_PRIMITIVE_TOPOLOGY;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VS>                     PIPELINE_STATE_SUBOBJECT_VS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_GS>                     PIPELINE_STATE_SUBOBJECT_GS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_STREAM_OUTPUT_DESC,            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_STREAM_OUTPUT>          PIPELINE_STATE_SUBOBJECT_STREAM_OUTPUT;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_HS>                     PIPELINE_STATE_SUBOBJECT_HS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DS>                     PIPELINE_STATE_SUBOBJECT_DS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_PS>                     PIPELINE_STATE_SUBOBJECT_PS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_AS>                     PIPELINE_STATE_SUBOBJECT_AS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_MS>                     PIPELINE_STATE_SUBOBJECT_MS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_SHADER_BYTECODE,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_CS>                     PIPELINE_STATE_SUBOBJECT_CS;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_BLEND_DESC,                    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_BLEND>                  PIPELINE_STATE_SUBOBJECT_BLEND;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_DEPTH_STENCIL_DESC,            D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL>          PIPELINE_STATE_SUBOBJECT_DEPTH_STENCIL;
    typedef PIPELINE_STATE_SUBOBJECT<DXGI_FORMAT,                         D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_DEPTH_STENCIL_FORMAT>   PIPELINE_STATE_SUBOBJECT_DEPTH_STENCIL_FORMAT;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_RASTERIZER_DESC,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RASTERIZER>             PIPELINE_STATE_SUBOBJECT_RASTERIZER;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_RT_FORMAT_ARRAY,               D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_RENDER_TARGET_FORMATS>  PIPELINE_STATE_SUBOBJECT_RENDER_TARGET_FORMATS;
    typedef PIPELINE_STATE_SUBOBJECT<DXGI_SAMPLE_DESC,                    D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_DESC>            PIPELINE_STATE_SUBOBJECT_SAMPLE_DESC;
    typedef PIPELINE_STATE_SUBOBJECT<UINT,                                D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_SAMPLE_MASK>            PIPELINE_STATE_SUBOBJECT_SAMPLE_MASK;
    typedef PIPELINE_STATE_SUBOBJECT<D3D12_VIEW_INSTANCING_DESC,          D3D12_PIPELINE_STATE_SUBOBJECT_TYPE_VIEW_INSTANCING>        PIPELINE_STATE_SUBOBJECT_VIEW_INSTANCING;

    struct MSPipelineStateStream
    {
      PIPELINE_STATE_SUBOBJECT_ROOT_SIGNATURE root_signature;
      PIPELINE_STATE_SUBOBJECT_NODE_MASK node_mask;
      PIPELINE_STATE_SUBOBJECT_AS as_bytecode;
      PIPELINE_STATE_SUBOBJECT_MS ms_bytecode;
      PIPELINE_STATE_SUBOBJECT_PS ps_bytecode;
      PIPELINE_STATE_SUBOBJECT_BLEND blend_desc;
      PIPELINE_STATE_SUBOBJECT_RASTERIZER raster_desc;
      PIPELINE_STATE_SUBOBJECT_DEPTH_STENCIL depth_desc;
      PIPELINE_STATE_SUBOBJECT_VIEW_INSTANCING view_desc;
      PIPELINE_STATE_SUBOBJECT_PRIMITIVE_TOPOLOGY topology_type;
      PIPELINE_STATE_SUBOBJECT_SAMPLE_MASK sample_mask;
      PIPELINE_STATE_SUBOBJECT_SAMPLE_DESC sample_desc;
      PIPELINE_STATE_SUBOBJECT_DEPTH_STENCIL_FORMAT ds_format;
      PIPELINE_STATE_SUBOBJECT_RENDER_TARGET_FORMATS rt_formats;
      PIPELINE_STATE_SUBOBJECT_FLAGS state_flags;
    };

  protected:
    ID3D12RootSignature* root_signature{ nullptr };
    ID3D12PipelineState* pipeline_state{ nullptr };
    ID3D12CommandSignature* command_signature{ nullptr };

  protected:
    ID3D12StateObject* state_object{ nullptr };
    ID3D12Resource* table_buffer{ nullptr };
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE rgen_region;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE miss_region;
    D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE xhit_region;    
    //D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE call_region;

  protected:
    ID3D12Resource* tlas_item{ nullptr };
    uint32_t tlas_slot{ uint32_t(-1) };
    std::vector<ID3D12Resource*> blas_items;
    ID3D12Resource* instances_item{ nullptr };

  protected:
    static constexpr const wchar_t* rgen_name{ L"rgen" };
    static constexpr const wchar_t* ahit_name{ L"ahit" };
    static constexpr const wchar_t* chit_name{ L"chit" };
    static constexpr const wchar_t* isec_name{ L"isec" };
    static constexpr const wchar_t* miss_name{ L"miss" };

  protected:
    static constexpr const wchar_t* xhit_name{ L"xhit" };

  protected:
    std::vector<D3D12_ROOT_PARAMETER> root_parameters;

  protected:
    std::vector<D3D12_STATIC_SAMPLER_DESC> sampler_descs;

  protected:
    std::vector<D3D12_GPU_VIRTUAL_ADDRESS> ub_items;
    std::vector<D3D12_GPU_VIRTUAL_ADDRESS> sb_items;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> rr_items;
    std::vector<D3D12_GPU_DESCRIPTOR_HANDLE> wr_items;

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  //public:
  //  ID3D12StateObject* GetStateObject() const { return state_object; }
  //  ID3D12Resource* GetTableBuffer() const { return table_buffer; }

  //public:
  //  const D3D12_GPU_VIRTUAL_ADDRESS_RANGE& GetRgenRegion() const { return rgen_region; }
  //  const D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE& GetXhitRegion() const { return xhit_region; }
  //  const D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE& GetMissRegion() const { return miss_region; }
  //  const D3D12_GPU_VIRTUAL_ADDRESS_RANGE_AND_STRIDE& GetCallRegion() const { return call_region; }

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