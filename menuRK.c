// menuRK - a simple menu program for X - Copyright (C) 2004-2008 Rick Klement

// added different colors for active
// Thu Oct 20 12:19:39 PDT 2005 added dialog mode
// Sat Dec 31 16:52:03 PST 2005 show current workspace
// Fri Jan  6 09:06:18 PST 2006 added clock
// Sat Jan  7 13:35:54 PST 2006 added timeout
// Fri Apr 14 20:19:09 PDT 2006 added special menu cases
// Wed Jun 21 22:34:14 PDT 2006 fixed %s title without RicksWM
// Thu Oct 26 06:54:37 PDT 2006 added non -i arguments
// Sun Oct 29 15:47:50 PST 2006 stripped \s*#.* from list mode output
// Sun Dec 17 01:05:19 PST 2006 colors can be different for each button
// Wed Jan 10 17:35:43 PST 2007 drag and drop
// Sun Jan 28 16:43:40 PST 2007 clip drawing to reduce flicker
// Wed Feb 14 19:59:30 PST 2007 oval buttons
// Mon Feb 19 08:49:55 PST 2007 use Shape to remove background
// Thu Feb 22 19:32:50 PST 2007 timeout on leaving buttons
// Fri Feb 23 16:49:51 PST 2007 gap between columns
// Sat Feb 24 13:00:28 PST 2007 redo mouse placement
// Mon Feb 26 14:33:12 PST 2007 north east south west
// Wed Mar  7 19:29:00 PST 2007 roundend buttons
// Mon Mar 19 10:24:09 PDT 2007 use Regions to redraw on Exposes
// Mon Mar 19 10:26:01 PDT 2007 add -into winid option
// Thu Mar 22 03:10:32 PDT 2007 slide on/off for -or menus
// Thu Mar 22 20:01:36 PDT 2007 3move to move -or menus
// Fri Mar 23 20:28:14 PDT 2007 fix for no timeout when button down
// Fri Mar 23 21:06:22 PDT 2007 shrink option for -or timeouts
// Fri Jul  6 16:31:59 PDT 2007 fix fades for different depths
// Sun Nov 18 20:44:33 PST 2007 add -square option
// Sun Jan 27 18:07:56 PST 2008 convert i to pointers
// Sat Jul  5 12:59:32 PDT 2008 add explain popup window
// Thu Jul 24 13:43:11 PDT 2008 add internal scroll

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#include <X11/extensions/Xdbe.h>
#include <X11/extensions/shape.h>
#include <ctype.h>

#define DEFAULT_FONT "10x20"
#define DEFAULT_TITLE "RicksWM Menu"
#define DEFAULT_BG "black"
#define DEFAULT_FG "white"
#define DEFAULT_UL "gray85"
#define DEFAULT_LR "gray50"
#define DEFAULT_MAXCHARS 80
#define HEADER_OFFSET 12
#define LINESIZE 512
#define MINWIDTH 140
#define MAXITEMS 300
#define MAXFILES 300
#define MOVETOOFAR 20

struct allcolors
  {
  unsigned long black;
  unsigned long slantblack;
  unsigned long white;
  unsigned long white_current;
  unsigned long black_current;
  unsigned long activeblack;
  unsigned long activewhite;
  unsigned long ul;
  unsigned long lr;
  };

struct item
  {
  Window window;
  char *name;
  char *command;
  char *explaintext;
  struct allcolors colors;
  Region region;
  int width;
  int offset;
  int centermode;
  int x;
  int y;
  int i; // needed for drag/drop output
  };

struct filedata
  {
  char *filename;
  struct allcolors colors;
  int type;
  int centermode;
  };

struct colors
  {
  struct colors *next;
  char *name;
  unsigned long color;
  };

struct colorstack
  {
  struct colorstack *next;
  struct allcolors colors;
  };

static Atom string_atom;
static Atom workspace_atom;
static Colormap colormap;
static Display *display;
static Visual *visual;
static int redmask;
static int greenmask;
static int bluemask;
static Window root;
static Window into = 0;
static Window clockwindow = 0;
static XdbeBackBuffer dbewindow = 0;
static time_t newtime;
static time_t starttime;

static char *version = "$Revision: 1.65 $";
static struct item items[MAXITEMS];
static struct item *dragfrom = 0;
static struct item *dropto = 0;
static struct item *pinside = NULL;
static char *arg0;
static char *cwsname = 0;
static int cwsactive = 0;
static char *execto = 0;
static char *dropprogram = 0;
static char *explaintext = 0;
static char explainchar = 0;
static int explainvert = 0;
static int explainoval = 0;
static int explainoffset = 0;
static int explainvpad = 0;
static int explainwidth = 0;
static int explainheight = 0;
static int dragx;
static int dragy;
static int centermode = 0;  // default left justify, 1=center, 2=right
static int num = 0;
static int maxchars = DEFAULT_MAXCHARS;
static int list = 0;
static int more = 1;
static int buttonstate = 0;
static int destroyed = 0;
static int oval = 0;
static int roundend = 0;
static int dial = 0;
static int fade = 0;
static int shape = 0;
static int shadow = 0;
static int slant = 0;
static int border = 2;
static int hpad = 5;
static int hoffset = 0;  // (shadow + slant) * 2
static int dialog = 0;
static int dialoggap = 0;
static int widest = 0;
static int across = 0;
static int maxwidth = MINWIDTH;
static int dialogwidth;
static int colwidth;
static int colgap = 0;
static int fitx = 0;
static int ifitx = 0;
static int isbutton;
static int buttoncount;
static int borderfudge;
static int haveclock = 0;
static int clockactive = 1;
static int clockx = 0;
static int depress = 0;
static int square = 0;
static int height = 0;
static int unsorted = 0;
static int scroll = 0;
static int scrollheight = 0;
static Window lastwin = 0;
static GC gc;
static GC gc_rev;
static GC gc_shade;
static GC gc_clock;
static GC shapeGC;
static XGCValues shapevalues;
static Pixmap shapemask = 0;
static Pixmap itemmask = 0;
static int mainwidth;
static int mainheight;
static XSegment seg[61];
static int needseg = 1;
static double d2r;
static int timeout = 0;
static int leavetime = 0;
static int slidex = 0;
static int slidey = 0;
static int deltax = 2;
static int deltay = 2;
static int entered = 0;
static int bell = 0;
static int explain = 0;
static Window explainwin = 0;
static GC explaingc = 0;
static Pixmap explainmask;
static GC explainmaskgc = 0;
static struct allcolors colors;
static unsigned long clockwhite;
static unsigned long clockblack;
static struct colors *colorhead = 0;
static struct colorstack *stackhead = 0;

static void
dragdrop(struct item *from, struct item *to)
  {
  if(list)
    {
    printf("%d %d\n", from->i, to->i);
    fflush(stdout);
    }
  else if(more == 0 || fork() == 0)
    {
    setpgid(0, 0);
    execlp(dropprogram, "drop", from->command,
      to->command, (char *)NULL);
    exit(1);
    }
  }

static void
pushcolors(void)
  {
  struct colorstack *p;

  if((p = (struct colorstack *)malloc(sizeof(struct colorstack))))
    {
    p->next = stackhead;
    p->colors = colors;
    stackhead = p;
    }
  else
    {
    fprintf(stderr, "%s: malloc failed\n", arg0);
    exit(1);
    }
  }

static void
popcolors(void)
  {
  struct colorstack *p = stackhead;
  if(p)
    {
    colors = p->colors;
    stackhead = p->next;
    free(p);
    }
  }

static unsigned long
colorbyname(char *name)
  {
  struct colors *p;
  XColor screen_def;

  if(name == 0) return(0);

  for(p = colorhead; p; p = p->next)
    {
    if(strcmp(name, p->name) == 0)
      {
      return(p->color);
      }
    }
  p = (struct colors *)malloc(sizeof(struct colors));
  if(p == 0)
    {
    fprintf(stderr, "%s: malloc failed\n", arg0);
    exit(1);
    }
  p->next = colorhead;
  p->name = strdup(name);
  if(p->name == 0)
    {
    fprintf(stderr, "%s: malloc failed\n", arg0);
    exit(1);
    }
  colorhead = p;

  XParseColor(display, colormap, name, &screen_def);
  XAllocColor(display, colormap, &screen_def);
  p->color = screen_def.pixel;
  return(p->color);
  }

