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
#include "../pass.h"
#include "d11_technique.h"

#include <dxgi.h>
#include <d3d11_1.h>

namespace RayGene3D
{
  class D11Pass : public Pass
  {

  protected:
    //ID3D11DeviceContext* context{ nullptr };
    //ID3D11CommandList* command_list{ nullptr };

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    const std::shared_ptr<Technique>& CreateTechnique(const std::string& name,
      const std::string& source,
      Technique::Compilation compilation,
      const std::pair<const std::pair<std::string, std::string>*, uint32_t>& defines,
      const Technique::IAState& ia_state,
      const Technique::RCState& rc_state,
      const Technique::DSState& ds_state,
      const Technique::OMState& om_state) override
    {
      return effects.emplace_back(new D11Technique(name, *this, source, compilation, defines, ia_state, rc_state, ds_state, om_state));
    }

  public:
    D11Pass(const std::string& name,
      Device& device,
      Pass::Type type,
      const std::pair<const Pass::RTAttachment*, uint32_t>& rt_attachments,
      const std::pair<const Pass::DSAttachment*, uint32_t>& ds_attachments,
      const View::Range& ins_or_grid_x = View::Range{ 0, 0 },
      const View::Range& vtx_or_grid_y = View::Range{ 0, 0 });
    virtual ~D11Pass();
  };
}