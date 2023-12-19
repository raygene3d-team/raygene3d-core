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


#include "batch.h"

namespace RayGene3D
{
  Batch::Batch(const std::string& name,
    Technique& technique,
    const std::pair<const std::shared_ptr<View>*, uint32_t> ce_views,
    const std::pair<const Batch::Command*, uint32_t>& commands,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& va_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ia_views,
    const std::pair<const Batch::Sampler*, uint32_t>& samplers,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ub_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& sb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& ri_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wi_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& rb_views,
    const std::pair<const std::shared_ptr<View>*, uint32_t>& wb_views)
    : Usable(name)
    , technique(technique)
    , ce_views(ce_views.first, ce_views.first + ce_views.second)
    , commands(commands.first, commands.first + commands.second)
    , va_views(va_views.first, va_views.first + va_views.second)
    , ia_views(ia_views.first, ia_views.first + ia_views.second)
    , samplers(samplers.first, samplers.first + samplers.second)
    , ub_views(ub_views.first, ub_views.first + ub_views.second)
    , sb_views(sb_views.first, sb_views.first + sb_views.second)
    , ri_views(ri_views.first, ri_views.first + ri_views.second)
    , wi_views(wi_views.first, wi_views.first + wi_views.second)
    , rb_views(rb_views.first, rb_views.first + rb_views.second)
    , wb_views(wb_views.first, wb_views.first + wb_views.second)
  {
  }

  Batch::~Batch()
  {
  }
}