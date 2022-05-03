// Main.c

// The H files
#include "project.h"
#include "TinyScope.h"

/* Arrays to hold channel waveform data for drawing */
GUI_HMEM ch1_spline;
GUI_HMEM ch2_spline;


// buffers used for buffing
uint32 DMA_1_Buff_1[BYTE_COUNT];
uint32 DMA_1_Buff_2[BYTE_COUNT];
uint32 DMA_2_Buff_1[BYTE_COUNT];
uint32 DMA_2_Buff_2[BYTE_COUNT];
int waveY1[MAXPOINTS];
int waveY2[MAXPOINTS];
int waveY1old[MAXPOINTS];
int waveY2old[MAXPOINTS];
int waveX[MAXPOINTS];


// global variables
uint8 stage = 0; // which buffers is being used
uint8 last = 0;  // last buffers to have been tabulated
int   trigger_level = 100; // can have values 0-3.3v in .1v increments, set in millivolts
uint8 trigger_slope = 0; // 0 == negative, 1 == positive
uint8 mode = 0; // 0 == free running, 1 == trigger
uint8 running = 1; // 0 == stopped, 1 == started
uint8 channel = 0; // trigger channel 0 for 1 , 1 for 2;
int   xscale = 1000; // can have values 100, 200, 500, 1000, 2000, 5000 or 10000 (us/div)
int   yscale = 1000; // can have values 500, 1000, 1500 or 2000 (mv/div)
int   ch1y = 30; // channel 1 y scroll value
int   ch2y = 120; // channel 2 y scroll value
int   scroll = 1; // channel to be adjusted by the scroller
int   freq1 = 0; // channel 1 freq
int   freq2 = 0; // channel 2 freq

int data[7];

// global functions 
// These functions are functions that must exist in this file
void Isr_dma_1(void);
void Isr_dma_2(void);
void config_DMA_1(void);
void config_DMA_2(void);
void initialize(void);


// FUNCTION:
// main function where the magic happens :)
int main(void)
{
     __enable_irq(); /* Enable global interrupts. */

    // Initialize EmWin Graphics and other components
    GUI_Init();
    initialize();
    
    // initialize the X data
    for (int i = 0; i < MAXPOINTS; i++) {
        waveX[i] = i;   
    }
   
    /* Display the startup screen for set seconds */
    startScreen();   
    Cy_SysLib_Delay(1000);
    setBackground();
    
    // MAIN FOR LOOP
    for (;;) {
        
        // Poll the UART to see if there is User Input
        if (Cy_SCB_GetNumInRxFifo(UART_HW) != 0) {
            manageInput(&trigger_level,
                        &trigger_slope,
                        &mode,
                        &running,
                        &channel,
                        &xscale,
                        &yscale,
                        &scroll);
        }
        
        adjustScroller(&ch1y, &ch2y, scroll); // adjust scroller poll
        displayConfig(xscale, yscale, freq1, freq2); // update config graphics
        
        if (running) { // do nothing if the program is not running
            if (stage == last) { // if nothing to be done, find the frequencies
                
            } else if (stage == 0) { // calculations for the second pair of buffers
                
                // create new plot data
                createNew (xscale, yscale, waveY1, ch1y, DMA_1_Buff_2, trigger_slope, mode&!channel, trigger_level);
                createNew (xscale, yscale, waveY2, ch2y, DMA_2_Buff_2, trigger_slope, mode&channel, trigger_level);
                updateWave(1, waveX, waveY1, waveY1old); // use old and new info to erase and create new plots
                updateWave(2, waveX, waveY2, waveY2old);  
                
                for (int i = 0; i < MAXPOINTS; i++) { // store the previous info
                    waveY1old[i] = waveY1[i];   
                    waveY2old[i] = waveY2[i];   
                }
                
                last = 0;
            } else if (stage == 1) { // calculations for the first pair of buffers
            
                // create new plot data
                createNew (xscale, yscale, waveY1, ch1y, DMA_1_Buff_1, trigger_slope, mode&!channel, trigger_level);
                createNew (xscale, yscale, waveY2, ch2y, DMA_2_Buff_1, trigger_slope, mode&channel, trigger_level);
                updateWave(1, waveX, waveY1, waveY1old); // use old and new info to erase and create new plots
                updateWave(2, waveX, waveY2, waveY2old);
                
                for (int i = 0; i < MAXPOINTS; i++) { // store the previous info
                    waveY1old[i] = waveY1[i];   
                    waveY2old[i] = waveY2[i];   
                }
                
                last = 1;
                
                // I do frequency calculations here
                // only doing it half the time to save time
                updateFreqs(&freq1, &freq2, DMA_1_Buff_1, DMA_2_Buff_1);
        
            }
        } else {
            Cy_SysLib_DelayUs(10000); // Rx input requires this delay, or else the program empty the fifo too fast
        }

    }
}

