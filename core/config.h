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
#include "view.h"

namespace RayGene3D
{
  class Device;

  typedef void(*DefineVisitor)(const std::string&, const std::string&);

  class Config : public Usable
  {
  protected:
    Device& device;

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
      COMPILATION_CHIT = 0x00002000,
      COMPILATION_AHIT = 0x00004000,
      COMPILATION_MISS = 0x00008000,
    };

  protected:
    std::vector<char> vs_bytecode;
    std::vector<char> hs_bytecode;
    std::vector<char> ds_bytecode;
    std::vector<char> gs_bytecode;
    std::vector<char> ps_bytecode;
    std::vector<char> cs_bytecode;

    std::vector<char> rgen_bytecode;
    std::vector<char> chit_bytecode;
    std::vector<char> ahit_bytecode;
    std::vector<char> miss_bytecode;

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
      Indexer indexer{ INDEXER_UNKNOWN };
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
    enum Argument
    {
      ARGUMENT_UNKNOWN = 0,
      ARGUMENT_ZERO = 1,
      ARGUMENT_ONE = 2,
      ARGUMENT_SRC_COLOR = 3,
      ARGUMENT_INV_SRC_COLOR = 4,
      ARGUMENT_SRC_ALPHA = 5,
      ARGUMENT_INV_SRC_ALPHA = 6,
      ARGUMENT_DEST_ALPHA = 7,
      ARGUMENT_INV_DEST_ALPHA = 8,
      ARGUMENT_DEST_COLOR = 9,
      ARGUMENT_INV_DEST_COLOR = 10,
      ARGUMENT_SRC_ALPHA_SAT = 11,
      ARGUMENT_BLEND_FACTOR = 14,
      ARGUMENT_INV_BLEND_FACTOR = 15,
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
      Argument src_color{ ARGUMENT_ONE };
      Argument dst_color{ ARGUMENT_ZERO };
      Operation blend_color{ OPERATION_ADD };
      Argument src_alpha{ ARGUMENT_ONE };
      Argument dst_alpha{ ARGUMENT_ZERO };
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

  public:
    Device& GetDevice() { return device; }

  //public:
  //  void SetVSBytecode(const std::vector<char>& source) { vs_bytecode = source; }
  //  const std::vector<char>& GetVSBytecode() const { return vs_bytecode; }
  //  void SetHSBytecode(const std::vector<char>& source) { hs_bytecode = source; }
  //  const std::vector<char>& GetHSBytecode() const { return hs_bytecode; }
  //  void SetDSBytecode(const std::vector<char>& source) { ds_bytecode = source; }
  //  const std::vector<char>& GetDSBytecode() const { return ds_bytecode; }
  //  void SetGSBytecode(const std::vector<char>& source) { gs_bytecode = source; }
  //  const std::vector<char>& GetGSBytecode() const { return gs_bytecode; }
  //  void SetPSBytecode(const std::vector<char>& source) { ps_bytecode = source; }
  //  const std::vector<char>& GetPSBytecode() const { return ps_bytecode; }
  //  void SetCSBytecode(const std::vector<char>& source) { cs_bytecode = source; }
  //  const std::vector<char>& GetCSBytecode() const { return cs_bytecode; }
  //
  //  void SetRGenBytecode(const std::vector<char>& source) { rgen_bytecode = source; }
  //  const std::vector<char>& GetRGenBytecode() const { return rgen_bytecode; }
  //  void SetCHitBytecode(const std::vector<char>& source) { chit_bytecode = source; }
  //  const std::vector<char>& GetCHitBytecode() const { return chit_bytecode; }
  //  void SetAHitBytecode(const std::vector<char>& source) { ahit_bytecode = source; }
  //  const std::vector<char>& GetAHitBytecode() const { return ahit_bytecode; }
  //  void SetMissBytecode(const std::vector<char>& source) { miss_bytecode = source; }
  //  const std::vector<char>& GetMissBytecode() const { return miss_bytecode; }

