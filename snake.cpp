/*
CS 349 A1 - Snake
Name: Weilyu Wang
ID: 20631080

- - - - - - - - - - - - - - - - - - - - - -

Commands to compile and run:

    g++ -o snake snake.cpp -L/usr/X11R6/lib -lX11 -lstdc++
    ./snake

Note: the -L option and -lstdc++ may not be needed on some machines.
*/

#include <sstream>
#include <iostream>
#include <list>
#include <cstdlib>
#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <unistd.h>
#include <vector>

/*
 * Header files for X functions
 */
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using namespace std;

/*
 * Global game state variables
 */
const int Border = 1;
const int BufferSize = 10;
const int FPS = 30;
const int width = 800;
const int height = 600;
const int blockSize = 10;

/*
 * Information to draw on the window.
 */
struct XInfo {
	Display	 *display;
	int		 screen;
	Window	 window;
	GC		 gc[8];
	int		width;		// size of window
	int		height;
  bool  pause;
  Pixmap pixmap; //double buffer
};


/*
 * Function to put out a message on error exits.
 */
void error( string str ) {
  cerr << str << endl;
  exit(0);
}

string toString(int n) {
  stringstream ss;
  ss << n;
  string str = ss.str();
  return str;
}

int myFPS = 30;
int SPEED = 5;
int score = 0;
int level = 1;
bool win = false;
bool splash = true;
bool endGame = false;

/*
 * An abstract class representing displayable things.
 */
class Displayable {
	public:
		virtual void paint(XInfo &xinfo) = 0;
};

class Body {
  public:
    int x;
    int y;
    Body(int const &x, int const &y) {
      this->x = x;
      this->y = y;
    }

    ~Body() {}
};

class Fruit;

class Snake;

class ScoreBoard : public Displayable {
  public:
    virtual void paint(XInfo &xinfo) {
      if(!splash) {
        string text = "";
        text = "SCORE: " + toString(score) + "     LEVEL: " + toString(level) +
               "     SPEED: " + toString(SPEED) + "     FPS: " + toString(FPS);
        XDrawImageString(xinfo.display, xinfo.pixmap, xinfo.gc[3], 10, 20, text.c_str(), text.length());
        XDrawImageString(xinfo.display, xinfo.pixmap, xinfo.gc[3], 12, height-28, "Press 'p' to start/pause", 24);
        XDrawImageString(xinfo.display, xinfo.pixmap, xinfo.gc[3], 12, height-10, "Press 'q' to quit", 17);
      }
    }
    ScoreBoard() {}
};


class SplashScreen : public Displayable {
  public:
    virtual void paint(XInfo &xinfo) {
      if(splash) {
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[5], 0, 0, width, height);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[2], 70, 40, 680, 500);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[4], 80, 50, 660, 480);

        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 100, "Name: Weilyu Wang", 17);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 130, "Student #: 20631080", 19);

        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 180, "Enhancements:", 13);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 210, "After eating certain number of fruits (the red apples),", 55);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 240, "the game level will be automactically increased,", 48);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 270, "and the speed of the snake will be correspondingly increased.", 61);

        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 320, "Instructions", 12);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 350, "Press 'P' to start or pause", 27);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 380, "Press 'Q' to quit", 17);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 410, "press 'R' to restart the game.", 30);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 440, "Press arrow keys or A,W,D,S to move left, up, right or down", 59);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[7], 200 , 470, "Press any key to get into the game", 34);
      }
      if(!win && endGame) {
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[5], 200, 200, 400, 200);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[4], 220, 220, 360, 160);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[6], 260 , 260, "GG, YOU LOSE.", 13);
        if(score <= 10) {
          XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[6], 260 , 285, "YOU LOSE SO FAST LMAO", 21);
        }
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[6], 260 , 325, "TO RESTART GAME, PRESS R", 24);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[6], 260 , 350, "TO LEAVE GAME, PRESS Q", 22);
      }
      if(win && endGame) {
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[5], 200, 200, 400, 200);
        XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[4], 220, 220, 360, 160);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[6], 260 , 280, "GOOD GAME WELL PLAYED !", 23);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[6], 260 , 325, "TO RESTART GAME, PRESS R", 24);
        XDrawString(xinfo.display, xinfo.pixmap, xinfo.gc[6], 260 , 350, "TO LEAVE GAME, PRESS Q", 22);
      }
    }
    SplashScreen() {}
};


