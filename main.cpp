#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/extensions/Xdbe.h>

#define BLOCK_SIZE 30

class node {
	public:
	node* parent;
	node* left;
	node* right;
	node* up;
	node* down;

	float x;
	float y;

	node()
	{
		parent = nullptr;
		left = nullptr;
		right = nullptr;
		up = nullptr;
		down = nullptr;
	}

	bool operator==(node n)
	{
		if(this->x == n.x && this->y == n.y)
			return true;
		return false;
	}

	void operator=(node n)
	{
		this->x = n.x;
		this->y = n.y;
		this->parent = n.parent;
		this->left = n.left;
		this->right = n.right;
		this->up = n.up;
		this->down = n.down;
	}

	~node()
	{
		parent = nullptr;
		left = nullptr;
		right = nullptr;
		up = nullptr;
		down = nullptr;
	}
};

// Global variables to be used
class global {
	public:
	int xres;
	int yres;
	int limit;
	float radius;
	node* map;
	node* start;
	node* end;
	node* p;
	int clear = 0;

	void init()
	{
		srand(time(NULL));
		radius = (float)BLOCK_SIZE;
		// Calculate grid size
		int ROWS = yres % BLOCK_SIZE ? (yres / BLOCK_SIZE)-1 : (yres / BLOCK_SIZE);
		int COLS = (xres / BLOCK_SIZE);

		limit = ROWS*COLS;
		// Generate the maze map
		map = new node[limit];
		float x = (float)BLOCK_SIZE / 2.0;
		float y = (float)BLOCK_SIZE / 2.0;

		// Setup node grid
		int k=0;
		for(int i=0; i<ROWS; i++) {
			x = (float)BLOCK_SIZE / 2.0;
			for(int j=0; j<COLS; j++) {
				map[k].x = x;
				map[k].y = y;
				k++;
				x += BLOCK_SIZE;
			}
			y += BLOCK_SIZE;
		}

		// Setup node neighbors
		k=0;
		int ent_it = rand() % ROWS;

		for(int i=0; i<ROWS; i++) {
			for(int j=0; j<COLS; j++) {
				if(j == 0 && i == ent_it)
					start = &map[k];
				if(j > 0)
					map[k].left = &map[k-1];
				if(j < COLS-1)
					map[k].right = &map[k+1];
				if(i > 0)
					map[k].up = &map[k-COLS];
				if(i < ROWS-1)
					map[k].down = &map[k+COLS];
				k++;
			}
		}
		p = start;
		node* cur = start;

		// Prim's algorithm to nodes
		// Start at random node in first column,
		// stop when the current node is in the last column
		while(cur->right != nullptr) {
			int chose = rand() % 4;
			switch(chose) {
				case 0:
					if(cur->left == nullptr)
						continue;
					if((cur->left)->parent == nullptr)
						(cur->left)->parent = cur;
					cur = cur->left;
					break;
				case 1:
					if(cur->right == nullptr) {
						printf("Made it\n");
						break;
					}
					if((cur->right)->parent == nullptr)
						(cur->right)->parent = cur;
					cur = cur->right;
					break;
				case 2:
					if(cur->up == nullptr)
						continue;
					if((cur->up)->parent == nullptr)
						(cur->up)->parent = cur;
					cur = cur->up;
					break;
				case 3:
					if(cur->down == nullptr)
						continue;
					if((cur->down)->parent == nullptr)
						(cur->down)->parent = cur;
					cur = cur->down;
					break;
			}
		}
		end = cur;

		// Assign parents to orphans to have more
		// maze-like display
		// Otherwise there will be a bunch of white squares &
		// maze will be easy to solve
		for(int i=0; i<limit; i++) {
			if(map[i].parent != nullptr)
				continue;
			int choose = rand() % 4;
			switch(choose) {
				case 0:
					if(map[i].left != nullptr) {
						map[i].parent = new node;
						*map[i].parent = *map[i].left;
						break;
					}
				case 1:
					if(map[i].right != nullptr) {
						map[i].parent = new node;
						*map[i].parent = *map[i].right;
						break;
					}
				case 2:
					if(map[i].up != nullptr) {
						map[i].parent = new node;
						*map[i].parent = *map[i].up;
						break;
					}
				case 3:
					if(map[i].down != nullptr) {
						map[i].parent = new node;
						*map[i].parent = *map[i].down;
						break;
					}
			}
		}
	}
} g;

