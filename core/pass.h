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

  protected:
    uint32_t size_x{ 0u };
    uint32_t size_y{ 0u };
    uint32_t layers{ 1u };

  protected:
    bool enabled{ false };

  protected:
    Device& device;

  protected:
    std::list<std::shared_ptr<Config>> configs;

  public:
    void SetType(Type type) { this->type = type; }
    Type GetType() const { return type; }

    void SetEnabled(bool enabled) { this->enabled = enabled; }
    bool GetEnabled() const { return enabled; }

  public:
    Device& GetDevice() { return device; }

  public:
    virtual const std::shared_ptr<Config>& CreateConfig(const std::string& name,
      const std::string& source,
      Config::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Config::IAState& ia_state,
      const Config::RCState& rc_state,
      const Config::DSState& ds_state,
      const Config::OMState& om_state) = 0;
    //void VisitConfig(std::function<void(const std::shared_ptr<Config>&)> visitor) { for (const auto& effect : effects) visitor(effect); }
    void DestroyConfig(const std::shared_ptr<Config>& config) 
    { 
      if(config) configs.remove(config);
    }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Pass(const std::string& name,
      Device& device,
      Pass::Type type,
      uint32_t size_x,
      uint32_t size_y,
      uint32_t layers,
      const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments,
      const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments
    );
    virtual ~Pass();
  };

  typedef std::shared_ptr<Pass> SPtrPass;
  typedef std::weak_ptr<Pass> WPtrPass;
  typedef std::unique_ptr<Pass> UPtrPass;
}