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

#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <cstdarg>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <cfloat>

#include <functional>
#include <algorithm>
#include <memory>
#include <utility>
#include <atomic>
#include <thread>
#include <chrono>
#include <random>

#include <vector>
#include <list>
#include <array>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <map>
#include <set>
#include <queue>
#include <regex>

#include <variant>
#include <optional>



inline std::string ExtractName(const std::string& path)
{
  const std::string::size_type slash_id = path.rfind("/");
  const std::string::size_type dot_id = path.rfind(".");
  const std::string::size_type offset = slash_id != std::string::npos ? slash_id + 1 : 0;
  const std::string::size_type size = dot_id != std::string::npos ? dot_id - offset : std::string::npos;
  return path.substr(offset, size);
}

inline std::string ExtractExtension(const std::string& path)
{
  const std::string::size_type dot_id = path.rfind(".");
  if (dot_id == std::string::npos)
  {
    return std::string();
  }
  return path.substr(dot_id + 1);
}


#define BLAST_ASSERT(X) \
{ \
  if (X == false) \
  { \
    std::string info( "assertion failed @" ); \
	  info += std::strrchr(__FILE__, '\\') ? std::strrchr(__FILE__, '\\') + 1 : __FILE__ ; \
	  info += "," + std::to_string(__LINE__) + '\n'; \
	  throw std::runtime_error(info); \
  } \
}  



namespace RayGene3D
{
  static void WriteLog(const char* format, ...)
  {
    std::time_t t = std::time(nullptr);
    std::tm* tm = localtime(&t);

    va_list ap;
    va_start(ap, format);
    char message[1024];
    vsnprintf(message, sizeof(message), format, ap);
    printf("[%2.2i/%2.2i/%4.4i %2.2i:%2.2i:%2.2i] >> %s\n", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec, message);
    FILE* file = fopen("blast3d.log", "a+");
    if (file)
    {
      fprintf(file, "[%2.2i/%2.2i/%4.4i %2.2i:%2.2i:%2.2i] >> %s\n", tm->tm_mday, tm->tm_mon + 1, tm->tm_year + 1900, tm->tm_hour, tm->tm_min, tm->tm_sec, message);
      fclose(file);
    }
    va_end(ap);
  }
}

