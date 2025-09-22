#ifndef SPEECH_H
#define SPEECH_H

#include <stdint.h>
#include <stdio.h>
#include "pv_picovoice.h"

uint8_t SPEECH_Init(void *memory_ptr);

//! Public callback functions for Picovoice
void SPEECH_WakeWordCallback(void);
void SPEECH_InferenceCallback(pv_inference_t *inference);

#endif