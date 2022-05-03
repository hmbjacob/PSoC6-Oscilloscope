// TinyScope.c
// Implementation of the Tiny Scope Header file

#include "TinyScope.h"

/*
MISC CALCULATION FUNCTIONS
*/

// FUNCTION
// updates the frequencies
void updateFreqs(int* freq1, int* freq2, uint32* buff1, uint32* buff2) {
    int result[2];
    getFreqs(buff1, buff2, result);
    if (result[0] != -1 && result[0] < 10000) { 
        *freq1 = result[0]/100*100; // round down to nearest hundred
    }
    if (result[1] != -1 && result[1] < 10000) { 
        *freq2 = result[1]/100*100; // round down to nearest hundred
    }
}


// depending on what the current setting is, 
// the potentiometer can scroll both channels
void adjustScroller(int* ch1y, int* ch2y, int scroll) {
    if (scroll == 1) {
        *ch1y = (int)ADC_GetResult32(2)/11+30;
    } else if (scroll == 2) {
        *ch2y = (int)ADC_GetResult32(2)/11+30;
    }
}

// FUNCTION
// take in integer buffers of max data size and find the two frequencies, 
// which will stored in a passed integer pointer
void getFreqs(uint32* buff1, uint32* buff2, int* result) {

            int myBuff_1[BYTE_COUNT];
            int myBuff_2[BYTE_COUNT];
    
            // convert to integers
            for (int i = 0; i < BYTE_COUNT-1; i++) {
                // fill in my buffer used for calculating, before ping may be overwritten
                myBuff_1[i] = (int)(buff1[i]&0xFFFF);   // buffer for AD2_1
                myBuff_2[i] = (int)(buff2[i]&0xFFFF);   // buffer for AD2_2
            }
            // find min and max
            int min_1 = 0;
            int max_1 = 0;
            int min_2 = 0;
            int max_2 = 0;
            for (int i = 0; i < BYTE_COUNT-1; i++) {
                // find min and max for first buffer
                if (myBuff_1[i] < myBuff_1[min_1]) min_1 = i;
                if (myBuff_1[i] > myBuff_1[max_1]) max_1 = i;  
                //printf("1: %d\r\n", myBuff_1[i]);  

                // find min and max for second buffer
                if (myBuff_2[i] < myBuff_2[min_2]) min_2 = i;
                if (myBuff_2[i] > myBuff_2[max_2]) max_2 = i;  
                //printf("2: %d\r\n", myBuff_2[i]);  
            }
            
            /*
            THE FOLLOWING SEQUENCE WILL FIND THE MIDPOINT VALUES, AND COUNT THE TOTAL NUMBER OF MIDPOINT CROSSINGS. THIS CAN BE USED TO FIND FREQ
            */
            
            // FIRST BUFFER
            // find mid value for buffer one
            int mid_1 = (myBuff_1[max_1])/2+myBuff_1[min_1]/2;
            int numMid1 = 0;
            //find the number of midpoints
            for (int i = 0; i < BYTE_COUNT-1; i++) {
                if (((myBuff_1[i] > mid_1) && (myBuff_1[i-1] < mid_1)) || ((myBuff_1[i] < mid_1 && myBuff_1[i-1] > mid_1))){
                    numMid1++;
                }
            }

            
            // SECOND BUFFER
            // find mid value for buffer one
            int mid_2 = (myBuff_2[max_2])/2+myBuff_2[min_2]/2;
            int numMid2 = 0;
            //find the number of midpoints
            for (int i = 0; i < BYTE_COUNT-1; i++) {
                if (((myBuff_2[i] > mid_2) && (myBuff_2[i-1] < mid_2)) || ((myBuff_2[i] < mid_2 && myBuff_2[i-1] > mid_2))){
                    numMid2++;
                }
            }

            //the frequency is the related to the distance between first two mids and the sample rate
            // make the result -1 if there is no readable frequency
            if (numMid1 > 2) {
                int waves = (numMid1-1)/2;
                result[0] = SAMPLE_RATE*waves/BYTE_COUNT; // calculate freq and store
            } else {
                result[0] = -1;
            }
            if (numMid2 > 2) {
                int waves = (numMid2-1)/2;
                result[1] = SAMPLE_RATE*waves/BYTE_COUNT; // calculate freq and store
            } else {
                result[1] = -1;
            }
}


