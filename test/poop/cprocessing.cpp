/*
 * graphics.cpp
 *
 *  Created on: Apr 28, 2011
 *      Author: esperanc
 */

#include "cprocessing.hpp"
//#include "ArrayList.hpp"

/// This will link with the client's functions
extern void draw();
extern void setup();
extern void mousePressed();
extern void mouseReleased();
extern void mouseMoved();
extern void mouseDragged();
extern void keyPressed();
extern void keyReleased();

using namespace cprocessing;

bool mouseRecordFlag = true; // When to record or not the mouse position
/// Variables and functions to maintain a backup buffer
char * backbuffer = 0;

static void allocbuffer() {
	if (backbuffer != 0) delete backbuffer;
	backbuffer = new char [width*height*4]; 
}

static void readbuffer() {
	if (backbuffer) {
		glFlush();
		glReadPixels (0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE , (void*) backbuffer);
	}
}

static void writebuffer() {
	if (backbuffer) {
		glPushMatrix();
		glLoadIdentity();
		glRasterPos3f(0,0,0);
		glDrawPixels (width, height, GL_RGBA, GL_UNSIGNED_BYTE , (void*) backbuffer);
		glPopMatrix();
	}
}

namespace cprocessing {

	//
    // Global variables
	//

	int mouseX = 0;  ///< Mouse x coordinate
	int mouseY = 0;  ///< Mouse y coordinate
	int pmouseX = 0;  ///< Previous mouse x coordinate
	int pmouseY = 0;  ///< Previous mouse y coordinate
	bool mousePressed = false; ///< Whether any mouse button is pressed
	int mouseButton = LEFT; ///< Which button is pressed
	bool keyPressed = false; ///< Whether a key was pressed
	unsigned char key; ///< Which (ASCII) key was pressed
	int keyCode;   ///< Code for the last pressed key

	int width;     ///< window width
	int height;    ///< window height
	
	bool looping = true;   ///redisplay if true

	unsigned config = HALF_PIXEL_SHIFT | Y_DOWN | BACK_BUFFER; ///< Configuration flags

	int framerate = 60; ///< Frames per second
    int frameCount = 0;
	int initialized = false; 	///< Whether or not initialization of glut took place

	color strokeColor (0,0,0);     ///< Line drawing color
	color fillColor   (255,255,255);   ///< Area drawing color
    


	/// 
	/// Global OpenGL initialization code. Should be called at least once when screen is established
	///
    static void init () {
    
    	// Enable depth buffer
    	glEnable(GL_DEPTH_TEST);
    	
    	// Make it possible to overwrite pixels
        glDepthFunc(GL_LEQUAL);
        
        // Helps when drawing in 3D with wireframe superimposed on the filled faces
        glPolygonOffset (1., -1.);
        
        // Cope with scaling transformations
        //glEnable(GL_RESCALE_NORMAL);
        glEnable(GL_NORMALIZE);
		
        // Disable the default additional ambient component
        float ambient [] = {0, 0, 0, 1};
        glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
 
		// By default, y is flipped, so front is clockwise
		glFrontFace(GL_CW);

    	// Make it possible to set material properties by using glcolor
	    glEnable(GL_COLOR_MATERIAL);
	    glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    	glColorMaterial(GL_FRONT_AND_BACK, GL_SPECULAR);
	}    	


    /// This is called for each frame
    static void display () {

		// Restore backing buffer if needed
		if (config&BACK_BUFFER) writebuffer();
        
        // Restore default state
        camera();
        perspective();
        noLights();

        // Call external display function
        ::draw();
        mouseRecordFlag = true;

		// Refresh backing buffer if needed
		if (config&BACK_BUFFER) readbuffer();

        // End by swapping front and back buffers
        glutSwapBuffers() ;
    }

    /// Called whenever window geometry is changed.
    /// @param wid New width of the window.
    /// @param hgt New height of the window.
    static void reshape (int wid, int hgt)
    {
        glViewport(0,0,wid,hgt);

        width = wid;
        height = hgt;

        // Default background is gray 70%
        background (200);
        
        // Initialize OPENGL modes
        init();
        
        // Reset backup buffer if needed
        if (config&BACK_BUFFER) {
        	allocbuffer();
        	readbuffer();
        }
       
    }

