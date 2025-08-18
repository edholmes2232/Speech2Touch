#include "audio.h"

#include "log.h"
#include "picovoice.h"
#include "sai.h"
#include "stm32wbxx_hal_sai.h"
#include "tx_api.h"

#include <stdio.h>
#include <stdlib.h>

#define PV_AUDIO_REC_AUDIO_FREQUENCY (16000U)
#define PICOVOICE_FRAME_SIZE (512)
#define AUDIO_BUFFER_SIZE_BYTES (PICOVOICE_FRAME_SIZE * sizeof(int16_t))

static int32_t _dma_buffer[PICOVOICE_FRAME_SIZE * 2];

static volatile uint8_t _is_recording = 0;
static volatile int32_t write_index = 0;
static volatile int32_t read_index = -1;
static int32_t last_read_index = -1;

// ---------------------------- Azure RTOS Config --------------------------- //
#define NUM_AUDIO_BUFFERS (4)

// Audio byte pool
static TX_BLOCK_POOL _audio_block_pool;
static UCHAR _block_pool_buffer[NUM_AUDIO_BUFFERS * AUDIO_BUFFER_SIZE_BYTES];

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
  // Enable DWT cycle counter for timing if not already enabled
  if (!(DWT->CTRL & DWT_CTRL_CYCCNTENA_Msk))
  {
    // log_fatal("Enabling DWT cycle counter");
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
  }
  HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)_dma_buffer, PICOVOICE_FRAME_SIZE * 2);
}

// A helper macro for clamping
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

static void processData(const int32_t *dma_src, int16_t *dest)
{
  uint32_t start = DWT->CYCCNT;
  for (uint32_t i = 0; i < PICOVOICE_FRAME_SIZE; i++)
  {
    // 1. Get the raw 32-bit sample from the DMA buffer
    int32_t sample = dma_src[i];

    // 2. Apply a gain factor. Start with 4.
    // (This is a 12dB gain. Use 2 for 6dB, 8 for 18dB, etc.)
    sample = sample * 8;

    // 3. IMPORTANT: Clamp the value to prevent overflow distortion.
    // The raw 24-bit data is in a 32-bit container, so we clamp to the 32-bit min/max.
    sample = CLAMP(sample, INT32_MIN, INT32_MAX);

    // 4. Now, shift the amplified and clamped value down to 16-bit
    dest[i] = (int16_t)(sample >> 16);
  }
  uint32_t end = DWT->CYCCNT;
  uint32_t cycles = end - start;
  uint32_t us = cycles / (SystemCoreClock / 1000000);
  // log_trace("processData took %lu us", us);
}

void dmaCallbackHandler(int32_t *dma_buffer)
{
  // Allocate buff from byte pool
  int16_t *buffer = NULL;
  UINT status = tx_block_allocate(&_audio_block_pool, (VOID **)&buffer, TX_NO_WAIT);
  if ((status != TX_SUCCESS) || (buffer == NULL))
  {
    log_error("Failed to allocate audio buffer from byte pool, dropping frame");
    return;
  }

  processData(_dma_buffer, buffer);

  // Send the buffer to the queue
  status = tx_queue_send(&_audio_data_queue, &buffer, TX_NO_WAIT);
  if (status != TX_SUCCESS)
  {
    log_error("Failed to send audio buffer to queue, dropping frame");
    tx_byte_release(buffer);

    return;
  }
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
  (void)hsai;
  dmaCallbackHandler(&_dma_buffer[0]);
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
  (void)hsai;
  dmaCallbackHandler(&_dma_buffer[PICOVOICE_FRAME_SIZE]);
}

uint8_t AUDIO_GetBuffer(int16_t **buffer)
{
  UINT status = tx_queue_receive(&_audio_data_queue, buffer, TX_WAIT_FOREVER);
  if (status != TX_SUCCESS)
  {
    log_error("Failed to receive audio buffer from queue: %d", status);
    return EXIT_FAILURE;
  }

  // log_trace("buffer=%p", *buffer);
  return EXIT_SUCCESS;
}

void AUDIO_ReleaseBuffer(int16_t *buffer)
{
  if (buffer != NULL)
  {
    UINT status = tx_block_release(buffer);
    if (status != TX_SUCCESS)
    {
      log_error("Failed to release audio buffer back to byte pool");
    }
  }
}