static void
drawclock(void)
  {
  struct tm *ptm;
  int mw = mainwidth/2;
  int mh = mainheight/2;
  int i;
  double angle;
  double delta;
  int radius = (mainwidth < mainheight ? mainwidth : mainheight) / 2 - 2;
  XPoint pt[3];
  double onetwenty = 3.1415926 * 2 / 3;
  XdbeSwapInfo swapi;

  if(dbewindow == 0)
    {
    dbewindow = XdbeAllocateBackBufferName(display,
      clockwindow, XdbeBackground);
    XClearWindow(display, clockwindow);
    }

  ptm = localtime(&newtime);

  if(needseg)
    {
    d2r = atan2(1.0, 0.0) / 90.0;
    for(i = 0; i < 60; i++)
      {
      angle = (double)i * 6.0 * d2r;
      delta = i % 5 ? 0.97 : 0.9;
      seg[i].x1 = mw + radius * delta * sin(angle);
      seg[i].y1 = mh - radius * delta * cos(angle);
      seg[i].x2 = mw + radius * sin(angle);
      seg[i].y2 = mh - radius * cos(angle);
      }
    needseg = 0;
    }
  angle = (double)ptm->tm_sec * 6.0 * d2r;
  seg[60].x1 = mw;
  seg[60].y1 = mh;
  seg[60].x2 = mw + radius * 0.9 * sin(angle);
  seg[60].y2 = mh - radius * 0.9 * cos(angle);
  XDrawSegments(display, dbewindow, gc_clock, seg, 61);

  angle = (double)ptm->tm_min * 6.0 * d2r;
  pt[0].x = mw + radius * 3 / 4 * sin(angle);
  pt[0].y = mh - radius * 3 / 4 * cos(angle);
  pt[1].x = mw + 6 * sin(angle + onetwenty);
  pt[1].y = mh - 6 * cos(angle + onetwenty);
  pt[2].x = mw + 6 * sin(angle - onetwenty);
  pt[2].y = mh - 6 * cos(angle - onetwenty);
  XFillPolygon(display, dbewindow, gc_clock, pt, 3,
    Nonconvex, CoordModeOrigin);

  angle = (double)(ptm->tm_hour * 60 + ptm->tm_min) / 2.0 * d2r;
  pt[0].x = mw + radius / 2 * sin(angle);
  pt[0].y = mh - radius / 2 * cos(angle);
  pt[1].x = mw + 6 * sin(angle + onetwenty);
  pt[1].y = mh - 6 * cos(angle + onetwenty);
  pt[2].x = mw + 6 * sin(angle - onetwenty);
  pt[2].y = mh - 6 * cos(angle - onetwenty);
  XFillPolygon(display, dbewindow, gc_clock, pt, 3,
    Nonconvex, CoordModeOrigin);

  swapi.swap_window = clockwindow;
  swapi.swap_action = XdbeBackground;
  XdbeSwapBuffers(display, &swapi, 1);
  }

static void
findcurrent(void)
  {
  Atom actual_atom;
  int actual_format;
  unsigned long n;
  unsigned long bytes_after;
  unsigned char *actual_property = NULL;

  if(Success == XGetWindowProperty(display, root, workspace_atom,
    0, 100, 0, string_atom, &actual_atom, &actual_format,
    &n, &bytes_after, &actual_property))
    {
    if(actual_atom != None && actual_property)
      {
      if(cwsname)
        {
        XFree(cwsname);
        }
      cwsname = (char *)actual_property;
      }
    }
  }

static int
compwindow(const void *i1, const void *i2)
  {
  struct item *pi1 = (struct item *) i1;
  struct item *pi2 = (struct item *) i2;
  return(pi1->window - pi2->window);
  }

static struct item *
find_item(Window window)
  {
  struct item it;
  struct item *pi;

  it.window = window;
  pi = bsearch(&it, items, num, sizeof(struct item), compwindow);
  return(pi && strlen(pi->name) ? pi : NULL);
  }

static int
iscurrent(struct item *pi)
  {
  return(cwsactive && cwsname != NULL && pi->name != NULL &&
    strcmp(cwsname, pi->name) == 0);
  }

static void
draw(struct item *pi)
  {
  Window window = pi->window;
  int on = pi == pinside;
  int cur = iscurrent(pi);
  XPoint points[5];
  unsigned long fadeend;
  int edge;
  int shadowfill = 0;
  int width = colwidth;
  int button = pi->command[0] != '#';
  int fslant = (slant * (height + 2 * shadow) +
    height / 2) / height;

  XSetForeground(display, gc_rev, on ? pi->colors.activewhite :
    cur ? pi->colors.white_current : pi->colors.white);
  XSetBackground(display, gc_rev, on ? pi->colors.activeblack :
    cur ? pi->colors.black_current : pi->colors.black);

  XSetForeground(display, gc, fadeend = on ? pi->colors.activeblack :
    cur ? pi->colors.black_current : pi->colors.black);
  XSetBackground(display, gc, on ? pi->colors.activewhite :
    cur ? pi->colors.white_current : pi->colors.white);

  if(dialog && button)
    {
    width = dialogwidth;
    }

  //??? XClearWindow(display, window);

  if(shadow && button)
    {
    points[0].x = 0;
    points[0].y = height + 2 * shadow;
    points[1].x = shadow;
    points[1].y = height + shadow;
    points[2].x = width - shadow;
    points[2].y = shadow;
    points[3].x = width;
    points[3].y = 0;
    points[4].x = fslant;
    points[4].y = 0;

    XSetForeground(display, gc_shade,
      on ? pi->colors.lr : pi->colors.ul);
    XFillPolygon(display, window, gc_shade,
      points, 5, Nonconvex, CoordModeOrigin);

    points[4].x = width - fslant;
    points[4].y = height + 2 * shadow;

    XSetForeground(display, gc_shade,
      on ? pi->colors.ul : pi->colors.lr);
    XFillPolygon(display, window, gc_shade,
      points, 5, Nonconvex, CoordModeOrigin);
    edge = shadow;
    }
  else
    {
    edge = 0;
    shadowfill = 2 * shadow;
    }

  points[0].x = edge + slant;
  points[0].y = edge;
  points[1].x = width - edge;
  points[1].y = edge;
  points[2].x = width - edge - slant;
  points[2].y = height + edge + shadowfill;
  points[3].x = edge;
  points[3].y = height + edge + shadowfill;

  if(fade && button)
    {
    int dx = 3;
    int dy = oval ? 3 : roundend ? 2 : 1;
    int fx = edge + slant;
    int fy = edge;
    int fwidth = width - edge - slant;
    int fheight = height + edge + shadowfill;

    int startred = pi->colors.slantblack & redmask;
    int startgreen = pi->colors.slantblack & greenmask;
    int startblue = pi->colors.slantblack & bluemask;

    int red = (fadeend & redmask) - startred;
    int green = (fadeend & greenmask) - startgreen;
    int blue = (fadeend & bluemask) - startblue;

    int xsteps = fwidth / dx;
    int ysteps = fheight / dy;
    int steps = xsteps < ysteps ? xsteps : ysteps;
    double percent;
    int j;

    if(steps > 10) steps = 10;

    for(j = steps - 1; j >= 0 && fwidth > 10 && fheight > 10; j--)
      {
      //percent = 1.0 - (double)j / (double)steps;
      percent = sqrt((double)steps*steps - j*j) / (double)steps;

      XSetForeground(display, gc,
        ((int)(red * percent + startred) & redmask) |
        ((int)(green * percent + startgreen) & greenmask) |
        ((int)(blue * percent + startblue) & bluemask));

      if(oval)
        {
        XFillArc(display, window, gc, fx, fy, fwidth, fheight, 0, 64 * 360);
        }
      else if(roundend)
        {
        XFillRectangle(display, window, gc,
          fx + fheight/2, fy,
          fwidth - fheight, fheight);
        XFillArc(display, window, gc,
          fx, fy, fheight, fheight,
          64 * 90, 64 * 180);
        XFillArc(display, window, gc,
          fx + fwidth - fheight, fy, fheight, fheight,
          64 * 270, 64 * 180);
        }
      else
        {
        XFillRectangle(display, window, gc,
            fx, fy, fwidth, fheight);
        }

      fx += dx;
      fy += dy;
      fwidth -= 2 * dx;
      fheight -= 2 * dy;
      }
    XSetForeground(display, gc, fadeend);
    }
  else if(oval && button)
    {
    XFillArc(display, window, gc, edge + slant, edge,
      width - edge - slant, height + edge + shadowfill,
      0, 64 * 360);
    }
  else if(roundend && button)
    {
    int hh = height + edge + shadowfill;
    int ww = width - edge - slant;

    XFillArc(display, window, gc,
      edge + slant, edge, hh, hh,
      64 * 90, 64 * 180);
    XFillArc(display, window, gc,
      edge + slant + ww - hh, edge, hh, hh,
      64 * 270, 64 * 180);
    XFillRectangle(display, window, gc,
      edge + slant + hh/2, edge,
      edge + slant + ww - hh, hh);
    }
  else
    {
    XFillPolygon(display, window, gc,
        points, 4, Convex, CoordModeOrigin);
    }

  on = on ? depress : 0;

  //XDrawImageString(display, window, gc_rev, (
  XDrawString(display, window, gc_rev, (
    pi->centermode == 1 ? (width - pi->width) / 2 :
    pi->centermode ? width - pi->width - hpad - shadow - slant :
    shadow + slant + hpad
    ) + on,
    pi->offset + shadow + on,
    pi->name,
    strlen(pi->name));
  }

static void
drawall(void)
  {
  int i;
  struct item *pi = items;

  for(i = 0; i < num; i++)
    {
    draw(pi++);
    }
  }

