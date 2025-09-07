#include "led.h"

#include "main.h"
#include "stm32wbxx_hal_gpio.h"
#include "stm32wbxx_hal_tim.h"
#include "tim.h"

// Breathing LED state
static uint8_t breathing_enabled = 0;

void LED_TimerCallback(TIM_HandleTypeDef *htim)
{
  static uint16_t pwm_value = 0;
  static int8_t direction = 1;
  const uint16_t pwm_min = 1;
  const uint16_t pwm_max = 998;
  const uint16_t pwm_step = 2; // Adjust for speed/smoothness

  if ((breathing_enabled) && (htim->Instance == TIM16))
  {
    pwm_value += direction * pwm_step;
    if (pwm_value >= pwm_max)
    {
      pwm_value = pwm_max;
      direction = -1;
    }
    else if (pwm_value <= pwm_min)
    {
      pwm_value = pwm_min;
      direction = 1;
    }
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, pwm_value);
  }
  else if ((!breathing_enabled) && (htim->Instance == TIM16))
  {
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);
  }
}

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
  case LED_3:
    if (state == 0)
    {
      HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
      HAL_TIM_Base_Stop_IT(&htim16);
      __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 0);
      breathing_enabled = 0;
    }
    else
    {
      breathing_enabled = 1;
      HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);
      HAL_TIM_Base_Start_IT(&htim16);
    }
    break;
  default:
    // Invalid LED index
    break;
  }
}