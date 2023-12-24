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
    using SBOffset = std::optional<std::array<uint32_t, 4>>;
    using PushData = std::optional<std::array<uint8_t, 128>>;

  public:
    struct Graphic
    {
      uint32_t idx_count{ 0 };
      uint32_t ins_count{ 0 };
      uint32_t idx_offset{ 0 };
      uint32_t vtx_offset{ 0 };
      uint32_t ins_offset{ 0 };
    };

    struct Compute
    {
      uint32_t grid_x{ 0 };
      uint32_t grid_y{ 0 };
      uint32_t grid_z{ 0 };
    };

  public:
    struct Subset
    {
      std::shared_ptr<View> arg_view;
      View::Range vtx_range = { 0u, 0u };
      View::Range idx_range = { 0u, 0u };
      View::Range ins_range = { 0u, 0u };
      uint32_t grid_x = 0u;
      uint32_t grid_y = 0u;
      uint32_t grid_z = 0u;
      Mesh::SBOffset sb_offset = std::nullopt;
      Mesh::PushData push_data = std::nullopt;
    };

  //protected:
  //  bool enabled{ false };

  protected:
    Batch& batch;

  //public:
  //  void SetEnabled(bool enabled) { this->enabled = enabled; }
  //  bool GetEnabled() const { return enabled; }

  protected:
    std::vector<std::shared_ptr<View>> va_views; //vertex arrays
    std::vector<std::shared_ptr<View>> ia_views; //index_arrays

  protected:
    std::vector<Subset> subsets;

  public:
    Batch& GetBatch() { return batch; }

  public:
    std::pair<const Subset*, uint32_t> GetSubsets() const { return { subsets.data(), uint32_t(subsets.size()) }; }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Mesh(const std::string& name,
      Batch& batch,
      const std::pair<const Mesh::Subset*, uint32_t>& subsets,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views = {},
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views = {}
    );
    virtual ~Mesh();
  };

  typedef std::shared_ptr<Mesh> SPtrMesh;
  typedef std::weak_ptr<Mesh> WPtrMesh;
  typedef std::unique_ptr<Mesh> UPtrMesh;
}