static void
addline(char *line)
  {
  char *p;
  char *pp;

  if((p = index(line, '\n')))
    {
    *p = 0;
    }
  if(num < MAXITEMS && index(line, '#'))
    {
    p = items[num].command = strdup(line);
    if(p == 0)
      {
      fprintf(stderr, "%s: malloc failed\n", arg0);
      exit(1);
      }
    items[num].name = p = index(p, '#') + 1;
    items[num].explaintext = 0;
    if(explain && explainchar > 0 && (pp = index(items[num].name, explainchar)))
      {
      items[num].explaintext = pp + 1;
      *pp = 0;
      }
    if(strlen(p) > maxchars)
      {
      p[maxchars] = 0;
      }
    items[num].colors = colors;
    items[num].centermode = centermode;
    items[num].region = 0;
    items[num].i = num;
    num++;
    }
  else if(strcmp(line, "left") == 0)
    {
    centermode = 0;
    }
  else if(strcmp(line, "center") == 0)
    {
    centermode = 1;
    }
  else if(strcmp(line, "right") == 0)
    {
    centermode = 2;
    }
  }

static void
readfile(char *filename)
  {
  FILE *fid;
  char line[LINESIZE+1];

  // open name or STDIN if no file given

  fid = strcmp(filename, "-") ? fopen(filename, "r") : stdin;
  if(fid == NULL)
    {
    sprintf(line, "open config file '%s'", filename);
    perror(line);
    exit(1);
    }
  while(num < MAXITEMS && fgets(line, LINESIZE, fid))
    {
    addline(line);
    }
  fclose(fid);
  }

void
list_or_run(char *cmd)
  {
  if(list)
    {
    char line[513];
    char *p;

    strncpy(line, cmd, 512);
    line[512] = 0; // null just in case
    p = index(line, '#');
    while(p && p > line && (*p == '#' || isspace(p[-1])))
      {
      *--p = 0;
      }
    puts(line);
    fflush(stdout);
    }
  else if(more == 0 || fork() == 0)
    {
    setpgid(0, 0);
    execl("/bin/sh", "sh", "-c", cmd, NULL);
    exit(1);
    }
  }

