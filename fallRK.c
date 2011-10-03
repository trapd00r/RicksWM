// fall.c - draw color scale on root or another window

// Copyright (C) 2005-2008  Rick Klement
// $Revision: 1.10 $
// Sun Nov 19 11:52:19 PST 2006 - change args, no background, redraw

#define CLIPS 100

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

static Display *display;
static Window target = 0;
static XWindowAttributes root_geo;
static GC gc;
static Atom color_atom;

static int root_color = 0;

static int low_red = 0;
static int low_green = 0;
static int low_blue = 0x60;

static int high_red = 0;
static int high_green = 0x90;
static int high_blue = 0xff;

static int x, y, z;
static long base = (0 << 16) | (0 << 8) | 0x60;
static int elipse = 0;
static int full = 0;
static int active = 0;

void
redraw(void)
	{
	double inc = 6;
	long previous = 0;
	long color;
	int mid_x, mid_y;
	int d_red = high_red - low_red;
	int d_green = high_green - low_green;
	int d_blue = high_blue - low_blue;
	double radius, percent;

	mid_x = root_geo.width / 2;
	mid_y = root_geo.height / 2;
	radius = mid_x < mid_y ? mid_x : mid_y;

	x = elipse ? full ? 1.43 * mid_x : mid_x : radius;
	y = elipse ? full ? 1.43 * mid_y : mid_y : radius;
	for(z = radius; z > 0; x -= inc, y -= inc, z -= inc, inc += 0.2)
		{
		if(z < radius)
			{
			percent = sqrt( radius*radius - z*z) / radius;
			color = 
				((int)(d_red * percent + low_red) << 16) |
				((int)(d_green * percent + low_green) << 8) |
				(int)(d_blue * percent + low_blue);
			if(color != previous)
				{
				XSetForeground(display, gc, previous = color);
				XFillArc(display, target, gc,
					mid_x - x, mid_y - y, x * 2, y * 2,
					0, 360*64);
				}
			}
		}
	}

void
set_color(void)
	{
	Atom actual_atom;
	int actual_format;
	unsigned long n;
	unsigned long bytes_after;
	unsigned char *actual_property = NULL;

	if(Success == XGetWindowProperty(display, target, color_atom,
		0, 7, 0, XA_STRING, &actual_atom, &actual_format,
		&n, &bytes_after, &actual_property))
		{
		if(actual_atom != None && actual_property)
			{
			root_color = strtol((char *)actual_property + 1, 0, 16);
			XFree(actual_property);
			//printf("root color: 0x%06X\n", root_color);

#if 1
			low_red = (root_color >> 16) & 255;
			low_green = (root_color >> 8) & 255;
			low_blue = (root_color) & 255;
			high_red = (low_red + 255) / 2;
			high_green = (low_green + 255) / 2;
			high_blue = (low_blue + 255) / 2;
#else
			low_red = low_green = low_blue = 0;
			high_red = (root_color >> 16) & 255;
			high_green = (root_color >> 8) & 255;
			high_blue = (root_color) & 255;
#endif
			}
		}
	}

// main routine ##################################################

int
main(int argc, char *argv[])
	{
	int background = 1;
	int continous = 0;
	char *callingname = argv[0];
	XRectangle rectangles[CLIPS];
	int nrect;


	// open connection to X server

  if((display = XOpenDisplay(NULL)) == 0)
    {
    fprintf(stderr, "%s: error -> XOpenDisplay failed <- leaving\n", argv[0]);
    exit(1);
    }
	target = DefaultRootWindow(display);

	while(argc > 1)
		{
		if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
			{
			puts("-c  continous (redraw)");
			puts("-a  active by root color");
			puts("-nb no background");
			puts("-e  elipse instead of circle");
			puts("-window <windowid>");
			exit(0);
			}
		else if(strcmp(argv[1], "-c") == 0)
			{
			continous = 1;
			argc--;
			argv++;
			}
		else if(strcmp(argv[1], "-a") == 0)
			{
			active = 1;
			argc--;
			argv++;
			}
		else if(strcmp(argv[1], "-nb") == 0)
			{
			background = 0;
			argc--;
			argv++;
			}
		else if(strcmp(argv[1], "-e") == 0)
			{
			elipse = 1;
			argc--;
			argv++;
			}
		else if(strcmp(argv[1], "-full") == 0)
			{
			full = 1;
			elipse = 1;
			argc--;
			argv++;
			}
		else if(argc > 2 && strcmp(argv[1], "-window") == 0)
			{
			if(strcmp(argv[2], "root") == 0)
				{
				target = DefaultRootWindow(display);
				}
			else
				{
				target = strtol(argv[2], NULL, 0);
				}
			argc -= 2;
			argv += 2;
			}
		else if(argc == 2 && argv[1][0] != '-')
			{
			if(strcmp(argv[1], "root") == 0)
				{
				target = DefaultRootWindow(display);
				}
			else
				{
				target = strtol(argv[1], NULL, 0);
				}
			argc--;
			argv++;
			}
		else
			{
			fprintf(stderr,
				"usage: %s [-c] [-a] [-e] [-nb] [-window <id>] [<id>]\n",
				callingname);
			exit(1);
			}
		}

	color_atom = XInternAtom(display, "_RICKSWM_COLOR", False);

	gc = XCreateGC(display, target, 0, NULL);
	XGetWindowAttributes(display, target, &root_geo);

	if(background)
		{
		XSetWindowBackground(display, target, base);
		XClearWindow(display, target);
		}

	if(active)
		{
		set_color();
		}

	redraw();

	if(background)
		{
		XSetWindowBackground(display, target, base);
		}
	XFlush(display);

	if(continous)
		{
		XSelectInput(display, target,
			active ?  ExposureMask | PropertyChangeMask : ExposureMask);
		XFlush(display);
		while(1)
			{
			XEvent event;

			XNextEvent(display, &event);
			switch(event.type)
				{
			case Expose:
				rectangles[0].x = event.xexpose.x;
				rectangles[0].y = event.xexpose.y;
				rectangles[0].width = event.xexpose.width;
				rectangles[0].height = event.xexpose.height;
				nrect = 1;
				usleep(50000);
				while(XCheckTypedEvent(display, Expose, &event) && nrect < CLIPS - 1)
					{
					rectangles[nrect].x = event.xexpose.x;
					rectangles[nrect].y = event.xexpose.y;
					rectangles[nrect].width = event.xexpose.width;
					rectangles[nrect].height = event.xexpose.height;
					nrect++;
					}
				XSetClipRectangles(display, gc, 0, 0, rectangles, nrect, Unsorted);
				redraw();
				XFlush(display);
				//usleep(200000);
				break;

			case PropertyNotify:

				if(event.xproperty.atom == color_atom &&
					event.xproperty.state == PropertyNewValue)
					{
					set_color();
					}
				break;
				}
			}
		}

	XFreeGC(display, gc);
	XCloseDisplay(display);
	exit(0);
	}
