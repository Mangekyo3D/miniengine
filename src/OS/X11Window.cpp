#include "X11Window.h"
#include <GL/glx.h>
#include <iostream>
#include <cstring>

X11Window::X11Window(Display *dpy)
{
	m_display = dpy;

	int attribs[] = {//GLX_BUFFER_SIZE, 24,
					 GLX_SAMPLE_BUFFERS, 0,
					 GLX_RENDER_TYPE, GLX_RGBA_BIT,
					 GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
					 GLX_X_RENDERABLE, True,
					 GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
					 GLX_CONFIG_CAVEAT, GLX_NONE,
					 GLX_TRANSPARENT_TYPE, GLX_NONE,
					 GLX_DOUBLEBUFFER, True,
					 GLX_STEREO, False,
					 GLX_AUX_BUFFERS, 0,
					 GLX_RED_SIZE, 8,
					 GLX_GREEN_SIZE, 8,
					 GLX_BLUE_SIZE, 8,
					 GLX_ALPHA_SIZE, 0,
					 GLX_DEPTH_SIZE, 0,
					 GLX_STENCIL_SIZE, 8,
					 None};

	int numOfConfigs;
	GLXFBConfig *config;
	int tmp;
	XVisualInfo *xvis;

	if (!glXQueryExtension(m_display, &tmp, &tmp))
	{
		std::cout << "No glX support found.\n";
	}

	config = glXChooseFBConfig(m_display, DefaultScreen(m_display), attribs, &numOfConfigs);

	if (numOfConfigs == 0)
	{
		std::cout << numOfConfigs << "No suitable framebuffer configurations found.\n";
	}
	if((xvis = glXGetVisualFromFBConfig(m_display, *config)) == nullptr)
	{
		std::cout << "No corresponding X visual found for framebuffer configuration.\n";
	}

	XSetWindowAttributes winAttribs = {0};

	winAttribs.event_mask = StructureNotifyMask | ButtonPressMask | ButtonReleaseMask | KeyPressMask | KeyReleaseMask;

	m_win = XCreateWindow(m_display, DefaultRootWindow(m_display), 0, 0, 800, 600, 0,
		xvis->depth, InputOutput, xvis->visual,
		CWEventMask, &winAttribs);

//	XWindowAttributes xwa;
//	XGetWindowAttributes(display, DefaultRootWindow(display), &xwa);
//	XMoveResizeWindow(display, win, 0, 0, xwa.width, xwa.height);

	XMapRaised(m_display, m_win);

	Pixmap blank;
	XColor dummy;
	char data = 0;
	Cursor cursor;

	blank = XCreateBitmapFromData (m_display, m_win, &data, 1, 1);
	cursor = XCreatePixmapCursor(m_display, blank, blank, &dummy, &dummy, 0, 0);

	XDefineCursor(m_display, m_win, cursor);

	XFreePixmap (m_display, blank);
	XFreeCursor(m_display, cursor);
	XFree(xvis);
	XFree(config);
}

X11Window::~X11Window()
{
	XDestroyWindow(m_display, m_win);
}

void X11Window::getClientSize(uint32_t& width, uint32_t& height)
{
	XWindowAttributes attr;
	XGetWindowAttributes(m_display, m_win, &attr);
	width = attr.width;
	height = attr.height;
}

void X11Window::handleOSEvents()
{
	while (XPending(m_display))
	{
		XEvent e;
		XNextEvent (m_display, &e);

		switch (e.type)
		{
			case ConfigureNotify:
			{
				XConfigureEvent resizeEvent = e.xconfigure;
				uint32_t width = resizeEvent.width;
				uint32_t height = resizeEvent.height;
				ResizeEvent event = {width, height};
				onResize(event);
				break;
			}

			case ButtonPress:
			case ButtonRelease:
			{
				XButtonEvent buttonPressEvent = e.xbutton;
				KeyEvent event;
				event.type = (e.type == ButtonPress) ? KeyEvent::EType::ePress : KeyEvent::EType::eRelease;

				switch (buttonPressEvent.button)
				{
					case Button1:
						event.key = KeyEvent::EKey::eLeftMouse;
						break;

					case Button2:
						event.key = KeyEvent::EKey::eRightMouse;
						break;

					case Button4:
					case Button5:
					{
						MouseWheelEvent wheelEvent;
						wheelEvent.ticks = (buttonPressEvent.button == Button4) ? 1 : -1;

						onMouseWheel(wheelEvent);
						return;
					}
				}

				onKey(event);
				break;
			}

			case KeyPress:
			case KeyRelease:
			{
				XKeyEvent keyPressEvent = e.xkey;

				KeySym ksym = XLookupKeysym(&keyPressEvent, 0);

				KeyEvent event;
				event.type = (e.type == KeyPress) ? KeyEvent::EType::ePress : KeyEvent::EType::eRelease;

				switch (ksym)
				{
					case XK_W:
					case XK_w:
						event.key = KeyEvent::EKey::eWKey;
						break;

					case XK_S:
					case XK_s:
						event.key = KeyEvent::EKey::eSKey;
						break;

					case XK_A:
					case XK_a:
						event.key = KeyEvent::EKey::eAKey;
						break;

					case XK_D:
					case XK_d:
						event.key = KeyEvent::EKey::eDKey;
						break;

					case XK_space:
						event.key = KeyEvent::EKey::eSpace;
						break;

					case XK_Escape:
						event.key = KeyEvent::EKey::eEscape;
						break;
				}

				onKey(event);
			}
		}
	}
}

void X11Window::maximize()
{
	XEvent xev;
	Atom wm_state = XInternAtom(m_display, "_NET_WM_STATE", False);
	Atom fullscreen = XInternAtom(m_display, "_NET_WM_STATE_FULLSCREEN", False);

	memset(&xev, 0, sizeof(xev));
	xev.type = ClientMessage;
	xev.xclient.window = m_win;
	xev.xclient.message_type = wm_state;
	xev.xclient.format = 32;
	xev.xclient.data.l[0] = 1;
	xev.xclient.data.l[1] = fullscreen;
	xev.xclient.data.l[2] = 0;

	XSendEvent(m_display, DefaultRootWindow(m_display), False, SubstructureNotifyMask, &xev);
}
