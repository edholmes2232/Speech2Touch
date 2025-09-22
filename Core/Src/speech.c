/*!
 * @file speech.c
 * @brief Speech processing module using Picovoice SDK
 */

#include "speech.h"

#include "audio.h"
#include "led.h"
#include "log.h"
#include "picovoice.h"
#include "pv.h"
#include "stm32wbxx_hal.h"
#include "touch_mapper.h"
#include "touch_targets.h"
#include "tx_api.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TIMEOUT_PERIOD_TICKS (8 * TX_TIMER_TICKS_PER_SECOND) // 8 seconds

//! Temporary storage of speech samples
static int16_t _speech_buffer[512];

//! Azure RTOS settings
#define SPEECH_THREAD_STACK_SIZE (1024)
#define SPEECH_THREAD_PRIO (1)
#define SPEECH_THREAD_PREEMPTION_THRESHOLD (SPEECH_THREAD_PRIO)
#define SPEECH_THREAD_TIME_SLICE (TX_NO_TIME_SLICE)

static TX_THREAD _speech_thread;
static TX_TIMER _speech_timeout_timer;

static void SPEECH_TimeoutCallback(ULONG arg);
static void SPEECH_Process(ULONG thread_input);
static void SPEECH_Reset(void);

#define START_SPEECH_TIMEOUT() tx_timer_activate(&_speech_timeout_timer);
#define RESET_SPEECH_TIMEOUT() tx_timer_change(&_speech_timeout_timer, TIMEOUT_PERIOD_TICKS, TIMEOUT_PERIOD_TICKS);
#define STOP_SPEECH_TIMEOUT() tx_timer_deactivate(&_speech_timeout_timer);

/**
 * @brief Wake word detection callback - called by Picovoice when wake word is detected.
 *
 * Sets LED to listening state, resets touch mapper, and starts timeout timer.
 */
void SPEECH_WakeWordCallback(void)
{
  log_info("[wake word]\n");
  LED_SetState(LED_STATE_LISTENING);

  TOUCHMAPPER_ResetState();

  // Start the timeout timer
  RESET_SPEECH_TIMEOUT();
  UINT status = START_SPEECH_TIMEOUT();
  if (status != TX_SUCCESS)
  {
    log_fatal("Failed to start SPEECH timeout timer: %d", status);
    return;
  }
}

/**
 * @brief Maps a beverage name string to a touch target enum.
 *
 * @param target_str The beverage name string from speech recognition
 * @return TARGET_T enum value or TARGET_COUNT if not found
 * @note Called by SPEECH_InferenceCallback when processing beverage orders
 */
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

/**
 * @brief Speech inference callback - called by Picovoice when speech intent is recognized.
 *
 * Processes beverage orders and cancel commands, maps them to touch targets.
 * Updates LED state and forwards valid targets to touch mapper.
 */
void SPEECH_InferenceCallback(pv_inference_t *inference)
{
  static const char *beverage_slot = "beverage";
  static uint8_t beverage_slot_len = 8; // Length of "beverage"
  static const char *cancel_slot = "cancel";
  static uint8_t cancel_slot_len = 6; // Length of "cancel"

  // Stop timer
  UINT status = STOP_SPEECH_TIMEOUT();
  if (status != TX_SUCCESS)
  {
    log_error("Failed to deactivate SPEECH timeout timer: %d", status);
  }

  if (inference->is_understood)
  {
    log_info("Command understood");
    log_info("Intent : '%s'", inference->intent);

    if (inference->num_slots > 0)
    {
      // No need to verify the intent, only one intent registered with num_slots > 0

      // Verify "beverage"
      if (strncmp(inference->slots[0], beverage_slot, beverage_slot_len) == 0)
      {
        TARGET_T target = getTargetFromString(inference->values[0]);
        if (target != TARGET_COUNT)
        {
          log_info("Target: %s", touch_targets[target].name);

          LED_SetState(LED_STATE_PROCESSING);

          TOUCHMAPPER_HandleTarget(target);
        }
      }
    }
    else if (inference->num_slots == 0)
    {
      if (strncmp(inference->intent, cancel_slot, cancel_slot_len) == 0)
      {
        log_info("Cancel command received");
        // LED should show same as an error
        LED_SetState(LED_STATE_ERROR);
      }
    }
    else
    {
      log_error("Command not understood");
      LED_SetState(LED_STATE_ERROR);
    }
  }
  pv_inference_delete(inference);
}

