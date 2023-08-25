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

  protected:
    Type type{ TYPE_UNKNOWN };

    //struct Graphic
    //{
    //  uint32_t idx_count{ 0 };
    //  uint32_t ins_count{ 0 };
    //  uint32_t idx_offset{ 0 };
    //  uint32_t vtx_offset{ 0 };
    //  uint32_t ins_offset{ 0 };
    //  uint32_t padding[3]{ uint32_t(-1), uint32_t(-1), uint32_t(-1) };
    //};

    //struct Compute
    //{
    //  uint32_t grid_x{ 0 };
    //  uint32_t grid_y{ 0 };
    //  uint32_t grid_z{ 0 };
    //  uint32_t padding{ uint32_t(-1) };
    //};

  public:
    struct Argument
    {
      uint32_t idx_count{ 0 };
      uint32_t ins_count{ 0 };
      uint32_t idx_offset{ 0 };
      uint32_t vtx_offset{ 0 };
      uint32_t ins_offset{ 0 };
      uint32_t grid_x{ 0 };
      uint32_t grid_y{ 0 };
      uint32_t grid_z{ 0 };
    };

    struct Command
    {
      std::shared_ptr<View> view;
      Argument argument;
      std::vector<uint32_t> offsets;
    };

  public:
    struct Subpass
    {
      std::shared_ptr<Config> config;
      std::shared_ptr<Layout> layout;

      std::vector<Command> commands;

      std::vector<std::shared_ptr<View>> va_views;
      std::vector<std::shared_ptr<View>> ia_views;
    };

  protected:
    std::vector<Subpass> subpasses;

  public:
    struct RTAttachment
    {
      std::shared_ptr<View> view;
      RTValue value;
    };

    struct DSAttachment
    {
      std::shared_ptr<View> view;
      DSValue value;
    };

  protected:
    std::vector<RTAttachment> rt_attachments;
    std::vector<DSAttachment> ds_attachments;

    std::vector<std::shared_ptr<View>> rt_views;
    std::vector<std::shared_ptr<View>> ds_views;
    std::vector<RTValue> rt_values;
    std::vector<DSValue> ds_values;

  protected:
    bool enabled{ false };

  protected:
    Device& device;

  public:
    void SetType(Type type) { this->type = type; }
    Type GetType() const { return type; }

    void SetEnabled(bool enabled) { this->enabled = enabled; }
    bool GetEnabled() const { return enabled; }

  public:
    void UpdateRTAttachments(std::pair<const RTAttachment*, uint32_t> attachments) {
      this->rt_attachments.assign(attachments.first, attachments.first + attachments.second);
    }
    void UpdateDSAttachments(std::pair<const DSAttachment*, uint32_t> attachments) {
      this->ds_attachments.assign(attachments.first, attachments.first + attachments.second);
    }
  public:
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
    void UpdateSubpasses(std::pair<const Subpass*, uint32_t> subpasses) {
      this->subpasses.assign(subpasses.first, subpasses.first + subpasses.second);
    }
  public:
    void SetSubpassCount(uint32_t count) { subpasses.resize(count); }
    uint32_t GetSubpassCount() const { return uint32_t(subpasses.size()); }

    void SetSubpassConfig(uint32_t subpass, const std::shared_ptr<Config>& config) { subpasses.at(subpass).config = config; }
    void SetSubpassLayout(uint32_t subpass, const std::shared_ptr<Layout>& layout) { subpasses.at(subpass).layout = layout; }

    void UpdateSubpassCommands(uint32_t subpass, std::pair<const Command*, uint32_t> commands) {
      subpasses.at(subpass).commands.assign(commands.first, commands.first + commands.second);
    }

  public:
    void UpdateSubpassVAViews(uint32_t subpass, std::pair<const std::shared_ptr<View>*, uint32_t> va_views) {
      subpasses.at(subpass).va_views.assign(va_views.first, va_views.first + va_views.second); 
    }
    void UpdateSubpassIAViews(uint32_t subpass, std::pair<const std::shared_ptr<View>*, uint32_t> ia_views) { 
      subpasses.at(subpass).ia_views.assign(ia_views.first, ia_views.first + ia_views.second); 
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