/*
GRAPHICS FUNCTIONS
*/



// FUNCTION
// Displays the initial screen
void startScreen(void)
{
    
    // Set the colors
    GUI_SetFont(GUI_FONT_32B_1);
    GUI_SetBkColor(BACKCOLOR);
    GUI_SetColor(GUI_GREEN);
    GUI_Clear();
  
    // Project Name
    GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);
    GUI_DispStringAt("TinyScope", 160, 60);
    
    // Project Info
    GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);
    GUI_DispStringAt("CSE 121/L Final Project", 160, 120);
    
    // Author
    GUI_SetTextAlign(GUI_TA_HCENTER | GUI_TA_VCENTER);
    GUI_DispStringAt("By Jacob McClellan", 160, 180);
}





// FUNCTION
// set the background for the scope screen
void setBackground() {

    // Create the background and margin box
    GUI_SetBkColor(GUI_BLACK);
    GUI_SetColor(BACKCOLOR);
    GUI_FillRect(0, 0, XSIZE, YSIZE);
    drawGrid();

}

// FUNCTION
// creates the grid
void drawGrid() {
    GUI_SetPenSize(1);
    GUI_SetColor(GUI_LIGHTGRAY);
    GUI_DrawRect(XMARGIN, YMARGIN, XSIZE-XMARGIN, YSIZE-YMARGIN);
    GUI_SetLineStyle(GUI_LS_DOT);
    
    int xstep = (XSIZE-XMARGIN*2)/XDIVISIONS;
    int ystep = (YSIZE-YMARGIN*2)/YDIVISIONS;
    
    GUI_SetPenSize(1);
    GUI_SetColor(GUI_LIGHTGRAY);
    GUI_SetLineStyle(GUI_LS_DOT);
    for (int i=1; i<YDIVISIONS; i++){
       GUI_DrawLine(XMARGIN, YMARGIN+i*ystep, XSIZE-XMARGIN, YMARGIN+i*ystep);
    }
    for (int i=1; i<XDIVISIONS; i++){
      GUI_DrawLine(XMARGIN+i*xstep, YMARGIN, XMARGIN+i*xstep, YSIZE-YMARGIN);
    }
}


// FUNCTION
// take in plotable info and previous plot info to erase and update line
void updateWave (int channel, int* waveDataX, int* waveDataY, int* waveDataYold) {
    GUI_SetPenSize(PENSIZE);
    // erase old wave
    for (int i = 1; i < MAXPOINTS; i++) {
        GUI_SetColor(BACKCOLOR);
        GUI_DrawLine(XMARGIN+waveDataX[i-1], waveDataYold[i-1], XMARGIN+waveDataX[i], waveDataYold[i]); 
    }
    drawGrid();
    // decide color for new wave
    if (channel == 1) {
        GUI_SetColor(CH1COLOR);
    } else if (channel == 2) {
        GUI_SetColor(CH2COLOR);
    }
    // draw new wave
    for (int i = 1; i < MAXPOINTS; i++) {
        GUI_SetPenSize(PENSIZE);
        GUI_DrawLine(XMARGIN+waveDataX[i-1], waveDataY[i-1], XMARGIN+waveDataX[i], waveDataY[i]); 
    }
    
}
 


// FUNCTION
// take in scale data and waveform data and return a plotable waveform
void createNew (int xscale, int yscale, int* waveDataY, int ypos, uint32* data, uint8 slope, uint8 mode, int level) {
    
    int lvl = level*ADC_CONST/1000; // adjust from millivolts to ADC scale
       
    // get the xscale factor
    int SpD = (xscale*(SAMPLE_RATE/1000))/1000;   // samples per division;
    int XpD = (XSIZE-XMARGIN*2)/XDIVISIONS; // pixels per division;
    int SpP = SpD/XpD;                      // samples per pixel;
    
    // get the yscale factor
    int NpD = ADC_CONST*yscale/1000;        // number per division
    int YpD = (YSIZE-YMARGIN*2)/YDIVISIONS; // pixels per division;
    int NpP = NpD/YpD;                      // number per pixel
    
    // this will adjust the program to the correct starting point (j0)
    int j0 = 0;
    if (mode & slope) { // the trigger is positive slope
        for (; j0 < BYTE_COUNT; j0++) {
            if ((int)(data[j0-1]&0xFFFF) < lvl && (int)(data[j0]&0xFFFF) > lvl) {
                break;
            }
        }
    } else if (mode & !slope) { // the trigger is negative slope
        for (; j0 < BYTE_COUNT; j0++) {
            if ((int)(data[j0-1]&0xFFFF) > lvl && (int)(data[j0]&0xFFFF) < lvl) {
                break;
            }
        }
    }
    
    
    for (int i = 0; i < MAXPOINTS; i++) {
        int j = (i*SpP+j0)%BYTE_COUNT;         // i is the incrmement for pixels, j is the increment for the samples
                                          // the modulo is included because there may not be enough samples
        waveDataY[i] = ypos-((int)(data[j]&0xFFFF))/NpP; // convert and scale data
    }
}




