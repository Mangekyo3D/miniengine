set (SRC
	matrix.cpp
	perlin.cpp
	vertex.cpp
	mathutils.cpp
	colorutils.cpp
	scopedtimer.cpp
)

set (INC
	matrix.h
	vertex.h
	perlin.h
	signal.h
	mathutils.h
	colorutils.h
	scopedtimer.h
)

add_library (Util STATIC ${SRC} ${INC})

