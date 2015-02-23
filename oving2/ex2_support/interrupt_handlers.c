#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"
#include "timer.h"
bool iterate = false;
int counter = 0;
int duration;



	
volatile uint32_t bla = 0;
volatile uint32_t bla2 = 0;
/* TIMER1 interrupt handler */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() 
{  
  /*
    TODO feed new samples to the DAC
    remember to clear the pending interrupt by writing 1 to TIMER1_IFC
  */

  //sawtooth(200, 20);
  bla2++;
  bla ++;
  if (bla == 1) { 
	*DAC0_CH0DATA = 0b00100011111;
	*DAC0_CH1DATA = 0b00100011111;
  }
 
  else if (bla == 2)  {
	*DAC0_CH0DATA = 0b00000000000;
	*DAC0_CH1DATA = 0b00000000000;
	bla = 0;
  }
  
  if (bla2 == 700) {
	*TIMER1_TOP = 700;
  } else if (bla2 == 14000) {
	*TIMER1_TOP = 14000;
         bla2 = 0;
  }
  *TIMER1_IFC = 1; 
}
void pin_interrupt(uint32_t i, bool even) {

if (even) {
switch (i) {
	case 0:
		*GPIO_PA_DOUT &= ~(1 << 8);
		break;
	case 1:
		timeroff();
		break;
	default:
		*GPIO_PA_DOUT &= ~(7 << 8);
	}
} else {
switch(i) {
	case 0:
		*GPIO_PA_DOUT |= (1 << 8);
		break;
	case 1:
		timeron();
		break;
	default:
		*GPIO_PA_DOUT |= (7 << 8);
	}
   } 
	
}

void GPIO_HANDLER(bool even) {
	for (int i; i < 8; i++) {
		if ( *GPIO_IF & (1 << (i))) {
			pin_interrupt(i, even);
			*GPIO_IFC = (1 << i);
		}
	}
}

/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
  GPIO_HANDLER(1);
}

/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
  GPIO_HANDLER(0);
}
