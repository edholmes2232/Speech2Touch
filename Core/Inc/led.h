#ifndef LED_H
#define LED_H

#include "tim.h"

#include <stdint.h>

typedef enum
{
  LED_0,
  LED_1,
  LED_2,
  LED_3,
  LED_COUNT,
} LED_T;

void LED_SetState(LED_T led, uint8_t state);
void LED_TimerCallback(TIM_HandleTypeDef *htim);

#endif
