set (SRC
	vulkandevice.cpp
	vulkandevicefactory.cpp
	)

set (INC
	vulkandevice.h
	vulkandevicefactory.h
	)

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
