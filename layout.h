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
#include "../base.h"
#include "view.h"

namespace RayGene3D
{
  class Device;

  class Layout : public Usable //TODO Rename into Layout
  {
  protected:
    Device& device;

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

  protected:
    std::vector<Sampler> samplers;

  protected:
    std::vector<std::shared_ptr<View>> ub_views; //uniform buffers
    std::vector<std::shared_ptr<View>> sb_views; //shifted buffers

    std::vector<std::shared_ptr<View>> ri_views; //read-only images
    std::vector<std::shared_ptr<View>> wi_views; //read-write images
    std::vector<std::shared_ptr<View>> rb_views; //read-only buffers
    std::vector<std::shared_ptr<View>> wb_views; //read-write buffers

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
    std::vector<RTXEntity> rtx_entities;
    
  public:
    Device& GetDevice() { return device; }

  public:
    void UpdateSamplers(std::pair<const Sampler*, uint32_t> samplers) { this->samplers.assign(samplers.first, samplers.first + samplers.second); }

  public:
    void UpdateUBViews(std::pair<const std::shared_ptr<View>*, uint32_t> ub_views) { this->ub_views.assign(ub_views.first, ub_views.first + ub_views.second); }
    void UpdateSBViews(std::pair<const std::shared_ptr<View>*, uint32_t> sb_views) { this->sb_views.assign(sb_views.first, sb_views.first + sb_views.second); }

  public:
    void UpdateRIViews(std::pair<const std::shared_ptr<View>*, uint32_t> ri_views) { this->ri_views.assign(ri_views.first, ri_views.first + ri_views.second); }
    void UpdateWIViews(std::pair<const std::shared_ptr<View>*, uint32_t> wi_views) { this->wi_views.assign(wi_views.first, wi_views.first + wi_views.second); }
    void UpdateRBViews(std::pair<const std::shared_ptr<View>*, uint32_t> rb_views) { this->rb_views.assign(rb_views.first, rb_views.first + rb_views.second); }
    void UpdateWBViews(std::pair<const std::shared_ptr<View>*, uint32_t> wb_views) { this->wb_views.assign(wb_views.first, wb_views.first + wb_views.second); }

  public:
    void SetRTXEntityCount(uint32_t count) 
    { 
      rtx_entities.resize(count);
    }
    void SetRTXEntityTransform(uint32_t index, const float transform[12])
    {
      memcpy(rtx_entities[index].transform, transform, 12 * sizeof(float));
    }
    void SetRTXEntityVAView(uint32_t index, const std::shared_ptr<View>& va_view, uint32_t va_offset, uint32_t va_count)
    {
      rtx_entities[index].va_view = va_view;
      rtx_entities[index].va_offset = va_offset;
      rtx_entities[index].va_count = va_count;
    }
    void SetRTXEntityIAView(uint32_t index, const std::shared_ptr<View>& ia_view, uint32_t ia_offset, uint32_t ia_count)
    {
      rtx_entities[index].ia_view = ia_view;
      rtx_entities[index].ia_offset = ia_offset;
      rtx_entities[index].ia_count = ia_count;
    }

  public:
    void Initialize() override = 0;
    void Use() override = 0;
    void Discard() override = 0;

  public:
    Layout(const std::string& name, Device& device);
    virtual ~Layout();
  };
}