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
#include "batch.h"

namespace RayGene3D
{
  class Pass;

  typedef void(*DefineVisitor)(const std::string&, const std::string&);

  class Config : public Usable
  {
  protected:
    Pass& pass;

  public:
    enum Compilation
    {
      COMPILATION_UNKNOWN = 0x00000000,
      COMPILATION_VS = 0x00000001,
      COMPILATION_HS = 0x00000002,
      COMPILATION_DS = 0x00000004,
      COMPILATION_GS = 0x00000008,
      COMPILATION_PS = 0x00000010,
      COMPILATION_CS = 0x00000100,
      COMPILATION_RGEN = 0x00001000,
      COMPILATION_ISEC = 0x00002000,
      COMPILATION_CHIT = 0x00004000,
      COMPILATION_AHIT = 0x00008000,
      COMPILATION_MISS = 0x00010000,
      COMPILATION_CALL = 0x00020000,
      COMPILATION_TASK = 0x00100000,
      COMPILATION_MESH = 0x00200000,
    };

  protected:
    std::vector<char> cs_bytecode;

    std::vector<char> vs_bytecode;
    std::vector<char> hs_bytecode;
    std::vector<char> ds_bytecode;
    std::vector<char> gs_bytecode;
    std::vector<char> ps_bytecode;

    std::vector<char> task_bytecode;
    std::vector<char> mesh_bytecode;

    std::vector<char> rgen_bytecode;
    std::vector<char> isec_bytecode;
    std::vector<char> chit_bytecode;
    std::vector<char> ahit_bytecode;
    std::vector<char> miss_bytecode;
    std::vector<char> call_bytecode;

  protected:
    std::string source;
    Compilation compilation;
    std::map<std::string, std::string> defines;

  public:
    enum Topology
    {
      TOPOLOGY_UNKNOWN = 0,
      TOPOLOGY_POINTLIST = 1,
      TOPOLOGY_LINELIST = 2,
      TOPOLOGY_LINESTRIP = 3,
      TOPOLOGY_TRIANGLELIST = 4,
      TOPOLOGY_TRIANGLESTRIP = 5,
      TOPOLOGY_LINELIST_ADJ = 10,
      TOPOLOGY_LINESTRIP_ADJ = 11,
      TOPOLOGY_TRIANGLELIST_ADJ = 12,
      TOPOLOGY_TRIANGLESTRIP_ADJ = 13,
      TOPOLOGY_1_CONTROL_POINT_PATCHLIST = 33,
      TOPOLOGY_2_CONTROL_POINT_PATCHLIST = 34,
      TOPOLOGY_3_CONTROL_POINT_PATCHLIST = 35,
      TOPOLOGY_4_CONTROL_POINT_PATCHLIST = 36,
      TOPOLOGY_5_CONTROL_POINT_PATCHLIST = 37,
      TOPOLOGY_6_CONTROL_POINT_PATCHLIST = 38,
      TOPOLOGY_7_CONTROL_POINT_PATCHLIST = 39,
      TOPOLOGY_8_CONTROL_POINT_PATCHLIST = 40,
      TOPOLOGY_9_CONTROL_POINT_PATCHLIST = 41,
      TOPOLOGY_10_CONTROL_POINT_PATCHLIST = 42,
      TOPOLOGY_11_CONTROL_POINT_PATCHLIST = 43,
      TOPOLOGY_12_CONTROL_POINT_PATCHLIST = 44,
      TOPOLOGY_13_CONTROL_POINT_PATCHLIST = 45,
      TOPOLOGY_14_CONTROL_POINT_PATCHLIST = 46,
      TOPOLOGY_15_CONTROL_POINT_PATCHLIST = 47,
      TOPOLOGY_16_CONTROL_POINT_PATCHLIST = 48,
      TOPOLOGY_17_CONTROL_POINT_PATCHLIST = 49,
      TOPOLOGY_18_CONTROL_POINT_PATCHLIST = 50,
      TOPOLOGY_19_CONTROL_POINT_PATCHLIST = 51,
      TOPOLOGY_20_CONTROL_POINT_PATCHLIST = 52,
      TOPOLOGY_21_CONTROL_POINT_PATCHLIST = 53,
      TOPOLOGY_22_CONTROL_POINT_PATCHLIST = 54,
      TOPOLOGY_23_CONTROL_POINT_PATCHLIST = 55,
      TOPOLOGY_24_CONTROL_POINT_PATCHLIST = 56,
      TOPOLOGY_25_CONTROL_POINT_PATCHLIST = 57,
      TOPOLOGY_26_CONTROL_POINT_PATCHLIST = 58,
      TOPOLOGY_27_CONTROL_POINT_PATCHLIST = 59,
      TOPOLOGY_28_CONTROL_POINT_PATCHLIST = 60,
      TOPOLOGY_29_CONTROL_POINT_PATCHLIST = 61,
      TOPOLOGY_30_CONTROL_POINT_PATCHLIST = 62,
      TOPOLOGY_31_CONTROL_POINT_PATCHLIST = 63,
      TOPOLOGY_32_CONTROL_POINT_PATCHLIST = 64,
    };

