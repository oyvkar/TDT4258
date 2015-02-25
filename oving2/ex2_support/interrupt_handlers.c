#include <stdint.h>
#include <stdbool.h>
#include "spillorama.h"
#include "efm32gg.h"
#include "timer.h"
volatile int sound = 0;
volatile bool busybool = 0;
volatile bool samplingfix = 0; //To ensure 44100 sampling rate the timer has to count twice, as it has only 32000 available from oscillator




/*LETIMER0 interrupt handler */
void __attribute__ ((interrupt)) LETIMER0_IRQHandler(){


if(~samplingfix){
	switch(sound) {
		case 0: 
			// Do nothing
			break;
		case 1: 
			busybool = laserf();
			break;
		case 2:
			busybool = dangerf();
			break;
		case 3:
			busybool = explosionf();
		}
}else{
	samplingfix= ~samplingfix;
}

*LETIMER0_IFC = 1;

}


/* TIMER1 interrupt handler (unused */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() {
*TIMER1_IFC = 1;  
}
void GPIO_HANDLER() { 

if(busybool){
	*GPIO_IFC = 0xff;
}
else{
	switch((*GPIO_PC_DIN)){
		case 0xfe:
			*GPIO_PA_DOUT = ~(*GPIO_PA_DOUT);
			LETimeron(); // Start the timer
			sound = 1;
			break;
		case 0xfd:
			LETimeron(); // Start the timer
			sound = 2;
			break;
		case 0xfb:
			LETimeron(); // Start the timer
			sound = 3;
			break;
		case 0xf7:
			break;
		case 0xef:
			break;
		case 0xdf:
			break;
		case 0xbf:
			break;
		case 0x7f:
			break;
	}

}
*GPIO_IFC = 0xff; //Clear interrupt flags
}

/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
  GPIO_HANDLER(); // Use common function for off and even GPIO interrupts
}

/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
  GPIO_HANDLER(); //Use common function for odd and even GPIO interrupts
}
