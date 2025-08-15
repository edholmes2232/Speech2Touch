#include "touch.h"

#include <stdlib.h>

// #include "stm32wbxx_hal_pwr_ex.h"
#include "log.h"
#include "usb.h"
#include "ux_api.h"
#include "ux_dcd_stm32.h"
#include "ux_device_class_hid.h"
#include "ux_utility.h"

static UX_SLAVE_CLASS_HID *_hid_instance = NULL;
static TX_QUEUE _usb_hid_msg_queue;

// Max size of the queue = Max number of pages to navigate
#define USB_HID_Q_FULL_SIZE (sizeof(TOUCH_EVENT_T) * 5)
// Delay between pressing and releasing the touch (30 ms) in ticks
#define PRESS_RELEASE_DELAY_TICKS (30 * TX_TIMER_TICKS_PER_SECOND / 1000)
#define MAX_X_COORD 10000
#define MAX_Y_COORD 10000

//! Called by AZURE RTOS USB Initialization function
UINT TOUCH_Init(VOID *memory_ptr)
{
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;

  CHAR *ptr;

  /* Allocate the stack for tx app queue.  */
  if (tx_byte_allocate(byte_pool, (VOID **)&ptr, USB_HID_Q_FULL_SIZE * sizeof(ULONG), TX_NO_WAIT) != TX_SUCCESS)
  {
    return TX_POOL_ERROR;
  }

  if (tx_queue_create(
          &_usb_hid_msg_queue, "usb hid msg queue", sizeof(TOUCH_EVENT_T), ptr, USB_HID_Q_FULL_SIZE * sizeof(ULONG))
      != TX_SUCCESS)
  {
    log_fatal("Failed to create USB HID message queue");
    return TX_QUEUE_ERROR;
  }

  return UX_SUCCESS;
}

UINT TOUCH_QueuePush(TOUCH_EVENT_T *event)
{
  return tx_queue_send(&_usb_hid_msg_queue, event, TX_NO_WAIT);
}

void TOUCH_Thread(ULONG thread_input)
{
  TX_PARAMETER_NOT_USED(thread_input);

  HAL_PWREx_EnableVddUSB();
  MX_USB_PCD_Init();

  HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x00, PCD_SNG_BUF, 0x0C);
  HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x80, PCD_SNG_BUF, 0x4C);
  HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x81, PCD_SNG_BUF, 0x8C);
  ux_dcd_stm32_initialize((ULONG)USB, (ULONG)&hpcd_USB_FS);
  /* Start the USB device */
  HAL_PCD_Start(&hpcd_USB_FS);

  log_info("USBX Device Thread Started");

  UINT status;

#pragma pack(1)
  typedef struct
  {
    uint8_t state; // Bit 0: Tip Switch
    uint16_t x;
    uint16_t y;
  } TouchScreenReport_t;
#pragma pack()

  TouchScreenReport_t touch_report;

  // Wait for USB to be activated
  for (;;)
  {
    // Wait for a QUEUE message indicating USB activation
    TOUCH_EVENT_T event;
    if (tx_queue_receive(&_usb_hid_msg_queue, &event, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
      log_debug("Received USB HID event: x=%d, y=%d, delay=%d", event.x, event.y, event.delay_ms);

      // Only send if USB is activated
      if (_hid_instance == UX_NULL)
      {
        log_warn("USB HID instance is NULL, cannot send report. USB may not be connected.");
      }
      else
      {
        // Match descriptor
        // Update touch_report
        touch_report.x = event.x;
        touch_report.y = event.y;

        // Create touch events
        UX_SLAVE_CLASS_HID_EVENT pressed_touch_event;
        ux_utility_memory_set(&pressed_touch_event, 0, sizeof(pressed_touch_event));
        // Set report id ourselves
        pressed_touch_event.ux_device_class_hid_event_report_id = 0x01;
        // Always this length
        pressed_touch_event.ux_device_class_hid_event_length = sizeof(TouchScreenReport_t);
        // Button pressed
        touch_report.state = 0x01; // Set Tip Switch bit to 1
        // Copy our event data into hid event
        ux_utility_memory_copy(
            pressed_touch_event.ux_device_class_hid_event_buffer, &touch_report, sizeof(TouchScreenReport_t));

        // Make a copy for release event
        UX_SLAVE_CLASS_HID_EVENT released_touch_event = pressed_touch_event;
        // Button released
        ((TouchScreenReport_t *)released_touch_event.ux_device_class_hid_event_buffer)->state = 0x00;

        status = _ux_device_class_hid_event_set(_hid_instance, &pressed_touch_event);
        if (status != UX_SUCCESS)
        {
          log_error("Failed to send USB HID report");
        }

        // Wait for host to process the "down" state before sending "up"
        tx_thread_sleep(PRESS_RELEASE_DELAY_TICKS);

        status = _ux_device_class_hid_event_set(_hid_instance, &released_touch_event);
        if (status != UX_SUCCESS)
        {
          log_error("Failed to send USB HID report");
        }

        if (event.delay_ms > 0)
        {
          log_info("Delaying for %d ms", event.delay_ms);
          tx_thread_sleep(event.delay_ms * TX_TIMER_TICKS_PER_SECOND / 1000);
          log_info("Delay complete");
        }
      }
    }
  }
}

void TOUCH_UsbHidActivate(void *hid_instance)
{
  _hid_instance = (UX_SLAVE_CLASS_HID *)hid_instance;
  log_info("USB HID Activated");
}

void TOUCH_UsbHidDeactivate(void *hid_instance)
{
  _hid_instance = (UX_SLAVE_CLASS_HID *)hid_instance;
  log_info("USB HID Deactivated");
}
