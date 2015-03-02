//File for playing of 8bit sounds.
#include "efm32gg.h"
#include "timer.h"
#include "sounds.h"
#include "dac.h"

// Sound files
#include "sound/laser1_uint12.h"
#include "sound/explosion3_uint12.h"
#include "sound/danger1_uint12.h"
#include "sound/beep3_uint12.h"
#include "sound/emergency_uint12.h"

volatile uint32_t count = 0;
volatile soundtype current;

void playSound(soundtype sound) {
	const uint32_t *playlen;
	const uint16_t *sound_p;
	
	// Reset count when changing sound
	if (sound != current) {
		count = 0;
		current = sound;
	}

	// Set pointers to point to correct sound
	switch (sound) {
	case explosion:
		playlen = &explosionlen;
		sound_p = explosionsound;
		break;
	case danger:
		playlen = &dangerlen;
		sound_p = dangersound;
		break;
	case laser:
		playlen = &laserlen;
		sound_p = lasersound;
		break;
	case beep:
		playlen = &beeplen;
		sound_p = beepsound;
		break;
	case emergency:
		playlen = &emergencylen;
		sound_p = emergencysound;
	}

	// Push data to DAC or stop timer when done	
	if (count == (*playlen - 1)) {
		count = 0;
		LETimeroff();
		dacoff();
		return;
	} else {
		*DAC0_CH0DATA = sound_p[count];
		*DAC0_CH1DATA = sound_p[count];
		count ++;
	}
}
