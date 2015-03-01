//File for playing of 8bit sounds.
#include "efm32gg.h"
#include "timer.h"
//#include "danger1.h"
#include "sound/laser1_uint12.h"
#include "sound/explosion3_uint12.h"
#include "sound/danger1_uint12.h"


const uint32_t LL = 14962;
const uint32_t EL = 43523;
const uint32_t DL = 79223;

volatile uint32_t count = 0;

int dangerf(){
	if(count == (DL-1)){
		count = 0;
		LETimeroff();
		return 0; // The chip is no longer playing audio
	}
	else{
		*DAC0_CH0DATA = danger[count];
		*DAC0_CH0DATA = danger[count];
		count++;
		return 1; // The chip is currently playing audio
	}	
}

int explosionf(){
	if(count == (EL-1)){
		count = 0;
		LETimeroff();
		return 0; // The chip is no longer playing audio, and is ready for new input
	}
	else{
		*DAC0_CH0DATA = explosion[count];
		*DAC0_CH1DATA = explosion[count];
		*GPIO_PA_DOUT &= ~(0 << 8);
		count++;
		return 1; // The chip is currently playing audio
	}	
}


int laserf(){
	if(count == (LL-1)){
		count = 0;
		LETimeroff();
		return 0; //The chip is no longer playing audio, and is ready for new input
	}
	else{
		*DAC0_CH0DATA = laser[count];
		*DAC0_CH1DATA = laser[count];
		count++;
		return 1; //The chip is currently playing audio
	}	
}

