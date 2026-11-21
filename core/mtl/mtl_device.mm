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


#include "mtl_device.h"
#include "mtl_view.h"

#import <Metal/Metal.h>
#import <QuartzCore/CAMetalLayer.h>

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <string>
#include <vector>

namespace RayGene3D
{
  namespace
  {
    // Decodes an unsigned small float (UFloat11: 6 mantissa bits, UFloat10: 5 mantissa bits).
    float DecodeSmallFloat(uint32_t bits, uint32_t mantissa_bits)
    {
      const uint32_t exponent = (bits >> mantissa_bits) & 0x1Fu;
      const uint32_t mantissa = bits & ((1u << mantissa_bits) - 1u);
      const float scale = float(1u << mantissa_bits);
      if (exponent == 0u) return float(mantissa) / scale * std::ldexp(1.0f, -14);
      if (exponent == 31u) return mantissa ? 0.0f : 1.0f; // NaN -> 0, Inf -> 1 (clamped later)
      return (1.0f + float(mantissa) / scale) * std::ldexp(1.0f, int(exponent) - 15);
    }

    // Writes a 24-bit BMP from top-down RGB8 pixels (width * height * 3 bytes).
    bool WriteBMP(const std::string& path, uint32_t width, uint32_t height, const std::vector<uint8_t>& pixels)
    {
      const uint32_t row_size = (width * 3u + 3u) & ~3u;
      const uint32_t data_size = row_size * height;

      uint8_t header[54] = {};
      const auto put16 = [&](size_t offset, uint32_t value) { header[offset] = uint8_t(value); header[offset + 1] = uint8_t(value >> 8); };
      const auto put32 = [&](size_t offset, uint32_t value) { put16(offset, value & 0xFFFFu); put16(offset + 2, value >> 16); };
      header[0] = 'B';
      header[1] = 'M';
      put32(2, 54u + data_size);
      put32(10, 54u);
      put32(14, 40u);
      put32(18, width);
      put32(22, height);
      put16(26, 1u);
      put16(28, 24u);
      put32(34, data_size);

      const auto parent = std::filesystem::path(path).parent_path();
      if (!parent.empty()) std::filesystem::create_directories(parent);

      FILE* file = std::fopen(path.c_str(), "wb");
      if (!file) return false;

      std::fwrite(header, 1, sizeof(header), file);
      std::vector<uint8_t> row(row_size, 0u);
      for (uint32_t y = height; y-- > 0;)
      {
        for (uint32_t x = 0; x < width; ++x)
        {
          const uint8_t* px = &pixels[(size_t(y) * width + x) * 3];
          row[x * 3 + 0] = px[2];
          row[x * 3 + 1] = px[1];
          row[x * 3 + 2] = px[0];
        }
        std::fwrite(row.data(), 1, row.size(), file);
      }
      std::fclose(file);
      return true;
    }

