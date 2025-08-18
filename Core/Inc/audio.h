#ifndef AUDIO_H
#define AUDIO_H

#include <stdint.h>
#include <stdio.h>

/**
 * @brief Initialize the audio subsystem.
 *
 * @param memory_ptr Pointer to the memory to be used for audio queue
 * @note Allocates it's own memory for audio buffers separately.
 * @return uint8_t Status of the initialization (EXIT_SUCCESS or EXIT_FAILURE).
 */
uint8_t AUDIO_Init(void *memory_ptr);

/**
 * @brief Start the audio processing.
 */
void AUDIO_Start(void);

/**
 * @brief Get an audio buffer for processing. Blocks until a buffer is available.
 *
 * @param buffer Pointer to pointer to audio buffer
 * @note The buffer must be released using AUDIO_ReleaseBuffer after processing.
 * @return uint8_t Status of the operation (EXIT_SUCCESS or EXIT_FAILURE).
 */
uint8_t AUDIO_GetBuffer(int16_t **buffer);

/**
 * @brief Release an audio buffer back to the pool.
 *
 * @param buffer Pointer to the audio buffer to be released
 */
void AUDIO_ReleaseBuffer(int16_t *buffer);

#endif