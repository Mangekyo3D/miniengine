set (SRC
	openglbuffer.cpp
	opengldevice.cpp
	opengltexture.cpp
	openglpipeline.cpp
	openglrenderpass.cpp
	openglprogram.cpp
	openglshader.cpp
)

set (INC
	opengldevice.h
	openglbuffer.h
	opengltexture.h
	openglpipeline.h
	openglrenderpass.h
	openglprogram.h
	openglshader.h
	)

if (WIN32)
	list(APPEND SRC openglswapchainwin32.cpp)
	list(APPEND INC openglswapchainwin32.h)
elseif (UNIX)
	list(APPEND SRC openglswapchainx11.cpp)
	list(APPEND INC openglswapchainx11.h)
endif()

add_library(openglbackend STATIC ${SRC} ${INC})
