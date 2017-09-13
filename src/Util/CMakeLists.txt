set (SRC
	matrix.cpp
	perlin.cpp
	vertex.cpp
)

set (INC
	matrix.h
	vertex.h
	perlin.h
	signal.h
)

add_library (Util STATIC ${SRC} ${INC})

