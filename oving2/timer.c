#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

#define TIMER_CLOCK 14000000

// To reduce the energy consumption a low energy timer is used in conjuction with deep sleep mode
void LETimeron(){
	*CMU_OSCENCMD = (1<<6); // Enable LFRCO
	*CMU_HFCORECLKEN0 |= (1<<4); // Enable clock for Low Energey pheripherals
	*LETIMER0_CTRL |= (1<<9); // Compare topvalue to LETIMER0_TOP
	*CMU_LFACLKEN0 |= (1<<2); // Enable LEtimer0
	*LETIMER0_TOP = 32768/44100; //Since our sampling rate is 44100, we use two periods of LETIMER0 to achieve our sampling rate
	*LETIMER0_IEN = 1; // Enable LETimer0 interrupt
	*LETIMER0_CMD = 1; // Enalbe LETimer0
}


void LETimeroff(){
	*LETIMER0_CMD = 0;
	*LETIMER0_IEN = 0;
	*CMU_LFACLKEN0 = 0; // Stop LETimer0
	*CMU_HFCORECLKEN0 &= ~(1 << 4); // Disable low energey per clock
	*CMU_OSCENCMD = (1<<7); // Disable LFCRO
}
