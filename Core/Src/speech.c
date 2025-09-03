#include "speech.h"

#include "SEGGER_RTT.h"
#include "audio.h"
#include "led.h"
#include "log.h"
#include "pv_picovoice.h"
#include "pv_porcupine_params.h"
#include "pv_rhino_params.h"
#include "stm32wbxx_hal.h"
#include "touch_mapper.h"
#include "touch_targets.h"
#include "tx_api.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//! Picovoice specific defines
#define MEMORY_BUFFER_SIZE (70 * 1024)
#define UUID_SIZE (12)
static const char *ACCESS_KEY = "gxAzFazhrfCb18Bh1TpM6zCuanch5AcspBfb6fnwteM4Vk+bRAfHPw==";

//! Picovoice statics
static int8_t _memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));
static uint8_t _uuid[UUID_SIZE];
static pv_picovoice_t *_handle = NULL;

//! Picovoice settings
static const float PORCUPINE_SENSITIVITY = 0.9f;
static const float RHINO_SENSITIVITY = 0.9f;
static const float RHINO_ENDPOINT_DURATION_SEC = 1.0f;
static const bool RHINO_REQUIRE_ENDPOINT = true;

//! Azure RTOS settings
#define SPEECH_THREAD_STACK_SIZE (1024)
#define SPEECH_THREAD_PRIO (1)
#define SPEECH_THREAD_PREEMPTION_THRESHOLD (SPEECH_THREAD_PRIO)
#define SPEECH_THREAD_TIME_SLICE (TX_NO_TIME_SLICE)

static TX_THREAD _speech_thread;

static void SPEECH_Process(ULONG thread_input);

static void wakeWordCallback(void)
{
  log_info("[wake word]\n");
  LED_SetState(LED_0, 1);

  TOUCHMAPPER_ResetState();
}

static TARGET_T getTargetFromString(const char *target_str)
{
  if (target_str == NULL)
  {
    return TARGET_COUNT; // Invalid target
  }

  for (uint8_t i = 0; i < TARGET_COUNT; i++)
  {
    if (strcmp(target_str, touch_targets[i].name) == 0)
    {
      return (TARGET_T)i;
    }
  }

  // Manually check for "latte". Labelled as "caffe latte" in the targets, but users
  // might say "latte" instead.
  if (strcmp(target_str, "latte") == 0)
  {
    return TARGET_CAFFE_LATTE; // Return the caffe latte target
  }

  log_error("Unknown target: %s", target_str);

  return TARGET_COUNT; // Not found
}

static void inferenceCallback(pv_inference_t *inference)
{
  static const char *beverage_slot = "beverage";
  static uint8_t beverage_slot_len = 8; // Length of "beverage"

  LED_SetState(LED_0, 0);
  printf("{\n");
  printf("    is_understood : '%s',\n", (inference->is_understood ? "true" : "false"));
  if (inference->is_understood)
  {
    printf("    intent : '%s',\n", inference->intent);

    if (inference->num_slots > 0)
    {

      // Check other slots?

      // Verify beverage
      if (strncmp(inference->slots[0], beverage_slot, beverage_slot_len) == 0)
      {
        TARGET_T target = getTargetFromString(inference->values[0]);
        if (target != TARGET_COUNT)
        {
          log_info("Target: %s", touch_targets[target].name);

          TOUCHMAPPER_HandleTarget(target);
        }
      }

      printf("    slots : {\n");
      for (int32_t i = 0; i < inference->num_slots; i++)
      {
        printf("        '%s' : '%s',\n", inference->slots[i], inference->values[i]);
      }
      printf("    }\n");
    }
  }
  printf("}\n\n");
  // for (int32_t i = 0; i < 10; i++)
  // {
  //   LED_SetState(LED_1, i % 2);
  //   HAL_Delay(30);
  // }

  pv_inference_delete(inference);
}

void printErrorMessage(char **message_stack, int32_t message_stack_depth)
{
  for (int32_t i = 0; i < message_stack_depth; i++)
  {
    log_error("[%ld] %s", i, message_stack[i]);
  }
}

