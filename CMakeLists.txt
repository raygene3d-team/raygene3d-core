find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

set(NAME raygene3d)

set(CORE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/core)
set(CORE_SOURCE
  ${CORE_DIR}/batch.h
  ${CORE_DIR}/batch.cpp
  ${CORE_DIR}/config.h
  ${CORE_DIR}/config.cpp
  ${CORE_DIR}/device.h
  ${CORE_DIR}/device.cpp
  ${CORE_DIR}/pass.h
  ${CORE_DIR}/pass.cpp
  ${CORE_DIR}/resource.h
  ${CORE_DIR}/resource.cpp
  ${CORE_DIR}/view.h
  ${CORE_DIR}/view.cpp
)
set(CORE_D11_DIR ${CORE_DIR}/d11)
set(CORE_D11_SOURCE
  ${CORE_D11_DIR}/d11_batch.h
  ${CORE_D11_DIR}/d11_batch.cpp
  ${CORE_D11_DIR}/d11_config.h
  ${CORE_D11_DIR}/d11_config.cpp
  ${CORE_D11_DIR}/d11_device.h
  ${CORE_D11_DIR}/d11_device.cpp
  ${CORE_D11_DIR}/d11_pass.h
  ${CORE_D11_DIR}/d11_pass.cpp
  ${CORE_D11_DIR}/d11_resource.h
  ${CORE_D11_DIR}/d11_resource.cpp
  ${CORE_D11_DIR}/d11_view.h
  ${CORE_D11_DIR}/d11_view.cpp
)
set(CORE_D12_DIR ${CORE_DIR}/d12)
set(CORE_D12_SOURCE
  ${CORE_D12_DIR}/d12_batch.h
  ${CORE_D12_DIR}/d12_batch.cpp
  ${CORE_D12_DIR}/d12_config.h
  ${CORE_D12_DIR}/d12_config.cpp
  ${CORE_D12_DIR}/d12_device.h
  ${CORE_D12_DIR}/d12_device.cpp
  ${CORE_D12_DIR}/d12_pass.h
  ${CORE_D12_DIR}/d12_pass.cpp
  ${CORE_D12_DIR}/d12_resource.h
  ${CORE_D12_DIR}/d12_resource.cpp
  ${CORE_D12_DIR}/d12_view.h
  ${CORE_D12_DIR}/d12_view.cpp
)
set(CORE_MTL_DIR ${CORE_DIR}/mtl)
set(CORE_MTL_SOURCE
	${CORE_MTL_DIR}/mtl_batch.h
	${CORE_MTL_DIR}/mtl_batch.mm
	${CORE_MTL_DIR}/mtl_config.h
	${CORE_MTL_DIR}/mtl_config.mm
	${CORE_MTL_DIR}/mtl_device.h
	${CORE_MTL_DIR}/mtl_device.mm
	${CORE_MTL_DIR}/mtl_dxil.h
	${CORE_MTL_DIR}/mtl_dxil.mm
	${CORE_MTL_DIR}/mtl_pass.h
	${CORE_MTL_DIR}/mtl_pass.mm
	${CORE_MTL_DIR}/mtl_resource.h
	${CORE_MTL_DIR}/mtl_resource.mm
	${CORE_MTL_DIR}/mtl_view.h
	${CORE_MTL_DIR}/mtl_view.mm
)
set(CORE_VLK_DIR ${CORE_DIR}/vlk)
set(CORE_VLK_SOURCE
  ${CORE_VLK_DIR}/vlk_batch.h
  ${CORE_VLK_DIR}/vlk_batch.cpp
  ${CORE_VLK_DIR}/vlk_config.h
  ${CORE_VLK_DIR}/vlk_config.cpp
  ${CORE_VLK_DIR}/vlk_device.h
  ${CORE_VLK_DIR}/vlk_device.cpp
  ${CORE_VLK_DIR}/vlk_pass.h
  ${CORE_VLK_DIR}/vlk_pass.cpp
  ${CORE_VLK_DIR}/vlk_resource.h
  ${CORE_VLK_DIR}/vlk_resource.cpp
  ${CORE_VLK_DIR}/vlk_view.h
  ${CORE_VLK_DIR}/vlk_view.cpp
)