    /// The refresh function is called periodically to redisplay
    static void refresh (int) {
        if(looping) {
            frameCount++;
        	glutPostRedisplay();
        	glutTimerFunc (1000/framerate, refresh, 0);
        }
    }

     color blendColor(const color a, const color b, unsigned mode) {
        assert (mode == REPLACE || mode == BLEND || mode == ADD || mode == SUBTRACT || mode == DARKEST || mode == LIGHTEST || mode == DIFFERENCE || mode == EXCLUSION || mode == MULTIPLY || mode == SCREEN || mode == OVERLAY || mode == HARD_LIGHT || mode == SOFT_LIGHT || mode == DODGE || mode == BURN);
        //TODO:: finish blend modes
        switch(mode) {
            case REPLACE:
                return color(b.rgba[0], b.rgba[1], b.rgba[2], b.rgba[3]);
                break;
            case BLEND:
                return color((a.rgba[0]+b.rgba[0])/2, (a.rgba[1]+b.rgba[1])/2, (a.rgba[2]+b.rgba[2])/2, (a.rgba[3]+b.rgba[3])/2);
                break;
            case ADD:
                return color(min(a.rgba[0]+b.rgba[0], 255), min(a.rgba[1]+b.rgba[1], 255), min(a.rgba[2]+b.rgba[2], 255), min(a.rgba[3]+b.rgba[3], 255));
                break;
            /*case ADD:
                return color((a.rgba[0]+b.rgba[0]), (a.rgba[1]+b.rgba[1]), (a.rgba[2]+b.rgba[2]), (a.rgba[3]+b.rgba[3]));
                break;*/
           /* case SUBTRACT:
                return color(max(a.rgba[0]-b.rgba[0], 0), max(a.rgba[1]-b.rgba[1], 0), max(a.rgba[2]-b.rgba[2], 0), max(a.rgba[3]-b.rgba[3], 0));
                break;*/
            default:
                return color(0, 0, 0);
                break;
        }
    }

    /// Called whenever mouse moves
    static void mousemotion (int x, int y) {
      if (mouseRecordFlag){
         pmouseX = mouseX;
         pmouseY = mouseY;
         mouseX = x;
         mouseY = y;
         mouseRecordFlag = false;
      }
      ::mouseMoved();
    	if (mousePressed) {
    		::mouseDragged();
    	}
    }

    /// Called whenever mouse button is pressed
    static void mouse (int button, int state, int x, int y) {
       if (mouseRecordFlag){
         pmouseX = mouseX;
         pmouseY = mouseY;
         mouseX = x;
         mouseY = y;
         mouseRecordFlag = false;
       }

       mousePressed = state == GLUT_DOWN;
  
       if (button == GLUT_LEFT_BUTTON) {
          mouseButton = LEFT;
       } else if (button == GLUT_RIGHT_BUTTON) {
          mouseButton = RIGHT;
       } else {
          mouseButton = CENTER;
       }
       if (mousePressed) {
          ::mousePressed();
       }
       else {
          ::mouseReleased();
       }
    }

    /// Called whenever a key is pressed
    static void keyboard (unsigned char ch, int x, int y) {
    	keyPressed = true;
    	key = ch;
    	keyCode = ch;
    	::keyPressed();
    }

    /// Called whenever a key is released
    static void keyboardup (unsigned char ch, int x, int y) {
    	keyPressed = false;
    	::keyReleased();
    }

