#include <stdint.h>
#include <stdbool.h>
#include "spillorama.h"
#include "efm32gg.h"
#include "timer.h"
bool iterate = false;
int counter = 0;
int duration;
int sound = 1;


	
volatile bool even = false;

/* TIMER1 interrupt handler */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() 
{  

  switch(sound) {
	case 1:
		explosionf();
		break;
	case 2:
		beepf();
		break;
	case 3:
		dangerf();
		break;
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
		*GPIO_PA_DOUT &= ~(1 << 8);
		break;
	case 2:
		*GPIO_PA_DOUT &= ~(1 << 8);
	//	sound = 1;
	//	timeron();
		break;
	case 3:
		*GPIO_PA_DOUT &= ~(1 << 8);
	//	sound = 2;
	//	timeron();
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
	//	timeron();
		break;
	default:
		*GPIO_PA_DOUT |= (7 << 8);
	}
   } 
	
}

void GPIO_HANDLER() {
	even = !even; 
	for (int i = 0; i < 8; i++) {
		if ( *GPIO_IF & (1 << (i))) {
			pin_interrupt(i, even);
			*GPIO_IFC = (1 << i);
		}
	}
}

/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
  GPIO_HANDLER();
}

/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
  GPIO_HANDLER();
}
