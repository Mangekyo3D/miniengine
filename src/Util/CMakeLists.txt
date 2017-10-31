set (SRC
	matrix.cpp
	perlin.cpp
	vertex.cpp
	mathutils.cpp
)

set (INC
	matrix.h
	vertex.h
	perlin.h
	signal.h
	mathutils.h
)

add_library (Util STATIC ${SRC} ${INC})

