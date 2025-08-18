#ifndef SPEECH_H
#define SPEECH_H

#include <stdint.h>
#include <stdio.h>

uint8_t SPEECH_Init(void *memory_ptr);
uint8_t SPEECH_Process(void);

#endif