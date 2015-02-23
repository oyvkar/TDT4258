//File for playing of 8bit sounds.
#include "efm32gg.h"
#include "timer.h"
#include "danger1.h"
#include "beep1.h"
#include "explosion1.h"

const uint32_t BL = 4096;
const uint32_t EL = 4096;
const uint32_t DL = 4096;

const uint8_t offset = 255;

volatile uint32_t count = 0;

void dangerf(){
	if(count == (DL-1)){
		count = 0;
	//	timeroff();
		return;
	}
	else{
		*DAC0_CH0DATA = danger[count] + offset;
		*DAC0_CH0DATA = danger[count] + offset;
		count++;
	}	
}

void explosionf(){
	if(count == (EL-1)){
		count = 0;
	//	timeroff();
		return;
	}
	else{
		*DAC0_CH0DATA = explosion[count] + offset;
		*DAC0_CH1DATA = explosion[count] + offset;
		count++;
	}	
}




void beepf(){
	if(count == (BL-1)){
		count = 0;
	//	timeroff();
		return;
	}
	else{
		*DAC0_CH0DATA = beep[count] + offset;
		*DAC0_CH1DATA = beep[count] + offset;
		count++;
	}	
}


