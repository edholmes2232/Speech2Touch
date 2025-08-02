#include "audio.h"

#include "picovoice.h"
#include "sai.h"
#include "stm32wbxx_hal_sai.h"

#include <stdio.h>
#include <stdlib.h>

#define PV_AUDIO_REC_AUDIO_FREQUENCY (16000U)
#define PICOVOICE_FRAME_SIZE (512)

volatile int16_t _ping_pong_buffer[2][PICOVOICE_FRAME_SIZE];

static int32_t _dma_buffer[PICOVOICE_FRAME_SIZE * 2];

static volatile uint8_t _is_recording = 0;
static volatile int32_t write_index = 0;
static volatile int32_t read_index = -1;
static int32_t last_read_index = -1;

void AUDIO_Start(void)
{
    printf("AUDIO_Start\n");
    HAL_SAI_Receive_DMA(&hsai_BlockA1, (uint8_t *)_dma_buffer, PICOVOICE_FRAME_SIZE * 2);
}

int16_t *AUDIO_GetBuffer(void)
{
    if ((read_index == -1) || (last_read_index == read_index))
    {
        // No new data available
        return NULL;
    }
    last_read_index = read_index;
    return _ping_pong_buffer[read_index];
}

// A helper macro for clamping
#define CLAMP(val, min, max) ((val) < (min) ? (min) : ((val) > (max) ? (max) : (val)))

static void processData(const int32_t *dma_src, int16_t *dest)
{
    for (uint32_t i = 0; i < PICOVOICE_FRAME_SIZE; i++)
    {
        // 1. Get the raw 32-bit sample from the DMA buffer
        int32_t sample = dma_src[i];

        // 2. Apply a gain factor. Start with 4.
        // (This is a 12dB gain. Use 2 for 6dB, 8 for 18dB, etc.)
        sample = sample * 4;

        // 3. IMPORTANT: Clamp the value to prevent overflow distortion.
        // The raw 24-bit data is in a 32-bit container, so we clamp to the 32-bit min/max.
        sample = CLAMP(sample, INT32_MIN, INT32_MAX);

        // 4. Now, shift the amplified and clamped value down to 16-bit
        dest[i] = (int16_t)(sample >> 16);
    }
}

void HAL_SAI_RxHalfCpltCallback(SAI_HandleTypeDef *hsai)
{
    processData(&_dma_buffer[0], _ping_pong_buffer[write_index]);

    read_index = write_index;

    write_index ^= 1; // Toggle between 0 and 1
}

void HAL_SAI_RxCpltCallback(SAI_HandleTypeDef *hsai)
{
    processData(&_dma_buffer[PICOVOICE_FRAME_SIZE], _ping_pong_buffer[write_index]);

    read_index = write_index;

    write_index ^= 1; // Toggle between 0 and 1
}