// FUNCTION:
// Configures the first DMA used in transferring data from ADC to ping pong set
void config_DMA_1(void){
    cy_stc_dma_channel_config_t channelConfig =
    {
        .descriptor = &DMA_1_Descriptor_1,
        .preemptable = DMA_1_PREEMPTABLE,
        .priority = DMA_1_PRIORITY,
        .enable = false
    };
    
    (void)Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_1, &DMA_1_Descriptor_1_config);
    (void)Cy_DMA_Descriptor_Init(&DMA_1_Descriptor_2, &DMA_1_Descriptor_2_config);
    (void)Cy_DMA_Channel_Init(DMA_1_HW, DMA_1_DW_CHANNEL, &channelConfig);
    
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_1, (uint32_t*) &(SAR->CHAN_RESULT[0]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_1, DMA_1_Buff_1);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_1_Descriptor_2, (uint32_t*) &(SAR->CHAN_RESULT[0]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_1_Descriptor_2, DMA_1_Buff_2);
    Cy_DMA_Channel_Enable(DMA_1_HW, DMA_1_DW_CHANNEL);
    Cy_DMA_Enable(DMA_1_HW);
    DMA_1_SetInterruptMask(1);
}

// FUNCTION:
// Configures the second DMA used in transferring data from ADC to ping pong set
void config_DMA_2(void){
    cy_stc_dma_channel_config_t channelConfig =
    {
        .descriptor = &DMA_2_Descriptor_1,
        .preemptable = DMA_2_PREEMPTABLE,
        .priority = DMA_2_PRIORITY,
        .enable = false
    };
    
    (void)Cy_DMA_Descriptor_Init(&DMA_2_Descriptor_1, &DMA_2_Descriptor_1_config);
    (void)Cy_DMA_Descriptor_Init(&DMA_2_Descriptor_2, &DMA_2_Descriptor_2_config);
    (void)Cy_DMA_Channel_Init(DMA_2_HW, DMA_2_DW_CHANNEL, &channelConfig);
    
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_2_Descriptor_1, (uint32_t*) &(SAR->CHAN_RESULT[1]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_2_Descriptor_1, DMA_2_Buff_1);
    Cy_DMA_Descriptor_SetSrcAddress(&DMA_2_Descriptor_2, (uint32_t*) &(SAR->CHAN_RESULT[1]));
    Cy_DMA_Descriptor_SetDstAddress(&DMA_2_Descriptor_2, DMA_2_Buff_2);
    Cy_DMA_Channel_Enable(DMA_2_HW, DMA_2_DW_CHANNEL);
    Cy_DMA_Enable(DMA_2_HW);
    DMA_2_SetInterruptMask(1);
}

// FUNCTION: 
// DMA ISR Handler
void Isr_dma_1(void) {
    //Cy_SCB_UART_PutString(UART_HW, "here\r\n");
    Cy_SysLib_DelayUs(1000);
    stage = !stage;
    DMA_1_ClearInterrupt();
    DMA_2_ClearInterrupt();
    NVIC_ClearPendingIRQ(DMA_Int_cfg.intrSrc);
}


// FUNCTION:
// Used to initialize all the componets that require initialization
// Intended to clear up main function
void initialize(void) {
    Cy_SCB_UART_Init(UART_HW, &UART_config,&UART_context);
    Cy_SCB_UART_Enable(UART_HW);
    ADC_Start();
    ADC_StartConvert();
    
    //Initiallize the interrupts
    Cy_SysInt_Init(&DMA_Int_cfg, Isr_dma_1);
    NVIC_EnableIRQ(DMA_Int_cfg.intrSrc);
  
    config_DMA_1();
    config_DMA_2();
    
    Cy_SCB_UART_PutString(UART_HW, "Began UART\r\n");
    
}

