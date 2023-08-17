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
      TYPE_IMAGE1D = 2,
      TYPE_IMAGE2D = 3,
      TYPE_IMAGE3D = 4,
    };

    enum Hint
    {
      HINT_UNKNOWN = 0,
      HINT_CUBEMAP_IMAGE = 0x1,
      HINT_LAYERED_IMAGE = 0x2,
      HINT_DYNAMIC_BUFFER = 0x10,
      HINT_FORCE_UINT = 0xffffffff
    };

  protected:
    std::list<std::shared_ptr<View>> views;

  protected:
    std::vector<std::pair<const void*, uint32_t>> interops;

  public:
    struct BufferDesc
    {
      uint32_t stride{ 0 };
      uint32_t count{ 0 };
      Usage usage{ USAGE_UNKNOWN };
    };

    struct Tex1DDesc
    {
      Format format{ FORMAT_UNKNOWN };
      uint32_t extent_x{ 1 };
      uint32_t layers{ 1 };
      uint32_t mipmaps{ 1 };
      Usage usage{ USAGE_UNKNOWN };
    };

    struct Tex2DDesc
    {
      Format format{ FORMAT_UNKNOWN };
      uint32_t extent_x{ 1 };
      uint32_t extent_y{ 1 };
      uint32_t layers{ 1 };
      uint32_t mipmaps{ 1 };
      Usage usage{ USAGE_UNKNOWN };
    };

    struct Tex3DDesc
    {
      Format format{ FORMAT_UNKNOWN };
      uint32_t extent_x{ 1 };
      uint32_t extent_y{ 1 };
      uint32_t extent_z{ 1 };
      uint32_t mipmaps{ 1 };
      Usage usage{ USAGE_UNKNOWN };
    };

    union Desc
    {
      BufferDesc buffer_desc;
      Tex1DDesc tex1d_desc;
      Tex2DDesc tex2d_desc;
      Tex3DDesc tex3d_desc;
    };
    
  protected:
    Desc desc{ 0 };

  protected:
    uint32_t stride{ 0 };
    uint32_t count{ 0 };

  protected:
    uint32_t extent_x{ 1 };
    uint32_t extent_y{ 1 };
    uint32_t extent_z{ 1 };
    uint32_t layers{ 1 };
    uint32_t mipmaps{ 1 };

  protected:
    Type type{ TYPE_UNKNOWN };
    Hint hint{ HINT_UNKNOWN };

  protected:
    Format format{ FORMAT_UNKNOWN };
    Usage usage{ USAGE_UNKNOWN };

  public:
    Device& GetDevice() { return device; }

  public:
    void SetBufferDesc(const BufferDesc desc) { this->desc.buffer_desc = desc; }
    const BufferDesc& GetBufferDesc() const { return desc.buffer_desc; }
    void SetTex1DDesc(const Tex1DDesc desc) { this->desc.tex1d_desc = desc; }
    const Tex1DDesc& GetTex1DDesc() const { return desc.tex1d_desc; }
    void SetTex2DDesc(const Tex2DDesc desc) { this->desc.tex2d_desc = desc; }
    const Tex2DDesc& GetTex2DDesc() const { return desc.tex2d_desc; }
    void SetTex3DDesc(const Tex3DDesc desc) { this->desc.tex3d_desc = desc; }
    const Tex3DDesc& GetTex3DDesc() const { return desc.tex3d_desc; }

  public:
    void SetFormat(Format format) { this->format = format; }
    Format GetFormat() const { return format; }

  public:
    void SetStride(uint32_t stride) { this->stride = stride; }
    uint32_t GetStride() const { return stride; }
    void SetCount(uint32_t count) { this->count = count; }
    uint32_t GetCount() const { return count; }

  public:
    void SetExtentX(uint32_t extent_x) { this->extent_x = extent_x; }
    uint32_t GetExtentX() const { return extent_x; }
    void SetExtentY(uint32_t extent_y) { this->extent_y = extent_y; }
    uint32_t GetExtentY() const { return extent_y; }
    void SetExtentZ(uint32_t extent_z) { this->extent_z = extent_z; }
    uint32_t GetExtentZ() const { return extent_z; }
    void SetLayers(uint32_t layers) { this->layers = layers; }
    uint32_t GetLayers() const { return layers; }
    void SetMipmaps(uint32_t mipmaps) { this->mipmaps = mipmaps; }
    uint32_t GetMipmaps() const { return mipmaps; }

  public:
    void SetType(Type type) { this->type = type; }
    Type GetType() const { return type; }
    void SetHint(Hint hint) { this->hint = hint; }
    Hint GetHint() const { return hint; }

  public:
    void SetUsage(Usage usage) { this->usage = usage; }
    Usage GetUsage() const { return usage; }

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
    virtual const std::shared_ptr<View>& CreateView(const std::string& name) = 0;
    virtual const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, View::Range bytes = {}) = 0;
    virtual const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, View::Bind bind, View::Range layers = {}, View::Range mipmaps = {}) = 0;
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
    Resource(const std::string& name, Device& device);
    virtual ~Resource();
  };
}
