set (SRC
	openalaudio.cpp
	batch.cpp
	bitmap.cpp
	camera.cpp
	engine.cpp
	explosion.cpp
	main.cpp
	plane.cpp
	renderer.cpp
	SMDmodel.cpp
	worldentity.cpp
	worldtile.cpp
	shader.cpp
	program.cpp
	resourcemanager.cpp
	texture.cpp
	compositingpipeline.cpp
	renderpass.cpp

	render/device.cpp
	)

set (INC
	audiointerface.h
	batch.h
	bitmap.h
	camera.h
	engine.h
	effect.h
	plane.h
	program.h
	renderer.h
	shader.h
	SMDmodel.h
	worldentity.h
	worldtile.h
	resourcemanager.h
	controller.h
	texture.h
	compositingpipeline.h
	renderpass.h

	render/device.h
	render/gpubuffer.h
	)

set (SHADER_SRC
	${CMAKE_SOURCE_DIR}/shaders/generic.vert
	${CMAKE_SOURCE_DIR}/shaders/generic.frag
	${CMAKE_SOURCE_DIR}/shaders/genericTextured.vert
	${CMAKE_SOURCE_DIR}/shaders/genericTextured.frag
	${CMAKE_SOURCE_DIR}/shaders/toneMapping.vert
	${CMAKE_SOURCE_DIR}/shaders/toneMapping.frag
	)

add_subdirectory(OS)
add_subdirectory(Util)
add_subdirectory(render/opengl)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (INC_DIRS)
set(LIBS
	openglbackend
	)

if (UNIX)
	list(APPEND LIBS
		X11
		GL
		)
elseif (WIN32)
	list(APPEND LIBS
		opengl32
		)
endif()

find_package(Vulkan)
if (Vulkan_FOUND)
	add_subdirectory(render/vulkan)
	add_definitions(-DBUILD_WITH_VULKAN)
	list(APPEND LIBS ${Vulkan_LIBRARIES} vulkanbackend)
endif()

find_package(OpenAL)
if (OPENAL_FOUND)
	add_definitions(-DOPENAL_FOUND)
	list(APPEND INC_DIRS ${OPENAL_INCLUDE_DIR})
	list(APPEND LIBS ${OPENAL_LIBRARY})
endif()

include_directories(${INC_DIRS})

add_executable(shooter ${SRC} ${INC} ${SHADER_SRC})

target_link_libraries(shooter ${LIBS} OS Util)
