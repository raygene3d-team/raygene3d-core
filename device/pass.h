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
#include "../base.h"
#include "config.h"
#include "layout.h"

namespace RayGene3D
{
  class Device;

  class Pass : public Usable
  {
  public:
    using RTValue = std::optional<std::array<float, 4>>;
    using DSValue = std::pair<std::optional<float>, std::optional<uint8_t>>;

    enum Type
    {
      TYPE_UNKNOWN = 0,
      TYPE_GRAPHIC = 1,
      TYPE_COMPUTE = 2,
      TYPE_RAYTRACING = 3,
    };

    struct Graphic
    {
      uint32_t idx_count{ 0 };
      uint32_t ins_count{ 0 };
      uint32_t idx_offset{ 0 };
      uint32_t vtx_offset{ 0 };
      uint32_t ins_offset{ 0 };
      uint32_t padding[3]{ uint32_t(-1), uint32_t(-1), uint32_t(-1) };
    };

    struct Compute
    {
      uint32_t grid_x{ 0 };
      uint32_t grid_y{ 0 };
      uint32_t grid_z{ 0 };
      uint32_t padding{ uint32_t(-1) };
    };

  protected:
    struct Subpass
    {
      std::shared_ptr<Config> shader;
      std::shared_ptr<Layout> layout;

      std::vector<std::shared_ptr<View>> va_views;
      std::vector<std::shared_ptr<View>> ia_views;
      std::vector<std::shared_ptr<View>> aa_views;

      std::vector<Graphic> graphic_tasks;
      std::vector<Compute> compute_tasks;

      std::vector<uint32_t> sb_offsets;
    };

    Type type{ TYPE_UNKNOWN };

    std::vector<std::shared_ptr<View>> rt_views;
    std::vector<std::shared_ptr<View>> ds_views;
    std::vector<RTValue> rt_values;
    std::vector<DSValue> ds_values;

    std::vector<Subpass> subpasses;

  protected:
    bool enabled{ false };

  protected:
    Device& device;

  public:
    void SetType(Type type) { this->type = type; }
    Type GetType() const { return type; }

    void SetEnabled(bool enabled) { this->enabled = enabled; }
    bool GetEnabled() const { return enabled; }

    void UpdateRTViews(std::pair<const std::shared_ptr<View>*, uint32_t> rt_views) { 
      this->rt_views.assign(rt_views.first, rt_views.first + rt_views.second);
    }
    void UpdateDSViews(std::pair<const std::shared_ptr<View>*, uint32_t> ds_views) {
     this-> ds_views.assign(ds_views.first, ds_views.first + ds_views.second); 
    }
    void UpdateRTValues(std::pair<const RTValue*, uint32_t> rt_values) { 
     this->rt_values.assign(rt_values.first, rt_values.first + rt_values.second);
    }
    void UpdateDSValues(std::pair<const DSValue*, uint32_t> ds_values) { 
      this->ds_values.assign(ds_values.first, ds_values.first + ds_values.second);
    }

  public:
    void SetSubpassCount(uint32_t count) { subpasses.resize(count); }
    uint32_t GetSubpassCount() const { return uint32_t(subpasses.size()); }

    void SetSubpassShader(uint32_t subpass, const std::shared_ptr<Config>& shader) { subpasses.at(subpass).shader = shader; }
    void SetSubpassLayout(uint32_t subpass, const std::shared_ptr<Layout>& layout) { subpasses.at(subpass).layout = layout; }

    void UpdateSubpassVAViews(uint32_t subpass, std::pair<const std::shared_ptr<View>*, uint32_t>  va_views) { 
      subpasses.at(subpass).va_views.assign(va_views.first, va_views.first + va_views.second);
    }
    void UpdateSubpassIAViews(uint32_t subpass, std::pair<const std::shared_ptr<View>*, uint32_t>  ia_views) {
      subpasses.at(subpass).ia_views.assign(ia_views.first, ia_views.first + ia_views.second);
    }
    void UpdateSubpassAAViews(uint32_t subpass, std::pair<const std::shared_ptr<View>*, uint32_t>  aa_views) {
      subpasses.at(subpass).aa_views.assign(aa_views.first, aa_views.first + aa_views.second);
    }
    void UpdateSubpassSBOffsets(uint32_t subpass, std::pair<const uint32_t*, uint32_t>  sb_offsets) {
      subpasses.at(subpass).sb_offsets.assign(sb_offsets.first, sb_offsets.first + sb_offsets.second);
    }
    void UpdateSubpassGraphicTasks(uint32_t subpass, std::pair<const Graphic*, uint32_t>  graphic_tasks) {
      subpasses.at(subpass).graphic_tasks.assign(graphic_tasks.first, graphic_tasks.first + graphic_tasks.second);
    }
    void UpdateSubpassComputeTasks(uint32_t subpass, std::pair<const Compute*, uint32_t>  compute_tasks) {
      subpasses.at(subpass).compute_tasks.assign(compute_tasks.first, compute_tasks.first + compute_tasks.second);
    }
  public:
    Device& GetDevice() { return device; }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Pass(const std::string& name, Device& device);
    virtual ~Pass();
  };
}