class Snake : public Displayable {
	public:
    vector<Body> snakeBody; //need to make it private

		virtual void paint(XInfo &xinfo) {
      if(!splash) {
        for(int i = 0; i < snakeBody.size(); i++) {
          XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[0],
            snakeBody[i].x - 0.5*SPEED, snakeBody[i].y - 0.5*SPEED, blockSize, blockSize);
        }
      }
    }


    Snake(int x, int y): x0(x), y0(y) {
      length = 50;
      preDirection = 0;
      curDirection = 0;
      for(int i = 0; i < length/SPEED; i++) { ////
        snakeBody.push_back(Body(x0 - i * SPEED, y0));
      }
		}

    void addHead() {
      switch (curDirection) {
        case 2: //left
          snakeBody.insert(snakeBody.begin(), Body(snakeBody[0].x-SPEED, snakeBody[0].y));
          break;
        case 0: //right
          snakeBody.insert(snakeBody.begin(), Body(snakeBody[0].x+SPEED, snakeBody[0].y));
          break;
        case 1:  //up
          snakeBody.insert(snakeBody.begin(), Body(snakeBody[0].x, snakeBody[0].y-SPEED));
          break;
        case 3:  //down
          snakeBody.insert(snakeBody.begin(), Body(snakeBody[0].x, snakeBody[0].y+SPEED));
          break;
      }
    }

    void removeTail() {
      snakeBody.pop_back();
    }

		void move(XInfo &xinfo) {
      checkHitObstacle();
      checkEatFruit();
      addHead();
      removeTail();
    }

		int getX() {
			return x0;
		}

		int getY() {
			return y0;
		}

    ~Snake() {}

    void checkEatFruit();

    bool checkHitItself(int x, int y) {
      int j = ceil(blockSize/SPEED);
      for(int i = j; i < snakeBody.size(); i++) {
        if(((x < snakeBody[i].x + 0.5 * 0.5*SPEED) && (x > snakeBody[i].x - 1.5 * 0.5*SPEED)) &&
           ((y < snakeBody[i].y + 0.5 * 0.5*SPEED) && (y > snakeBody[i].y - 1.5 * 0.5*SPEED))) {
             return true;
        }
      }
      return false;
    }


    void checkHitObstacle() {
      if(snakeBody[0].x <= 0 || snakeBody[0].x >= 800 ||
         snakeBody[0].y <= 0 || snakeBody[0].y >= 600 ||
         checkHitItself(snakeBody[0].x - 0.5*SPEED, snakeBody[0].y -0.5*SPEED)) {
        cout << "GAME OVER!" << endl;
        cout << "Your final score : " << score << endl;
        endGame = true;
      }
    }

    void setDir(int d) {
      switch (d) {
        case  0:
          if(curDirection != 2) {
            preDirection = curDirection;
            curDirection = d;
          }
          break;
        case  1:
          if(curDirection != 3) {
            preDirection = curDirection;
            curDirection = d;
          }
          break;
        case  2:
          if(curDirection != 0) {
            preDirection = curDirection;
            curDirection = d;
          }
          break;
        case  3:
          if(curDirection != 1) {
            preDirection = curDirection;
            curDirection = d;
          }
          break;
      }
    }

    int getCurDir() {
      return curDirection;
    }

    int getPreDir() {
      return preDirection;
    }

    void upScore() {
      score++ ;
    }

	private:
    int x0;
    int y0;
    int length;
    int curDirection;
    int preDirection;
};

Snake snake(100, 450);

