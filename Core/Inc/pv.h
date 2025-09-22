#ifndef PV_H
#define PV_H

#include "picovoice.h"
#include "pv_picovoice.h"

#include <stdint.h>

//! Callback function types
typedef void (*pv_wake_word_callback_t)(void);
typedef void (*pv_inference_callback_t)(pv_inference_t *inference);

//! Public functions
pv_status_t PV_Init(pv_wake_word_callback_t wake_callback, pv_inference_callback_t inference_callback);
void PV_Delete(void);
pv_status_t PV_Process(const int16_t *audio_buffer);

#endif