set (SRC
	vulkandevice.cpp
	vulkandevicefactory.cpp
	)

set (INC
	vulkandevice.h
	vulkandevicefactory.h
	)

if (WIN32)
	list(APPEND SRC vulkanswapchainwin32.cpp)
	list(APPEND INC vulkanswapchainwin32.h)
elseif (UNIX)
	list(APPEND SRC vulkanswapchainx11.cpp)
	list(APPEND INC vulkanswapchainx11.h)
endif()

set (INC_DIRS)
set (LIBS
	)

if (UNIX)
	add_definitions(-DVK_USE_PLATFORM_XLIB_KHR)
	list(APPEND LIBS
		dl)
elseif (WIN32)
	add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
endif()
add_definitions(-DVK_NO_PROTOTYPES)

include_directories(${Vulkan_INCLUDE_DIRS} ${INC_DIRS})

add_library(vulkanbackend STATIC ${SRC} ${INC})
target_link_libraries(vulkanbackend ${LIBS})