    // Reads a 2D texture back to the CPU (blit to a shared buffer, then wait) and writes it as BMP.
    // Only 8-bit RGBA/BGRA and R11G11B10 are supported; R11G11B10 is gamma-encoded like render_3d_present.hlsl.
    void DumpTexture(id<MTLDevice> mtl_device, id<MTLCommandQueue> mtl_queue, id<MTLTexture> texture, const std::string& path)
    {
      const auto width = uint32_t(texture.width);
      const auto height = uint32_t(texture.height);
      const auto format = texture.pixelFormat;

      const bool is_bgra = format == MTLPixelFormatBGRA8Unorm;
      const bool is_rgba = format == MTLPixelFormatRGBA8Unorm || format == MTLPixelFormatRGBA8Unorm_sRGB;
      const bool is_r11g11b10 = format == MTLPixelFormatRG11B10Float;
      if (!is_bgra && !is_rgba && !is_r11g11b10)
      {
        BLAST_LOG("Dump skipped (unsupported pixel format %lu): %s", (unsigned long)format, path.c_str());
        return;
      }

      const size_t bytes_per_row = size_t(width) * 4u;
      // newBufferWithLength returns +1; released once the readback below is complete.
      id<MTLBuffer> staging = [mtl_device newBufferWithLength:bytes_per_row * height options:MTLResourceStorageModeShared];

      id<MTLCommandBuffer> mtl_command_buffer = [mtl_queue commandBuffer];
      id<MTLBlitCommandEncoder> blit = [mtl_command_buffer blitCommandEncoder];
      [blit copyFromTexture:texture
                sourceSlice:0
                sourceLevel:0
               sourceOrigin:MTLOriginMake(0, 0, 0)
                 sourceSize:MTLSizeMake(width, height, 1)
                   toBuffer:staging
          destinationOffset:0
     destinationBytesPerRow:bytes_per_row
   destinationBytesPerImage:bytes_per_row * height];
      [blit endEncoding];
      [mtl_command_buffer commit];
      [mtl_command_buffer waitUntilCompleted];

      const auto* src = static_cast<const uint8_t*>([staging contents]);
      std::vector<uint8_t> pixels(size_t(width) * height * 3u, 0u);
      size_t nonzero = 0;
      uint8_t max_value = 0;

      const auto encode = [](float value) -> uint8_t
      {
        if (!(value > 0.0f)) return 0;
        const float gamma = std::pow(std::min(value, 1.0f), 1.0f / 2.2f);
        return uint8_t(gamma * 255.0f + 0.5f);
      };

      for (size_t i = 0; i < size_t(width) * height; ++i)
      {
        const uint8_t* p = src + i * 4;
        uint8_t rgb[3] = {};
        if (is_bgra)
        {
          rgb[0] = p[2]; rgb[1] = p[1]; rgb[2] = p[0];
        }
        else if (is_rgba)
        {
          rgb[0] = p[0]; rgb[1] = p[1]; rgb[2] = p[2];
        }
        else
        {
          uint32_t packed = 0;
          std::memcpy(&packed, p, sizeof(packed));
          rgb[0] = encode(DecodeSmallFloat(packed & 0x7FFu, 6));
          rgb[1] = encode(DecodeSmallFloat((packed >> 11) & 0x7FFu, 6));
          rgb[2] = encode(DecodeSmallFloat((packed >> 22) & 0x3FFu, 5));
        }

        for (uint32_t c = 0; c < 3; ++c)
        {
          pixels[i * 3 + c] = rgb[c];
          max_value = std::max(max_value, rgb[c]);
        }
        if (rgb[0] | rgb[1] | rgb[2]) ++nonzero;
      }

      [staging release];
      const bool written = WriteBMP(path, width, height, pixels);
      BLAST_LOG("Dump [%s] %ux%u: nonzero pixels %zu of %zu, max channel %u -> %s%s",
        path.c_str(), width, height, nonzero, size_t(width) * height, unsigned(max_value),
        path.c_str(), written ? "" : " (WRITE FAILED)");
    }
  }

  void MTLDevice::Initialize()
  {
    // window is the CAMetalLayer created in main.cpp (Apple branch).
    layer = window;

    // MTLCreateSystemDefaultDevice and newCommandQueue return +1, so assignment takes ownership.
    device = MTLCreateSystemDefaultDevice();
    BLAST_ASSERT(device != nil);

    queue = [device newCommandQueue];
    BLAST_ASSERT(queue != nil);

    CAMetalLayer* metal_layer = (CAMetalLayer*)layer;
    metal_layer.device = device;
    metal_layer.pixelFormat = MTLPixelFormatBGRA8Unorm;
    metal_layer.framebufferOnly = NO;
    metal_layer.drawableSize = CGSizeMake(extent_x, extent_y);

    const std::string device_name = [[device name] UTF8String];
    const bool ray_tracing_supported = [device supportsRaytracing];
    const bool mesh_shader_supported = [device supportsFamily:MTLGPUFamilyMetal3];

    name = device_name + " (Metal API)";

    BLAST_LOG("Device is created on %s [RT:%s, MS:%s]",
      device_name.c_str(),
      ray_tracing_supported ? "On" : "Off",
      mesh_shader_supported ? "On" : "Off");

    // Debug gate, same switch as Vulkan (VK_LAYER_KHRONOS_validation) and D3D12 (EnableDebugLayer).
    // Metal has no in-code equivalent of those layers: API validation and shader validation are
    // enabled by the Xcode scheme (Diagnostics), not from code. Here we log the device state and
    // check that GPU frame capture (MTLCaptureManager) is available for this debug session.
    if (debug)
    {
      BLAST_LOG("Metal debug enabled on device: %s", [[device name] UTF8String]);
      const auto capture_supported = [[MTLCaptureManager sharedCaptureManager] supportsDestination:MTLCaptureDestinationDeveloperTools];
      BLAST_LOG("Metal GPU capture (developer tools) supported: %s", capture_supported ? "yes" : "no");
      BLAST_LOG("Metal API validation: enable in Xcode scheme (Run > Diagnostics > Metal API Validation)");
    }
  }

