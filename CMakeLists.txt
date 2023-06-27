find_package(Vulkan REQUIRED)

set(NAME raygene3d)

set(CORE_DIR ${CMAKE_SOURCE_DIR}/${NAME}-core/core)
set(CORE_SOURCE
	${CORE_DIR}/base.h
	${CORE_DIR}/base.cpp
	${CORE_DIR}/config.h
	${CORE_DIR}/config.cpp
	${CORE_DIR}/device.h
	${CORE_DIR}/device.cpp
	${CORE_DIR}/layout.h
	${CORE_DIR}/layout.cpp
	${CORE_DIR}/pass.h
	${CORE_DIR}/pass.cpp
	${CORE_DIR}/resource.h
	${CORE_DIR}/resource.cpp
	${CORE_DIR}/view.h
	${CORE_DIR}/view.cpp
)
set(CORE_D11_DIR ${CORE_DIR}/d11)
set(CORE_D11_SOURCE
	${CORE_D11_DIR}/d11_config.h
	${CORE_D11_DIR}/d11_config.cpp
	${CORE_D11_DIR}/d11_device.h
	${CORE_D11_DIR}/d11_device.cpp
	${CORE_D11_DIR}/d11_layout.h
	${CORE_D11_DIR}/d11_layout.cpp
	${CORE_D11_DIR}/d11_pass.h
	${CORE_D11_DIR}/d11_pass.cpp
	${CORE_D11_DIR}/d11_resource.h
	${CORE_D11_DIR}/d11_resource.cpp
	${CORE_D11_DIR}/d11_view.h
	${CORE_D11_DIR}/d11_view.cpp
)
set(CORE_VLK_DIR ${CORE_DIR}/vlk)
set(CORE_VLK_SOURCE
	${CORE_VLK_DIR}/vlk_config.h
	${CORE_VLK_DIR}/vlk_config.cpp
	${CORE_VLK_DIR}/vlk_device.h
	${CORE_VLK_DIR}/vlk_device.cpp
	${CORE_VLK_DIR}/vlk_layout.h
	${CORE_VLK_DIR}/vlk_layout.cpp
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

target_link_libraries(${NAME}-core PRIVATE
	Vulkan::Vulkan
)
	
target_include_directories(${NAME}-core PUBLIC
	${CMAKE_SOURCE_DIR}/3rdparty
)

set(GLSLANG_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/glslang/lib/win64)
set(SPIRV_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/SPIRV/lib/win64)
set(SPIRVTOOLS_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/spirv-tools/lib/win64)

if(UNIX)
	if(APPLE)
		set(GLSLANG_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/glslang/lib/macos)
		set(SPIRV_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/SPIRV/lib/macos)
		set(SPIRVTOOLS_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/spirv-tools/lib/macos)
	else(APPLE)
		set(GLSLANG_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/glslang/lib/linux)
		set(SPIRV_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/SPIRV/lib/linux)
		set(SPIRVTOOLS_LIB_PATH ${CMAKE_SOURCE_DIR}/3rdparty/spirv-tools/lib/linux)
	endif(APPLE)
endif(UNIX)

IF(WIN32)
target_link_libraries(raygene3d-core PRIVATE
	debug ${GLSLANG_LIB_PATH}/glslangd.lib optimized ${GLSLANG_LIB_PATH}/glslang.lib
	debug ${GLSLANG_LIB_PATH}/HLSLd.lib optimized ${GLSLANG_LIB_PATH}/HLSL.lib
	debug ${GLSLANG_LIB_PATH}/OGLCompilerd.lib optimized ${GLSLANG_LIB_PATH}/OGLCompiler.lib
	debug ${GLSLANG_LIB_PATH}/OSDependentd.lib optimized ${GLSLANG_LIB_PATH}/OSDependent.lib
)

target_link_libraries(raygene3d-core PRIVATE
	debug ${SPIRV_LIB_PATH}/SPIRVd.lib optimized ${SPIRV_LIB_PATH}/SPIRV.lib
	debug ${SPIRV_LIB_PATH}/SPVRemapperd.lib optimized ${SPIRV_LIB_PATH}/SPVRemapper.lib
	debug ${SPIRV_LIB_PATH}/SPIRVd.lib optimized ${SPIRV_LIB_PATH}/SPIRV.lib
)

target_link_libraries(raygene3d-core PRIVATE
	debug ${SPIRVTOOLS_LIB_PATH}/SPIRV-Toolsd.lib optimized ${SPIRVTOOLS_LIB_PATH}/SPIRV-Tools.lib
	debug ${SPIRVTOOLS_LIB_PATH}/SPIRV-Tools-optd.lib optimized ${SPIRVTOOLS_LIB_PATH}/SPIRV-Tools-opt.lib
	debug ${SPIRVTOOLS_LIB_PATH}/SPIRV-Tools-reduced.lib optimized ${SPIRVTOOLS_LIB_PATH}/SPIRV-Tools-reduce.lib
)
ELSE(WIN32)
target_link_libraries(${NAME}-core PRIVATE
	optimized ${GLSLANG_LIB_PATH}/libMachineIndependent.a
	optimized ${GLSLANG_LIB_PATH}/libGenericCodeGen.a
	optimized ${GLSLANG_LIB_PATH}/libglslang.a
	optimized ${GLSLANG_LIB_PATH}/libHLSL.a
	optimized ${GLSLANG_LIB_PATH}/libOGLCompiler.a
	optimized ${GLSLANG_LIB_PATH}/libOSDependent.a
)

target_link_libraries(${NAME}-core PRIVATE
	optimized ${SPIRV_LIB_PATH}/libSPIRV.a
	optimized ${SPIRV_LIB_PATH}/libSPVRemapper.a
	optimized ${SPIRV_LIB_PATH}/libSPIRV.a
)

target_link_libraries(${NAME}-core PRIVATE
	optimized ${SPIRVTOOLS_LIB_PATH}/libSPIRV-Tools.a
	optimized ${SPIRVTOOLS_LIB_PATH}/libSPIRV-Tools-opt.a
	optimized ${SPIRVTOOLS_LIB_PATH}/libSPIRV-Tools-reduce.a
)
target_link_libraries(${NAME}-core PRIVATE
	optimized -ldl
	optimized -lpthread
)
ENDIF(WIN32)
