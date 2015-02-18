#include <stdint.h>
#include <stdbool.h>

#include "efm32gg.h"

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

void GPIO_HANDLER()
{
uint32_t a = ( *GPIO_PC_DIN & (1 << 0));
uint32_t b = ( *GPIO_PC_DIN & (1 << 1));

  if (a) {
	*GPIO_PA_DOUT &= ~(1 << 8);
        timeroff();
  }
  if (b) { 
	*GPIO_PA_DOUT |= (1 << 8);
	timeron();
  } 
	
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
