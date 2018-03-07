set (SRC
	openglbuffer.cpp
	opengldevice.cpp
)

set (INC
	opengldevice.h
	openglbuffer.h
	)
	
add_library(openglbackend STATIC ${SRC} ${INC})
