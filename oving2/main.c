#include <stdint.h>
#include <stdbool.h>
#include "timer.h"
#include "efm32gg.h"

/* Declaration of peripheral setup functions */
void setupDAC();
void setupNVIC();
void setupGPIO();
/* Your code will start executing here */
int main(void) 
{  
  /* Call the peripheral setup functions */
  setupGPIO();
  setupDAC();
  LETimeron(); // Setup Low Energy Timer 
  /* Enable interrupt handling */
  setupNVIC(); 
  *SCR = 6; // Setup deep sleep, note: conflicts with regular timer, therefore Low Energy timer is used instead
  __asm("WFI");
  return 0;
}

void setupNVIC() { 
  *ISER0 |= (1 << 11); // Enable GPIO ODD interupt
  *ISER0 |= (1 << 1); //  Enanle GPIO EVEN interupt
  *ISER0 |= (1 << 26); // Enable LETIMER0 interrupt
}