    enum Indexer
    {
      INDEXER_UNKNOWN = 0,
      INDEXER_8_BIT = 1,
      INDEXER_16_BIT = 2,
      INDEXER_32_BIT = 3,
    };

    struct Attribute
    {
      uint32_t slot{ 0 };
      uint32_t offset{ 0 };
      uint32_t stride{ 0 };
      Format format{ FORMAT_UNKNOWN };
      bool instance{ true };
    };

    struct IAState
    {
      Topology topology{ TOPOLOGY_UNKNOWN };
      Indexer indexer{ INDEXER_UNKNOWN };  //TODO: Remove
      std::vector<Attribute> attributes;
    };

  protected:
    IAState ia_state;

  public:
    enum Fill
    {
      FILL_UNKNOWN = 0,
      FILL_POINT = 1,
      FILL_LINE = 2,
      FILL_SOLID = 3,
    };

    enum Cull
    {
      CULL_UNKNOWN = 0,
      CULL_NONE = 1,
      CULL_FRONT = 2,
      CULL_BACK = 3,
    };

    struct Viewport
    {
      float origin_x;
      float origin_y;
      float extent_x;
      float extent_y;
      float min_z;
      float max_z;
    };

    struct Scissor
    {
      int32_t origin_x;
      int32_t origin_y;
      uint32_t extent_x;
      uint32_t extent_y;
    };

    struct RCState
    {
      Fill fill_mode{ FILL_SOLID };
      Cull cull_mode{ CULL_BACK };
      std::vector<Viewport> viewports;
      std::vector<Scissor> scissors;
      int32_t depth_bias{ 0 };
      float bias_clamp{ 0.0f };
      float bias_slope{ 0.0f };
      bool clip_enabled{ false };
      bool scissor_enabled{ false };
      bool multisample_enabled{ false };
      float line_width{ 1.0f };
    };

  protected:
    RCState rc_state;

  public:
    enum Comparison
    {
      COMPARISON_UNKNOWN = 0,
      COMPARISON_NEVER = 1,
      COMPARISON_LESS = 2,
      COMPARISON_EQUAL = 3,
      COMPARISON_LESS_EQUAL = 4,
      COMPARISON_GREATER = 5,
      COMPARISON_NOT_EQUAL = 6,
      COMPARISON_GREATER_EQUAL = 7,
      COMPARISON_ALWAYS = 8,
    };

    enum Action
    {
      ACTION_UNKNOWN = 0,
      ACTION_KEEP = 1,
      ACTION_ZERO = 2,
      ACTION_REPLACE = 3,
      ACTION_INCR_SAT = 4,
      ACTION_DECR_SAT = 5,
      ACTION_INVERT = 6,
      ACTION_INCR = 7,
      ACTION_DECR = 8
    };

