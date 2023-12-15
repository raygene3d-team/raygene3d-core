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
  class Batch;

  class Mesh : public Usable
  {
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

    std::vector<std::shared_ptr<View>> va_views;
    std::vector<std::shared_ptr<View>> ia_views;

    uint32_t va_count{ 0 };
    uint32_t va_offset{ 0 };
    uint32_t ia_count{ 0 };
    uint32_t ia_offset{ 0 };

    std::shared_ptr<View> aa_view;

    std::vector<float[16]> transforms;
    std::vector<uint32_t> sb_offsets;

  protected:
    bool enabled{ false };

  protected:
    Batch& batch;

  public:
    void SetEnabled(bool enabled) { this->enabled = enabled; }
    bool GetEnabled() const { return enabled; }

  public:
    Batch& GetBatch() { return batch; }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Mesh(const std::string& name,
      Batch& batch,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
      uint32_t va_count,
      uint32_t va_offset,
      uint32_t ia_count, 
      uint32_t ia_offset,
      const std::pair<const uint32_t*, uint32_t>& sb_offsets = {});
    Mesh(const std::string& name,
      Batch& batch,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
      const std::shared_ptr<View>& aa_view,
      const std::pair<const uint32_t*, uint32_t>& sb_offsets = {});
    Mesh(const std::string& name,
      Batch& batch,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
      uint32_t va_count,
      uint32_t va_offset,
      uint32_t ia_count,
      uint32_t ia_offset,
      const std::pair<const float* [16], uint32_t>& transforms);
    Mesh(const std::string& name,
      Batch& batch,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
      const std::shared_ptr<View>& aa_view,
      const std::pair<const float* [16], uint32_t>& transforms);
    virtual ~Mesh();
  };

  typedef std::shared_ptr<RayGene3D::Mesh> SPtrMesh;
  typedef std::weak_ptr<RayGene3D::Mesh> WPtrMesh;
  typedef std::unique_ptr<RayGene3D::Mesh> UPtrMesh;
}