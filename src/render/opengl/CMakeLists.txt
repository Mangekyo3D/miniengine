set (SRC
	openglbuffer.cpp
	opengldevice.cpp
	openglswapchainwin32.cpp
)

set (INC
	opengldevice.h
	openglbuffer.h
	openglswapchainwin32.h
	)

add_library(openglbackend STATIC ${SRC} ${INC})