    struct Mode
    {
      Comparison comparison{ COMPARISON_ALWAYS };
      Action depth_fail{ ACTION_KEEP };
      Action stencil_fail{ ACTION_KEEP };
      Action stencil_pass{ ACTION_KEEP };
    };

  public:
    struct DSState
    {
      bool depth_enabled{ true };
      bool depth_write{ true };
      Comparison depth_comparison{ COMPARISON_LESS };
      bool stencil_enabled{ false };
      uint8_t stencil_rmask{ 0xFF };
      uint8_t stencil_wmask{ 0xFF };
      uint32_t stencil_reference{ 0 };
      Mode stencil_fface_mode;
      Mode stencil_bface_mode;
    };

  protected:
    DSState ds_state;
    
  public:
    enum Operand
    {
      OPERAND_UNKNOWN = 0,
      OPERAND_ZERO = 1,
      OPERAND_ONE = 2,
      OPERAND_SRC_COLOR = 3,
      OPERAND_INV_SRC_COLOR = 4,
      OPERAND_SRC_ALPHA = 5,
      OPERAND_INV_SRC_ALPHA = 6,
      OPERAND_DEST_ALPHA = 7,
      OPERAND_INV_DEST_ALPHA = 8,
      OPERAND_DEST_COLOR = 9,
      OPERAND_INV_DEST_COLOR = 10,
      OPERAND_SRC_ALPHA_SAT = 11,
      OPERAND_BLEND_FACTOR = 14,
      OPERAND_INV_BLEND_FACTOR = 15,
    };

    enum Operation
    {
      OPERATION_UNKNOWN = 0,
      OPERATION_ADD = 1,
      OPERATION_SUBTRACT = 2,
      OPERATION_REV_SUBTRACT = 3,
      OPERATION_MIN = 4,
      OPERATION_MAX = 5,
    };

    struct Blend
    {
      bool blend_enabled{ false };
      Operand src_color{ OPERAND_ONE };
      Operand dst_color{ OPERAND_ZERO };
      Operation blend_color{ OPERATION_ADD };
      Operand src_alpha{ OPERAND_ONE };
      Operand dst_alpha{ OPERAND_ZERO };
      Operation blend_alpha{ OPERATION_ADD };
      uint8_t write_mask{ 0xF };
    };

  public:
    struct OMState
    {
      bool separated_blend{ false };
      std::vector<Blend> target_blends;
      bool atc_enabled{ false };
      float blend_factor[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
      uint32_t sample_mask{ 0xFFFFFFFF };
    };

  protected:
    OMState om_state;

  protected:
    std::list<std::shared_ptr<Batch>> batches;

  public:
    Pass& GetPass() { return pass; }

  public:
    virtual const std::shared_ptr<Batch>& CreateBatch(const std::string& name,
      const std::pair<const Batch::Entity*, uint32_t>& entities,
      const std::pair<const Batch::Sampler*, uint32_t>& samplers = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views = {}
    ) = 0;
    //void VisitBatch(std::function<void(const std::shared_ptr<Batch>&)> visitor) { for (const auto& batch : batches) visitor(batch); }
    void DestroyBatch(const std::shared_ptr<Batch>& batch) 
    {
      if(batch) batches.remove(batch);
    }
  
  public:
    const IAState& GetIAState() const { return ia_state; }
    const RCState& GetRCState() const { return rc_state; }
    const DSState& GetDSState() const { return ds_state; }
    const OMState& GetOMState() const { return om_state; }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Config(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    Config(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    Config(const std::string& name,
      Pass& pass,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines);
    virtual ~Config();
  };

  typedef std::shared_ptr<Config> SPtrConfig;
  typedef std::weak_ptr<Config> WPtrConfig;
  typedef std::unique_ptr<Config> UPtrConfig;
}