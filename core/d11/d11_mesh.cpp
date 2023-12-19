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
#include "d11_mesh.h"
#include "d11_batch.h"
#include "d11_technique.h"
#include "d11_pass.h"
#include "d11_device.h"

#pragma comment (lib, "dxgi.lib")
#pragma comment (lib, "d3d11.lib")

namespace RayGene3D
{
  void D11Mesh::Initialize()
  {
  }

  void D11Mesh::Use()
  {
  }

  void D11Mesh::Discard()
  {
  }

  D11Mesh::D11Mesh(const std::string& name,
    Batch& batch,
    uint32_t va_count,
    uint32_t va_offset,
    uint32_t ia_count,
    uint32_t ia_offset)
    : Mesh(name, batch, va_count, va_offset, ia_count, ia_offset)
  {
    D11Mesh::Initialize();
  }

  D11Mesh::~D11Mesh()
  {
    D11Mesh::Discard();
  }
}