find_package(Vulkan REQUIRED)

set (SRC
	openalaudio.cpp
	bitmap.cpp
	camera.cpp
	engine.cpp
	explosion.cpp
	main.cpp
	plane.cpp
	SMDmodel.cpp
	worldentity.cpp
	worldtile.cpp
	resourcemanager.cpp
	)

set (INC
	audiointerface.h
	bitmap.h
	camera.h
	engine.h
	effect.h
	plane.h
	SMDmodel.h
	worldentity.h
	worldtile.h
	resourcemanager.h
	controller.h
	)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (INC_DIRS)
set(LIBS
	render)

if (UNIX)
	list(APPEND LIBS
		X11
		)
elseif (WIN32)
	list(APPEND LIBS
		)
	add_definitions(-DNOMINMAX)
endif()

find_package(OpenAL)
if (OPENAL_FOUND)
	add_definitions(-DOPENAL_FOUND)
	list(APPEND INC_DIRS ${OPENAL_INCLUDE_DIR})
	list(APPEND LIBS ${OPENAL_LIBRARY})
endif()

include_directories(${INC_DIRS})

add_subdirectory(OS)
add_subdirectory(Util)
add_subdirectory(render)

add_executable(shooter ${SRC} ${INC})

target_link_libraries(shooter ${LIBS} OS Util)
