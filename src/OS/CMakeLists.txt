set (SRC
	GameWindow.cpp
	GameWindowFactory.cpp
	OSFactory.cpp
)

set (INC
	GameWindow.h
	OSFactory.h
)

if (UNIX)
	set (SRC
		${SRC}
		X11Window.cpp
		X11PathUtils.cpp
	)
	set (INC
		${INC}
		X11Factory.h
		X11Window.h
		X11PathUtils.h
	)
	set (LIBS 
		X11
		GL
	)
elseif (WIN32)
	set (SRC
		${SRC}
		Win32Window.cpp
		Win32PathUtils.cpp
	)
	set (INC
		${INC}
		Win32Factory.h
		Win32Window.h
		Win32PathUtils.h
	)
	set (LIBS 
		opengl32
	)
endif()


add_library (OS STATIC ${SRC} ${INC})
target_link_libraries(OS ${LIBS})