    /// Called whenever a special key is pressed
    static void special (int ch, int x, int y) {
    	keyPressed = true;
    	key = CODED;
    	keyCode = 0;
    	switch (ch) {
			case GLUT_KEY_F1:
			case GLUT_KEY_F2:
			case GLUT_KEY_F3:
			case GLUT_KEY_F4:
			case GLUT_KEY_F5:
			case GLUT_KEY_F6:
			case GLUT_KEY_F7:
			case GLUT_KEY_F8:
			case GLUT_KEY_F9:
			case GLUT_KEY_F10:
			case GLUT_KEY_F11:
			case GLUT_KEY_F12:
				keyCode = ch + F1 - GLUT_KEY_F1;
				break;
			case GLUT_KEY_LEFT:
				keyCode = LEFT;
				break;
			case GLUT_KEY_UP:
				keyCode = UP;
				break;
			case GLUT_KEY_RIGHT:
				keyCode = RIGHT;
				break;
			case GLUT_KEY_DOWN:
				keyCode = DOWN;
				break;
			case GLUT_KEY_PAGE_UP:
				keyCode = PAGEUP;
				break;
			case GLUT_KEY_PAGE_DOWN:
				keyCode = PAGEDOWN;
				break;
			case GLUT_KEY_HOME:
				keyCode = HOME;
				break;
			case GLUT_KEY_END:
				keyCode = END;
				break;
			case GLUT_KEY_INSERT:
				keyCode = INSERT;
				break;
    	}
    	::keyPressed();
    }


    /// Called whenever a special key is released
    static void specialup (int ch, int x, int y) {
		// Restore backing buffer if needed
		if (config&BACK_BUFFER) writebuffer();

    	keyPressed = false;
    	::keyReleased();

		// Refresh backing buffer if needed
		if (config&BACK_BUFFER) readbuffer();
    }

    /// Sets up a window of the given size
    /// @param wid Desired window width in pixels.
    /// @param hgt Desired window height in pixels.
    /// @param name Desired window title.
    void size (unsigned wid, unsigned hgt, const char* name) {
    	if (initialized) {
    		glutReshapeWindow (wid, hgt);
    		glutSetWindowTitle (name);
    	} else {
			glutInitWindowSize (wid, hgt);
			width = wid;
			height = hgt;
			glutCreateWindow (name);
			glutReshapeFunc(reshape);
			glutDisplayFunc(display);
			glutMotionFunc (mousemotion);
			glutPassiveMotionFunc (mousemotion);
			glutMouseFunc (mouse);
			glutKeyboardFunc (keyboard);
			glutKeyboardUpFunc (keyboardup);
			glutSpecialFunc(special);
			glutSpecialUpFunc(specialup);
			initialized = true;
    	}

    }


        /// Sets line color
    /// @param color: New line color
    void stroke (const color& c) {
        strokeColor = c;
    }

    /// Sets area color
    /// @param color: New area color
    void fill (const color& c) {
        fillColor = c;
    }

    /// Sets line / point width
    /// @param weight: New breadth of line in pixels
    void strokeWeight (int weight) {
        glLineWidth (weight);
        glPointSize (weight);
    }


    /// Clear the window with a background color
    /// @param color: background color
    void background (const color& c) {
        glClearColor (c.rgba[0] * (1.0/255),
                      c.rgba[1] * (1.0/255),
                      c.rgba[2] * (1.0/255),
                      c.rgba[3] * (1.0/255));
        glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }

    /// Sets state so that lines are rendered antialiased.
    void smooth() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_LINE_SMOOTH);
        glEnable(GL_POINT_SMOOTH);
        glEnable(GL_POLYGON_SMOOTH);
        glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    }

    /// Sets state so that lines are rendered quickly.
    void noSmooth() {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_LINE_SMOOTH);
        glDisable(GL_POINT_SMOOTH);
        glDisable(GL_POLYGON_SMOOTH);
    }

    //sets a point on the screen 
	//TODO: make this draw ontop of screen instead of affected by transformations
	void set (int x, int y, color c) {
	    if(c.rgba[3] > 0) {
	        glColor4ubv(c.rgba);
		    glBegin (GL_POINTS);
		    glVertex2d (x,y);
		    glEnd();
		}
	}
    
    /// Initializes and runs the application main event loop
    void run() {
		int argc = 0;
		char **argv = 0;
        glutInit(&argc, argv);
	    glutInitDisplayMode (GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
	    glutTimerFunc (1000/framerate, refresh, 0);
        //Magick::InitializeMagick(*argv);

    	bezierDetail(50);
    	ellipseDetail(50);
    	sphereDetail(30,30);
    	::setup();
        glutMainLoop();
    }
}