  //public:
  //  void SetSource(const std::string& source) { this->source = source; }
  //  const std::string& GetSource() const { return source; }
  //  void SetCompilation(Compilation compilation) { this->compilation = compilation; }
  //  Compilation GetCompilation() const { return compilation; }
  //
  //public:
  //  void SetDefineItem(const std::string& key, const std::string& value) { defines[key] = value; }
  //  const std::string& GetDefineItem(const std::string& key) const { return defines.at(key); }
  //  bool CheckDefineItem(const std::string& key) { return defines.find(key) != defines.end(); }
  //  void RemoveDefineItem(const std::string& key) { defines.erase(key); }
  //  //uint32_t CountDefineItem() { return uint32_t(defines.size()); }
  //  //void VisitDefineItem(DefineVisitor visitor) { for (const auto& define : defines) visitor(define.first, define.second); }
  
  public:
  //  void SetIAState(const IAState& state) { ia_state = state; }
    const IAState& GetIAState() const { return ia_state; }
  //public: //obsolete methods
  //  void SetTopology(Topology topology) { this->ia_state.topology = topology; }
  //  Topology GetTopology() const { return ia_state.topology; }
  //  void SetIndexer(Indexer indexer) { this->ia_state.indexer = indexer; }
  //  Indexer GetIndexer() const { return ia_state.indexer; }
  //  void UpdateAttributes(std::pair<const Attribute*, uint32_t> attributes) { this->ia_state.attributes.assign(attributes.first, attributes.first + attributes.second); }
  //  
  public:
  //  void SetRCState(const RCState& state) { rc_state = state; }
    const RCState& GetRCState() const { return rc_state; }
  //public: //obsolete methods
  //  void SetFillMode(Fill fill_mode) { this->rc_state.fill_mode = fill_mode; }
  //  Fill GetFillMode() const { return this->rc_state.fill_mode; }
  //  void SetCullMode(Cull cull_mode) { this->rc_state.cull_mode = cull_mode; }
  //  Cull GetCullMode() const { return this->rc_state.cull_mode; }
  //  void SetClipEnabled(bool clip_enabled) { this->rc_state.clip_enabled = clip_enabled; }
  //  bool GetClipEnabled() const { return this->rc_state.clip_enabled; }
  //  void UpdateViewports(std::pair<const Viewport*, uint32_t> viewports) { this->rc_state.viewports.assign(viewports.first, viewports.first + viewports.second); }
  //  void UpdateScissors(std::pair<const Scissor*, uint32_t> scissors) { this->rc_state.scissors.assign(scissors.first, scissors.first + scissors.second); }
  //
  public:
  //  void SetDSState(const DSState& state) { ds_state = state; }
    const DSState& GetDSState() const { return ds_state; }
  //public: //obsolete methods
  //  void SetDepthEnabled(bool depth_enabled) { this->ds_state.depth_enabled = depth_enabled; }
  //  bool GetDepthEnabled() const { return this->ds_state.depth_enabled; }
  //  void SetDepthWrite(bool depth_write) { this->ds_state.depth_write = depth_write; }
  //  bool GetDepthWrite() const { return this->ds_state.depth_write; }
  //  void SetDepthComparison(Comparison depth_comparison) { this->ds_state.depth_comparison = depth_comparison; }
  //  Comparison GetDepthComparison() const { return this->ds_state.depth_comparison; }
  //
  public:
  //  void SetOMState(const OMState& state) { om_state = state; }
    const OMState& GetOMState() const { return om_state; }
  //public: //obsolete methods
  //  void SetATCEnabled(bool atc_enabled) { this->om_state.atc_enabled = atc_enabled; }
  //  bool GetATCEnabled() const { return this->om_state.atc_enabled; }
  //  void UpdateBlends(std::pair<const Blend*, uint32_t> blends) { this->om_state.target_blends.assign(blends.first, blends.first + blends.second); }
  //
  //public:


  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Config(const std::string& name,
      Device& device,
      const std::string& source,
      Config::Compilation compilation, 
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state);
    virtual ~Config();
  };
}