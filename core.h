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
#include "core/device.h"

namespace RayGene3D
{
  class Core : public Usable
  {
  public:

    enum DeviceType
    {
      DEVICE_UNKNOWN = 0,
      DEVICE_D11 = 1,
      DEVICE_VLK = 2,
    };

  protected:
    DeviceType type;

  protected:
    std::unique_ptr<Device> device;

  protected:
    std::list<std::weak_ptr<View>> views;

  public:
    void Initialize() override;
    void Use() override;
    void Discard() override;

  public:
    const std::unique_ptr<Device>& GetDevice() { return device; }

  public:
    void AddView(const std::shared_ptr<View>& view) { views.push_back(view); }
    void VisitView(std::function<void(const std::shared_ptr<View>&)> visitor) { for (const auto& view : views) visitor(view.lock()); }
    //void RemoveView(const std::shared_ptr<View>& view) { views.remove(view); }

  public:
    Core(DeviceType type);
    virtual ~Core();
  };
}