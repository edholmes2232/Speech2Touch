#ifndef LED_H
#define LED_H

#include <stdint.h>

typedef enum
{
  LED_0,
  LED_1,
  LED_2,
  LED_COUNT,
} LED_T;

void LED_SetState(LED_T led, uint8_t state);

#endif
