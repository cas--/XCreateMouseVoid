////////////////////////////////////////////////////////////////////////////////
// Written by Patrick Stalph (patrick.stalph@gmx.de)
// Copyright by the author. This is unmaintained, no-warranty free software.
// Please use freely. It is appreciated (but by no means mandatory) to
// acknowledge the author's contribution. Thank you.
// Date: 2009-11-29

// Tweaked and extended by anonomous coward on 2011-03-29

// Creates an undecorated window with that prevents the mouse from entering
// it's "visible" area. This is usefull for dual-screen environments, where
// the two monitors cannot fully cover the virtual screen of the X-server
// (consequently, a void exists on the virtual screen). When writing this code,
// the mouse could enter this void area and disappear, thus confusing the user.
//
// When initialized properly, the created window can exactly cover the void,
// thus preventing the mouse cursor from entering it! :-)

#include <stdio.h>
#include <stdlib.h>
#include <X11/Xlib.h>

int void_x;
int void_y;
int void_width;
int void_height;
int screen_height;
int screen_width;
char mode = 0; // 'd'=down, 'u'=up, 'l'=left, 'r'=right, default is to block on all edges(=0)

void printUsage(char* argv0) {
	printf(
		"Author: Patrick Stalph, Version: 0.1\n\n"
		"Usage:\n\n"
		"%s x y w h [mode]\n\n"
		"Where 'x' and 'y' are screen coordinates of upper left corner of the void\n"
		"and 'w' and 'h' specify the width and height of the void.\n"
		"\n"
		"The optional mode is one of 'd', 'u', 'l', 'r' (down, up, left, right),\n"
		"and it overides the default 'block on all edges of the void' behavior.\n\n"
		"For example: if 'd' is specified, and the mouse encounters a vertical\n"
		"edge of the void, the mouse will jump down to the bottom edged of the\n"
		"void.  This behavior allows the user to move the mouse in a horizontal\n"
		"direction without being stopped at the vertical edge of the void:\n"
		"the mouse will simply move down to the bottom edge of the void and\n"
		"continue along its horizontal path.\n\n",
		argv0
	);
}

void processArguments(int argc, char* argv[]) {
	if (argc < 5 || argc > 6) {
		fprintf(stderr, "\nError: Wrong number of arguments.\n\n");
		printUsage(argv[0]);
		exit(1);
	}
	void_x = atoi(argv[1]);
	void_y = atoi(argv[2]);
	void_width = atoi(argv[3]);
	void_height = atoi(argv[4]);
	if( argc == 6 ) {
		mode = argv[5][0];
		if( mode != 'd' && mode != 'u' && mode != 'l' && mode != 'r' ) {
			fprintf( stderr, "Illegal mode %c, expected one of ['d','u','l','r'].\n", mode );
			printUsage( argv[0] );
			exit( 1 );
		}
	}

	if (void_x < 0 || void_y < 0 || void_width < 1 || void_height < 1) {
		fprintf(
			stderr,
			"Bad arguments (negative coordinate or zero width/height):\n"
			" x=%s, y=%s, w=%s, h=%s\n", argv[1], argv[2], argv[3], argv[4]
		);
		printUsage( argv[0] );
		exit( 1 );
	}
#ifdef DEBUG
	printf( "Settings: void at %d, %d, width %d, height %d", void_x, void_y, void_width, void_height );
	if( mode != 0 )
		printf( ", mode is '%c'", mode );
	printf( "\n" );
#endif
}

// creates an undecorated black area (window)
Window createUndecoratedWindow( Display* display ) {
	int screen = DefaultScreen( display );
	screen_height = DisplayHeight( display, screen );
	screen_width = DisplayWidth( display, screen );

	XSetWindowAttributes wattr;
	wattr.background_pixmap = None;
	wattr.background_pixel = None;
	wattr.border_pixel = 0;
	wattr.win_gravity = NorthWestGravity;
	wattr.bit_gravity = ForgetGravity;
	wattr.save_under = 0;
	wattr.event_mask = NoEventMask;
	wattr.do_not_propagate_mask = NoEventMask;
	wattr.override_redirect = 0;
	wattr.colormap = DefaultColormap( display, screen );
	wattr.background_pixel = None;

	unsigned long mask = CWEventMask;

	Window window
	=	XCreateWindow
		(	display,
			DefaultRootWindow( display ),
			void_x,
			void_y,
			void_width,
			void_height,
			0,
			CopyFromParent,
			InputOnly,
			CopyFromParent,
			mask,
			&wattr
		);

	// tell the window manager to exclude this window from window management
	wattr.override_redirect = 1;
	XChangeWindowAttributes( display, window, CWOverrideRedirect, &wattr );

	// map the window (i.e. make it appear on the screen)
	XMapWindow( display, window );

	return window;
}

