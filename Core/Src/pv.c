/*!
 * @file pv.c
 * @brief Picovoice SDK wrapper module
 */

#include "pv.h"

#include "log.h"
#include "picovoice.h"
#include "pv_access_key.h"
#include "pv_picovoice.h"
#include "pv_porcupine_params.h"
#include "pv_rhino_params.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

//! Picovoice specific defines
#define MEMORY_BUFFER_SIZE (70 * 1024)

#ifndef PV_ACCESS_KEY
#error "ACCESS_KEY must be defined in pv_access_key.h"
#endif
static const char *_access_key = PV_ACCESS_KEY;

//! Picovoice statics
static int8_t _memory_buffer[MEMORY_BUFFER_SIZE] __attribute__((aligned(16)));
static pv_picovoice_t *_handle = NULL;

//! Picovoice settings
static const float PORCUPINE_SENSITIVITY = 0.9f;
static const float RHINO_SENSITIVITY = 0.9f;
static const float RHINO_ENDPOINT_DURATION_SEC = 1.0f;
static const bool RHINO_REQUIRE_ENDPOINT = true;

/**
 * @brief Prints Picovoice error messages from error stack.
 *
 * @param message_stack Array of error message strings
 * @param message_stack_depth Number of messages in the stack
 */
static void printErrorMessage(char **message_stack, int32_t message_stack_depth)
{
  for (int32_t i = 0; i < message_stack_depth; i++)
  {
    log_error("[%ld] %s", i, message_stack[i]);
  }
}

/**
 * @brief Initializes the Picovoice engine.
 *
 * @param wake_callback Callback function for wake word detection
 * @param inference_callback Callback function for speech inference
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on error
 */
pv_status_t PV_Init(pv_wake_word_callback_t wake_callback, pv_inference_callback_t inference_callback)
{
  char **message_stack = NULL;
  int32_t message_stack_depth = 0;
  pv_status_t error_status;

  pv_status_t status = pv_picovoice_init(_access_key, // access key
                                         MEMORY_BUFFER_SIZE, // memory size
                                         _memory_buffer, // memory buffer
                                         sizeof(KEYWORD_ARRAY), // keyword model size
                                         KEYWORD_ARRAY, // keyword model
                                         PORCUPINE_SENSITIVITY, // wake word sensitivity
                                         wake_callback, // wake word callback
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
    log_fatal("Picovoice init failed: %s", pv_status_to_string(status));

    error_status = pv_get_error_stack(&message_stack, &message_stack_depth);
    if (error_status != PV_STATUS_SUCCESS)
    {
      log_fatal("Failed to get error stack: %s", pv_status_to_string(error_status));
      return error_status;
    }

    printErrorMessage(message_stack, message_stack_depth);
    pv_free_error_stack(message_stack);
    return status;
  }

  const char *rhino_context = NULL;
  status = pv_picovoice_context_info(_handle, &rhino_context);
  if (status != PV_STATUS_SUCCESS)
  {
    log_error("retrieving context info failed with '%s'", pv_status_to_string(status));
    return status;
  }

  return status;
}

/**
 * @brief Deletes the Picovoice engine instance.
 */
void PV_Delete(void)
{
  if (_handle == NULL)
  {
    log_error("Handle == NULL, failed to delete Picovoice instance");
  }
  else
  {
    pv_picovoice_delete(_handle);
    _handle = NULL;
  }
}

/**
 * @brief Processes audio buffer through Picovoice.
 *
 * @param audio_buffer Pointer to audio samples
 * @return pv_status_t status of the operation
 */
pv_status_t PV_Process(const int16_t *audio_buffer)
{
  return pv_picovoice_process(_handle, audio_buffer);
}