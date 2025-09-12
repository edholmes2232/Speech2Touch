#include "led.h"

#include "log.h"
#include "main.h"
#include "stm32wbxx_hal_gpio.h"
#include "stm32wbxx_hal_tim.h"
#include "tim.h"
// Internal state
static LED_STATE_T _current_state = LED_STATE_IDLE;
static uint8_t _error_flash_count = 0;
static uint8_t _error_flash_phase = 0;
static uint16_t _error_timer_count = 0;

void LED_SetState(LED_STATE_T state)
{
  if (state == _current_state)
  {
    return;
  }

  HAL_TIM_PWM_Start(&htim16, TIM_CHANNEL_1);

  _current_state = state;
  switch (state)
  {
  case LED_STATE_IDLE:
    log_trace("LED State: IDLE");
    // HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
    HAL_TIM_Base_Stop_IT(&htim16);
    __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 0);
    break;
  case LED_STATE_LISTENING:
    log_trace("LED State: LISTENING");
    // HAL_TIM_PWM_Stop(&htim16, TIM_CHANNEL_1);
    HAL_TIM_Base_Stop_IT(&htim16);
    __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 999);
    break;
  case LED_STATE_PROCESSING:
    log_trace("LED State: PROCESSING");
    HAL_TIM_Base_Start_IT(&htim16);
    break;
  case LED_STATE_ERROR:
    log_trace("LED State: ERROR");
    _error_flash_count = 0;
    _error_flash_phase = 1; // Start with LED ON
    _error_timer_count = 0;
    HAL_TIM_Base_Start_IT(&htim16);
    __HAL_TIM_SET_COMPARE(&htim16, TIM_CHANNEL_1, 999);
    break;
  }
}

void LED_TimerCallback(TIM_HandleTypeDef *htim)
{
  static uint16_t pwm_value = 0;
  static int8_t direction = 1;
  const uint16_t pwm_min = 1;
  const uint16_t pwm_max = 998;
  const uint16_t pwm_step = 2;

  // 300ms = 300 ticks of timer (1kHz)
  const uint16_t flash_duration_ticks = 200;

  if (htim->Instance != TIM16)
  {
    return;
  }

  if (_current_state == LED_STATE_PROCESSING)
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
  else if (_current_state == LED_STATE_ERROR)
  {
    _error_timer_count++;

    if (_error_timer_count >= flash_duration_ticks)
    {
      _error_timer_count = 0;
      _error_flash_phase = !_error_flash_phase;

      if (_error_flash_phase)
      {
        // Just turned ON, count a full ON-OFF cycle
        _error_flash_count++;
        if (_error_flash_count >= 3)
        {
          LED_SetState(LED_STATE_IDLE);
          return;
        }
      }
    }

    if (_error_flash_phase)
    {
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 999);
    }
    else
    {
      __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);
    }
  }
  else
  {
    __HAL_TIM_SET_COMPARE(htim, TIM_CHANNEL_1, 0);
  }
}