// FUNCTION
// displays frequency, xscale, yscle
void displayConfig(int xscale, int yscale, int freq1, int freq2) {
    GUI_SetBkColor(GUI_DARKCYAN);
    GUI_SetFont(GUI_FONT_16B_1);
    GUI_SetColor(GUI_LIGHTGRAY);
    char text[STR_LEN];
    
    // display the frequencies
    sprintf(text, "Ch 1 Freq: %d Hz", freq1);
    GUI_DispStringAt(text, 10, 20);
    sprintf(text, "Ch 2 Freq: %d Hz", freq2);
    GUI_DispStringAt(text, 10, 35);
    
    // display xscale
    if (xscale >= 1000) {
       sprintf(text, "Xscale: %0d ms/div", xscale/1000);
    } else {
       sprintf(text, "Xscale: %0d us/div", xscale);
    }
    GUI_DispStringAt(text, 200, 15);
    int yscaleVolts = yscale/1000;
    int yscalemVolts = yscale % 1000;
    
    // display yscale
    if (yscale >= 1000) {
      sprintf(text, "Yscale: %0d V/div", yscaleVolts);
    } else {
      sprintf(text, "Yscale: %0d.%0d V/div", yscaleVolts, yscalemVolts/100);
    }
    GUI_DispStringAt(text, 200, 35);
    
}

/*
COMMUNICATIONS FUNCTIONS
*/


// FUNCION
// returns the next string excluding white space from the command line
void getLine(char* str) {
    
    int i = 0;
    while(Cy_SCB_GetNumInRxFifo(UART_HW) > 0) {
        if (i == STR_LEN-1) { // if the input is too long, terminate
            printf("\r\nError: input too long\r\n");
            exit(1);
        }
        
        str[i] = (char)Cy_SCB_UART_Get(UART_HW); // grab input from rx fif0
        i++;
    }
    str[i-1] = ' '; // space to help tokenize
    str[i] = '\0'; // include terminator
}

