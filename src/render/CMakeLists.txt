set (SRC
	batch.cpp
	icommandbuffer.cpp
	itexture.cpp
	idevice.cpp
	compositingpipeline.cpp
	renderer.cpp
	rendergraph.cpp
	)

set (INC
	batch.h
	icommandbuffer.h
	irenderpass.h
	itexture.h
	idevice.h
	igpubuffer.h
	iswapchain.h
	ipipeline.h
	compositingpipeline.h
	renderer.h
	rendergraph.h
	)

set (SHADERS
	generic.vert
	generic.frag
	genericTextured.vert
	genericTextured.frag
	toneMapping.vert
	toneMapping.frag
	)

find_package(Vulkan REQUIRED)

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
    set(SHADER_FILE ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER})
    add_custom_command(OUTPUT ${SHADER_BINARY_UNOPTIMIZED}
        COMMAND ${VULKAN_BIN_DIR}/glslangValidator -V ${SHADER_FILE} -e main -o ${SHADER_BINARY_UNOPTIMIZED}
        COMMENT "SPIR-V: Building ${SHADER}"
        DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/shaders/${SHADER} shaderbindir)
    add_custom_command(OUTPUT ${SHADER_BINARY}
        COMMAND ${VULKAN_BIN_DIR}/spirv-opt
		-O ${SHADER_BINARY_UNOPTIMIZED} -o ${SHADER_BINARY}
        COMMENT "SPIR-V: Optimizing ${SHADER}"
        DEPENDS ${SHADER_BINARY_UNOPTIMIZED})
    list(APPEND SHADER_BINARIES ${SHADER_BINARY})
    list(APPEND SHADER_FILES ${SHADER_FILE})
endforeach()

set(LIBS
	openglbackend
	vulkanbackend
	${Vulkan_LIBRARIES}
	)

if (UNIX)
list(APPEND LIBS
	GL
	)
elseif (WIN32)
	list(APPEND LIBS
	opengl32
	)
endif()

include_directories(${INC_DIRS})

add_subdirectory(opengl)
add_subdirectory(vulkan)

add_library(render ${SRC} ${INC} ${SHADER_FILES} ${SHADER_BINARIES})

target_link_libraries(render ${LIBS} Util)