class X11_wrapper {
private:
        Display *dpy;
        Window win;
        GC gc;
        XdbeBackBuffer backBuffer;
        XdbeSwapInfo swapInfo;
public:
        X11_wrapper() {
                int major, minor;
                XSetWindowAttributes attributes;
                XdbeBackBufferAttributes *backAttr;
                dpy = XOpenDisplay(NULL);
                attributes.event_mask = ExposureMask | StructureNotifyMask |
                        PointerMotionMask | ButtonPressMask |
                        ButtonReleaseMask | KeyPressMask | KeyReleaseMask;
                //Various window attributes
                attributes.backing_store = Always;
                attributes.save_under = True;
                attributes.override_redirect = False;
                attributes.background_pixel = 0x00000000;
                //Get default root window
                Window root = DefaultRootWindow(dpy);
		XWindowAttributes main;
		XGetWindowAttributes(dpy, root, &main);
		g.xres = main.width;
		g.yres = main.height;
                //Create a window
                win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
                                                    CopyFromParent, InputOutput, CopyFromParent,
                                                    CWBackingStore | CWOverrideRedirect | CWEventMask |
                                                        CWSaveUnder | CWBackPixel, &attributes);
                //Create gc
                gc = XCreateGC(dpy, win, 0, NULL);
                backBuffer = XdbeAllocateBackBufferName(dpy, win, XdbeUndefined);
                backAttr = XdbeGetBackBufferAttributes(dpy, backBuffer);
            	swapInfo.swap_window = backAttr->window;
                swapInfo.swap_action = XdbeUndefined;
                XFree(backAttr);
                //Map and raise window
                char ts[256];
                sprintf(ts, "Random Maze Generation  %i x %i", g.xres, g.yres);
                setWindowTitle(ts);
		XMapWindow(dpy, win);
                XRaiseWindow(dpy, win);
        }
        ~X11_wrapper() {
                //Do not change. Deallocate back buffer.
                if(!XdbeDeallocateBackBufferName(dpy, backBuffer)) {
                        fprintf(stderr,"Error : unable to deallocate back buffer.\n");
                }
                XFreeGC(dpy, gc);
                XDestroyWindow(dpy, win);
                XCloseDisplay(dpy);
        }
        void swapBuffers() {
                XdbeSwapBuffers(dpy, &swapInfo, 1);
                usleep(4000);
        }
        bool getPending() {
                return XPending(dpy);
        }
        void getNextEvent(XEvent *e) {
                XNextEvent(dpy, e);
        }
        void set_color_3i(int r, int g, int b) {
                unsigned long cref = 0L;
                cref += r;
                cref <<= 8;
                cref += g;
                cref <<= 8;
                cref += b;
                XSetForeground(dpy, gc, cref);
        }
        void setWindowTitle(char* ts) {
                XStoreName(dpy, win, ts);
        }
        void clear_screen() {
                //XClearWindow(dpy, backBuffer);
                XSetForeground(dpy, gc, 0x00050505);
                XFillRectangle(dpy, backBuffer, gc, 0, 0, g.xres, g.yres);
        }
        void drawString(int x, int y, const char *message) {
                XDrawString(dpy, backBuffer, gc, x, y, message, strlen(message));
        }
        void drawPoint(int x, int y) {
                XDrawPoint(dpy, backBuffer, gc, x, y);
        }
        void drawLine(int x0, int y0, int x1, int y1) {
                XDrawLine(dpy, backBuffer, gc, x0, y0, x1, y1);
        }
 	void drawRectangle(int x, int y, int w, int h) {
                XDrawRectangle(dpy, backBuffer, gc, x, y, w, h);
        }
        void fillRectangle(int x, int y, int w, int h) {
                XFillRectangle(dpy, backBuffer, gc, x, y, w, h);
        }
	void fillCircle(int x, int y, float radius) {
		XFillArc(dpy, backBuffer, gc, x, y, radius, radius, 0.0, 360.0*64);
	}
} x11;