class Fruit : public Displayable {
	public:
		virtual void paint(XInfo &xinfo) {
      if(!splash) {
			  XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[6], x, y, blockSize, blockSize);
      }
    }

    Fruit() {
      fruitRegenerate();
    }

    void fruitRegenerate() {
      srand (time(NULL));
      x = rand() % 700 + 50;
      y = rand() % 500 + 50;
      while(fruitIsInsideSnake() || fruitIsInFrontOfSnake()) {
        x = rand() % 700 + 50;
        y = rand() % 500 + 50;
      }
    }

    bool fruitIsInFrontOfSnake() {
      int snakex = snake.snakeBody[0].x;
      int snakey = snake.snakeBody[0].y;
      switch (snake.getCurDir()) {
        case 2: //left
          if((x < snakex && x > snakex - 40) && (y > snakey - 10 && y < snakey + 10)) {
            return true;
          }
          break;
        case 0: //right
          if((x < snakex + 40 && x > snakex) && (y > snakey - 10 && y < snakey + 10)) {
            return true;
          }
          break;
        case 1:  //up
          if((x > snakex - 10 && x < snakex + 10) && (y > snakey - 40 && y < snakey)) {
            return true;
          }
          break;
        case 3:  //down
          if((x > snakex - 10 && x < snakex + 10) && (y > snakey && y < snakey + 40)) {
            return true;
          }
          break;
      }
      return false;
    }

    bool fruitIsInsideSnake() {
      for(int i = 0; i < snake.snakeBody.size() - 1; i++) {
        if(((x < snake.snakeBody[i].x + 0.5 * blockSize) && (x > snake.snakeBody[i].x - 1.5 * blockSize)) &&
           ((y < snake.snakeBody[i].y + 0.5 * blockSize) && (y > snake.snakeBody[i].y - 1.5 * blockSize))) {
             return true;
        }
      }
      return false;
    }

    int getX() {
      return x;
    }

    int getY() {
      return y;
    }

  private:
    int x;
    int y;
};

list<Displayable *> dList;           // list of Displayables
Fruit fruit;
ScoreBoard scoreboard;
SplashScreen splashscreen;

void Snake::checkEatFruit() {
  if((snake.snakeBody[0].x - 0.5 * blockSize < fruit.getX() + blockSize) &&
     (snake.snakeBody[0].x + 0.5 * blockSize > fruit.getX()) &&
     (snake.snakeBody[0].y - 0.5 * blockSize < fruit.getY() + blockSize) &&
     (snake.snakeBody[0].y + 0.5 * blockSize > fruit.getY())) {
       fruit.fruitRegenerate();
       for(int i = 0; i < 20/SPEED; i++) {
         snake.addHead();
       }
       snake.upScore();
       if(score >= 5 && score < 15) {
         level = 2;
         SPEED = 4;
       } else if(score >= 15 && score < 25) {
         level = 3;
         SPEED = 5;
       } else if(score >= 25 && score < 35) {
         level = 4;
         SPEED = 6;
       } else if(score >= 35 && score < 40) {
         level = 4;
         SPEED = 7;
       } else if(score >= 40 && score < 45) {
         level = 5;
         SPEED = 8;
       } else if(score >= 45 && score < 50) {
         level = 6;
         SPEED = 9;
       } else if(score >= 50 && score < 55) {
         level = 7;
         SPEED = 10;
       } else if(score == 60) {
         win = true;
         endGame = true;
         cout << "CONGRATS! YOU WIN!" << endl;
       }
       cout << "YOUR SCORE NOW: " << score << endl;
     }
}
/*
 * Initialize X and create a window
 */
 int argc_ ;
 char *argv_[3];

