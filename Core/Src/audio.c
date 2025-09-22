#include "audio.h"

#include "led.h"
#include "log.h"
#include "main.h"
#include "sai.h"
#include "stm32wbxx_hal_sai.h"
#include "tx_api.h"

#include <stdint.h>
#include <stdlib.h>

#define PV_AUDIO_REC_AUDIO_FREQUENCY (16000U)
#define PICOVOICE_FRAME_SIZE (512)
#define AUDIO_BUFFER_SIZE_BYTES (PICOVOICE_FRAME_SIZE * sizeof(int16_t))

static int32_t _dma_buffer[PICOVOICE_FRAME_SIZE * 2];
static volatile uint8_t _audio_active = 0;
// ---------------------------- Azure RTOS Config --------------------------- //
#define NUM_AUDIO_BUFFERS (16)

// Audio byte pool
static TX_BLOCK_POOL _audio_block_pool;
static UCHAR _block_pool_buffer[NUM_AUDIO_BUFFERS * AUDIO_BUFFER_SIZE_BYTES];

// TODO ULONG Queue size
// Queue containing int16_t* audio buffer addresses
static TX_QUEUE _audio_data_queue;

uint8_t AUDIO_Init(VOID *memory_ptr)
{
  UINT status;
  // Create audio block pool
  status = tx_block_pool_create(
      &_audio_block_pool, "Audio Buffer Pool", AUDIO_BUFFER_SIZE_BYTES, _block_pool_buffer, sizeof(_block_pool_buffer));
  if (status != TX_SUCCESS)
  {
    log_fatal("Failed to create audio block pool: %d", status);
    return EXIT_FAILURE;
  }

  // Create audio data queue storage from global byte pool
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;
  CHAR *queue_storage_ptr;
  status = tx_byte_allocate(byte_pool, (VOID **)&queue_storage_ptr, NUM_AUDIO_BUFFERS * sizeof(int16_t *), TX_NO_WAIT);
  if (status != TX_SUCCESS)
  {
    log_fatal("Failed to allocate queue storage: %d", status);
    return EXIT_FAILURE;
  }

  // Create audio data queue
  status = tx_queue_create(&_audio_data_queue,
                           "Audio Data Queue",
                           sizeof(int16_t *) / sizeof(ULONG),
                           queue_storage_ptr,
                           NUM_AUDIO_BUFFERS * sizeof(int16_t *));
  if (status != TX_SUCCESS)
  {
    log_fatal("Failed to create audio data queue: %d", status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void AUDIO_Start(void)
{
  log_info("AUDIO_Start");

  _audio_active = 1;

  HAL_SAI_Receive_DMA(&hsai_BlockB1, (uint8_t *)_dma_buffer, PICOVOICE_FRAME_SIZE * 2);
}

void AUDIO_Stop(void)
{
  log_info("Stopping audio capture");

  // Stop DMA
  _audio_active = 0;
  HAL_SAI_DMAStop(&hsai_BlockB1);

  // Wait 1ms for remaining audio
  tx_thread_sleep((1 * TX_TIMER_TICKS_PER_SECOND) / 1000);

  // Drain queue, release all pending buffers
  int16_t *buffer;
  UINT status;

  while (tx_queue_receive(&_audio_data_queue, &buffer, TX_NO_WAIT) == TX_SUCCESS)
  {
    // Release the buffer back to the byte pool
    status = tx_block_release(buffer);
    if (status != TX_SUCCESS)
    {
      log_error("Failed to release audio buffer back to byte pool: %d", status);
    }
  }
}

// A helper macro for clamping
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

static void processData(const int32_t *dma_src, int16_t *dest)
{
  for (uint32_t i = 0; i < PICOVOICE_FRAME_SIZE; i++)
  {
    // Get the raw 32-bit sample
    int32_t sample = dma_src[i];

    // Apply a gain factor
    // (This is a 12dB gain. Use 2 for 6dB, 8 for 18dB, etc.)
    sample = sample * 8;

    // Clamp the value to prevent overflow distortion.
    // The raw 24-bit data is in a 32-bit container, so we clamp to the 32-bit min/max.
    sample = CLAMP(sample, INT32_MIN, INT32_MAX);

    // Shift amplified and clamped value down to 16-bit
    dest[i] = (int16_t)(sample >> 16);
  }
}

void dmaCallbackHandler(int32_t *dma_buffer)
{
  if (!_audio_active)
  {
    log_debug("Audio inactive, dropping frame");
    return;
  }

  // Allocate buff from byte pool
  int16_t *buffer = NULL;
  UINT status = tx_block_allocate(&_audio_block_pool, (VOID **)&buffer, TX_NO_WAIT);
  if ((status != TX_SUCCESS) || (buffer == NULL))
  {
    log_error("Failed to allocate audio buffer from byte pool, dropping frame");
    return;
  }

  processData(dma_buffer, buffer);

  // Send the buffer to the queue
  status = tx_queue_send(&_audio_data_queue, &buffer, TX_NO_WAIT);
  if (status != TX_SUCCESS)
  {
    log_error("Failed to send audio buffer to queue, dropping frame: %d", status);
    UINT release_status = tx_block_release(buffer);
    if (release_status != TX_SUCCESS)
    {
      log_error("Failed to release audio buffer back to byte pool: %d", release_status);
    }
    return;
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  (void)hsai;
  dmaCallbackHandler(&_dma_buffer[PICOVOICE_FRAME_SIZE]);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  (void)hsai;
  dmaCallbackHandler(&_dma_buffer[0]);
}

uint8_t AUDIO_GetBuffer(int16_t **buffer)
{
  UINT status = tx_queue_receive(&_audio_data_queue, buffer, TX_WAIT_FOREVER);
  if ((status != TX_SUCCESS) && status != TX_QUEUE_EMPTY)
  {
    log_error("Failed to receive audio buffer from queue: %d", status);
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}

void AUDIO_ReleaseBuffer(int16_t *buffer)
{
  if (buffer != NULL)
  {
    UINT status = tx_block_release(buffer);
    if (status != TX_SUCCESS)
    {
      log_error("Failed to release audio buffer back to byte pool: %d", status);
    }
  }
}
