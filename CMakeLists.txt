find_package(Vulkan REQUIRED COMPONENTS shaderc_combined)

set(NAME raygene3d)

set(CORE_DIR ${CMAKE_SOURCE_DIR}/${NAME}-core/core)
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
	${CMAKE_SOURCE_DIR}/3rdparty
)

target_link_libraries(${NAME}-core INTERFACE 
    Vulkan::Vulkan Vulkan::shaderc_combined)
target_link_libraries(${NAME}-core PRIVATE 
    debug ${Vulkan_shaderc_combined_LIBRARY}
    optimized ${Vulkan_shaderc_combined_LIBRARY}
)

target_link_libraries(${NAME}-core PRIVATE
	Vulkan::Vulkan
)
