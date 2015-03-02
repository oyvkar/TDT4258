#ifndef SPILLORAMA_H
#define SPILLORAMA_H

// Enum containg names for all sounds
typedef enum { laser, danger, explosion, beep, emergency, melodi } soundtype;
void playSound(soundtype sound);

#endif