// FUNCTION
// Takes in pointers to global varaibles and deciphers terminal input to update those variables
void manageInput(int*   trigger_level,
                 uint8* trigger_slope,
                 uint8* mode,
                 uint8* running,
                 uint8* channel,
                 int*   xscale,
                 int*   yscale,
                 int*   scroll) {
            
            // used to store prased input
            char str[STR_LEN]; // will take in command line input
            char* tok1 = NULL;
            char* tok2 = NULL;
            char* tok3 = NULL;
            
            getLine(str); // read in line
            tok1 = strtok(str, " ");
            if (tok1 != NULL) {
                tok2 = strtok(NULL, " ");
                if (tok2 != NULL) {
                    tok3 = strtok(NULL, " ");
                }
            }
            
            // set to free running more or triggered mode
            if (!*running && strcmp(tok1, "set") == 0 && strcmp(tok2, "mode") == 0) {
                if (strcmp(tok3, "free") == 0) {
                    *mode = 0; // 0 indicates free
                    Cy_SCB_UART_PutString(UART_HW, "Mode set to free-running\r\n");
                } else if (strcmp(tok3, "trigger") == 0) {
                    *mode = 1; // 1 indicates trigger
                    Cy_SCB_UART_PutString(UART_HW, "Mode set to trigger\r\n");
                } else  {
                    Cy_SCB_UART_PutString(UART_HW, "Invalid Mode\r\n");
                }
            
            // set the trigger level
            } else if (!*running && strcmp(tok1, "set") == 0 && strcmp(tok2, "trigger_level") == 0) {
                int val = atoi(tok3);
                if (val <= 3300 && val >= 0) {
                    *trigger_level = val - val%100; // round down to nearest whole hundred
                    sprintf(str, "Trigger level set to %d\r\n", *trigger_level);
                    Cy_SCB_UART_PutString(UART_HW, str);
                } else {
                    Cy_SCB_UART_PutString(UART_HW, "Invalid trigger level value\r\n");
                }
                
            // determine if slope is positive or negative
            } else if (!*running &&strcmp(tok1, "set") == 0 && strcmp(tok2, "trigger_slope") == 0) {
                if (strcmp(tok3, "positive") == 0) {
                    *trigger_slope = 1; // 1 indicates positive
                    Cy_SCB_UART_PutString(UART_HW, "Trigger slope set to positive\r\n");
                } else if (strcmp(tok3, "negative") == 0) {
                   *trigger_slope = 0; // 0 indicates negative
                    Cy_SCB_UART_PutString(UART_HW, "Trigger slope set to negative\r\n");
                } else  {
                    Cy_SCB_UART_PutString(UART_HW, "Invalid slope type\r\n");
                }
                
            // determine trigger channel
            } else if (!*running && strcmp(tok1, "set") == 0 && strcmp(tok2, "trigger_channel") == 0) {
                int val = atoi(tok3);
                if (val == 1 || val == 2) {
                    *channel = val-1;
                    sprintf(str, "Trigger channel set to %d\r\n", *channel+1);
                    Cy_SCB_UART_PutString(UART_HW, str);
                } else {
                    Cy_SCB_UART_PutString(UART_HW, "Invalid channel\r\n");
                }
              
            // adjust the xscale value if there is an acceptable amount     
            } else if (strcmp(tok1, "set") == 0 && strcmp(tok2, "xscale") == 0) {
                int val = atoi(tok3);
                if (val == 100 || val == 200 || val == 500 || val == 1000 || val == 2000 || val == 5000 || val == 10000) {
                    *xscale = val;
                    sprintf(str, "xscale set to %d\r\n", val);
                    Cy_SCB_UART_PutString(UART_HW, str);
                } else {
                    Cy_SCB_UART_PutString(UART_HW, "Invalid xscale value; can have values 100, 200, 500, 1000, 2000, 5000 or 10000 (us/div)\r\n");
                }
                
            // adjust the yscale value if there is an acceptable amount 
            } else if (strcmp(tok1, "set") == 0 && strcmp(tok2, "yscale") == 0) {
                int val = atoi(tok3);
                if (val == 500 || val == 1000 || val == 1500 || val == 2000){
                    *yscale = val;
                    sprintf(str, "yscale set to %d\r\n", val);
                    Cy_SCB_UART_PutString(UART_HW, str);
                } else {
                    Cy_SCB_UART_PutString(UART_HW, "Invalid yscale value; can have values 500, 1000, 1500 or 2000 (mv/div)\r\n");
                }
                
            // start the program is stopped
            } else if (strcmp(tok1, "start") == 0) { 
                if (*running == 0) {
                    *running = 1;
                    Cy_SCB_UART_PutString(UART_HW, "Started running\r\n");
                } else {
                    Cy_SCB_UART_PutString(UART_HW, "Already running!\r\n");
                }
                
            // stop the program from running  
            } else if (strcmp(tok1, "stop") == 0) { 
                if (*running == 1) {
                    *running = 0;
                    Cy_SCB_UART_PutString(UART_HW, "Stopped running\r\n");
                } else {
                    Cy_SCB_UART_PutString(UART_HW, "Already stopped!\r\n");
                }
            
            
            // Determine what channel will be getting scrolled
            } else if (strcmp(tok1, "set") == 0 && strcmp(tok2, "scroll") == 0) { 
                int val = atoi(tok3);
                if (val == 1 || val == 2) {
                    *scroll = val;
                    sprintf(str, "Now scrolling channel %d\r\n", val);
                    Cy_SCB_UART_PutString(UART_HW, str);   
                } else {
                    Cy_SCB_UART_PutString(UART_HW, "Invalid channel!\r\n");
                }
            
            // if the input is invlid
                
            } else {
                Cy_SCB_UART_PutString(UART_HW, "Invalid input\r\n");
            }
            
            Cy_SCB_ClearRxFifo(UART_HW);
}
                


