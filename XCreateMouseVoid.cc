// Written by Patrick Stalph (patrick.stalph@gmx.de)
// Copyright by the author. This is unmaintained, no-warranty free software.
// Please use freely. It is appreciated (but by no means mandatory) to
// acknowledge the author's contribution. Thank you.
// Date: 2009-11-29

// Creates an undecorated window with black background that prevents the
// mouse from entering its "visible" area. This is usefull for dual-screen
// environments, where the two monitors cannot fully cover the virtual screen
// of the X-server (consequently, a void exists on the virtual screen). When
// writing this code, the mouse could enter this void area and disapear, thus
// confusing the user.
//
// When initialized properly, the created window can exactly cover the void,
// thus preventing the mouse cursor from entering it! :-)

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h> // Every Xlib program must include this
int void_x;
int void_y;
int void_width;
int void_height;
char mode = 'd'; // d=down, u=up, l=left, r=right

// prints the usage
void printUsage(char* argv0) {
	printf("Usage:\n"
		" %s x y w h [mode]\n"
		"where x and y are screen coordinates, w and h specify\n"
		"the width and height of the void. The optional mode\n"
		"is one of 'd', 'u', 'l', 'r' (down, up, left, right).\n"
		"The mode specifies the mouse pointer direction, if the\n"
		"void is hit. Default is down (void placed at the top\n"
		"right corner of the screen.\n", argv0);
}

// show usage or parse and validate arguments
void processArguments(int argc, char* argv[]) {
	if (argc < 5 || argc > 6) {
		fprintf(stderr, "Wrong number of arguments.\n");
		printUsage(argv[0]);
		exit(0);
	}
	void_x = atoi(argv[1]);
	void_y = atoi(argv[2]);
	void_width = atoi(argv[3]);
	void_height = atoi(argv[4]);
	if (argc == 6) {
		mode = argv[5][0];
		if (mode != 'd' && mode != 'u' && mode != 'l' && mode != 'r') {
			fprintf(stderr,
					"Illegal mode %c, expected one of ['d','u','l','r'].\n",
					mode);
			printUsage(argv[0]);
			exit(0);
		}
	}

	if (void_x < 0 || void_y < 0 || void_width < 1 || void_height < 1) {
		fprintf(stderr,
				"Bad arguments (negative coordinate or zero width/height):\n"
					" x=%s, y=%s, w=%s, h=%s\n", argv[1], argv[2], argv[3],
				argv[4]);
		printUsage(argv[0]);
		exit(0);
	}

	printf("Settings: void at %d, %d, width %d, height %d, mode is '%c'\n",
			void_x, void_y, void_width, void_height, mode);
}

// creates an undecorated black area (window)
Window createUndecoratedWindow(Display* display) {
	XSetWindowAttributes wattr;
	XColor black, black_e;
	int screen = DefaultScreen(display);

	wattr.background_pixmap = None;
	wattr.background_pixel = black.pixel;
	wattr.border_pixel = 0;
	wattr.win_gravity = NorthWestGravity;
	wattr.bit_gravity = ForgetGravity;
	wattr.save_under = 1;
	wattr.event_mask = (StructureNotifyMask | ExposureMask | PropertyChangeMask
			| EnterWindowMask | LeaveWindowMask | KeyPressMask | KeyReleaseMask
			| KeymapStateMask);
	wattr.do_not_propagate_mask = (KeyPressMask | KeyReleaseMask
			|ButtonPressMask | ButtonReleaseMask | PointerMotionMask
			|ButtonMotionMask);
	wattr.override_redirect = 0;
	wattr.colormap = DefaultColormap(display, screen);

	XAllocNamedColor(display, DefaultColormap(display, screen), "black",
			&black, &black_e);
	wattr.background_pixel = black.pixel;

	unsigned long mask = CWBackPixmap | CWBackPixel | CWBorderPixel
			|CWWinGravity | CWBitGravity | CWSaveUnder | CWEventMask
			|CWDontPropagate | CWOverrideRedirect | CWColormap;
	Window window = XCreateWindow(display, DefaultRootWindow(display), void_x,
			void_y, void_width, void_height, 1, CopyFromParent, InputOutput,
			CopyFromParent, mask, &wattr);

	// tell the window manager not to draw window borders (frame) or title.
	wattr.override_redirect = 1;
	XChangeWindowAttributes(display, window, CWOverrideRedirect, &wattr);
	// map the window (that is, make it appear on the screen)
	XMapWindow(display, window);

	return window;
}

/**
 * main method, args used
 *
 *  arg[1] = x
 *  arg[2] = y
 *  arg[3] = width
 *  arg[4] = height
 */
int main(int argc, char* argv[]) {
	// parse and validate arguments
	processArguments(argc, argv);

	// open connection to X server
	Display* display; /* pointer to X display */
	printf("%-50s", "connect to X-server...");
	display = XOpenDisplay(getenv("DISPLAY"));
	if (display == NULL) {
		fprintf(stderr, "\nCannot connect to X server '%s'\n",
				getenv("DISPLAY"));
		exit(1);
	}
	printf("[ok]\n");

	// create a undecorated window - the void :)
	printf("%-50s", "creating undecorated window...");
	Window w = createUndecoratedWindow(display);
	printf("[ok]\n");

	// register for the relevant MotionNotify event
	printf("%-50s", "registering for MotionEvents...");
	XSelectInput(display, w, PointerMotionMask);
	printf("[ok]\n");

	printf("now entering event loop (infinite); ctrl-c to abort.\n");

	XEvent e;
	int x, y;
	int dx = 0;
	int dy = 0;

	for (;;) {
		// non-busy blocking wait, if eventqueue is empty.
		// we only get events, if the mouse enters the window!
		XNextEvent(display, &e);

		if (e.type == MotionNotify) {
			x = e.xbutton.x; /* x mouse coordinate */
			y = e.xbutton.y; /* y mouse coodrinate */

			switch (mode) {
			case 'd': // always move down
				dy = void_height - y + 1;
				break;
			case 'u': // always move up
				dy = -y - 2;
				break;
			case 'l': // always move left
				dx = -x - 2;
				break;
			case 'r': // always move right
				dx = void_width - x + 1;
				break;
			default:
				printf("Unknown mode: %d\n", mode);
				exit(0);
				break;
			}

			// printf("mouse @ %4d %4d warpby %4d %4d\n", x, y, dx, dy);
			XWarpPointer(display, None, None, 0, 0, 0, 0, dx, dy);
		}
	}

	XCloseDisplay(display);
	return 0;
}
