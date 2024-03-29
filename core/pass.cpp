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


#include "pass.h"

namespace RayGene3D
{
  Pass::Pass(const std::string& name,
    Device& device,
    Pass::Type type,
    const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments,
    const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments,
    const View::Range& ins_or_grid_x,
    const View::Range& vtx_or_grid_y)
    : Usable(name)
    , device(device)
    , type(type)
    , rt_attachments(rt_attachments.first, rt_attachments.first + rt_attachments.second)
    , ds_attachments(ds_attachments.first, ds_attachments.first + ds_attachments.second)
    , ins_or_grid_x(ins_or_grid_x)
    , vtx_or_grid_y(vtx_or_grid_y)
  {
  }
  
  Pass::~Pass()
  {
  }
}