void initX(int argc, char *argv[], XInfo &xinfo) {
  argc_  = argc;
  for(int i = 0; i < argc; i++) {
    argv_[i] = argv[i];
  }

  if (argc == 1) {
    myFPS = FPS;
    SPEED = 3;
	}
  if (argc == 2) {
   	istringstream ss(argv[1]);
   	ss >> myFPS;
  }
  if(argc == 3) {
   	istringstream ss1(argv[1]);
   	ss1 >> myFPS;
   	istringstream ss2(argv[2]);
   	ss2 >> SPEED;
    snake = Snake(100, 450);
  }
	XSizeHints hints;
	unsigned long white, black;
  xinfo.pause = true;

   /*
	* Display opening uses the DISPLAY	environment variable.
	* It can go wrong if DISPLAY isn't set, or you don't have permission.
	*/
	xinfo.display = XOpenDisplay( "" );
	if ( !xinfo.display )	{
		error( "Can't open display." );
	}

   /*
	* Find out some things about the display you're using.
	*/
	xinfo.screen = DefaultScreen( xinfo.display );

	white = XWhitePixel( xinfo.display, xinfo.screen );
	black = XBlackPixel( xinfo.display, xinfo.screen );

	hints.x = 100;
	hints.y = 100;
	hints.width = 800;
	hints.height = 600;
	hints.flags = PPosition | PSize;

	xinfo.window = XCreateSimpleWindow(
		xinfo.display,				// display where window appears
		DefaultRootWindow( xinfo.display ), // window's parent in window tree
		hints.x, hints.y,			// upper left corner location
		hints.width, hints.height,	// size of the window
		Border,						// width of window's border
		black,						// window border colour
		white );					// window background colour

	XSetStandardProperties(
		xinfo.display,		// display containing the window
		xinfo.window,		// window whose properties are set
		"Weilyu's Snake Game",		// window's title
		"Animate",			// icon's title
		None,				// pixmap for the icon
		argv, argc,			// applications command line args
		&hints );			// size hints for the window


    /*
    * Allocating colors
    */
    Colormap colors;
    XColor grey, cyan, violet, brown, red, gold;
    colors = DefaultColormap(xinfo.display, xinfo.screen);
    XAllocNamedColor(xinfo.display, colors, "grey", &grey, &grey);
    XAllocNamedColor(xinfo.display, colors, "cyan", &cyan, &cyan);
    XAllocNamedColor(xinfo.display, colors, "violet", &violet, &violet);
    XAllocNamedColor(xinfo.display, colors, "red", &red, &red);
    XAllocNamedColor(xinfo.display, colors, "gold", &gold, &gold);


    /*
  	 * Create Graphics Contexts
  	 */
  	int i = 0;
  	xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
  	XSetForeground(xinfo.display, xinfo.gc[i], BlackPixel(xinfo.display, xinfo.screen));
  	XSetBackground(xinfo.display, xinfo.gc[i], WhitePixel(xinfo.display, xinfo.screen));
  	XSetFillStyle(xinfo.display, xinfo.gc[i], FillSolid);
  	XSetLineAttributes(xinfo.display, xinfo.gc[i],
  	                     1, LineSolid, CapButt, JoinRound);
    XFontStruct *font1;
    font1 = XLoadQueryFont(xinfo.display, "-*-helvetica-bold-r-*-*-17-*");
    XSetFont(xinfo.display, xinfo.gc[0], font1->fid);
    //reverse video
  	i = 1;
  	xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
  	XSetForeground(xinfo.display, xinfo.gc[i], WhitePixel(xinfo.display, xinfo.screen));
  	XSetBackground(xinfo.display, xinfo.gc[i], BlackPixel(xinfo.display, xinfo.screen));
  	XSetFillStyle(xinfo.display, xinfo.gc[i], FillSolid);
  	XSetLineAttributes(xinfo.display, xinfo.gc[i],
  	                     1, LineSolid, CapButt, JoinRound);

    i = 2;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], black);

    i = 3;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], white);

    XFontStruct *font2;
    font2 = XLoadQueryFont(xinfo.display,  "-*-helvetica-bold-r-*-*-12-*");
    XSetFont(xinfo.display, xinfo.gc[3], font2->fid);

    i = 4;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], grey.pixel);

    i = 5;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], cyan.pixel);

    i = 6;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], red.pixel);

    XSetFont(xinfo.display, xinfo.gc[6], font1->fid);

    i = 7;
    xinfo.gc[i] = XCreateGC(xinfo.display, xinfo.window, 0, 0);
    XSetForeground(xinfo.display, xinfo.gc[i], black);

    XSetFont(xinfo.display, xinfo.gc[7], font2->fid);

    int depth = DefaultDepth(xinfo.display, DefaultScreen(xinfo.display));
    xinfo.pixmap = XCreatePixmap(xinfo.display, xinfo.window, hints.width, hints.height, depth);
    xinfo.width = hints.width;
    xinfo.height = hints.height;


    XSelectInput(xinfo.display, xinfo.window,
      ButtonPressMask | KeyPressMask |
      PointerMotionMask |
      EnterWindowMask | LeaveWindowMask |
      StructureNotifyMask);  // for resize events

    XSetWindowBackgroundPixmap(xinfo.display, xinfo.window, None);
   /*
	 * Put the window on the screen.
	 */
	XMapRaised( xinfo.display, xinfo.window );
	XFlush(xinfo.display);
}

/*
 * Function to repaint a display list
 */
 void repaint( XInfo &xinfo) {
 	list<Displayable *>::const_iterator begin = dList.begin();
 	list<Displayable *>::const_iterator end = dList.end();

  XFillRectangle(xinfo.display, xinfo.pixmap, xinfo.gc[3], 0, 0, xinfo.width, xinfo.height);
 	// big black rectangle to clear background

 	// draw display list
 	while( begin != end ) {
 		Displayable *d = *begin;
 		d->paint(xinfo);
 		begin++;
 	}

  XCopyArea(xinfo.display, xinfo.pixmap, xinfo.window, xinfo.gc[0],
        0, 0, xinfo.width, xinfo.height,  // region of pixmap to copy
        0, 0); // position to put top left corner of pixmap in window
 	XFlush( xinfo.display );
 }

