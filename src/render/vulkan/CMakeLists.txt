set (SRC
	vulkanshadermodule.cpp
	vulkanbuffer.cpp
	vulkantexture.cpp
	vulkandevice.cpp
	vulkandevicefactory.cpp
	vulkanswapchain.cpp
	vulkanrenderpass.cpp
	vulkancommandbuffer.cpp
	vulkanpipeline.cpp
	)

set (INC
	vulkanshadermodule.h
	vulkanbuffer.h
	vulkantexture.h
	vulkandevice.h
	vulkandevicefactory.h
	vulkanswapchain.h
	vulkanrenderpass.h
	vulkancommandbuffer.h
	vulkanpipeline.h
	)

set (INC_DIRS)
set (LIBS
	)

if (WIN32)
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
elseif (UNIX)
	add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
	list(APPEND LIBS dl)
endif()

add_definitions(-DVK_NO_PROTOTYPES)

include_directories(${Vulkan_INCLUDE_DIRS} ${INC_DIRS})

add_library(vulkanbackend STATIC ${SRC} ${INC})
target_link_libraries(vulkanbackend ${LIBS})
