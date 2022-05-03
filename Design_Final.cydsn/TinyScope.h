// TinyScope.h
// Header file for the Tiny Scope. 
// List of functions and macros important to this implementation.


// The H files!!!!
#include "project.h"
#include "GUI.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
    
#ifndef TINYSCOPE_H
#define TINYSCOPE_H
  
// Global constants
#define BYTE_COUNT 4096
#define SAMPLE_RATE 231481
#define ADC_CONST 620
#define STR_LEN 100
    
// Global GUI Definitions
#define XSIZE 320                 // Width of LCD screen in pixels
#define YSIZE 240                 // Height of LCF screen in pixels
#define XDIVISIONS 10             // Number of x-axis divisions for grid
#define YDIVISIONS 8              // Number of y-axis divisions for grid
#define XMARGIN 5                 // Margin around screen on x-axis
#define YMARGIN 4                 // Margin around screen on y-axis
#define MAXPOINTS XSIZE-2*XMARGIN // Maximum number of points in wave for plotting

// Design Options
#define BACKCOLOR  GUI_DARKCYAN   // Background color
#define STARTCOLOR GUI_GREEN      // Start screen text color
#define CH1COLOR   GUI_RED        // Channel 1 color
#define CH2COLOR   GUI_YELLOW     // Channel 2 color
#define PENSIZE 2

#define PI 3.14159265
   
    
/*
GRAPHICS FUNCTIONS
*/
    
// FUNCTION
// Displays the initial screen
void startScreen(void);

// FUNCTION
// set the background for the scope screen
void setBackground();

// FUNCTION
// creates the grid
void drawGrid();

// FUNCTION
// take in plotable info and previous plot info to erase and update line
void updateWave (int channel, int* waveDataX, int* waveDataY, int* waveDataYold);

// FUNCTION
// take in scale data and waveform data and return a plotable waveform
void createNew (int xscale, int yscale, int* waveDataY, int ypos, uint32* data, uint8 slope, uint8 mode, int level);

// FUNCTION
// displays frequency, xscale, yscle
void displayConfig(int xscale, int yscale, int freq1, int freq2);


/*
MISC CALCULATION FUNCTIONS
*/

// FUNCTION
// takes in two buffers are returns thier frequencies
void updateFreqs(int* freq1, int* freq2, uint32* buff1, uint32* buff2);
    
    
// FUNCTION
// take in integer buffers of max data size and find the two frequencies, 
// which will stored in a passed integer pointer
void getFreqs(uint32* buff1, uint32* buff2, int* result);

// FUNCTION
// takes in scroll info and updates it
void adjustScroller(int* ch1y, int* ch2y, int scroll);

/*
COMMUNICATIONS FUNCTIONS
*/

// FUNCTION
// returns the next string excluding white space from the command line
void getLine(char* str);

// FUNCTION
// Takes in pointers to global varaibles and deciphers terminal input to update those variables
void manageInput(int*   trigger_level,
                 uint8* trigger_slope,
                 uint8* mode,
                 uint8* running,
                 uint8* channel,
                 int*   xscale,
                 int*   yscale,
                 int*   scroll);

#endif