  void MTLDevice::Use()
  {
    CAMetalLayer* metal_layer = (CAMetalLayer*)layer;

    // Debug: RAYGENE_CAPTURE_FRAME=<n> records frame n as a GPU trace into ./capture/frame_<n>.gputrace.
    // Open the bundle in Xcode to inspect passes, resources and shaders.
    // When running outside Xcode, MTL_CAPTURE_ENABLED=1 must be set for GPU trace documents.
    static uint64_t frame_index = 0;
    const char* capture_env = std::getenv("RAYGENE_CAPTURE_FRAME");
    const bool capturing = capture_env && frame_index == std::strtoull(capture_env, nullptr, 10);
    if (capturing)
    {
      MTLCaptureManager* manager = [MTLCaptureManager sharedCaptureManager];
      NSString* path = [NSString stringWithFormat:@"./capture/frame_%llu.gputrace", (unsigned long long)frame_index];
      NSFileManager* file_manager = [NSFileManager defaultManager];
      [file_manager createDirectoryAtPath:@"./capture" withIntermediateDirectories:YES attributes:nil error:nil];

      if (![manager supportsDestination:MTLCaptureDestinationGPUTraceDocument])
      {
        BLAST_LOG("Metal GPU trace capture unsupported (set MTL_CAPTURE_ENABLED=1 outside Xcode)");
      }
      else if ([file_manager fileExistsAtPath:path])
      {
        BLAST_LOG("Metal GPU trace skipped, already exists: %s", [path UTF8String]);
      }
      else
      {
        MTLCaptureDescriptor* descriptor = [[MTLCaptureDescriptor alloc] init];
        descriptor.captureObject = queue;
        descriptor.destination = MTLCaptureDestinationGPUTraceDocument;
        descriptor.outputURL = [NSURL fileURLWithPath:path];

        NSError* error = nil;
        const BOOL started = [manager startCaptureWithDescriptor:descriptor error:&error];
        [descriptor release];
        if (!started)
        {
          BLAST_LOG("Metal GPU trace start failed: %s", [[error localizedDescription] UTF8String]);
        }
        else
        {
          BLAST_LOG("Metal GPU trace capturing frame %llu -> %s", (unsigned long long)frame_index, [path UTF8String]);
        }
      }
    }

    // One command buffer per frame: passes encode into it, then the screen is blitted to the drawable.
    // commandBuffer is autoreleased, so retain it for the duration of this frame.
    command_buffer = [[queue commandBuffer] retain];
    id<MTLCommandBuffer> mtl_command_buffer = command_buffer;

    for (auto& pass : passes)
    {
      pass->Use();
    }

    id<CAMetalDrawable> drawable = [metal_layer nextDrawable];
    if (drawable != nil && screen)
    {
      id<MTLTexture> src = reinterpret_cast<MTLResource*>(screen.get())->GetNative();
      id<MTLBlitCommandEncoder> blit = [mtl_command_buffer blitCommandEncoder];
      [blit copyFromTexture:src
                sourceSlice:0
                sourceLevel:0
               sourceOrigin:MTLOriginMake(0, 0, 0)
                 sourceSize:MTLSizeMake(extent_x, extent_y, 1)
                  toTexture:drawable.texture
           destinationSlice:0
           destinationLevel:0
          destinationOrigin:MTLOriginMake(0, 0, 0)];
      [blit endEncoding];

      [mtl_command_buffer presentDrawable:drawable];
    }

    [mtl_command_buffer commit];
    if ([MTLCaptureManager sharedCaptureManager].isCapturing)
    {
      [[MTLCaptureManager sharedCaptureManager] stopCapture];
    }
    // Debug: RAYGENE_DUMP_FRAME=<n> dumps all 2D textures and the backbuffer after frame n into ./dump/frame_<n>/.
    const char* dump_env = std::getenv("RAYGENE_DUMP_FRAME");
    if (dump_env && frame_index == std::strtoull(dump_env, nullptr, 10))
    {
      // Wait for this frame's GPU work to finish so the readback sees the completed frame.
      [mtl_command_buffer waitUntilCompleted];
      const auto dir = std::string("./dump/frame_") + std::to_string(frame_index) + "/";

      for (const auto& resource : resources)
      {
        if (resource->GetType() != Resource::TYPE_TEX2D) continue;
        id<MTLTexture> texture = reinterpret_cast<MTLResource*>(resource.get())->GetNative();
        if (texture == nil) continue;
        DumpTexture(device, queue, texture, dir + resource->GetName() + ".bmp");
      }

      if (screen)
      {
        id<MTLTexture> texture = reinterpret_cast<MTLResource*>(screen.get())->GetNative();
        DumpTexture(device, queue, texture, dir + "backbuffer.bmp");
      }
    }
    // The frame's command buffer is retained above; release it now that the frame is submitted.
    [command_buffer release];
    command_buffer = nil;
    ++frame_index;
  }

  void MTLDevice::Discard()
  {
    passes.clear();
    resources.clear();
    screen.reset();

    [command_buffer release];
    command_buffer = nil;
    [queue release];
    queue = nil;
    [device release];
    device = nil;

    layer = nullptr;
  }

  MTLDevice::MTLDevice(const std::string& name)
    : Device(name)
  {
  }

  MTLDevice::~MTLDevice()
  {
    MTLDevice::Discard();
  }
}
