#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

#define TIMER_CLOCK 14000000

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
