//File for playing of 8bit sounds.
#include "efm32gg.h"
#include "danger1.h"
#include "beep1.h"
#include "explosion1.h"

const uint32_l BL = length(beep);
const uint32_l EL = length(explosion);
const uint32_l DL = length(danger);


volatile uint32_l counter = 0;

void dangerf(){
	if(counter == (DL-1)){
		counter == 0
		timeroff();
		return();
	}
	else{
		DAC0_CH0DATA = danger(counter);
		DAC1_CH1DATA = danger(counter);
		counter++;
	}	
}

void explosionf(){
	if(counter == (EL-1)){
		counter == 0
		timeroff();
		return();
	}
	else{
		DAC0_CH0DATA = explosion(counter);
		DAC1_CH1DATA = explosion(counter);
		counter++;
	}	
}




void beepf(){
	if(counter == (BL-1)){
		counter == 0
		timeroff();
		return();
	}
	else{
		DAC0_CH0DATA = beep(counter);
		DAC1_CH1DATA = beep(counter);
		counter++;
	}	
}


