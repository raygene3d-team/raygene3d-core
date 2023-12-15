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
#include "mesh.h"

namespace RayGene3D
{
  class Technique;

  class Batch : public Usable //TODO Rename into Batch
  {
  protected:
    Technique& technique;

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

  public:
    struct Sampler
    {
      enum Filtering
      {
        FILTERING_UNKNOWN = 0,
        FILTERING_NEAREST = 1,
        FILTERING_LINEAR = 2,
        FILTERING_ANISOTROPIC = 3,
      };

      enum Addressing
      {
        ADDRESSING_UNKNOWN = 0,
        ADDRESSING_REPEAT = 1,
        ADDRESSING_MIRROR = 2,
        ADDRESSING_CLAMP = 3,
        ADDRESSING_BORDER = 4,
      };

      enum Comparison
      {
        COMPARION_UNKNOWN = 0,
        COMPARISON_NEVER = 1,
        COMPARISON_LESS = 2,
        COMPARISON_EQUAL = 3,
        COMPARISON_LESS_EQUAL = 4,
        COMPARISON_GREATER = 5,
        COMPARISON_NOT_EQUAL = 6,
        COMPARISON_GREATER_EQUAL = 7,
        COMPARISON_ALWAYS = 8,
      };

      Filtering filtering{ FILTERING_LINEAR };
      int anisotropy{ 16 };
      Addressing addressing{ ADDRESSING_CLAMP };
      Comparison comparison{ COMPARISON_NEVER };
      float color[4]{ 0.0f, 0.0f, 0.0f, 0.0f };
      float min_lod{-FLT_MAX };
      float max_lod{ FLT_MAX };
      float bias_lod{ 0.0f };
    };

  public:
    struct RTXEntity
    {
      float transform[12];

      std::shared_ptr<View> va_view;
      uint32_t va_offset;
      uint32_t va_count;

      std::shared_ptr<View> ia_view;
      uint32_t ia_offset;
      uint32_t ia_count;
    };

  protected:
    std::vector<std::shared_ptr<View>> ub_views; //uniform buffers
    std::vector<std::shared_ptr<View>> sb_views; //shifted buffers

  protected:
    std::vector<std::shared_ptr<View>> ri_views; //read-only images
    std::vector<std::shared_ptr<View>> wi_views; //read-write images

  protected:
    std::vector<std::shared_ptr<View>> rb_views; //read-only buffers
    std::vector<std::shared_ptr<View>> wb_views; //read-write buffers

  protected:
    std::vector<Sampler> samplers;

  protected:
    std::shared_ptr<View> aa_view;

  protected:
    uint32_t grid_x{ 0 };
    uint32_t grid_y{ 0 };
    uint32_t grid_z{ 0 };

  protected:
    std::vector<RTXEntity> rtx_entities;

  protected:
    std::list<std::shared_ptr<Mesh>> meshes;
    
  public:
    Technique& GetTechnique() { return technique; }

  public:
    virtual const std::shared_ptr<Mesh>& CreateMesh(const std::string& name,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
      uint32_t va_count,
      uint32_t va_offset,
      uint32_t ia_count,
      uint32_t ia_offset) = 0;
    virtual const std::shared_ptr<Mesh>& CreateMesh(const std::string& name,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views) = 0;
    void VisitMesh(std::function<void(const std::shared_ptr<Mesh>&)> visitor) { for (const auto& mesh : meshes) visitor(mesh); }
    void DestroyMesh(const std::shared_ptr<Mesh>& mesh) { meshes.remove(mesh); }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Batch(const std::string& name,
      Technique& technique,
      const std::pair<const Batch::Sampler*, uint32_t>& samplers,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views,
      const std::shared_ptr<View>& aa_view);
    Batch(const std::string& name,
      Technique& technique,
      const std::pair<const Batch::Sampler*, uint32_t>& samplers,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
      const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views,
      uint32_t grid_x,
      uint32_t grid_y,
      uint32_t grid_z);
    virtual ~Batch();
  };

  typedef std::shared_ptr<Batch> SPtrBatch;
  typedef std::weak_ptr<Batch> WPtrBatch;
  typedef std::unique_ptr<Batch> UPtrBatch;
}