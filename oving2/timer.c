#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

#define TIMER_CLOCK 14000000

volatile uint16_t period;

/* function to setup the timer */
void setupTimer(uint16_t a)
{
	period = a;

}
void timeron() {
	*CMU_HFPERCLKEN0 |= (1 << 6); 
	*TIMER1_TOP = period;
	//*CMU_HFPERCLKEN0 |= (1 << 6); //Set bit 6 high to enable clock to timer
	*TIMER1_IEN = 1; 
	*TIMER1_CMD = 1;
	//*ISER0 |= (1 << 12); // Enable timer interupt
}

void timeroff() {
	*TIMER1_IEN = 0;
	*TIMER1_CMD = 0;
        *CMU_HFPERCLKEN0 &= ~(1 << 6); //Set bit 6 high to enable clock to timer
        //*ISER0 &= ~(1 << 12); // Enable timer interupt
}

// To reduce the energy consumption a low energy timer is used in conjuction with deep sleep mode
void LETimeron(){
	*CMU_OSCENCMD = (1<<6);
	*CMU_HFCORECLKEN0 |= (1<<4);
	*LETIMER0_CTRL |= (1<<9);
	*CMU_LFACLKEN0 |= (1<<2);
	*LETIMER0_TOP = 32768/44100; //Since our sampling rate is 44100, we use two periods of LETIMER0 to achieve our sampling rate
	*LETIMER0_IEN = 1;
	*LETIMER0_CMD = 1;
}


void LETimeroff(){
	*CMU_OSCENCMD &= ~(1<<6);
	*CMU_HFCORECLKEN0 &= (1 << 4);
	*LETIMER0_CTRL &= (1<<9);
	*CMU_LFACLKEN0 &= (1<<2);
	*LETIMER0_TOP = 0;
	*LETIMER0_IEN = 0;
	*LETIMER0_CMD = 0;
}
