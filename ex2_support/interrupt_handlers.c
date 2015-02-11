#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

bool iterate = false;
int counter = 0;
int duration;

void sawtooth(int note, int time)
{
	int sampling = 20000/note;
	
	int slope = (1024)/sampling;
	int y = (slope*duration);

	*DAC0_CH0DATA = y;
	*DAC0_CH1DATA = y;

	duration++;

	if(duration >= sampling)
	{
		duration = 0;
	}
	if(counter==time)
	{
		iterate = true;
		counter = 0;
	}
}
volatile uint32_t bla = 0;
/* TIMER1 interrupt handler */
void __attribute__ ((interrupt)) TIMER1_IRQHandler() 
{  
  /*
    TODO feed new samples to the DAC
    remember to clear the pending interrupt by writing 1 to TIMER1_IFC
  */

  sawtooth(200, 20);
  *TIMER1_IFC = 1;
   
   
}

void GPIO_HANDLER()
{
  uint32_t a = *GPIO_PC_DIN;
  a = a << 8;
  *GPIO_PA_DOUT = a;

  *GPIO_IFC = 1;
}

/* GPIO even pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_EVEN_IRQHandler() 
{
    /* TODO handle button pressed event, remember to clear pending interrupt */
  //*GPIO_IFC = 0xffff;
  GPIO_HANDLER();
}

/* GPIO odd pin interrupt handler */
void __attribute__ ((interrupt)) GPIO_ODD_IRQHandler() 
{
    /* TODO handle button pressed event, remember to clear pending interrupt */
  //*GPIO_IFC = 0xff;
  GPIO_HANDLER();
}
