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


#include "resource.h"

namespace RayGene3D
{
  Resource::Resource(const std::string& name, Device& device, const Resource::BufferDesc& desc,
    Resource::Hint hint, const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Usable(name)
    , device(device)
  {
    this->type = TYPE_BUFFER;
    this->usage = desc.usage;
    this->stride = desc.stride;
    this->count = desc.count;
    this->hint = hint;
    this->interops.assign(interops.first, interops.first + interops.second);
  }

  Resource::Resource(const std::string& name, Device& device, const Resource::Tex1DDesc& desc,
    Resource::Hint hint, const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Usable(name)
    , device(device)
  {
    this->type = TYPE_TEX1D;
    this->usage = desc.usage;
    this->stride = desc.mipmaps;
    this->count = desc.layers;
    this->format = desc.format;
    this->size_x = desc.size_x;
    this->hint = hint;
    this->interops.assign(interops.first, interops.first + interops.second);
  }

  Resource::Resource(const std::string& name, Device& device, const Resource::Tex2DDesc& desc,
    Resource::Hint hint, const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Usable(name)
    , device(device)
  {
    this->type = TYPE_TEX2D;
    this->usage = desc.usage;
    this->stride = desc.mipmaps;
    this->count = desc.layers;
    this->format = desc.format;
    this->size_x = desc.size_x;
    this->size_y = desc.size_y;
    this->hint = hint;
    this->interops.assign(interops.first, interops.first + interops.second);
  }

  Resource::Resource(const std::string& name, Device& device, const Resource::Tex3DDesc& desc,
    Resource::Hint hint, const std::pair<std::pair<const void*, uint32_t>*, uint32_t>& interops)
    : Usable(name)
    , device(device)
  {
    this->type = TYPE_TEX3D;
    this->usage = desc.usage;
    this->stride = desc.mipmaps;
    this->count = desc.layers;
    this->format = desc.format;
    this->size_x = desc.size_x;
    this->size_y = desc.size_y;
    this->size_z = desc.size_z;
    this->hint = hint;
    this->interops.assign(interops.first, interops.first + interops.second);
  }

  Resource::~Resource()
  {
  }
}