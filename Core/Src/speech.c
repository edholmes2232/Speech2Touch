#include "audio.h"
#include "led.h"
#include "pv_params.h"
#include "pv_picovoice.h"
#include "pv_porcupine_params.h"
#include "stm32wbxx_hal.h"
#include "usart.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MEMORY_BUFFER_SIZE (70 * 1024)
#define UUID_SIZE (12)
static const char *ACCESS_KEY = "gxAzFazhrfCb18Bh1TpM6zCuanch5AcspBfb6fnwteM4Vk+bRAfHPw==";

static int8_t _memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));
static uint8_t _uuid[UUID_SIZE];
static pv_picovoice_t *_handle = NULL;

static const float PORCUPINE_SENSITIVITY = 0.9f;
static const float RHINO_SENSITIVITY = 0.9f;
static const float RHINO_ENDPOINT_DURATION_SEC = 1.0f;
static const bool RHINO_REQUIRE_ENDPOINT = true;

static void wake_word_callback(void)
{
  printf("[wake word]\n");
  LED_SetState(LED_0, 1);
}

static void inference_callback(pv_inference_t *inference)
{
  // BSP_LED_Off(LED_GREEN);
  LED_SetState(LED_0, 0);
  printf("{\n");
  printf("    is_understood : '%s',\n", (inference->is_understood ? "true" : "false"));
  if (inference->is_understood)
  {
    printf("    intent : '%s',\n", inference->intent);
    if (inference->num_slots > 0)
    {
      printf("    slots : {\n");
      for (int32_t i = 0; i < inference->num_slots; i++)
      {
        printf("        '%s' : '%s',\n", inference->slots[i], inference->values[i]);
      }
      printf("    }\n");
    }
  }
  printf("}\n\n");
  for (int32_t i = 0; i < 10; i++)
  {
    // BSP_LED_Toggle(LED_RED);
    LED_SetState(LED_1, i % 2);
    HAL_Delay(30);
  }
  pv_inference_delete(inference);
}

static void error_handler(void)
{
  printf("\r\n");
  while (true)
    ;
}

void print_error_message(char **message_stack, int32_t message_stack_depth)
{
  for (int32_t i = 0; i < message_stack_depth; i++)
  {
    printf("[%ld] %s\n", i, message_stack[i]);
  }
}

uint8_t SPEECH_Init(void)
{
  // Board BSP
  memcpy(_uuid, (uint8_t *)UID_BASE, UUID_SIZE);
  printf("UUID: ");
  for (int i = 0; i < UUID_SIZE; i++)
  {
    printf("%.2x", _uuid[i]);
  }

  printf("\nAUDIO starting...\n");
  AUDIO_Start();

  char **message_stack = NULL;
  int32_t message_stack_depth = 0;
  pv_status_t error_status;

  pv_status_t status = pv_picovoice_init(ACCESS_KEY, // access key
                                         MEMORY_BUFFER_SIZE, // memory size
                                         _memory_buffer, // memory buffer
                                         sizeof(KEYWORD_ARRAY), // keyword model size
                                         KEYWORD_ARRAY, // keyword model
                                         PORCUPINE_SENSITIVITY, // wake word sensitivity
                                         wake_word_callback, // wake word callback
                                         sizeof(CONTEXT_ARRAY), // context model size
                                         CONTEXT_ARRAY, // context model
                                         RHINO_SENSITIVITY, // inference sensitivity
                                         RHINO_ENDPOINT_DURATION_SEC, // endpoint duration
                                         RHINO_REQUIRE_ENDPOINT, // require endpoint
                                         inference_callback, // inference callback
                                         &_handle // handle
  );
  if (status != PV_STATUS_SUCCESS)
  {
    printf("Picovoice init failed: %s\n", pv_status_to_string(status));

    error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
    if (error_status != PV_STATUS_SUCCESS)
    {
      printf("Failed to get error stack: %s\n", pv_status_to_string(error_status));
      error_handler();
    }

    print_error_message(message_stack, message_stack_depth);
    pv_free_error_stack(message_stack);
    error_handler();
  }

  const char *rhino_context = NULL;
  status = pv_picovoice_context_info(_handle, &rhino_context);
  if (status != PV_STATUS_SUCCESS)
  {
    printf("retrieving context info failed with '%s'", pv_status_to_string(status));
    error_handler();
  }
  printf("Rhino context info: %s\r\n", rhino_context);

  printf("Picovoice expecting frame_length: %d, sample_rate: %d\n", pv_picovoice_frame_length(), pv_sample_rate());

  return EXIT_SUCCESS;
}

// to be called in while loop
uint8_t SPEECH_Process(void)
{
  static int16_t speech_buffer[512];

  const int16_t *buffer = AUDIO_GetBuffer();

  if (buffer)
  {
    memcpy(speech_buffer, buffer, 512 * sizeof(int16_t));

    // uint32_t start = HAL_GetTick();

#ifdef AUDIO_OVER_USART
    HAL_UART_Transmit(&huart1, (uint8_t *)buffer, 512 * sizeof(int16_t), HAL_MAX_DELAY);
#else

    const pv_status_t status = pv_picovoice_process(_handle, speech_buffer);
    if (status != PV_STATUS_SUCCESS)
    {
      printf("Picovoice process failed: %s\n", pv_status_to_string(status));
      error_handler();
    }
#endif
  }

  return EXIT_SUCCESS;
}