int
main(int argc, char *argv[])
  {
  Atom wm_delete, wm_protocols, wm_state, wm_take_focus;
  Atom wm_both_protocols[2];
  XWindowAttributes root_geo;
  Window root_ret, child_ret;
  Window mainwindow;
  Window scrollwindow;
  XSetWindowAttributes attr;
  XCharStruct overall;
  XEvent event;
  XFontStruct *font_info;
  XFontStruct *explain_font_info = 0;
  char *command_three = 0;
  char *title = DEFAULT_TITLE;
  char *font = DEFAULT_FONT;
  char *explainfont = DEFAULT_FONT;
  char *explaincolor = "yellow";
  char *explainbd = "black";
  char *workspace = "";
  char buffer[256];
  struct filedata files[MAXFILES];
  int fileargs = 0;
  int ascent, descent, direction;
  int explainascent;
  int i;
  int enter = 0;
  int screen;
  int vpad = 2;
  int ipad = 0;
  int x_ret;
  int y_ret;
  int x_w, y_w;
  int xpos, ypos;
  int rows, cols;
  int wantheight;
  int position_from_cursor = 1;
  int real_x = 0;
  int real_y = 0;
  int x3 = 0;
  int y3 = 0;
  int yscroll = 0;
  int x3offset = 0;
  int y3offset = 0;
  int threemove = 0;
  char *px;
  char *py;
  int persist = 0;
  int maxrows = MAXITEMS;
  int override = 0;
  int shrink = 0;
  int ovalshrink = 0;
  int dive = 0;
  int midscreen = 0;
  int midcursor = 0;
  int north = 0;
  int east = 0;
  int south = 0;
  int west = 0;
  int dgpad;
  unsigned int mask_r;
  char **save_argv = argv;
  int save_argc = argc;
  XSizeHints size_hints;
  XWMHints wm_hints;
  XClassHint class_hints;
  fd_set fd;
  time_t lasttime = 0;
  struct timeval tv;
  struct item *pi;

  arg0 = argv[0];

  if((display = XOpenDisplay(getenv("DISPLAY") ?
    getenv("DISPLAY") : ":0.0")) == 0)
    {
    fprintf(stderr, "XOpenDisplay failed\n");
    exit(1);
    }

  fcntl(ConnectionNumber(display), F_SETFD, 1);
  screen = DefaultScreen(display);

  visual = DefaultVisual(display, screen); // needed for different depths
  redmask = visual->red_mask;
  greenmask = visual->green_mask;
  bluemask = visual->blue_mask;

  colormap = DefaultColormap(display, screen);
  colors.white = colors.white_current = clockwhite = colorbyname(DEFAULT_FG);
  colors.black = colors.black_current = clockblack = colorbyname(DEFAULT_BG);
  colors.activewhite = colors.black;
  colors.activeblack = colors.white;
  colors.slantblack = colors.black;
  colors.ul = colorbyname(DEFAULT_UL);
  colors.lr = colorbyname(DEFAULT_LR);

  while(argc > 1)
    {
    if(argc >= 3 && strcmp(argv[1], "-f") == 0) // name of menu file
      {
      if(fileargs < MAXFILES - 1)
        {
        files[fileargs].filename = argv[2];
        files[fileargs].type = 0;
        files[fileargs].centermode = centermode;
        files[fileargs].colors = colors;
        fileargs++;
        }
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-i") == 0) // menu item
      {
      if(fileargs < MAXFILES - 1)
        {
        files[fileargs].filename = argv[2];
        files[fileargs].type = 1;
        files[fileargs].centermode = centermode;
        files[fileargs].colors = colors;
        fileargs++;
        }
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-t") == 0) // title string
      {
      title = argv[2];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-slantbg") == 0) // bkrg if slant
      {
      colors.slantblack = colorbyname(argv[2]);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 4 && strcmp(argv[1], "-active") == 0) // active colors
      {
      colors.activewhite = colorbyname(argv[2]);
      colors.activeblack = colorbyname(argv[3]);
      argc -= 3;
      argv += 3;
      }
    else if(argc >= 3 && strcmp(argv[1], "-c") == 0) // max characters in label
      {
      maxchars = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-dialoggap") == 0) // horizontal pad
      {
      dialoggap = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-gap") == 0) // between columns
      {
      colgap = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 2 && strcmp(argv[1], "-fitx") == 0) // make wide
      {
      fitx = 1;
      argc -= 1;
      argv += 1;
      }
    else if(argc >= 2 && strcmp(argv[1], "-ifitx") == 0) // make wide
      {
      ifitx = 1;
      argc -= 1;
      argv += 1;
      }
    else if(argc >= 3 && strcmp(argv[1], "-hpad") == 0) // horizontal pad
      {
      hpad = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-vpad") == 0) // vertical pad
      {
      vpad = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-ipad") == 0) // internal vert pad
      {
      ipad = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-shadow") == 0) // around buttons
      {
      shadow = (strtol(argv[2], NULL, 0));
      if(shadow < 0) shadow = 0;
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-slant") == 0) // slanted buttons
      {
      slant = (strtol(argv[2], NULL, 0));
      if(slant < 0) slant = 0;
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-depress") == 0) // shift text
      {
      depress = (strtol(argv[2], NULL, 0));
      if(depress < 0) depress = 0;
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && 
      (strcmp(argv[1], "-bg") == 0 || strcmp(argv[1], "-background") == 0))
      {
      colors.black = colors.activewhite =
        colors.slantblack = colorbyname(argv[2]);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && 
      (strcmp(argv[1], "-fg") == 0 || strcmp(argv[1], "-foreground") == 0))
      {
      colors.white = colors.activeblack = colorbyname(argv[2]);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 4 && (strcmp(argv[1], "-cws") == 0)) // current color
      {
      cwsactive = 1;
      colors.white_current = colorbyname(argv[2]);
      colors.black_current = colorbyname(argv[3]);
      argc -= 3;
      argv += 3;
      }
    else if(argc >= 3 && (strcmp(argv[1], "-ul") == 0)) // upper left color
      {
      colors.ul = colorbyname(argv[2]);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && (strcmp(argv[1], "-lr") == 0)) // lower right color
      {
      colors.lr = colorbyname(argv[2]);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-fn") == 0) // font to use
      {
      font = argv[2];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-ws") == 0) // designate workspace
      {
      workspace = argv[2];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-3") == 0) // command for b3
      {
      command_three = argv[2];
      threemove = 0;
      argc -= 2;
      argv += 2;
      }
    else if(strcmp(argv[1], "-3move") == 0) // allow button 3 to move window
      {
      threemove = 1;
      command_three = 0;
      argc -= 1;
      argv += 1;
      }
    else if(argc >= 3 && strcmp(argv[1], "-p") == 0) // persistent (B1 == B2)
      {
      persist = atoi(argv[2]);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-minw") == 0) // max rows
      {
      maxwidth = atoi(argv[2]);
      if(maxwidth < 1) maxwidth = 1;
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-timeout") == 0) // exit seconds
      {
      timeout = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-dial") == 0) // buttons in circle
      {
      dial = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-border") == 0) // window border
      {
      border = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(strcmp(argv[1], "-bell") == 0) // ring bell and show screen
      {
      bell = 100;
      argc -= 1;
      argv += 1;
      }
    else if(argc >= 3 && strcmp(argv[1], "-mr") == 0) // max rows
      {
      maxrows = atoi(argv[2]);
      if(maxrows < 1) maxrows = 1;
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-drop") == 0) // drop program
      {
      dropprogram = argv[2];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-geometry") == 0) // geometry
      {
      if((px = strpbrk(argv[2], "+-")) && (py = strpbrk(px+1, "+-")))
        {
        position_from_cursor = 0;
        real_x = atoi(px);
        if(*px == '-') real_x--;
        real_y = atoi(py);
        if(*py == '-') real_y--;
        }
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 4 && strcmp(argv[1], "-xy") == 0) // set x & y
      {
      position_from_cursor = 0;
      real_x = atoi(argv[2]);
      if(argv[2][0] == '-') real_x--;
      real_y = atoi(argv[3]);
      if(argv[3][0] == '-') real_y--;
      argc -= 3;
      argv += 3;
      }
    else if(argc >= 4 && strcmp(argv[1], "-slide") == 0) // slide x & y
      {
      slidex = atoi(argv[2]);
      slidey = atoi(argv[3]);
      argc -= 3;
      argv += 3;
      }
    else if(argc >= 3 && strcmp(argv[1], "-into") == 0) // window to add to
      {
      into = strtol(argv[2], NULL, 0);
      position_from_cursor = real_x = real_y = 0;
      argc -= 2;
      argv += 2;
      }
    else if(strcmp(argv[1], "-v") == 0 || strcmp(argv[1], "--version") == 0)
      {
      puts(version);
      exit(0);
      }
    else if(strcmp(argv[1], "-h") == 0 || strcmp(argv[1], "--help") == 0)
      {
      break; // so usage gets printed later...
      }
    else if(strcmp(argv[1], "-l") == 0) // print to stdout instead of exec
      {
      list = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-or") == 0) // set override redirect
      {
      override = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-shrink") == 0) // timeout or by shrinking away
      {
      shrink = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-ovalshrink") == 0) // timeout or by shrinking away
      {
      shrink = 1;
      ovalshrink = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-dive") == 0) // timeout or by moving off bottom
      {
      shrink = 1;
      dive = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-e") == 0) // do command at each enter
      {
      enter = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-left") == 0) // left justify labels
      {
      centermode = 0;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-center") == 0) // center labels
      {
      centermode = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-right") == 0) // right justify labels
      {
      centermode = 2;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-midscreen") == 0) // place in middle of display
      {
      midscreen = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-midcursor") == 0) // center menu around cursor
      {
      midcursor = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-north") == 0) // place in middle top
      {
      north = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-east") == 0) // place in middle left
      {
      east = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-south") == 0) // place in middle bottom
      {
      south = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-west") == 0) // place in middle right
      {
      west = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-dialog") == 0) // all buttons along bottom
      {
      dialog = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-wide") == 0) // widest that fit screen
      {
      widest = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-across") == 0) // fill buttons across
      {
      across = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-oval") == 0) // all buttons are ovals
      {
      oval = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-round") == 0) // buttons have rounded ends
      {
      roundend = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-fade") == 0) // oval buttons fade to black
      {
      fade = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-shape") == 0) // remove background from buttons
      {
      shape = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-leave") == 0) // timeout only after leave
      {
      leavetime = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-square") == 0) // force window to square
      {
      square = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-clock") == 0) // clock if not focus
      {
      haveclock = 1;
      clockwhite = colors.white;
      clockblack = colors.black;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-notice") == 0) // special case for "notice"
      {
      title = "Notice";
      dialog = 1;
      colors.activewhite = colorbyname("white");
      colors.activeblack = colorbyname("red3");
      shadow = 5;
      slant = 7;
      depress = 1;
      colors.black = colorbyname("red3");
      colors.white = colorbyname("white");
      colors.ul = colorbyname("red");
      colors.lr = colorbyname("red4");
      colors.slantblack = colorbyname("red3");
      bell = 100;
      centermode = 1;
      font = "-adobe-new century schoolbook-medium-r-normal--24-240-75-75-p-137-iso8859-1";
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-task") == 0) // special case for "task menu"
      {
      maxwidth = 100;
      centermode = 1;
      shadow = 9;
      title = "Pick a Task";
      colors.white = colorbyname("black");
      colors.black = colorbyname("gray85");
      colors.ul = colorbyname("gray95");
      ipad = 2;
      depress = 1;
      slant = 9;
      colors.slantblack = colorbyname("gray85");
      colors.activewhite = colorbyname("black");
      colors.activeblack = colorbyname("gray75");
      argc -= 1;
      argv += 1;
      }
    else if(strcasecmp(argv[1], "-3d") == 0) // special case for "3d menu"
      {
      vpad = 0;
      depress = 1;
      shadow = 6;
      colors.white = colorbyname("black");
      colors.black = colors.slantblack = colorbyname("gray90");
      colors.ul = colorbyname("white");
      colors.activewhite = colorbyname("black");
      colors.activeblack = colorbyname("gray85");
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-push") == 0) // save colors
      {
      pushcolors();
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-pop") == 0) // restore colors
      {
      popcolors();
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-explain") == 0) // explain with popup window
      {
      explain = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-explainvert") == 0) // explain popup above or below
      {
      explain = 1;
      explainvert = 1;
      argc -= 1;
      argv += 1;
      }
    else if(strcmp(argv[1], "-explainoval") == 0) // explain is oval shaped
      {
      explain = 1;
      explainoval = 1;
      argc -= 1;
      argv += 1;
      }
    else if(argc >= 3 && strcmp(argv[1], "-explainchar") == 0) // char to designate explain
      {
      explain = 1;
      explainchar = argv[2][0];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-explainfont") == 0) // explain font
      {
      explain = 1;
      explainfont = argv[2];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-explaincolor") == 0) // explain color
      {
      explain = 1;
      explaincolor = argv[2];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-explainbd") == 0) // explain bg color
      {
      explain = 1;
      explainbd = argv[2];
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-explainvpad") == 0) // explain font
      {
      explain = 1;
      explainvpad = strtol(argv[2], NULL, 0);
      argc -= 2;
      argv += 2;
      }
    else if(argc >= 3 && strcmp(argv[1], "-scroll") == 0) // internal scroll, visible buttons
      {
      scroll = strtol(argv[2], NULL, 0);
      if(scroll < 0) scroll = 0;
      argc -= 2;
      argv += 2;
      }
    else if(argv[1][0] == '-') // bad option, otherwise a command
      {
      fprintf(stderr,"%s doesn't understand option: %s  try -h for help\n",
        arg0, argv[1]);
      //break;  // let if() below catch and report it
      exit(1);
      }
    else // a command with/without a label
      {
      if(fileargs < MAXFILES - 1)
        {
        if(index(argv[1], '#'))
          {
          files[fileargs].filename = argv[1];
          }
        else
          {
          char bufferline[513];

          if(strlen(argv[1]) > 255) argv[1][255] = 0; // avoid overflow
          sprintf(bufferline, "%s #%s", argv[1], argv[1]);
          files[fileargs].filename = strdup(bufferline);
          }
        files[fileargs].type = 1;
        files[fileargs].centermode = centermode;
        files[fileargs].colors = colors;
        fileargs++;
        }
      argc -= 1;
      argv += 1;
      }
    }

  if(argc > 1)
    {
    fprintf(stderr, "usage: %s <args...>\n", arg0);
    fputs(
    "       where args are\n"
    "       -h                   print this message\n"
    "       --help               \"\n"
    "       --version            print version\n"
    "       -i cmdline           take one button info from arg\n"
    "       -f filename          take button info from file\n"
    "       -f -                 take button info from stdin(default)\n"
    "                            (multiple -f & -i may be used)\n"
    "       -fn font             use this font(default 10x20)\n"
    "       -c chars             chop strings to # of characters\n"
    "       -bg color            background color(default black)\n"
    "       -fg color            foreground color(default white)\n"
    "       -t title             window title (%s for workspace name)\n"
    "       -l                   print selections instead of running them\n"
    "       -or                  set override-redirect bit(no decoration)\n"
    "       -shrink              shrink timeout closes\n"
    "       -ovalshrink          shrink timeout closes in oval shape\n"
    "       -dive                move timeout closes off bottom\n"
    "       -e                   trigger on cursor entering item window\n"
    "       -center              center justify item labels\n"
    "       -right               right justify item labels\n"
    "       -midscreen           center menu in display\n"
    "       -midcursor           center menu around cursor\n"
    "       -north               top center menu in display\n"
    "       -east                left center menu in display\n"
    "       -south               bottom center menu in display\n"
    "       -west                right center menu in display\n"
    "       -cws fg bg           highlight current workspace name\n"
    "       -dialog              put all buttons along the bottom\n"
    "       -dialoggap pixels    gap between dialog buttons\n"
    "       -gap pixels          gap between non-dialog columns\n"
    "       -mr maxrows          limit number of rows\n"
    "       -wide                widest within screen\n"
    "       -fitx                stretch width between buttons\n"
    "       -ifitx               stretch width inside buttons\n"
    "       -across              across instead of down\n"
    "       -minw minimumwidth   minimum item width in pixels\n"
    "       -ws workspacename    initial workspace\n"
    "       -p 1                 treat button 1 like button 2 (persist)\n"
    "       -p 2                 treat button 2 like button 1 (no persist)\n"
    "       -3 command           run command for button 3\n"
    "       -3move               allow button 3 to move window\n"
    "       -ipad pixels         internal vertical pad for items\n"
    "       -vpad pixels         vertical space between items\n"
    "       -hpad pixels         horizontal space between items\n"
    "       -shadow pixels       3D bevel size\n"
    "       -slant pixels        lean for buttons\n"
    "       -slantbg color       background color if slant used\n"
    "       -ul color            upper left bevel color\n"
    "       -lr color            lower right bevel color\n"
    "       -depress pixels      amount to shift active text\n"
    "       -active color color  active foreground & background colors\n"
    "       -clock               display clock if not focused\n"
    "       -timeout seconds     time to exit\n"
    "       -leave               timeout only applies after leaving\n"
    "       -slide x y           starting offset to slide in\n"
    "       -border pixels       border width\n"
    "       -bell                ring bell\n"
    "       -oval                oval buttons\n"
    "       -round               round end buttons\n"
    "       -dial pixels         buttons in a circle of pixel radius\n"
    "       -fade                buttons fade to background color\n"
    "       -shape               remove background\n"
    "       -explain             put up flyover bubbles\n"
    "       -explainchar         flyover text separator\n"
    "       -explainoval         flyover in oval bubble\n"
    "       -explainvert         flyover above or below instead of side\n"
    "       -explainvpad         flyover padding above and below\n"
    "       -explainfont font    flyover font\n"
    "       -explaincolor color  flyover color\n"
    "       -explainbd color     flyover border color\n"
    "       -square              force window to square shape\n"
    "       -scroll n            number of buttons visible on internal scroll\n"
    "       -notice              options for a Notice window\n"
    "       -task                options for a task window\n"
    "       -3d                  options for a 3d window\n"
    "       -push                save colors for temporary change\n"
    "       -pop                 restore colors from push\n"
    "       -drop programpath    enable drag&drop to program\n"
    "       -into id             use id as parent instead of root\n"
    "       -xy x y              initial position (otherwise cursor)\n"
    "       -geometry '+x+y'     \"\n"
    "       cmdline              just like the -i option without the -i\n"
    "                            except that a cmdline without a # becomes\n"
    "                            cmdline #cmdline\n"
      , stderr);
    exit(1);
    }

  for(i = 0; i < fileargs; i++)
    {
    centermode = files[i].centermode;
    colors = files[i].colors;
    if(files[i].type)
      {
      addline(files[i].filename);
      }
    else
      {
      readfile(files[i].filename);
      }
    }

  // default if no file switch given

  if(fileargs == 0)
    {
    readfile("-");
    }

  // check for no entries

  if(num == 0)
    {
    fputs("no menu items found\n", stderr);
    exit(1);
    }
  signal(SIGCHLD, SIG_IGN); // for no zombies

  if(bell)
    {
    XBell(display, bell);
    XResetScreenSaver(display);
    }

  if((font_info = XLoadQueryFont(display, font)) == 0)
    {
    fprintf(stderr, "%s: XLoadQueryFont failed for font %s\n",
      arg0, font);
    exit(1);
    }

  // first pass on menu items

  hoffset = 2 * (shadow + slant);
  dialogwidth = 0;
  buttoncount = 0;
  for(i = 0, pi = items; i < num; i++, pi++)
    {
    XTextExtents(font_info, pi->name, strlen(pi->name),
      &direction, &ascent, &descent, &overall);
    pi->width = overall.width + depress;
    height = ascent + descent + 2 * ipad + depress;
    pi->offset = ascent + ipad;
    if(dialog && pi->command[0] != '#')
      {
      buttoncount++;
      if(dialogwidth < pi->width) dialogwidth = pi->width;
      }
    else if(maxwidth < pi->width) maxwidth = pi->width;
    }
  if(dialog)
    {
    dgpad = dialoggap * (buttoncount - 1);
    dialogwidth += 2 * hpad + hoffset;
    colwidth = dialogwidth * buttoncount + dgpad;

    if(colwidth < maxwidth + 2 * hpad + hoffset)
      colwidth = maxwidth + 2 * hpad + hoffset;

    if(buttoncount && dialogwidth < (colwidth - dgpad) / buttoncount)
      dialogwidth = (colwidth - dgpad) / buttoncount;
    }
  else
    {
    colwidth = maxwidth + 2 * hpad + hoffset;
    }

  root = RootWindow(display, screen);
  XGetWindowAttributes(display, root, &root_geo);

  if(dialog)
    {
    cols = 1;
    rows = num - buttoncount + 1;
    }
  else if(widest)
    {
    int wantwidth = override ? root_geo.width - 2 * border :
      root_geo.width * 9 / 10;
    cols = wantwidth / (colwidth + colgap);
    if(cols > num) cols = num;
    if(cols < 1) cols = 1;
    rows = (num + cols - 1) / cols;
    if(rows > 1)
      {
      cols = (num + rows - 1) / rows;
      }
    if(ifitx)
      {
      int deltah = (wantwidth - cols * colwidth) / cols / 2;
      hpad += deltah;
      colwidth += deltah * 2;
      }
    if(fitx > 0 && cols > 1)
      {
      colgap = (wantwidth - cols * colwidth) / (cols - 1);
      }
    }
  else if(scroll > 0) // forces single column
    {
    rows = num;
    cols = 1;
    if(scroll > rows) scroll = rows;
    }
  else
    {
    wantheight = root_geo.height * 7 / 8;
    rows = wantheight / (height + vpad + 2 * shadow);
    if(rows > num) rows = num;
    if(rows > maxrows) rows = maxrows;
    cols = (num + rows - 1) / rows;
    if(cols > 1)
      {
      rows = (num + cols - 1) / cols;
      }
    }

  if(dial > 0 && num > 1)
    {
    mainwidth = colwidth + 2 * dial;
    mainheight = height + 2 * shadow + 2 * dial;
    }
  else
    {
    mainwidth = colwidth * cols + colgap * (cols - 1);
    mainheight = rows * (height + 2 * shadow + vpad) - vpad;
    if(square)
      {
      int extra = abs(mainwidth - mainheight);

      if(mainwidth < mainheight)
        {
        colwidth += extra / cols;
        mainwidth = colwidth * cols + colgap * (cols - 1);
        }
      else if(mainwidth > mainheight && rows > 1)
        {
        vpad += extra / (rows - 1);
        mainheight = rows * (height + 2 * shadow + vpad) - vpad;
        }
      }
    }
  borderfudge = override ? border : 0;

  scrollheight = scroll ? (height + 2 * shadow) * scroll + vpad * (scroll - 1) : mainheight;

  if(midscreen)
    {
    real_x = (root_geo.width - mainwidth) / 2 - borderfudge;
    real_y = (root_geo.height - scrollheight) / 2 - borderfudge;
    }
  else if(north)
    {
    real_x = (root_geo.width - mainwidth) / 2 - borderfudge;
    real_y = 0;
    }
  else if(west)
    {
    real_x = 0;
    real_y = (root_geo.height - scrollheight) / 2 - borderfudge;
    }
  else if(south)
    {
    real_x = (root_geo.width - mainwidth) / 2 - borderfudge;
    real_y = root_geo.height - scrollheight - borderfudge;
    }
  else if(east)
    {
    real_x = root_geo.width - mainwidth - borderfudge;
    real_y = (root_geo.height - scrollheight) / 2 - borderfudge;
    }
  else if(position_from_cursor)
    {
    XQueryPointer(display, root, &root_ret, &child_ret, &x_ret, &y_ret,
      &x_w, &y_w, &mask_r);

    real_x = x_ret - mainwidth / 2;
    if(real_x > root_geo.width - mainwidth - 2)
      {
      real_x = root_geo.width - mainwidth - 2;
      }
    if(real_x < 0)
      {
      real_x = 0;
      }
    real_y = midcursor ? y_ret - scrollheight / 2 : y_ret;
    if(real_y + scrollheight > root_geo.height - 24)
      {
      real_y = root_geo.height - scrollheight - 24;
      }
    if(real_y < 10)
      {
      real_y = 10;
      }
    real_y -= HEADER_OFFSET;
    }
  else // defined position
    {
    if(real_x < 0)
      {
      real_x += root_geo.width + 3 - mainwidth - borderfudge;
      }
    else
      {
      real_x -= borderfudge;
      }
    if(real_y < 0)
      {
      real_y += root_geo.height + 3 - scrollheight - borderfudge;
      }
    else
      {
      real_y -= borderfudge;
      }
    }

  if(override == 0)
    {
    slidex = slidey = 0;
    }
  if(slidex > 0 && real_x + slidex > root_geo.width)
    {
    slidex = root_geo.width - real_x;
    }
  if(slidey > 0 && real_y + slidey > root_geo.height)
    {
    slidey = root_geo.height - real_y;
    }
  if(slidex < 0 && real_x + mainwidth + slidex < 0)
    {
    slidex = -real_x - mainwidth;
    }
  if(slidey < 0 && real_y + scrollheight + slidey < 0)
    {
    slidey = -real_y - scrollheight;
    }
  deltax = abs(slidex) / 16;
  if(deltax < 2) deltax = 2;
  deltay = abs(slidey) / 16;
  if(deltay < 2) deltay = 2;

  if(into == 0) into = root;

  attr.background_pixel = colors.slantblack;
  attr.border_pixel = colors.slantblack;
  attr.bit_gravity = StaticGravity;
  attr.override_redirect = override ? True : False;
  attr.event_mask = ButtonPressMask | ButtonReleaseMask |
    FocusChangeMask | StructureNotifyMask | Button3MotionMask;

  mainwindow = XCreateWindow(display, into,
    real_x + slidex, real_y + slidey,
    mainwidth, scrollheight, border,
    CopyFromParent, InputOutput, CopyFromParent,
    CWBackPixel | CWBitGravity | CWOverrideRedirect |
    CWEventMask | CWBorderPixel, &attr);

  if(shape)
    {
    shapemask = XCreatePixmap(display, mainwindow, mainwidth, mainheight, 1);
    shapevalues.foreground = 0;
    shapeGC = XCreateGC(display, shapemask, GCForeground, &shapevalues);
    XFillRectangle(display, shapemask, shapeGC,
      0, 0, mainwidth, mainheight);
    shapevalues.foreground = 1;
    XChangeGC(display, shapeGC, GCForeground, &shapevalues);
    }

  wm_delete = XInternAtom(display, "WM_DELETE_WINDOW", False);
  wm_take_focus = XInternAtom(display, "WM_TAKE_FOCUS", False);
  wm_protocols = XInternAtom(display, "WM_PROTOCOLS", False);
  wm_state = XInternAtom(display, "WM_STATE", False);
  workspace_atom = XInternAtom(display, "_RICKSWM_WORKSPACE", False);
  string_atom = XInternAtom(display, "STRING", False);

  if(cwsactive || strchr(title, '%'))
    {
    XSelectInput(display, root, PropertyChangeMask);
    findcurrent();
    }

  size_hints.flags = PMinSize | PMaxSize | PBaseSize | PResizeInc;
  size_hints.min_width = mainwidth;
  size_hints.min_height = scrollheight;
  size_hints.max_width = mainwidth;
  size_hints.max_height = mainheight;
  size_hints.base_width = mainwidth;
  size_hints.base_height = scrollheight;
  size_hints.width_inc = colwidth;
  size_hints.height_inc = height + 2 * shadow + vpad;

  wm_hints.flags = InputHint | StateHint;
  wm_hints.input = True;
  wm_hints.initial_state = NormalState;

  class_hints.res_name = arg0;
  class_hints.res_class = "menuRK";

  XSetWMProperties(display, mainwindow, 0, 0, save_argv, save_argc,
    &size_hints, &wm_hints, &class_hints);
    
  wm_both_protocols[0] = wm_delete;
  wm_both_protocols[1] = wm_take_focus;
  XSetWMProtocols(display, mainwindow, wm_both_protocols, 2);

  XStoreName(display, mainwindow, title);
  if(strchr(title, '%'))
    {
    sprintf(buffer, title, cwsname ? cwsname : "");
    XStoreName(display, mainwindow, buffer);
    }
  else
    {
    XStoreName(display, mainwindow, title);
    }

  gc = XCreateGC(display, into, 0, NULL);
  gc_rev = XCreateGC(display, into, 0, NULL);
  gc_shade = XCreateGC(display, into, 0, NULL);
  XSetFont(display, gc, font_info->fid);
  XSetFont(display, gc_rev, font_info->fid);
  if(scroll)
    {
    attr.background_pixel = colors.slantblack;
    attr.event_mask = ButtonPressMask | ButtonReleaseMask | Button3MotionMask;
    scrollwindow = XCreateWindow(display, mainwindow,
      0, 0, mainwidth, mainheight, 0,
      CopyFromParent, InputOutput, CopyFromParent,
      CWEventMask | CWBackPixel, &attr);
    }
  else
    {
    scrollwindow = mainwindow;
    }

  // second pass placing items in subwindows

  xpos = 0;
  ypos = 0;
  for(i = 0, pi = items; i < num; i++, pi++)
    {
    isbutton = dialog && pi->command[0] != '#';
    if(dial)
      {
      double angle = 2 * 3.1415926 * (double)i / (double)num;
      xpos = mainwidth / 2 - colwidth / 2 + dial * sin(angle);
      ypos = mainheight / 2 - (height + 2 * shadow) / 2 - dial * cos(angle);
      }
    else if(!dialog && i > 0 && (across ? (i % cols) == 0 : (i % rows) == 0))
      {
      if(across)
        {
        xpos = 0;
        ypos += height + vpad + 2 * shadow;  // move down
        }
      else
        {
        xpos += colwidth + colgap;
        ypos = 0;
        }
      }
    pi->x = xpos;
    pi->y = ypos;

    pi->window = XCreateSimpleWindow(display, scrollwindow,
      xpos, ypos,
      isbutton ? dialogwidth : colwidth,
      height + 2 * shadow,
      0, pi->colors.slantblack, pi->colors.slantblack);

    if(pi->window < lastwin) unsorted++;
    lastwin = pi->window;

    if(shape)
      {
      if(oval)
        {
        XFillArc(display, shapemask, shapeGC, xpos, ypos,
          isbutton ? dialogwidth : colwidth,
          height + 2 * shadow,
          0, 64 * 360);
        }
      else if(roundend)
        {
        int ww = isbutton ? dialogwidth : colwidth;
        int hh = height + 2 * shadow;

        XFillRectangle(display, shapemask, shapeGC,
          xpos + hh/2, ypos, ww - hh + 1, hh);
        XFillArc(display, shapemask, shapeGC,
          xpos, ypos, hh, hh,
          64 * 90, 64 * 180);
        XFillArc(display, shapemask, shapeGC,
          xpos + ww - hh, ypos, hh, hh,
          64 * 270, 64 * 180);
        }
      else if(slant)
        {
        XPoint points[5];
        int fslant = (slant * (height + 2 * shadow) +
          height / 2) / height;

        points[0].x = xpos + fslant;
        points[0].y = ypos;
        points[1].x = xpos + (isbutton ? dialogwidth : colwidth);
        points[1].y = ypos;
        points[2].x = xpos - fslant + (isbutton ? dialogwidth : colwidth);
        points[2].y = ypos + height + 2 * shadow;
        points[3].x = xpos;
        points[3].y = ypos + height + 2 * shadow;

        XFillPolygon(display, shapemask, shapeGC,
            points, 4, Convex, CoordModeOrigin);
        }
      else
        {
        XFillRectangle(display, shapemask, shapeGC, xpos, ypos,
          isbutton ? dialogwidth : colwidth,
          height + 2 * shadow);
        }

      if(dial > 0)
        {
        if(itemmask == 0)
          {
          itemmask = XCreatePixmap(display, pi->window,
            colwidth, height + 2 * shadow, 1);
          shapevalues.foreground = 0;
          XChangeGC(display, shapeGC, GCForeground, &shapevalues);
          XFillRectangle(display, itemmask, shapeGC,
            0, 0, colwidth, height + 2 * shadow);
          shapevalues.foreground = 1;
          XChangeGC(display, shapeGC, GCForeground, &shapevalues);

          if(oval)
            {
            XFillArc(display, itemmask, shapeGC, 0, 0,
              isbutton ? dialogwidth : colwidth,
              height + 2 * shadow,
              0, 64 * 360);
            }
          else if(slant)
            {
            XPoint points[5];
            int fslant = (slant * (height + 2 * shadow) +
              height / 2) / height;

            points[0].x = 0 + fslant;
            points[0].y = 0;
            points[1].x = 0 + (isbutton ? dialogwidth : colwidth);
            points[1].y = 0;
            points[2].x = 0 - fslant + (isbutton ? dialogwidth : colwidth);
            points[2].y = 0 + height + 2 * shadow;
            points[3].x = 0;
            points[3].y = 0 + height + 2 * shadow;

            XFillPolygon(display, itemmask, shapeGC,
                points, 4, Convex, CoordModeOrigin);
            }
          else
            {
            XFillRectangle(display, itemmask, shapeGC, 0, 0,
              isbutton ? dialogwidth : colwidth,
              height + 2 * shadow);
            }

          }
        XShapeCombineMask(display, pi->window, ShapeBounding,
          0, 0, itemmask, ShapeSet);
        }
      }

    // separator line only gets Exposure, others get more

    XSelectInput(display, pi->window,
      (pi->command[0] == '#' ?
        ExposureMask :
        dropprogram ?
        ExposureMask | ButtonPressMask | ButtonReleaseMask |
          EnterWindowMask | LeaveWindowMask |
          Button1MotionMask | Button2MotionMask :
        ExposureMask | ButtonPressMask | ButtonReleaseMask |
          EnterWindowMask | LeaveWindowMask) |
      (threemove || scroll > 0 ? Button3MotionMask : 0)
      );
    if(isbutton)
      {
      xpos += dialogwidth + dialoggap;   // move across
      }
    else if(across)
      {
      xpos += colwidth + colgap;
      }
    else
      {
      ypos += height + vpad + 2 * shadow;  // move down
      }
    }

  if(strlen(workspace))
    {
    XChangeProperty(display, mainwindow, workspace_atom, string_atom,
      8, PropModeReplace, (unsigned char *)workspace, strlen(workspace));
    }

  if(shape)
    {
    XShapeCombineMask(display, mainwindow, ShapeBounding, 0, 0,
      shapemask, ShapeSet);
    }

  // only sort if necessary
  if(unsorted) qsort(items, num, sizeof(struct item), compwindow);

  XMapSubwindows(display, scrollwindow);
  if(scrollwindow != mainwindow) XMapWindow(display, scrollwindow);
  XMapWindow(display, mainwindow);

  if(explain)
    {
    attr.background_pixel = colorbyname(explaincolor);
    attr.border_pixel = colorbyname(explainbd);
    attr.bit_gravity = StaticGravity;
    attr.override_redirect = True;
    attr.event_mask = ExposureMask;

    explainwin = XCreateWindow(display, root,
      0, 0, 200, 14, 2,
      CopyFromParent, InputOutput, CopyFromParent, CWBackPixel | CWBitGravity |
      CWOverrideRedirect | CWEventMask | CWBorderPixel, &attr);

    explaingc = XCreateGC(display, explainwin, 0, NULL);
    XSetForeground(display, explaingc, colorbyname("black"));
    XSetBackground(display, explaingc, colorbyname("yellow"));
    if((explain_font_info = XLoadQueryFont(display, explainfont)) == 0)
      {
      fprintf(stderr, "%s: XLoadQueryFont failed for font fixed\n", arg0);
      exit(1);
      }
    XSetFont(display, explaingc, explain_font_info->fid);
    XSetLineAttributes(display, explaingc, 5, LineSolid, CapNotLast, JoinRound);
    if(explainoval)
      {
      explainmask = XCreatePixmap(display, explainwin, root_geo.width, root_geo.height, 1);
      explainmaskgc = XCreateGC(display, explainmask, 0, NULL);
      }
    }

  if(haveclock)
    {
    clockx = 2 * mainwidth;
    clockwindow = XCreateSimpleWindow(display, mainwindow,
      clockx, 0, mainwidth, mainheight, 0, 0, clockblack);
    XSelectInput(display, clockwindow, ExposureMask);
    clockactive = 1;

    gc_clock = XCreateGC(display, clockwindow, 0, NULL);
    XSetForeground(display, gc_clock, clockwhite);
    XSetBackground(display, gc_clock, clockblack);
    }

  starttime = time(&newtime);

  while(more)  /////////////////////////////////////////// event loop
    {
    if((timeout == 0 && clockwindow == 0 && slidex == 0 && slidey == 0) ||
      QLength(display) > 0)
      {
      XNextEvent(display, &event);
      }
    else
      {
      XFlush(display);
      FD_ZERO(&fd);
      FD_SET(ConnectionNumber(display), &fd);
      event.type = LASTEvent;
      tv.tv_sec = 0;
      tv.tv_usec = clockactive ? clockx ? 25000 : 100000 : 500000;
      if(slidex || slidey) tv.tv_usec = 20000;
      if(select(ConnectionNumber(display) + 1, &fd, NULL, NULL, &tv) > 0)
        {
        XNextEvent(display, &event);
        }
      }

    switch(event.type)
      {

    case LASTEvent:  // update on timeout

      time(&newtime);
      if(clockwindow)
        {
        if(clockactive && clockx > 0)
          {
          clockx -= 8;
          if(clockx < 0) clockx = 0;
          XMoveWindow(display, clockwindow, clockx, 0);
          }
        if(lasttime != newtime)
          {
          lasttime = newtime;
          drawclock();
          }
        }
      if(leavetime && entered)
        {
        starttime = newtime;
        }
      else if(buttonstate == 0 && timeout > 0 &&
        newtime - starttime >= timeout)
        {
        if(override && dive)
          {
          int delta = 1;

          while(real_y < root_geo.height)
            {
            XMoveWindow(display, mainwindow, real_x, real_y);
            XFlush(display);
            usleep(1000);
            delta += 1 + delta / 2;
            real_y += delta;
            }
          }
        else if(override && shrink)
          {
          int xx = 0;
          int yy = 0;
          int ww = mainwidth;
          int hh = mainheight;
          int deltax = mainwidth / 100;
          int deltay = mainheight / 100;
          if(deltax < 1) deltax = 1;
          if(deltay < 1) deltay = 1;
          if(shapemask == 0)
            {
            shapemask = XCreatePixmap(display, mainwindow, ww, hh, 1);
            shapeGC = XCreateGC(display, shapemask, 0, &shapevalues);
            }

#ifdef SHOOT
          if(1)
            {
            int ii;
            int diameter = (ww < hh ? ww : hh) / 4;

            XSetForeground(display, shapeGC, 1);
            XFillRectangle(display, shapemask, shapeGC,
              0, 0, mainwidth, mainheight);
            XSetForeground(display, shapeGC, 0);
            srandom(time(NULL));
            for(ii = 0; ii < 50; ii++)
              {
              XFillArc(display, shapemask, shapeGC,
                random() % ww, random() % hh,
                diameter, diameter, 0, 360*64);

              XShapeCombineMask(display, window, ShapeBounding,
                0, 0, shapemask, ShapeIntersect);
              XFlush(display);
              }
            }
#endif

          while(ww > 0 && hh > 0)
            {
            XSetForeground(display, shapeGC, 0);
            XFillRectangle(display, shapemask, shapeGC,
              0, 0, mainwidth, mainheight);

            XSetForeground(display, shapeGC, 1);
            if(ovalshrink)
              {
              XFillArc(display, shapemask, shapeGC, xx, yy, ww, hh, 0, 360*64);
              }
            else
              {
              XFillRectangle(display, shapemask, shapeGC, xx, yy, ww, hh);
              }

            XShapeCombineMask(display, mainwindow, ShapeBounding,
              0, 0, shapemask, ShapeIntersect);
            XFlush(display);

            xx += deltax;
            yy += deltay;
            ww -= 2 * deltax;
            hh -= 2 * deltay;
            }
          XFreeGC(display, shapeGC);
          XFreePixmap(display, shapemask);
          }
        more = 0;  // the timeout exit...
        }
      if(slidex || slidey)
        {
        if(slidex > 0)
          {
          slidex -= deltax;
          if(slidex < 0) slidex = 0;
          }
        else
          {
          slidex += deltax;
          if(slidex > 0) slidex = 0;
          }
        if(slidey > 0)
          {
          slidey -= deltay;
          if(slidey < 0) slidey = 0;
          }
        else
          {
          slidey += deltay;
          if(slidey > 0) slidey = 0;
          }
        XMoveWindow(display, mainwindow, real_x + slidex, real_y + slidey);
        }
      break;

    case EnterNotify:

      entered = 1;
      if(leavetime) time(&starttime);

      pinside = find_item(event.xcrossing.window);
      if(pinside)
        {
        draw(pinside);
        if(enter)
          {
          list_or_run(pinside->command);
          }
        if(explainwin)
          {
          int direction, descent;

          explaintext = pinside->explaintext ? pinside->explaintext : pinside->name;
          XTextExtents(explain_font_info, explaintext, strlen(explaintext),
            &direction, &explainascent, &descent, &overall);
          explainwidth = overall.width + 20;
          explainoffset = 10;
          if(explainwidth < 100)
            {
            explainoffset = (100 - explainwidth) / 2 + 10;
            explainwidth = 100;
            }
          explainheight = ascent + descent + 4 + 2 * explainvpad;
          if(explainvert)
            {
            int x = event.xcrossing.x_root - event.xcrossing.x +
              colwidth / 2 - explainwidth / 2;

            if(x < 2)
              {
              x = 2;
              }
            else if(x > root_geo.width - explainwidth - 6)
              {
              x = root_geo.width - explainwidth - 6;
              }
            if(event.xcrossing.y_root < root_geo.height / 2)
              {
              XMoveResizeWindow(display, explainwin,
                x,
                event.xcrossing.y_root - event.xcrossing.y + height + 2 * shadow,
                explainwidth, explainheight);
              }
            else
              {
              XMoveResizeWindow(display, explainwin,
                x,
                event.xcrossing.y_root - event.xcrossing.y - explainheight - 4,
                explainwidth, explainheight);
              }
            }
          else if(event.xcrossing.x_root < root_geo.width / 2)
            {
            XMoveResizeWindow(display, explainwin,
              event.xcrossing.x_root - event.xcrossing.x + colwidth,
              event.xcrossing.y_root - event.xcrossing.y +
                (height + shadow - explainheight) / 2 - 3,
              explainwidth, explainheight);
            }
          else
            {
            XMoveResizeWindow(display, explainwin,
              event.xcrossing.x_root - event.xcrossing.x - explainwidth - 4,
              event.xcrossing.y_root - event.xcrossing.y +
                (height + shadow - explainheight) / 2 - 3,
              explainwidth, explainheight);
            }
          if(explainoval)
            {
            XSetForeground(display, explainmaskgc, 0);
            XFillRectangle(display, explainmask, explainmaskgc,
              0, 0, root_geo.width, root_geo.height);
            XSetForeground(display, explainmaskgc, 1);
            XFillArc(display, explainmask, explainmaskgc,
              0, 0, explainwidth, explainheight, 0, 360 * 64);
            XShapeCombineMask(display, explainwin, ShapeBounding, 0, 0,
              explainmask, ShapeSet);
            }
          XRaiseWindow(display, explainwin);
          XMapWindow(display, explainwin);
          }
        }
      break;

    case LeaveNotify:

      entered = 0;
      if(leavetime) time(&starttime);

      if(find_item(event.xcrossing.window) == NULL) break;
      if(pinside)
        {
        struct item *pi = pinside;
        pinside = NULL;
        draw(pi);
        if(explainwin) XUnmapWindow(display, explainwin);
        }
      break;

    case Expose:

      if(clockwindow && event.xexpose.window == clockwindow)
        {
        if(event.xexpose.count == 0)
          {
          drawclock();
          }
        }
      else if(explainwin && event.xexpose.window == explainwin)
        {
        if(explainoval)
          {
          XSetForeground(display, explaingc, colorbyname(explainbd));
          XDrawArc(display, explainwin, explaingc, 0, 0, explainwidth, explainheight,
          0, 360 * 64);
          }
        XDrawString(display, explainwin, explaingc, 2 + explainoffset,
          2 + explainascent + explainvpad,
          explaintext, strlen(explaintext));
        }
      else
        {
        struct item *pi = find_item(event.xexpose.window);
        if(pi)
          {
          XRectangle cliprectangle;

          if(pi->region == 0)
            {
            pi->region = XCreateRegion();
            }

          cliprectangle.x = event.xexpose.x;
          cliprectangle.y = event.xexpose.y;
          cliprectangle.width = event.xexpose.width;
          cliprectangle.height = event.xexpose.height;
          XUnionRectWithRegion(&cliprectangle, pi->region,
            pi->region);

          if(event.xexpose.count == 0)
            {
            if(pi->region)
              {
              XSetRegion(display, gc, pi->region);
              XSetRegion(display, gc_rev, pi->region);
              XSetRegion(display, gc_shade, pi->region);
              }
            draw(pi);
            if(pi->region)
              {
              XDestroyRegion(pi->region);
              pi->region = 0;
              XSetClipMask(display, gc, None);
              XSetClipMask(display, gc_rev, None);
              XSetClipMask(display, gc_shade, None);
              }
            }
          }
        }
      break;

    case ButtonPress:

      buttonstate = 1;
      if(event.xbutton.button == 3)
        {
        if(scroll)
          {
          y3 = event.xbutton.y_root;
          }
        else
          {
          x3 = event.xbutton.x_root;
          y3 = event.xbutton.y_root;
          x3offset = event.xbutton.x_root - real_x;
          y3offset = event.xbutton.y_root - real_y;
          }
        }
      else if(pinside && dropprogram)
        {
        dragfrom = pinside;
        XRaiseWindow(display, pinside->window);
        dragx = event.xbutton.x_root;
        dragy = event.xbutton.y_root;
        }
      break;

    case MotionNotify:

      if(dragfrom)
        {
        XMoveWindow(display, dragfrom->window,
          dragfrom->x + event.xmotion.x_root - dragx,
          dragfrom->y + event.xmotion.y_root - dragy);
        }
      else if(scroll > 0 && (event.xmotion.state & Button3Mask))
        {
        yscroll += event.xmotion.y_root - y3;
        if(yscroll > 0) yscroll = 0;
        if(yscroll < scrollheight - mainheight) yscroll = scrollheight - mainheight;
        XMoveWindow(display, scrollwindow, 0, yscroll);
        y3 = event.xmotion.y_root;
        }
      else if(override && threemove &&
        (event.xmotion.state & Button3Mask))
        {
        while(XCheckTypedEvent(display, MotionNotify, &event));
        XMoveWindow(display, mainwindow,
          real_x = event.xmotion.x_root - x3offset,
          real_y = event.xmotion.y_root - y3offset);
        }
      break;

    case ButtonRelease:

      buttonstate = 0;
      if(event.xbutton.button == 1 && persist == 1)
        {
        event.xbutton.button = 2;
        }
      else if(event.xbutton.button == 2 && persist == 2)
        {
        event.xbutton.button = 1;
        }

      if(scroll > 0 && event.xbutton.button == 3)
        {
        int offset = -yscroll % (height + 2 * shadow + vpad);

        if( offset < (height + 2 * shadow + vpad) / 2)
          {
          yscroll += offset;
          }
        else
          {
          yscroll += offset - (height + 2 * shadow + vpad);
          }
        XMoveWindow(display, scrollwindow, 0, yscroll);
        }
      else if(event.xbutton.window == mainwindow || event.xbutton.button == 3)
        {
        if(event.xbutton.button == 3 && command_three &&
          abs(event.xbutton.x_root - x3) < MOVETOOFAR &&
          abs(event.xbutton.y_root - y3) < MOVETOOFAR)
          {
          list_or_run(command_three);
          }
        }
      else if(dragfrom)
        {
        Window rootwin, childwin;
        int rx, ry, wx, wy;
        unsigned int mask;

        XMoveWindow(display, dragfrom->window,
          dragfrom->x, dragfrom->y);

        if(XQueryPointer(display, mainwindow, &rootwin, &childwin,
          &rx, &ry, &wx, &wy, &mask))
          {
          struct item *pi = find_item(childwin);
          if(pi)
            {
            if(pi == dragfrom)
              {
              if(event.xbutton.button == 1)  // single shot
                {
                more = 0;
                execto = pinside->command;
                }
              else
                {
                list_or_run(pinside->command);
                }
              }
            else if(event.xbutton.button == 1)  // single shot
              {
              more = 0;
              dropto = pi;
              }
            else
              {
              dragdrop(dragfrom, pi);
              dragfrom = NULL;
              }
            }
          }
        }
      else if(pinside)
        {
        if(event.xbutton.button == 1)  // single shot
          {
          more = 0;
          execto = pinside->command;
          }
        else
          {
          list_or_run(pinside->command);
          }
        }
      break;

    case PropertyNotify:

      if(event.xproperty.atom == workspace_atom &&
        event.xproperty.state == PropertyNewValue)
        {
        findcurrent();
        if(cwsactive) drawall();
        if(strchr(title, '%'))
          {
          sprintf(buffer, title, cwsname);
          XStoreName(display, mainwindow, buffer);
          }
        }
      break;

    case FocusIn:

      if(clockwindow)
        {
        XUnmapWindow(display, clockwindow);
        clockactive = 0;
        clockx = mainwidth + 10;
        XMoveWindow(display, clockwindow, clockx, 0);
        }
      break;

    case FocusOut:

      if(clockwindow)
        {
        clockactive = 1;
        XMapWindow(display, clockwindow);
        }
      break;

    case ConfigureNotify:

      if(scroll && mainwindow == event.xconfigure.window &&
        scrollheight != event.xconfigure.height)
        {
        scrollheight = event.xconfigure.height;
        scroll = (scrollheight - height - 2 * shadow) / (height + 2 * shadow + vpad);
        if(yscroll < scrollheight - mainheight)
          {
          yscroll = scrollheight - mainheight;
          XMoveWindow(display, scrollwindow, 0, yscroll);
          }
        }
      break;

    case ClientMessage:

      if(event.xclient.window == mainwindow &&
        event.xclient.message_type == wm_protocols &&
        event.xclient.format == 32 &&
        event.xclient.data.l[0] == wm_delete)
        {
        more = 0;
        }
      break;

    case DestroyNotify:

      more = 0;
      destroyed = 1;
      break;
      }
    }
  if(destroyed == 0) XUnmapWindow(display, mainwindow);
  XFreeFont(display, font_info);
  XFreeGC(display, gc);
  XFreeGC(display, gc_rev);
  XFreeGC(display, gc_shade);
  if(haveclock) XFreeGC(display, gc_clock);
  XCloseDisplay(display);

  if(execto)
    {
    list_or_run(execto);
    }
  else if(dropto)
    {
    dragdrop(dragfrom, dropto);
    }
  exit(0);
  }
