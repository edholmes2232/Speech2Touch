#ifndef TOUCH_H
#define TOUCH_H

#include "tx_port.h"

#include <stdint.h>

#define TOUCH_MAX_X_COORD 10000
#define TOUCH_MAX_Y_COORD 10000

typedef struct
{
  uint16_t x; //< X coordinate in pixels (0-TOUCH_MAX_X_COORD)
  uint16_t y; //< Y coordinate in pixels (0-TOUCH_MAX_Y_COORD)
  uint32_t delay_ms; //< Optional delay in ms after sending
} TOUCH_EVENT_T;

UINT TOUCH_Init(VOID *memory_ptr);

UINT TOUCH_QueuePush(TOUCH_EVENT_T *event);

VOID TOUCH_Thread(ULONG thread_input);

// Used by USBX on USB connection
VOID TOUCH_UsbHidActivate(void *hid_instance);
// Used by USBX on USB disconnection
VOID TOUCH_UsbHidDeactivate(void *hid_instance);

#endif