int main( int argc, char* argv[] ) {
	processArguments( argc, argv );

	// open connection to X server
	Display* display; /* pointer to X display */
#ifdef DEBUG
	printf("%-50s", "connect to X-server...");
#endif
	display = XOpenDisplay( getenv( "DISPLAY" ) );
	if( display == NULL ) {
		fprintf( stderr, "\nCannot connect to X server '%s'\n", getenv( "DISPLAY" ) );
		exit( 1 );
	}
#ifdef DEBUG
	printf( "[ok]\n" );
#endif

#ifdef DEBUG
	printf( "%-50s", "creating undecorated window..." );
#endif
	Window w = createUndecoratedWindow( display );
#ifdef DEBUG
	printf( "[ok]\n" );
#endif

#ifdef DEBUG
	printf( "%-50s", "registering for input events..." );
#endif
	XSelectInput( display, w, PointerMotionMask );
#ifdef DEBUG
	printf( "[ok]\n" );
#endif

#ifdef DEBUG
	printf( "now entering event loop (infinite); ctrl-c to abort.\n" );
#endif
	XEvent e;
	for (;;) {
		// non-busy blocking wait if eventqueue is empty.
		// we only get events if the mouse enters the window!
		XNextEvent( display, &e );

		if( e.type == MotionNotify ) {
			int x = e.xmotion.x; /* x mouse coordinate */
			int y = e.xmotion.y; /* y mouse coordinate */
			int dx = 0, dy = 0;

			if( mode == 0 )
			{
				// these are used to determine closest edge
				int ldist = x;
				int rdist = void_width - x;
				int tdist = y;
				int bdist = void_height - y;
	
				// determine action required
				// First, handle special cases where mouse is near screen edge
				if( ((x == 0) && (void_x == 0)) || ((x + void_x) >= (screen_width - 1)) )
				{ // void is aligned with vertical screen edge and mouse is at vertical screen edge
					if( tdist < bdist )
						dy = -y - 1;
					else
						dy = void_height - y;
				}
				else if( ((y == 0) && (void_y == 0)) || ((y + void_y) >= (screen_height - 1)) )
				{ // void is aligned with horizontal screen edge and mouse is at horizontal screen edge
					if( ldist < rdist )
						dx = -x - 1;
					else
						dx = void_width - x;
				}
				// now, handle normal cases
				else if( ldist < rdist ) {
					if( ldist < tdist ) {
						if( ldist < bdist )
							dx = -x - 1;
						else
							dy = void_height - y;
					} else {
						if( tdist < bdist )
							dy = -y - 1;
						else
							dy = void_height - y;
					}
				} else {
					if( rdist < tdist ) {
						if( rdist < bdist )
							dx = void_width - x;
						else
							dy = void_height - y;
					} else {
						if( tdist < bdist )
							dy = -y - 1;
						else
							dy = void_height - y;
					}
				}
			}
			else
			{
				switch (mode) {
					case 'd':
						dy = void_height - y;
						break;

					case 'u':
						dy = -y - 1;
						break;

					case 'l':
						dx = -x - 1;
						break;

					case 'r':
						dx = void_width - x;
						break;

					default:
						printf( "Unknown mode: %d\n", mode );
						exit( 1 );
						break;
				}
			}

#ifdef DEBUG
			printf( "mouse @ %4d %4d warpby %4d %4d\n", x, y, dx, dy );
#endif
			XWarpPointer( display, None, None, 0, 0, 0, 0, dx, dy );
		}
	}

	XCloseDisplay( display );
	return 0;
}
