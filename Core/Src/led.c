#include "led.h"

#include "main.h"
#include "stm32wbxx_hal_gpio.h"

void LED_SetState(LED_T led, uint8_t state)
{
  switch (led)
  {
  case LED_0:
    HAL_GPIO_WritePin(LD1_GPIO_Port, LD1_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case LED_1:
    HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  case LED_2:
    HAL_GPIO_WritePin(LD3_GPIO_Port, LD3_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    break;
  default:
    // Invalid LED index
    break;
  }
}