/**
 * @brief Initializes the speech processing module.
 *
 * @param memory_ptr Pointer to ThreadX byte pool for memory allocation
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
uint8_t SPEECH_Init(void *memory_ptr)
{
  // Initialize Picovoice
  pv_status_t pv_status = PV_Init(SPEECH_WakeWordCallback, SPEECH_InferenceCallback);
  if (pv_status != PV_STATUS_SUCCESS)
  {
    log_fatal("Failed to initialize Picovoice");
    return EXIT_FAILURE;
  }

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

  // Allocate memory for the SPEECH timeout timer
  rtos_status = tx_timer_create(&_speech_timeout_timer,
                                "SPEECH Timeout Timer",
                                SPEECH_TimeoutCallback,
                                0,
                                TIMEOUT_PERIOD_TICKS,
                                TIMEOUT_PERIOD_TICKS,
                                TX_NO_ACTIVATE);
  if (rtos_status != TX_SUCCESS)
  {
    log_fatal("Failed to create SPEECH timeout timer: %d", rtos_status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

/**
 * @brief Timeout callback for speech processing - called by ThreadX timer.
 *
 * Resets Picovoice when no speech is detected within timeout period.
 * Deactivates timeout timer and sets LED to error state.
 */
static void SPEECH_TimeoutCallback(ULONG arg)
{
  (void)arg;

  // Stop timer
  UINT status = STOP_SPEECH_TIMEOUT();
  if (status != TX_SUCCESS)
  {
    log_error("Failed to deactivate SPEECH timeout timer: %d", status);
  }

  log_warn("SPEECH timeout, resetting Picovoice");
  LED_SetState(LED_STATE_ERROR);
  SPEECH_Reset();
}

/**
 * @brief Main speech processing thread - runs continuously as ThreadX thread.
 *
 * Captures audio buffers and processes them through Picovoice.
 */
static void SPEECH_Process(ULONG thread_input)
{
  log_debug("SPEECH_Process started");
  (void)thread_input;
  int16_t *buffer;
  uint8_t status;
  pv_status_t pv_status;

  log_info("AUDIO starting...");
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

    memcpy(_speech_buffer, buffer, 512 * sizeof(int16_t));

    AUDIO_ReleaseBuffer(buffer);
    buffer = NULL;

#ifdef AUDIO_OVER_USART
    // HAL_UART_Transmit(&huart1, (uint8_t *)speech_buffer, 512 * sizeof(int16_t), HAL_MAX_DELAY);
#else
    pv_status = PV_Process(_speech_buffer);
    if (pv_status != PV_STATUS_SUCCESS)
    {
      log_error("Picovoice process failed");
    }
#endif
  }

  log_debug("Terminated thread");
}

/**
 * @brief Resets the speech processing engine after timeout or error.
 *
 * Stops audio, deletes and reinitializes Picovoice, then resumes audio.
 */
static void SPEECH_Reset(void)
{
  // DEBUG: record start time from sysclk
  uint32_t start_time = HAL_GetTick();

  // Stop AUDIO DMAs
  AUDIO_Stop();

  // Delete Picovoice instance
  PV_Delete();

  if (PV_Init(SPEECH_WakeWordCallback, SPEECH_InferenceCallback) != PV_STATUS_SUCCESS)
  {
    log_fatal("Failed to re-initialize Picovoice");
    return;
  }

  // Resume audio capture
  AUDIO_Start();

  log_debug("SPEECH_Reset completed in %d ms", HAL_GetTick() - start_time);
}
