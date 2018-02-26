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
	device.cpp
	program.cpp
	gpubuffer.cpp
	resourcemanager.cpp
	texture.cpp
	compositingpipeline.cpp
	renderpass.cpp

	opengldevice.cpp
)

set (INC
	audiointerface.h
	batch.h
	bitmap.h
	camera.h
	device.h
	engine.h
	effect.h
	gpubuffer.h
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

	opengldevice.h
	openglbuffer.h
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

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

find_package(Vulkan)

set (INC_DIRS)

if (UNIX)
	set (LIBS 
		X11
		GL
		)
elseif (WIN32)
	set (LIBS 
		opengl32
		)
	if (Vulkan_FOUND)
		add_definitions(-DVK_USE_PLATFORM_WIN32_KHR)
	endif()
endif()

find_package(OpenAL)

if (Vulkan_FOUND)
	add_definitions(-DBUILD_WITH_VULKAN -DVK_NO_PROTOTYPES)
	list(APPEND LIBS ${Vulkan_LIBRARIES})
	list(APPEND INC_DIRS ${Vulkan_INCLUDE_DIRS})
	list(APPEND SRC
		vulkandevice.cpp
		)
	list(APPEND INC
		vulkandevice.h
		)
endif()

if (OPENAL_FOUND)
	add_definitions(-DOPENAL_FOUND)
	list(APPEND INC_DIRS ${OPENAL_INCLUDE_DIR})
	list(APPEND LIBS ${OPENAL_LIBRARY})
endif()

include_directories(${INC_DIRS})

add_executable(shooter ${SRC} ${INC} ${SHADER_SRC})

target_link_libraries(shooter ${LIBS} OS Util)
