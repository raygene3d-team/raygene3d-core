find_package(Vulkan REQUIRED)

set(NAME raygene3d)
 
set(CORE_DIR ${CMAKE_SOURCE_DIR}/${NAME}-core)
#set(ext_path ${CMAKE_SOURCE_DIR}/ext)

#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

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


set(RENDERER_DIR ${CORE_DIR}/device)
set(RENDERER_SOURCE
	${RENDERER_DIR}/config.h
	${RENDERER_DIR}/config.cpp
	${RENDERER_DIR}/device.h
	${RENDERER_DIR}/device.cpp
	${RENDERER_DIR}/layout.h
	${RENDERER_DIR}/layout.cpp
	${RENDERER_DIR}/pass.h
	${RENDERER_DIR}/pass.cpp
	${RENDERER_DIR}/resource.h
	${RENDERER_DIR}/resource.cpp
	${RENDERER_DIR}/view.h
	${RENDERER_DIR}/view.cpp
)
set(D11_RENDERER_DIR ${RENDERER_DIR}/d11)
set(D11_RENDERER_SOURCE
	${D11_RENDERER_DIR}/d11_config.h
	${D11_RENDERER_DIR}/d11_config.cpp
	${D11_RENDERER_DIR}/d11_device.h
	${D11_RENDERER_DIR}/d11_device.cpp
	${D11_RENDERER_DIR}/d11_layout.h
	${D11_RENDERER_DIR}/d11_layout.cpp
	${D11_RENDERER_DIR}/d11_pass.h
	${D11_RENDERER_DIR}/d11_pass.cpp
	${D11_RENDERER_DIR}/d11_resource.h
	${D11_RENDERER_DIR}/d11_resource.cpp
	${D11_RENDERER_DIR}/d11_view.h
	${D11_RENDERER_DIR}/d11_view.cpp
)
set(VLK_RENDERER_DIR ${RENDERER_DIR}/vlk)
set(VLK_RENDERER_SOURCE
	${VLK_RENDERER_DIR}/vlk_config.h
	${VLK_RENDERER_DIR}/vlk_config.cpp
	${VLK_RENDERER_DIR}/vlk_device.h
	${VLK_RENDERER_DIR}/vlk_device.cpp
	${VLK_RENDERER_DIR}/vlk_layout.h
	${VLK_RENDERER_DIR}/vlk_layout.cpp
	${VLK_RENDERER_DIR}/vlk_pass.h
	${VLK_RENDERER_DIR}/vlk_pass.cpp
	${VLK_RENDERER_DIR}/vlk_resource.h
	${VLK_RENDERER_DIR}/vlk_resource.cpp
	${VLK_RENDERER_DIR}/vlk_view.h
	${VLK_RENDERER_DIR}/vlk_view.cpp
)

IF(WIN32)
set(SOURCE core.h core.cpp base.h base.cpp
	${RENDERER_SOURCE}
	${D11_RENDERER_SOURCE}
	${VLK_RENDERER_SOURCE}
	${UTILITY_SOURCE}
)
ELSE(WIN32)
set(SOURCE core.h core.cpp base.h base.cpp
	${RENDERER_SOURCE}
	${VLK_RENDERER_SOURCE}
	${UTILITY_SOURCE}
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