int check_keys(XEvent*);
void check_mouse(XEvent*);
void check_resize(XEvent*);
void physics();
void render();

int main() {
	srand(time(NULL));
	g.init();
	int done = 0;
	while(!done) {
		while(x11.getPending()) {
			XEvent e;
			x11.getNextEvent(&e);
			check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
	}

	return(0);
}

void check_resize(XEvent *e)
{
        //ConfigureNotify is sent when the window is resized.
        if (e->type != ConfigureNotify)
                return;
        XConfigureEvent xce = e->xconfigure;
        g.xres = xce.width;
        g.yres = xce.height;
	char ts[256];
	sprintf(ts, "Random Maze Generation  %i x %i", g.xres, g.yres);
	x11.setWindowTitle(ts);
}

// Show highlighted path from start -> end of maze
void solve()
{
	g.clear = 1;
	// Start at end & traverse through parents to 
	// arrive at start
	node* cur = g.end->parent;
	x11.set_color_3i(255, 255, 0);
	while(!(*cur == *g.start)) {
		x11.fillRectangle(cur->x-(BLOCK_SIZE/2),
				cur->y-(BLOCK_SIZE/2),
				BLOCK_SIZE, BLOCK_SIZE);
		x11.swapBuffers();
		cur = cur->parent;
	}
}

int check_keys(XEvent *e)
{
        static int shift=0;
        int key = XLookupKeysym(&e->xkey, 0);
        if (e->type == KeyRelease) {
                if (key == XK_Shift_L || key == XK_Shift_R)
                        shift=0;
                return 0;
        }
        if (e->type == KeyPress) {
                if (key == XK_Shift_L || key == XK_Shift_R) {
                        shift=1;
                        return 0;
                }
        } else {
                return 0;
        }
        //a key was pressed
        switch (key) {
		case XK_s: {
			solve();
			char mes[] = {"Press c to clear screen & move"};
			x11.setWindowTitle(mes);
			break;
			   }
		case XK_c: {
			g.clear = 0;
			char ts[256];
			sprintf(ts, "Random Maze Generation  %i x %i", g.xres, g.yres);
			x11.setWindowTitle(ts);
			break;
			   }
		case XK_r:
			g.clear = 0;
			g.init();
			break;
		case XK_Left:
			if(g.p->left == nullptr || g.clear)
				break;
			if(*(g.p) == *((g.p->left)->parent) ||
					*(g.p->parent) == *(g.p->left)) {
				g.p = g.p->left;
			}
			break;
		case XK_Right:
			if(g.p->right == nullptr || g.clear)
				break;
			if(*(g.p) == *((g.p->right)->parent) ||
					*(g.p->parent) == *(g.p->right)) {
				g.p = g.p->right;
			}
			break;
		case XK_Up:
			if(g.p->up == nullptr || g.clear)
				break;
			if(*(g.p) == *((g.p->up)->parent) ||
					*(g.p->parent) == *(g.p->up)) {
				g.p = g.p->up;
			}
			break;
		case XK_Down:
			if(g.p->down == nullptr || g.clear)
				break;
			if(*(g.p) == *((g.p->down)->parent) ||
					*(g.p->parent) == *(g.p->down)) {
				g.p = g.p->down;
			}
			break;
		case XK_Escape:
			return(1);
			break;
	}

	return(0);
}

void check_mouse(XEvent *e)
{
        //Did the mouse move?
        //Was a mouse button clicked?
        static int savex = 0;
        static int savey = 0;
        //
        if (e->type == ButtonRelease) {
                return;
        }
        if (e->type == ButtonPress) {
                if (e->xbutton.button==1) {
                        //Left button is down
                }
                if (e->xbutton.button==3) {
                        //Right button is down
                }
        }
        if (savex != e->xbutton.x || savey != e->xbutton.y) {
                //Mouse moved
                savex = e->xbutton.x;
                savey = e->xbutton.y;
        }
}

void physics()
{
	// If player reaches end of maze,
	// generate a new maze
	if(*g.p == *g.end)
		g.init();
}

void render()
{
	if(!g.clear) {
		x11.clear_screen();
		for(int i=0; i<g.limit; i++) {
			// Draw white square at all nodes locations
			x11.set_color_3i(255, 255, 255);
			x11.fillRectangle(g.map[i].x-(BLOCK_SIZE/2),
					g.map[i].y-(BLOCK_SIZE/2),
					BLOCK_SIZE, BLOCK_SIZE);
			// If it doesn't have a parent, make node black
			if(g.map[i].parent == nullptr) {
				x11.set_color_3i(0, 0, 0);
				x11.drawRectangle(g.map[i].x-(BLOCK_SIZE/2),
						g.map[i].y-(BLOCK_SIZE/2),
						BLOCK_SIZE, BLOCK_SIZE);
				continue;
			}
			// Set current node's neighbors into array
			node* neighbor = new node[4];
			if(g.map[i].left != nullptr)
				neighbor[0] = *g.map[i].left;
			else
				memset(neighbor, '\0', sizeof(node));
			if(g.map[i].right != nullptr)
				neighbor[1] = *g.map[i].right;
			else
				memset(neighbor+1, '\0', sizeof(node));
			if(g.map[i].up != nullptr)
				neighbor[2] = *g.map[i].up;
			else
				memset(neighbor+2, '\0', sizeof(node));
			if(g.map[i].down != nullptr)
				neighbor[3] = *g.map[i].down;
			else
				memset(neighbor+3, '\0', sizeof(node));
			x11.set_color_3i(0, 0, 0);
			// Traverse through neighbors
			for(int j=0; j<4; j++) {
				if(neighbor+j == nullptr ||
						neighbor[j].parent == nullptr)
					continue;
				// Don't draw a border between child & parent
				if((neighbor[j] == *(g.map[i].parent)) ||
						(g.map[i] == *(neighbor[j].parent)))
					continue;
				// Otherwise, draw black line between neighbor & current node
				switch(j) {
					case 0:
						x11.drawLine(neighbor[j].x+(BLOCK_SIZE/2),
								neighbor[j].y-(BLOCK_SIZE/2),
								neighbor[j].x+(BLOCK_SIZE/2),
								neighbor[j].y+(BLOCK_SIZE/2));
						break;
					case 1:
						x11.drawLine(neighbor[j].x-(BLOCK_SIZE/2),
								neighbor[j].y-(BLOCK_SIZE/2),
								neighbor[j].x-(BLOCK_SIZE/2),
								neighbor[j].y+(BLOCK_SIZE/2));
						break;
					case 2:
						x11.drawLine(neighbor[j].x-(BLOCK_SIZE/2),
								neighbor[j].y+(BLOCK_SIZE/2),
								neighbor[j].x+(BLOCK_SIZE/2),
								neighbor[j].y+(BLOCK_SIZE/2));
						break;
					case 3:
						x11.drawLine(neighbor[j].x-(BLOCK_SIZE/2),
								neighbor[j].y-(BLOCK_SIZE/2),
								neighbor[j].x+(BLOCK_SIZE/2),
								neighbor[j].y-(BLOCK_SIZE/2));
						break;
				}
			}
			neighbor = nullptr;
		}
	}
	// Make start green
	x11.set_color_3i(0, 255, 0);
	x11.fillRectangle(g.start->x-(BLOCK_SIZE/2),
			g.start->y-(BLOCK_SIZE/2),
			BLOCK_SIZE, BLOCK_SIZE);
	// Make exit red
	x11.set_color_3i(255, 0, 0);
	x11.fillRectangle(g.end->x-(BLOCK_SIZE/2),
			g.end->y-(BLOCK_SIZE/2),
			BLOCK_SIZE, BLOCK_SIZE);
	// Draw the player
	x11.set_color_3i(0, 0, 255);
	x11.fillCircle(g.p->x-(BLOCK_SIZE/2),
			g.p->y-(BLOCK_SIZE/2),
			g.radius);
}
