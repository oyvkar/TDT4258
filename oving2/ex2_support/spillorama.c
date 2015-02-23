//File for playing of 8bit sounds.
#include Danger1.h
#include Beep1.h
#include Explosion1.h

const uint32_l BL = length(beep);
const uint32_l EL = length(explosion);
const uint32_l DL = length(danger);


volatile uint32_l counter = 0;

void danger(){
	if(counter == (DL-1)){
		counter == 0
		timeroff();
		return();
	}
	else{
		DAC0_CH0DATA = danger(counter);
		DAC0_DC1DATA = danger(counter);
		counter++;
	}	
}

void explosion(){
	if(counter == (EL-1)){
		counter == 0
		timeroff();
		return();
	}
	else{
		DAC0_CH0DATA = explosion(counter);
		DAC0_DC1DATA = explosion(counter);
		counter++;
	}	
}




void beep(){
	if(counter == (BL-1)){
		counter == 0
		timeroff();
		return();
	}
	else{
		DAC0_CH0DATA = beep(counter);
		DAC0_DC1DATA = beep(counter);
		counter++;
	}	
}


