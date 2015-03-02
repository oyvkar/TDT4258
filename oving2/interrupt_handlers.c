#include <stdint.h>
#include <stdbool.h>

#include "sounds.h"
#include "efm32gg.h"
#include "timer.h"
#include "dac.h"

volatile soundtype sound = laser;
volatile bool samplingfix = 0; 


/*LETIMER0 interrupt handler */
void __attribute__ ((interrupt)) LETIMER0_IRQHandler(){


//To ensure 44100 sampling rate the timer has to count twice, as it has only 32000 available from oscillator
if(~samplingfix){
	playSound(sound);
}else{
	samplingfix= ~samplingfix;
}

*LETIMER0_IFC = 1;

}

void GPIO_HANDLER() { 
	switch((*GPIO_PC_DIN)){
		case 0xfe:
			dacon();
			LETimeron(); // Start the timer
			sound = laser;
			break;
		case 0xfd:
			dacon();
			LETimeron(); // Start the timer
			sound = danger;
			break;
		case 0xfb:
			dacon();
			LETimeron(); // Start the timer
			sound = explosion;
			break;
		case 0xf7:
			dacon();
			LETimeron(); // Start the timer
			sound = beep;
			break;
		case 0xef:
			dacon();
			LETimeron();
			sound = emergency;
			break;
		case 0xdf:
			break;
		case 0xbf:
			break;
		case 0x7f:
			LETimeroff();
			dacoff();
			break;
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
