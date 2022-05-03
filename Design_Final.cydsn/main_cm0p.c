/******************************************************************************
* File Name: main_cm0p.c
*
* This core will be used for terminal communications
*
*/
#include "project.h"
#include "TinyScope.h"

int main(void)
{
    /* __enable_irq(); */ /* Enable global interrupts. */
	
    /* Enable CM4.  CY_CORTEX_M4_APPL_ADDR must be updated if CM4 memory layout is changed. */
    Cy_SysEnableCM4(CY_CORTEX_M4_APPL_ADDR); 
    /*
    UART_Start();
    C2C_Start();
    printf("Started UART\r\n");
    
    int data[7];

    char str[STR_LEN]; // will take in command line input
    char* tok1 = NULL;
    char* tok2 = NULL;
    char* tok3 = NULL;
    for (;;) {
        getLine(str); // read in line
        tok1 = strtok(str, " ");
        if (tok1 != NULL) {
            tok2 = strtok(NULL, " ");
            if (tok2 != NULL) {
                tok3 = strtok(NULL, " ");
             }
        }
        printf("%s %s %s\r\n", tok1, tok2, tok3);
        
    }
    uint32 test = 4;
    C2C_Put(test);
    */
}

/* [] END OF FILE */
