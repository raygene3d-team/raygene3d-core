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
#include "../resource.h"
#include "vlk_view.h"

#ifdef __linux__
#define VK_USE_PLATFORM_XLIB_KHR
#elif _WIN32
#define VK_USE_PLATFORM_WIN32_KHR
#elif __OBJC__
#define VK_USE_PLATFORM_METAL_EXT
#endif
#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.h>

namespace RayGene3D
{
  class VLKResource : public Resource
  {
  protected:
    //VkDeviceSize size{ 0 };
    VkDeviceMemory memory{ nullptr };
    VkBuffer buffer{ nullptr };
    VkImage image{ nullptr };

  public:
    const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, const View::Range& bytes = {}) override
    {
      return views.emplace_back(new VLKView(name, *this, usage, bytes));
    }

    const std::shared_ptr<View>& CreateView(const std::string& name,
      Usage usage, View::Bind bind, const View::Range& mipmaps = {}, const View::Range& layers = {}) override
    {
      return views.emplace_back(new VLKView(name, *this, usage, bind, mipmaps, layers));
    }

  public:
    VkBuffer GetBuffer() const { return buffer; }
    VkImage GetImage() const { return image; }

  public:
    void Commit(uint32_t index) override;
    void Retrieve(uint32_t index) override;
    void Blit(const std::shared_ptr<Resource>& resource) override;

  public:
    void* Map() override;
    void Unmap() override;

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    VLKResource(const std::string& name,
      Device& device,
      const Resource::BufferDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    VLKResource(const std::string& name,
      Device& device,
      const Resource::Tex1DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    VLKResource(const std::string& name,
      Device& device,
      const Resource::Tex2DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    VLKResource(const std::string& name,
      Device& device, const Resource::Tex3DDesc& desc,
      Resource::Hint hint = Resource::HINT_UNKNOWN,
      const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops = {});
    virtual ~VLKResource();
  };
}
