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


#include "subset.h"

namespace RayGene3D
{
  Mesh::Mesh(const std::string& name,
    Batch& batch,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
    uint32_t va_count, uint32_t va_offset,
    uint32_t ia_count, uint32_t ia_offset,
    const std::pair<const uint32_t*, uint32_t>& sb_offsets)
    : Usable(name)
    , batch(batch)
    , va_views(va_views.first, va_views.first + va_views.second)
    , ia_views(ia_views.first, ia_views.first + ia_views.second)
    , va_count(va_count)
    , va_offset(va_offset)
    , ia_count(ia_count)
    , ia_offset(ia_offset)
    , sb_offsets(sb_offsets.first, sb_offsets.first + sb_offsets.second)
  {}

  Mesh::Mesh(const std::string& name,
    Batch& batch,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
    const std::shared_ptr<View>& aa_view,
    const std::pair<const uint32_t*, uint32_t>& sb_offsets)
    : Usable(name)
    , batch(batch)
    , va_views(va_views.first, va_views.first + va_views.second)
    , ia_views(ia_views.first, ia_views.first + ia_views.second)
    , aa_view(aa_view)
    , sb_offsets(sb_offsets.first, sb_offsets.first + sb_offsets.second)
  {}

  Mesh::Mesh(const std::string& name,
    Batch& batch,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
    uint32_t va_count, uint32_t va_offset,
    uint32_t ia_count, uint32_t ia_offset,
    const std::pair<const float* [16], uint32_t>& transforms)
    : Usable(name)
    , batch(batch)
    , va_views(va_views.first, va_views.first + va_views.second)
    , ia_views(ia_views.first, ia_views.first + ia_views.second)
    , va_count(va_count)
    , va_offset(va_offset)
    , ia_count(ia_count)
    , ia_offset(ia_offset)
    , transforms(transforms.first, transforms.first + transforms.second)
  {}

  Mesh::Mesh(const std::string& name,
    Batch& batch,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
    const std::shared_ptr<View>& aa_view,
    const std::pair<const float* [16], uint32_t>& transforms)
    : Usable(name)
    , batch(batch)
    , va_views(va_views.first, va_views.first + va_views.second)
    , ia_views(ia_views.first, ia_views.first + ia_views.second)
    , aa_view(aa_view)
    , transforms(transforms.first, transforms.first + transforms.second)
  {}
  
  Mesh::~Mesh()
  {
  }
}