uint8_t SPEECH_Init(void *memory_ptr)
{
  // Board BSP
  memcpy(_uuid, (uint8_t *)UID_BASE, UUID_SIZE);
  printf("UUID: ");
  for (int i = 0; i < UUID_SIZE; i++)
  {
    printf("%.2x", _uuid[i]);
  }

  char **message_stack = NULL;
  int32_t message_stack_depth = 0;
  pv_status_t error_status;

  pv_status_t status = pv_picovoice_init(ACCESS_KEY, // access key
                                         MEMORY_BUFFER_SIZE, // memory size
                                         _memory_buffer, // memory buffer
                                         sizeof(KEYWORD_ARRAY), // keyword model size
                                         KEYWORD_ARRAY, // keyword model
                                         PORCUPINE_SENSITIVITY, // wake word sensitivity
                                         wakeWordCallback, // wake word callback
                                         sizeof(CONTEXT_ARRAY), // context model size
                                         CONTEXT_ARRAY, // context model
                                         RHINO_SENSITIVITY, // inference sensitivity
                                         RHINO_ENDPOINT_DURATION_SEC, // endpoint duration
                                         RHINO_REQUIRE_ENDPOINT, // require endpoint
                                         inferenceCallback, // inference callback
                                         &_handle // handle
  );
  if (status != PV_STATUS_SUCCESS)
  {
    log_fatal("Picovoice init failed: %s", pv_status_to_string(status));

    error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
    if (error_status != PV_STATUS_SUCCESS)
    {
      log_fatal("Failed to get error stack: %s", pv_status_to_string(error_status));
      return EXIT_FAILURE;
    }

    printErrorMessage(message_stack, message_stack_depth);
    pv_free_error_stack(message_stack);
    return EXIT_FAILURE;
  }

  const char *rhino_context = NULL;
  status = pv_picovoice_context_info(_handle, &rhino_context);
  if (status != PV_STATUS_SUCCESS)
  {
    log_error("retrieving context info failed with '%s'", pv_status_to_string(status));
    return EXIT_FAILURE;
  }
  log_debug("Rhino context info: %s", rhino_context);
  log_debug("Picovoice expecting frame_length: %d, sample_rate: %d", pv_picovoice_frame_length(), pv_sample_rate());

  // Setup AUDIO
  uint8_t ret = AUDIO_Init(memory_ptr);
  if (ret != EXIT_SUCCESS)
  {
    log_fatal("AUDIO_Init failed");
    return EXIT_FAILURE;
  }

  // Setup ThreadX
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;
  CHAR *thread_storage_ptr;
  // Allocate memory for the SPEECH thread
  UINT rtos_status = tx_byte_allocate(byte_pool, (VOID **)&thread_storage_ptr, SPEECH_THREAD_STACK_SIZE, TX_NO_WAIT);
  if (rtos_status != TX_SUCCESS)
  {
    log_fatal("Failed to allocate memory for SPEECH thread");
    return EXIT_FAILURE;
  }
  // Create the SPEECH thread
  rtos_status = tx_thread_create(&_speech_thread,
                                 "SPEECH Thread",
                                 SPEECH_Process,
                                 0,
                                 thread_storage_ptr,
                                 SPEECH_THREAD_STACK_SIZE,
                                 SPEECH_THREAD_PRIO,
                                 SPEECH_THREAD_PREEMPTION_THRESHOLD,
                                 SPEECH_THREAD_TIME_SLICE,
                                 TX_AUTO_START);
  if (rtos_status != TX_SUCCESS)
  {
    log_fatal("Failed to create SPEECH thread: %d", rtos_status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

int16_t speech_buffer[512];
// to be called in while loop
static void SPEECH_Process(ULONG thread_input)
{
  log_debug("SPEECH_Process started");
  (void)thread_input;
  int16_t *buffer;
  uint8_t status;

  printf("\nAUDIO starting...\n");
  AUDIO_Start();
  for (;;)
  {
    // Blocking call to get audio buffer
    status = AUDIO_GetBuffer(&buffer);
    if ((status != EXIT_SUCCESS) || (buffer == NULL))
    {
      log_error("Failed to get audio buffer: %d", status);
      continue; // Skip processing if buffer is not available
    }
    uint32_t start = HAL_GetTick();

    memcpy(speech_buffer, buffer, 512 * sizeof(int16_t));

    AUDIO_ReleaseBuffer(buffer);
    buffer = NULL;

#ifdef AUDIO_OVER_USART
    // HAL_UART_Transmit(&huart1, (uint8_t *)speech_buffer, 512 * sizeof(int16_t), HAL_MAX_DELAY);
#else

    const pv_status_t status = pv_picovoice_process(_handle, speech_buffer);
    if (status != PV_STATUS_SUCCESS)
    {
      log_error("Picovoice process failed: %s", pv_status_to_string(status));
    }
#endif

    uint32_t end = HAL_GetTick();

    // if ((end - start) > 19)
    // {
    //   log_trace("Took %lu ms", end - start);
    // }
  }
}
