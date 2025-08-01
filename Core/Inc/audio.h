#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdio.h>

void AUDIO_Start(void);
int16_t *AUDIO_GetBuffer(void);

#endif