IF(WIN32)
set(SOURCE core.h core.cpp
	${CORE_SOURCE}
	${CORE_D11_SOURCE}
	${CORE_D12_SOURCE}
	${CORE_VLK_SOURCE}
)
ELSEIF(APPLE)
set(SOURCE core.h core.cpp
	${CORE_SOURCE}
	${CORE_MTL_SOURCE}
	${CORE_VLK_SOURCE}
)
ELSE(WIN32)
set(SOURCE core.h core.cpp
	${CORE_SOURCE}
	${CORE_VLK_SOURCE}
)
ENDIF(WIN32)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${SOURCE})

add_library(${NAME}-core STATIC ${SOURCE})

target_include_directories(${NAME}-core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/../3rdparty
)

target_link_libraries(${NAME}-core PUBLIC
  Vulkan::Vulkan
  Vulkan::shaderc_combined
)

# Metal backend (Apple only): Objective-C++ (.mm) sources and SPIRV-Cross MSL translation.
if(APPLE)
  target_include_directories(${NAME}-core PRIVATE /usr/local/include)
  target_link_libraries(${NAME}-core PRIVATE
    /usr/local/lib/libspirv-cross-msl.a
    /usr/local/lib/libspirv-cross-glsl.a
    /usr/local/lib/libspirv-cross-core.a
    "$<LINK_LIBRARY:FRAMEWORK,Metal>"
    "$<LINK_LIBRARY:FRAMEWORK,QuartzCore>"
    "$<LINK_LIBRARY:FRAMEWORK,Foundation>"
  )

  # Apple Metal Shader Converter: DXIL -> Metal library (libmetalirconverter).
  # Install from developer.apple.com/metal/shader-converter, or set METAL_IRCONVERTER_ROOT.
  option(USE_METAL_IRCONVERTER "Link libmetalirconverter (DXIL -> metallib)" ON)
  if(USE_METAL_IRCONVERTER)
    set(METAL_IRCONVERTER_ROOT "/usr/local" CACHE PATH "Metal Shader Converter install prefix")

    # Header lives at <include>/metal_irconverter/metal_irconverter.h
    find_path(METAL_IRCONVERTER_INCLUDE_DIR metal_irconverter/metal_irconverter.h
      HINTS ${METAL_IRCONVERTER_ROOT}/include /usr/local/include
    )
    find_library(METAL_IRCONVERTER_LIBRARY metalirconverter
      HINTS ${METAL_IRCONVERTER_ROOT}/lib /usr/local/lib
    )

    if(NOT METAL_IRCONVERTER_INCLUDE_DIR OR NOT METAL_IRCONVERTER_LIBRARY)
      message(FATAL_ERROR "libmetalirconverter not found. Install Metal Shader Converter, set METAL_IRCONVERTER_ROOT, or configure with -DUSE_METAL_IRCONVERTER=OFF.")
    endif()

    target_include_directories(${NAME}-core PRIVATE ${METAL_IRCONVERTER_INCLUDE_DIR})
    target_link_libraries(${NAME}-core PRIVATE ${METAL_IRCONVERTER_LIBRARY})
    target_compile_definitions(${NAME}-core PRIVATE RAYGENE_HAS_METAL_IRCONVERTER=1)
  endif()

  # MSL source path for vertex/fragment/compute shaders.
  #   OFF (default): shaderc (glslang) HLSL/GLSL -> SPIR-V, the previous approach.
  #   ON:            in-process DXC -spirv HLSL -> SPIR-V (GLSL is not supported in this mode).
  # Mesh and ray tracing always use Metal Shader Converter (DXIL -> metallib) when enabled.
  option(USE_DXC_SPIRV_MSL "Compile MSL HLSL via DXC -spirv instead of shaderc" OFF)
  if(USE_DXC_SPIRV_MSL)
    target_compile_definitions(${NAME}-core PRIVATE RAYGENE_MSL_USE_DXC_SPIRV=1)
  else()
    target_compile_definitions(${NAME}-core PRIVATE RAYGENE_MSL_LEGACY_SHADERC=1)
  endif()

  # DXC (in-process HLSL -> DXIL). Header and dylib come from the DXC install under /usr/local.
  find_path(DXC_INCLUDE_DIR dxc/dxcapi.h HINTS /usr/local/include)
  find_library(DXC_LIBRARY dxcompiler HINTS /usr/local/lib)
  if(NOT DXC_INCLUDE_DIR OR NOT DXC_LIBRARY)
    message(FATAL_ERROR "dxcompiler not found. Install DXC under /usr/local (dxc/dxcapi.h and libdxcompiler.dylib).")
  endif()
  target_include_directories(${NAME}-core PRIVATE ${DXC_INCLUDE_DIR})
  target_link_libraries(${NAME}-core PRIVATE ${DXC_LIBRARY})
endif()
