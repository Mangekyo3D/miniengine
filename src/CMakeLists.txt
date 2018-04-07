find_package(Vulkan REQUIRED)

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
	compositingpipeline.cpp
	renderpass.cpp

	render/irenderpass.cpp
	render/itexture.cpp
	render/ipipeline.cpp
	render/idevice.cpp
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
	compositingpipeline.h
	renderpass.h

	render/irenderpass.h
	render/itexture.h
	render/idevice.h
	render/igpubuffer.h
	render/iswapchain.h
	render/ipipeline.h
	)

set (SHADERS
	generic.vert
	generic.frag
	genericTextured.vert
	genericTextured.frag
	toneMapping.vert
	toneMapping.frag
	)

add_custom_target(shaderbindir ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/bin/shaders/
    COMMENT "Creating shader binary dir")
get_filename_component(VULKAN_HOME_DIR ${Vulkan_INCLUDE_DIR} DIRECTORY)
if (WIN32)
	set(VULKAN_BIN_DIR "${VULKAN_HOME_DIR}/Bin")
elseif(UNIX)
	set(VULKAN_BIN_DIR "${VULKAN_HOME_DIR}/bin")
endif()

# generate spir-v modules from source files
foreach(SHADER ${SHADERS})
    set(SHADER_BINARY ${CMAKE_BINARY_DIR}/bin/shaders/${SHADER}.spv)
    set(SHADER_BINARY_UNOPTIMIZED ${CMAKE_BINARY_DIR}/bin/shaders/${SHADER}_unopt.spv)
    set(SHADER_FILE ${CMAKE_SOURCE_DIR}/shaders/${SHADER})
    add_custom_command(OUTPUT ${SHADER_BINARY_UNOPTIMIZED}
        COMMAND ${VULKAN_BIN_DIR}/glslangValidator -V ${SHADER_FILE} -e main -o ${SHADER_BINARY_UNOPTIMIZED}
        COMMENT "SPIR-V: Building ${SHADER}"
        DEPENDS ${CMAKE_SOURCE_DIR}/shaders/${SHADER} shaderbindir)
    add_custom_command(OUTPUT ${SHADER_BINARY}
        COMMAND ${VULKAN_BIN_DIR}/spirv-opt
        --inline-entry-points-exhaustive
        --convert-local-access-chains
        --eliminate-local-single-block
        --eliminate-local-single-store
        --eliminate-insert-extract
        --eliminate-dead-code-aggressive
        --eliminate-dead-branches
        --merge-blocks
        --eliminate-local-single-block
        --eliminate-local-single-store
        --eliminate-local-multi-store
        --eliminate-insert-extract
        --eliminate-dead-code-aggressive
        --eliminate-common-uniform ${SHADER_BINARY_UNOPTIMIZED} -o ${SHADER_BINARY}
        COMMENT "SPIR-V: Optimizing ${SHADER}"
        DEPENDS ${SHADER_BINARY_UNOPTIMIZED})
    list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    list(APPEND SHADER_FILES ${SHADER_FILE})
endforeach()

add_subdirectory(OS)
add_subdirectory(Util)
add_subdirectory(render/opengl)
add_subdirectory(render/vulkan)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set (INC_DIRS)
set(LIBS
	openglbackend
	vulkanbackend
	${Vulkan_LIBRARIES}
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

find_package(OpenAL)
if (OPENAL_FOUND)
	add_definitions(-DOPENAL_FOUND)
	list(APPEND INC_DIRS ${OPENAL_INCLUDE_DIR})
	list(APPEND LIBS ${OPENAL_LIBRARY})
endif()

include_directories(${INC_DIRS})

add_executable(shooter ${SRC} ${INC} ${SHADER_FILES} ${SHADER_BINARIES})

target_link_libraries(shooter ${LIBS} OS Util)
