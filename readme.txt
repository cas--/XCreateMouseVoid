
NAME
	XCreateMouseVoid - creates an undecorated black window and prevents the
	mouse from entering.

SYNTAX
	XCreateMouseVoid x y w h [mode]

ARGUMENTS
	x		
	y		Specifies the x and y coordinates of the void (upper left corner).

	w		
	h		Specifies the width and height of the rectangle

	mode	Optional. Specifies the behavior of the mouse pointer, when the
			void is hit. Valid values are 'd', 'u', 'l', 'r' (short for: down,
			up,	left, right). The default is 'd', where the mouse pointer is
			always warped to the bottom of the void (+1), while the vertical
			coordinate remains unchanged.

DESCRIPTION
	The XCreateMouseVoid function paints a black rectangle at the given
	coordinates. If the mouse pointer enters the rectangle, it is warped to a
	position outside the rectangle depending on the mode.

	The code does not occupy CPU (polling approaches would), if the rectangle
	is not hit. The X server only notifies the program, when the mouse pointer
	enters the rectangle.

AUTHOR
	Written by Patrick Stalph.

REPORTING BUGS
	Report bugs to patrick.stalph@gmx.de

-------------------------------------------------------------------------------

 Example
=========

At work, I have my laptop with one additional screen plugged into the VGA of my
laptop. The left monitor has a resolution of 1280*1024, while my laptop monitor
has a resolution of 1280*800. Laptop is right-of the main monitor, bottom-
aligned. It looks like this:

+----------------+ - - - - - - - -
|                |   THE VOID     '
|                +----------------+
|                |                |
|  VGA monitor   |     laptop     |
|                |                |
+----------------+----------------+

Consequently, the void starts at coordinates (1280, 0) and spans 1280*224
pixels. When hitting the left side of the void, the pointer should move to
the right screen (but jump down to the bottom of the void). If the pointer hits
the bottom of the void, it should just be blocked. In both cases the desired
behavior is to warp the mouse pointer downwards (mode 'd'). Thus, calling
	XCreateMouseVoid 1280 0 1280 223 d
creates a rectangle that exactly covers the invisible area of the virtual
screen. Furthermore, the mouse pointer is always warped to the bottom of the
rectangle, that is to the visible area of the laptop screen. Note that the
height needs to be one pixel smaller than the void. The four different modes
should make this tool useful for various layouts, even vertical ones.

To run this at the startup of Ubuntu, you should use
	System -> Preferences -> Startup Applications
IMPORTANT: Take care, when using your computer with beamers or generally in
varying environments. If blindly activated on startup every time, you may
endup having a black rectangle on your next presentation :) In this emergency
case, enter a terminal and type
	killall XCreateMouseVoid

 Known Problems
================
Sometimes the mouse pointer is flickering when hitting the void, because the
cursor changes its icon.

The code won't run without an X server. The connection is established using the
DISPLAY environment variable.

 Compiling the Code
====================

The code only requires Xlib, i.e. the package libx11-dev. Calling
	g++ XCreateMouseVoid.cc -lX11 -o XCreateMouseVoid
should suffice. Of course it doesn't make sense to run this code in
environments without an X server.


