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

  class Resource : public Usable
  {
  protected:
    Device& device;

  public:
    enum Type
    {
      TYPE_UNKNOWN = 0,
      TYPE_BUFFER = 1,
      TYPE_TEX1D = 2,
      TYPE_TEX2D = 3,
      TYPE_TEX3D = 4,
    };

    enum Hint
    {
      HINT_UNKNOWN = 0,
      HINT_CUBEMAP_IMAGE = 0x1,
      HINT_LAYERED_IMAGE = 0x2,
      HINT_DYNAMIC_BUFFER = 0x10,
      HINT_ADDRESS_BUFFER = 0x20,
      HINT_FORCE_UINT = 0xffffffff
    };

  protected:
    Usage usage{ USAGE_UNKNOWN };

  protected:
    uint32_t mipmaps_or_count{ 0 };
    uint32_t layers_or_stride{ 0 };

  protected:
    Format format{ FORMAT_UNKNOWN };
    uint32_t size_x{ 0 };
    uint32_t size_y{ 0 };
    uint32_t size_z{ 0 };

  protected:
    Type type{ TYPE_UNKNOWN };
    Hint hint{ HINT_UNKNOWN };

  protected:
    std::list<std::shared_ptr<View>> views;

  protected:
    std::vector<std::pair<const void*, uint32_t>> interops;

  public:
    struct BufferDesc
    {
      Usage usage{ USAGE_UNKNOWN };
      uint32_t stride{ 0 };
      uint32_t count{ 0 };
    };

    struct Tex1DDesc
    {
      Usage usage{ USAGE_UNKNOWN };
      uint32_t mipmaps{ 0 };
      uint32_t layers{ 0 };
      Format format{ FORMAT_UNKNOWN };
      uint32_t size_x{ 0 };
    };

    struct Tex2DDesc
    {
      Usage usage{ USAGE_UNKNOWN };
      uint32_t mipmaps{ 0 };
      uint32_t layers{ 0 };
      Format format{ FORMAT_UNKNOWN };
      uint32_t size_x{ 0 };
      uint32_t size_y{ 0 };
    };

    struct Tex3DDesc
    {
      Usage usage{ USAGE_UNKNOWN };
      uint32_t mipmaps{ 0 };
      uint32_t layers{ 0 };
      Format format{ FORMAT_UNKNOWN };
      uint32_t size_x{ 0 };
      uint32_t size_y{ 0 };
      uint32_t size_z{ 0 };
    };

  public:
    Device& GetDevice() { return device; }

  public:
    Usage GetUsage() const { return usage; }

  public:
    uint32_t GetMipmapsOrCount() const { return mipmaps_or_count; }
    uint32_t GetLayersOrStride() const { return layers_or_stride; }

  public:
    Format GetFormat() const { return format; }
    uint32_t GetSizeX() const { return size_x; }
    uint32_t GetSizeY() const { return size_y; }
    uint32_t GetSizeZ() const { return size_z; }

  public:
    Type GetType() const { return type; }
    Hint GetHint() const { return hint; }

  public:
    virtual void Commit(uint32_t index) = 0;
    //virtual void Commit(uint32_t index, uint32_t offset_x, uint32_t offset_y, uint32_t offset_z, uint32_t count_x, uint32_t count_y, uint32_t count_z) = 0;
    virtual void Retrieve(uint32_t index) = 0;
    //virtual void Retrieve(uint32_t index, uint32_t offset_x, uint32_t offset_y, uint32_t offset_z, uint32_t count_x, uint32_t count_y, uint32_t count_z) = 0;
    virtual void Blit(const std::shared_ptr<Resource>& resource) = 0;
    //virtual void Blit(const std::shared_ptr<Resource>& resource, uint32_t index, uint32_t offset_x, uint32_t offset_y, uint32_t offset_z, uint32_t count_x, uint32_t count_y, uint32_t count_z) = 0;

    virtual void* Map() = 0;
    //virtual void* Map(void** data, uint32_t offset, uint32_t count) = 0;
    virtual void Unmap() = 0;

  //public:
  //  std::list<std::shared_ptr<View>>& AccessViews() { return views; }

  public:
    virtual const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, 
      const View::Range& mipmaps_or_count = View::Range{ 0, uint32_t(-1) },
      const View::Range& layers_or_stride = View::Range{ 0, uint32_t(-1) },
      View::Bind bind = View::BIND_UNKNOWN
    ) = 0;
    void VisitView(std::function<void(const std::shared_ptr<View>&)> visitor) { for (const auto& view : views) visitor(view); }
    void DestroyView(const std::shared_ptr<View>& view) { views.remove(view); };

  public:
    void SetInteropCount(uint32_t count) { interops.resize(count); }
    uint32_t GetInteropCount() const { return uint32_t(interops.size()); }
    void SetInteropItem(uint32_t index, std::pair<const void*, uint32_t> item) { interops.at(index) = item; }
    std::pair<const void*, uint32_t> GetInteropItem(uint32_t index) { return interops.at(index); }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Resource(const std::string& name,
      Device& device, const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    Resource(const std::string& name,
      Device& device, const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    Resource(const std::string& name,
      Device& device, const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    Resource(const std::string& name,
      Device& device, const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    virtual ~Resource();
  };

  typedef std::shared_ptr<Resource> SPtrResource;
  typedef std::weak_ptr<Resource> WPtrResource;
  typedef std::unique_ptr<Resource> UPtrResource;
}