#define BLAST_LOG(X,...) \
{ \
  RayGene3D::WriteLog(X, ##__VA_ARGS__); \
} 

namespace RayGene3D
{
  enum Format
  {
    FORMAT_UNKNOWN = 0,
    FORMAT_R32G32B32A32_FLOAT = 1,
    FORMAT_R32G32B32A32_UINT = 2,
    FORMAT_R32G32B32A32_SINT = 3,
    FORMAT_R32G32B32_FLOAT = 4,
    FORMAT_R32G32B32_UINT = 5,
    FORMAT_R32G32B32_SINT = 6,
    FORMAT_R16G16B16A16_FLOAT = 7,
    FORMAT_R16G16B16A16_UNORM = 8,
    FORMAT_R16G16B16A16_UINT = 9,
    FORMAT_R16G16B16A16_SNORM = 10,
    FORMAT_R16G16B16A16_SINT = 11,
    FORMAT_R32G32_FLOAT = 12,
    FORMAT_R32G32_UINT = 13,
    FORMAT_R32G32_SINT = 14,
    FORMAT_D32_FLOAT_S8X24_UINT = 15,
    FORMAT_R10G10B10A2_UNORM = 16,
    FORMAT_R10G10B10A2_UINT = 17,
    FORMAT_R11G11B10_FLOAT = 18,
    FORMAT_R8G8B8A8_UNORM = 19,
    FORMAT_R8G8B8A8_SRGB = 20,
    FORMAT_R8G8B8A8_UINT = 31,
    FORMAT_R8G8B8A8_SNORM = 32,
    FORMAT_R8G8B8A8_SINT = 33,
    FORMAT_R16G16_FLOAT = 34,
    FORMAT_R16G16_UNORM = 35,
    FORMAT_R16G16_UINT = 36,
    FORMAT_R16G16_SNORM = 37,
    FORMAT_R16G16_SINT = 38,
    FORMAT_D32_FLOAT = 39,
    FORMAT_R32_FLOAT = 40,
    FORMAT_R32_UINT = 41,
    FORMAT_R32_SINT = 42,
    FORMAT_D24_UNORM_S8_UINT = 43,
    FORMAT_R8G8_UNORM = 44,
    FORMAT_R8G8_UINT = 45,
    FORMAT_R8G8_SNORM = 46,
    FORMAT_R8G8_SINT = 47,
    FORMAT_R16_FLOAT = 48,
    FORMAT_D16_UNORM = 49,
    FORMAT_R16_UNORM = 50,
    FORMAT_R16_UINT = 51,
    FORMAT_R16_SNORM = 52,
    FORMAT_R16_SINT = 53,
    FORMAT_R8_UNORM = 54,
    FORMAT_R8_UINT = 55,
    FORMAT_R8_SNORM = 56,
    FORMAT_R8_SINT = 57,
    FORMAT_R9G9B9E5_SHAREDEXP = 58,
    FORMAT_R8G8_B8G8_UNORM = 59,
    FORMAT_G8R8_G8B8_UNORM = 60,
    FORMAT_BC1_UNORM = 61,
    FORMAT_BC1_SRGB = 62,
    FORMAT_BC2_UNORM = 63,
    FORMAT_BC2_SRGB = 64,
    FORMAT_BC3_UNORM = 65,
    FORMAT_BC3_SRGB = 66,
    FORMAT_BC4_UNORM = 67,
    FORMAT_BC4_SNORM = 68,
    FORMAT_BC5_UNORM = 69,
    FORMAT_BC5_SNORM = 70,
    FORMAT_B5G6R5_UNORM = 71,
    FORMAT_B5G5R5A1_UNORM = 72,
    FORMAT_B8G8R8A8_UNORM = 73,
    FORMAT_B8G8R8X8_UNORM = 74,
    FORMAT_B8G8R8A8_SRGB = 75,
    FORMAT_B8G8R8X8_SRGB = 76,
    FORMAT_BC6H_UF16 = 77,
    FORMAT_BC6H_SF16 = 78,
    FORMAT_BC7_UNORM = 79,
    FORMAT_BC7_SRGB = 80,
    FORMAT_FORCE_UINT = 0xffffffff
  };

  inline uint32_t BitCount(Format format)
  {
    switch (format)
    {
    default: return 0;
    case FORMAT_R32G32B32A32_FLOAT: return 128;
    case FORMAT_R32G32B32A32_UINT: return 128;
    case FORMAT_R32G32B32A32_SINT: return 128;
    case FORMAT_R32G32B32_FLOAT: return 96;
    case FORMAT_R32G32B32_UINT: return 96;
    case FORMAT_R32G32B32_SINT: return 96;
    case FORMAT_R16G16B16A16_FLOAT: return 64;
    case FORMAT_R16G16B16A16_UNORM: return 64;
    case FORMAT_R16G16B16A16_UINT: return 64;
    case FORMAT_R16G16B16A16_SNORM: return 64;
    case FORMAT_R16G16B16A16_SINT: return 64;
    case FORMAT_R32G32_FLOAT: return 64;
    case FORMAT_R32G32_UINT: return 64;
    case FORMAT_R32G32_SINT: return 64;
    case FORMAT_D32_FLOAT_S8X24_UINT: return 64;
    case FORMAT_R10G10B10A2_UNORM: return 32;
    case FORMAT_R10G10B10A2_UINT: return 32;
    case FORMAT_R11G11B10_FLOAT: return 32;
    case FORMAT_R8G8B8A8_UNORM: return 32;
    case FORMAT_R8G8B8A8_SRGB: return 32;
    case FORMAT_R8G8B8A8_UINT: return 32;
    case FORMAT_R8G8B8A8_SNORM: return 32;
    case FORMAT_R8G8B8A8_SINT: return 32;
    case FORMAT_R16G16_FLOAT: return 32;
    case FORMAT_R16G16_UNORM: return 32;
    case FORMAT_R16G16_UINT: return 32;
    case FORMAT_R16G16_SNORM: return 32;
    case FORMAT_R16G16_SINT: return 32;
    case FORMAT_D32_FLOAT: return 32;
    case FORMAT_R32_FLOAT: return 32;
    case FORMAT_R32_UINT: return 32;
    case FORMAT_R32_SINT: return 32;
    case FORMAT_D24_UNORM_S8_UINT: return 32;
    case FORMAT_R8G8_UNORM: return 16;
    case FORMAT_R8G8_UINT: return 16;
    case FORMAT_R8G8_SNORM: return 16;
    case FORMAT_R8G8_SINT: return 16;
    case FORMAT_R16_FLOAT: return 16;
    case FORMAT_D16_UNORM: return 16;
    case FORMAT_R16_UNORM: return 16;
    case FORMAT_R16_UINT: return 16;
    case FORMAT_R16_SNORM: return 16;
    case FORMAT_R16_SINT: return 16;
    case FORMAT_R8_UNORM: return 8;
    case FORMAT_R8_UINT: return 8;
    case FORMAT_R8_SNORM: return 8;
    case FORMAT_R8_SINT: return 8;
    case FORMAT_R9G9B9E5_SHAREDEXP: return 32;
    case FORMAT_R8G8_B8G8_UNORM: return 32;
    case FORMAT_G8R8_G8B8_UNORM: return 32;
    case FORMAT_BC1_UNORM: return 4;
    case FORMAT_BC1_SRGB: return 4;
    case FORMAT_BC2_UNORM: return 8;
    case FORMAT_BC2_SRGB: return 8;
    case FORMAT_BC3_UNORM: return 8;
    case FORMAT_BC3_SRGB: return 8;
    case FORMAT_BC4_UNORM: return 4;
    case FORMAT_BC4_SNORM: return 4;
    case FORMAT_BC5_UNORM: return 8;
    case FORMAT_BC5_SNORM: return 8;
    case FORMAT_B5G6R5_UNORM: return 16;
    case FORMAT_B5G5R5A1_UNORM: return 16;
    case FORMAT_B8G8R8A8_UNORM: return 32;
    case FORMAT_B8G8R8X8_UNORM: return 32;
    case FORMAT_B8G8R8A8_SRGB: return 32;
    case FORMAT_B8G8R8X8_SRGB: return 32;
    case FORMAT_BC6H_UF16: return 8;
    case FORMAT_BC6H_SF16: return 8;
    case FORMAT_BC7_UNORM: return 8;
    case FORMAT_BC7_SRGB: return 8;
    }

    return 0;
  }

  class Usable
  {
  protected:
    std::string name;

  public:
    virtual void Initialize() = 0;
    virtual void Use() = 0;
    virtual void Discard() = 0;

  public:
    const std::string& GetName() const { return name; }

  public:
    Usable(const std::string& name) : name(name) {};
    virtual ~Usable() {}
  };
}

