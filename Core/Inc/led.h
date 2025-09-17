#ifndef LED_H
#define LED_H

#include "tim.h"

#include <stdint.h>

// System LED states
typedef enum
{
  LED_STATE_IDLE,
  LED_STATE_LISTENING,
  LED_STATE_PROCESSING,
  LED_STATE_ERROR
} LED_STATE_T;

void LED_SetState(LED_STATE_T state);
void LED_TimerCallback(TIM_HandleTypeDef *htim);
void LED_SetProcessingComplete(void);

#endif
