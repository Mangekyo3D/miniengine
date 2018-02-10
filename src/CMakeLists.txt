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

add_executable(shooter ${SRC} ${INC} ${SHADER_SRC})

if (UNIX)
	set (LIBS 
		X11
		GL
		)
elseif (WIN32)
	set (LIBS 
		opengl32
		)
endif()

set (INC_DIRS)

find_package(OpenAL)

if (OPENAL_FOUND)
	add_definitions(-DOPENAL_FOUND)
	list(APPEND INC_DIRS ${OPENAL_INCLUDE_DIR})
	list(APPEND LIBS ${OPENAL_LIBRARY})
endif()

include_directories(${INC_DIRS})

target_link_libraries(shooter ${LIBS} OS Util)
