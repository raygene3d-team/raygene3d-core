
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
#include "resource.h"
#include "pass.h"

namespace RayGene3D
{
  class Device : public Usable
  {
  protected:
    void* display{ nullptr };
    void* window{ nullptr };
    std::string name;

  protected:
    uint32_t ordinal{ 0 };
    bool debug{ false };

  protected:
    std::string path;
    uint32_t extent_x{ 0 };
    uint32_t extent_y{ 0 };
    std::shared_ptr<Resource> screen;

  protected:
    std::list<std::shared_ptr<Resource>> resources;
    
    
    std::list<std::shared_ptr<Pass>> passes;

  public:
    //const void* GetHandle() const { return handle; }
    const std::string& GetName() const { return name; }

  public:
    void SetOrdinal(uint32_t ordinal) { this->ordinal = ordinal; }
    uint32_t GetOrdinal() const { return ordinal; }
    void SetDebug(bool debug) { this->debug = debug; }
    bool GetDebug() const { return debug; }

  public:
    void SetWindow(void* window) { this->window = window; }
    void SetDisplay(void* display) { this->display = display; }

  public:
    void SetPath(const std::string& path) { this->path = path; }
    const std::string& GetPath() const { return path; }
    void SetExtentX(uint32_t extent_x) { this->extent_x = extent_x; }
    uint32_t GetExtentX() const { return extent_x; }
    void SetExtentY(uint32_t extent_y) { this->extent_y = extent_y; }
    uint32_t GetExtentY() const { return extent_y; }
    void SetScreen(const std::shared_ptr<Resource>& screen) { this->screen = screen; }
    const std::shared_ptr<Resource>& GetScreen() const { return screen; }

  public:
    virtual const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::BufferDesc& desc, 
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) = 0;
    virtual const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) = 0;
    virtual const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) = 0;
    virtual const std::shared_ptr<Resource>& CreateResource(const std::string& name,
      const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {}) = 0;
    void VisitResource(std::function<void(const std::shared_ptr<Resource>&)> visitor) { for (const auto& resource : resources) visitor(resource); }
    void DestroyResource(const std::shared_ptr<Resource>& resource) { resources.remove(resource); }

    virtual const std::shared_ptr<Pass>& CreatePass(const std::string& name,
      Pass::Type type,
      const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments,
      const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments,
      const View::Range& ins_or_grid_x = View::Range{ 0, 0 },
      const View::Range& vtx_or_grid_y = View::Range{ 0, 0 }) = 0;
    void VisitPass(std::function<void(const std::shared_ptr<Pass>&)> visitor) { for (const auto& pass : passes) visitor(pass); }
    void DestroyPass(const std::shared_ptr<Pass>& pass) { passes.remove(pass); }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Device(const std::string& name);
    virtual ~Device();
  };

  typedef std::shared_ptr<Device> SPtrDevice;
  typedef std::weak_ptr<Device> WPtrDevice;
  typedef std::unique_ptr<Device> UPtrDevice;
}