void handleKeyPress(XInfo &xinfo, XEvent &event) {
  splash = false;
	KeySym key;
	char text[BufferSize];

	/*
	 * Exit when 'q' is typed.
	 * This is a simplified approach that does NOT use localization.
	 */
	int i = XLookupString(
		(XKeyEvent *)&event, 	// the keyboard event
		text, 					// buffer when text will be written
		BufferSize, 			// size of the text buffer
		&key, 					// workstation-independent key symbol
		NULL );					// pointer to a composeStatus structure (unused)
	if ( i == 1) {
		printf("Got key press -- %c\n", text[0]);
		if (text[0] == 'q') {
			error("Terminating normally.");
		} else if(text[0] == 'w') {
      snake.setDir(1);
    } else if(text[0] == 's') {
      snake.setDir(3);
    } else if(text[0] == 'a') {
      snake.setDir(2);
    } else if(text[0] == 'd') {
      snake.setDir(0);
    }
    if (text[0] == 'u') {
      if(SPEED < 10) {
        SPEED++;
        cout << "Current Speed: " << SPEED << endl;
      } else {
        cout << "This is the MAXIMUM speed!" << endl;
      }
    }
    if(text[0] == 'g') {
      win = true;
    }
    if (text[0] == 'i') {
      if(SPEED > 1) {
        SPEED--;
        cout << "Current Speed: " << SPEED << endl;
      } else {
        cout << "This is the MINIMUM speed!" << endl;
      }
    }
    if(text[0] == 'p') {
      if(xinfo.pause) {
        xinfo.pause = false;
      } else {
        xinfo.pause = true;
      }
    }
    if(text[0] == 'r') {
        splash = true;
        endGame = false;
        win = false;
        level = 1;
        score = 0;
        SPEED = 3;
        snake = Snake(100, 450);
        XCloseDisplay(xinfo.display);
        initX(argc_, argv_, xinfo);
    }
	} else if(i == 0) {
    if(key == XK_Up) {
      snake.setDir(1);
    } else if(key == XK_Down) {
      snake.setDir(3);
    } else if(key == XK_Left) {
      snake.setDir(2);
    } else if(key == XK_Right) {
      snake.setDir(0);
    }
  }
}

void handleAnimation(XInfo &xinfo) {
	if((!xinfo.pause)&&(!endGame)&&(!win)) {
    snake.move(xinfo);
  }
}

// get microseconds
unsigned long now() {
	timeval tv;
	gettimeofday(&tv, NULL);
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

void eventLoop(XInfo &xinfo) {
	// Add stuff to paint to the display list
	dList.push_front(&snake);
  dList.push_front(&fruit);
  dList.push_front(&scoreboard);
  dList.push_front(&splashscreen);

	XEvent event;
	unsigned long lastRepaint = 0;
	int inside = 0;

	while( true ) {
		/*
		 * This is NOT a performant event loop!
		 * It needs help!
		 */

		if (XPending(xinfo.display) > 0) {
			XNextEvent(xinfo.display, &event);
			//cout << "event.type=" << event.type << "\n";
			switch( event.type ) {
				case KeyPress:
					handleKeyPress(xinfo, event);
					break;
				case EnterNotify:
					inside = 1;
					break;
				case LeaveNotify:
					inside = 0;
					break;
			}
		}
    unsigned long endRepaint = now();
    if (endRepaint - lastRepaint > 1000000/FPS) {
      if(!endGame) handleAnimation(xinfo);
      repaint(xinfo);
      lastRepaint = now();
    } else if(XPending(xinfo.display) == 0) {
      usleep(1000000/FPS - (endRepaint - lastRepaint));
    }
    /*
		usleep(1000000/myFPS);
		handleAnimation(xinfo, inside);
		repaint(xinfo);
    */
	}
}


/*
 * Start executing here.
 *	 First initialize window.
 *	 Next loop responding to events.
 *	 Exit forcing window manager to clean up - cheesy, but easy.
 */
int main ( int argc, char *argv[] ) {
	XInfo xinfo;
	initX(argc, argv, xinfo);
	eventLoop(xinfo);
	XCloseDisplay(xinfo.display);
}
