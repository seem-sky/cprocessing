#include <cprocessing/cprocessing.hpp>
using namespace cprocessing;

/**
 * Redraw. 
 * 
 * The redraw() function makes draw() execute once.  
 * In this example, draw() is executed once every time 
 * the mouse is clicked. 
 */
 
float y;
 
// The statements in the setup() function 
// execute once when the program begins
void setup() {
  size(640, 360);  // Size should be the first statement
  stroke(255);     // Set line drawing color to white
  y = height * 0.5;
}

// The statements in draw() are executed until the 
// program is stopped. Each statement is executed in 
// sequence and after the last line is read, the first 
// line is executed again.
// In this example noLoop() pauses the drawing
void draw() { 
  background(0);
  y = y - 4; 
  if (y < 0) { y = height; } 
  line(0, y, width, y); 
  line(random(0, width), 0, width, height);
  noLoop();
} 

void mousePressed() {
  redraw();
}
