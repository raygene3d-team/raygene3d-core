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


#include "technique.h"

namespace RayGene3D
{
  Technique::Technique(const std::string& name,
    Pass& pass,
    const std::string& source,
    Technique::Compilation compilation, 
    const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
    const Technique::IAState& ia_state,
    const Technique::RCState& rc_state,
    const Technique::DSState& ds_state,
    const Technique::OMState& om_state)
    : Usable(name)
    , pass(pass)
    , source(source)
    , compilation(compilation)
    , defines(defines.first, defines.first + defines.second)
    , ia_state(ia_state)
    , rc_state(rc_state)
    , ds_state(ds_state)
    , om_state(om_state)
  {
  }

  Technique::~Technique()
  {
  }
}