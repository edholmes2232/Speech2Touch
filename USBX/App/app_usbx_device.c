/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file    app_usbx_device.c
 * @author  MCD Application Team
 * @brief   USBX Device applicative file
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2020-2021 STMicroelectronics.
 * All rights reserved.
 *
 * This software is licensed under terms that can be found in the LICENSE file
 * in the root directory of this software component.
 * If no LICENSE file comes with this software, it is provided AS-IS.
 *
 ******************************************************************************
 */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "app_usbx_device.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32wbxx_hal_pwr_ex.h"
#include "touch_targets.h"
#include "usb.h"
#include "ux_dcd_stm32.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

static ULONG hid_custom_interface_number;
static ULONG hid_custom_configuration_number;
static UX_SLAVE_CLASS_HID_PARAMETER custom_hid_parameter;
static TX_THREAD ux_device_app_thread;

/* USER CODE BEGIN PV */
static volatile UX_SLAVE_CLASS_HID *_hid_instance = NULL;

TX_QUEUE _usb_hid_msg_queue;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
static VOID app_ux_device_thread_entry(ULONG thread_input);
/* USER CODE BEGIN PFP */

VOID USBD_HID_Activate(VOID *hid_instance);
VOID USBD_HID_Deactivate(void *hid_instance);
/* USER CODE END PFP */

/**
 * @brief  Application USBX Device Initialization.
 * @param  memory_ptr: memory pointer
 * @retval status
 */
UINT MX_USBX_Device_Init(VOID *memory_ptr)
{
  UINT ret = UX_SUCCESS;
  UCHAR *device_framework_high_speed;
  UCHAR *device_framework_full_speed;
  ULONG device_framework_hs_length;
  ULONG device_framework_fs_length;
  ULONG string_framework_length;
  ULONG language_id_framework_length;
  UCHAR *string_framework;
  UCHAR *language_id_framework;
  UCHAR *pointer;
  TX_BYTE_POOL *byte_pool = (TX_BYTE_POOL *)memory_ptr;

  /* USER CODE BEGIN MX_USBX_Device_Init0 */

  /* USER CODE END MX_USBX_Device_Init0 */

  /* Allocate the stack for USBX Memory */
  if (tx_byte_allocate(byte_pool, (VOID **)&pointer, USBX_DEVICE_MEMORY_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_ALLOCATE_STACK_ERORR */
    return TX_POOL_ERROR;
    /* USER CODE END USBX_ALLOCATE_STACK_ERORR */
  }

  /* Initialize USBX Memory */
  if (ux_system_initialize(pointer, USBX_DEVICE_MEMORY_STACK_SIZE, UX_NULL, 0) != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_SYSTEM_INITIALIZE_ERORR */
    return UX_ERROR;
    /* USER CODE END USBX_SYSTEM_INITIALIZE_ERORR */
  }

  /* Get Device Framework High Speed and get the length */
  device_framework_high_speed = USBD_Get_Device_Framework_Speed(USBD_HIGH_SPEED, &device_framework_hs_length);

  /* Get Device Framework Full Speed and get the length */
  device_framework_full_speed = USBD_Get_Device_Framework_Speed(USBD_FULL_SPEED, &device_framework_fs_length);

  /* Get String Framework and get the length */
  string_framework = USBD_Get_String_Framework(&string_framework_length);

  /* Get Language Id Framework and get the length */
  language_id_framework = USBD_Get_Language_Id_Framework(&language_id_framework_length);

  /* Install the device portion of USBX */
  if (ux_device_stack_initialize(device_framework_high_speed,
                                 device_framework_hs_length,
                                 device_framework_full_speed,
                                 device_framework_fs_length,
                                 string_framework,
                                 string_framework_length,
                                 language_id_framework,
                                 language_id_framework_length,
                                 UX_NULL)
      != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_INITIALIZE_ERORR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_INITIALIZE_ERORR */
  }

  /* Initialize the hid custom class parameters for the device */
  custom_hid_parameter.ux_slave_class_hid_instance_activate = USBD_Custom_HID_Activate;
  custom_hid_parameter.ux_slave_class_hid_instance_deactivate = USBD_Custom_HID_Deactivate;
  custom_hid_parameter.ux_device_class_hid_parameter_report_address = USBD_HID_ReportDesc(INTERFACE_HID_CUSTOM);
  custom_hid_parameter.ux_device_class_hid_parameter_report_length = USBD_HID_ReportDesc_length(INTERFACE_HID_CUSTOM);
  custom_hid_parameter.ux_device_class_hid_parameter_report_id = UX_TRUE;
  custom_hid_parameter.ux_device_class_hid_parameter_callback = USBD_Custom_HID_SetFeature;
  custom_hid_parameter.ux_device_class_hid_parameter_get_callback = USBD_Custom_HID_GetReport;
#ifdef UX_DEVICE_CLASS_HID_INTERRUPT_OUT_SUPPORT
  custom_hid_parameter.ux_device_class_hid_parameter_receiver_initialize = ux_device_class_hid_receiver_initialize;
  custom_hid_parameter.ux_device_class_hid_parameter_receiver_event_max_number = USBD_Custom_HID_EventMaxNumber();
  custom_hid_parameter.ux_device_class_hid_parameter_receiver_event_max_length = USBD_Custom_HID_EventMaxLength();
  custom_hid_parameter.ux_device_class_hid_parameter_receiver_event_callback = USBD_Custom_HID_SetReport;
#endif /* UX_DEVICE_CLASS_HID_INTERRUPT_OUT_SUPPORT */

  /* USER CODE BEGIN CUSTOM_HID_PARAMETER */

  custom_hid_parameter.ux_slave_class_hid_instance_activate = USBD_HID_Activate;
  custom_hid_parameter.ux_slave_class_hid_instance_deactivate = USBD_HID_Deactivate;

  /* USER CODE END CUSTOM_HID_PARAMETER */

  /* Get Custom hid configuration number */
  hid_custom_configuration_number = USBD_Get_Configuration_Number(CLASS_TYPE_HID, INTERFACE_HID_CUSTOM);

  /* Find Custom hid interface number */
  hid_custom_interface_number = USBD_Get_Interface_Number(CLASS_TYPE_HID, INTERFACE_HID_CUSTOM);

  /* Initialize the device hid custom class */
  if (ux_device_stack_class_register(_ux_system_slave_class_hid_name,
                                     ux_device_class_hid_entry,
                                     hid_custom_configuration_number,
                                     hid_custom_interface_number,
                                     &custom_hid_parameter)
      != UX_SUCCESS)
  {
    /* USER CODE BEGIN USBX_DEVICE_HID_CUSTOM_REGISTER_ERORR */
    return UX_ERROR;
    /* USER CODE END USBX_DEVICE_HID_CUSTOM_REGISTER_ERORR */
  }

  /* Allocate the stack for device application main thread */
  if (tx_byte_allocate(byte_pool, (VOID **)&pointer, UX_DEVICE_APP_THREAD_STACK_SIZE, TX_NO_WAIT) != TX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_THREAD_ALLOCATE_STACK_ERORR */
    return TX_POOL_ERROR;
    /* USER CODE END MAIN_THREAD_ALLOCATE_STACK_ERORR */
  }

  /* Create the device application main thread */
  if (tx_thread_create(&ux_device_app_thread,
                       UX_DEVICE_APP_THREAD_NAME,
                       app_ux_device_thread_entry,
                       0,
                       pointer,
                       UX_DEVICE_APP_THREAD_STACK_SIZE,
                       UX_DEVICE_APP_THREAD_PRIO,
                       UX_DEVICE_APP_THREAD_PREEMPTION_THRESHOLD,
                       UX_DEVICE_APP_THREAD_TIME_SLICE,
                       UX_DEVICE_APP_THREAD_START_OPTION)
      != TX_SUCCESS)
  {
    /* USER CODE BEGIN MAIN_THREAD_CREATE_ERORR */
    return TX_THREAD_ERROR;
    /* USER CODE END MAIN_THREAD_CREATE_ERORR */
  }

  /* USER CODE BEGIN MX_USBX_Device_Init1 */
  CHAR *ptr;
  if (tx_queue_create(
          &_usb_hid_msg_queue, "usb hid msg queue", sizeof(TARGET_T), ptr, (sizeof(TARGET_T) * 2) * sizeof(ULONG))
      != TX_SUCCESS)
  {
    printf("Failed to create USB HID message queue\n");
    return TX_QUEUE_ERROR;
  }

  /* USER CODE END MX_USBX_Device_Init1 */

  return ret;
}

/**
 * @brief  Function implementing app_ux_device_thread_entry.
 * @param  thread_input: User thread input parameter.
 * @retval none
 */
static VOID app_ux_device_thread_entry(ULONG thread_input)
{
  /* USER CODE BEGIN app_ux_device_thread_entry */
  TX_PARAMETER_NOT_USED(thread_input);

  HAL_PWREx_EnableVddUSB();
  MX_USB_PCD_Init();

  HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x00, PCD_SNG_BUF, 0x0C);
  HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x80, PCD_SNG_BUF, 0x4C);
  HAL_PCDEx_PMAConfig(&hpcd_USB_FS, 0x81, PCD_SNG_BUF, 0x8C);
  ux_dcd_stm32_initialize((ULONG)USB, (ULONG)&hpcd_USB_FS);
  /* Start the USB device */
  HAL_PCD_Start(&hpcd_USB_FS);

  printf("USBX Device Thread Started\n");

  UINT status;

#pragma pack(1)
  typedef struct
  {
    uint8_t report_id;
    uint8_t state; // Bit 0: Tip Switch
    uint16_t x;
    uint16_t y;
  } TouchScreenReport_t;
#pragma pack()

  // Create hid event
  UX_SLAVE_CLASS_HID_EVENT touch_event;
  touch_event.ux_device_class_hid_event_report_id = UX_FALSE; // Not using report ID
  touch_event.ux_device_class_hid_event_length = sizeof(TouchScreenReport_t);

  TouchScreenReport_t *touch_report = (TouchScreenReport_t *)touch_event.ux_device_class_hid_event_buffer;
  touch_report->report_id = 0x01;
  // touch_report.report_id = 0x01; // Matches the descriptor

  // Wait for USB to be activated
  for (;;)
  {
    // Wait for a QUEUE message indicating USB activation
    TARGET_T target;
    if (tx_queue_receive(&_usb_hid_msg_queue, &target, TX_WAIT_FOREVER) == TX_SUCCESS)
    {
      printf("Received USB HID target: %d\n", target);

      // Only send if USB is activated
      if (_hid_instance == UX_NULL)
      {
        printf("USB HID instance is NULL, cannot send report\n");
      }
      else
      {
        // Create touch report
        touch_report->x = 5000;
        touch_report->y = 5000;
        // Button pressed
        touch_report->state = 0x01; // Set Tip Switch bit to 1

        status = _ux_device_class_hid_event_set(_hid_instance, (UX_SLAVE_CLASS_HID_EVENT *)&touch_event);
        if (status != UX_SUCCESS)
        {
          printf("Failed to send USB HID report\n");
        }

        // Wait for host to process the "down" state before sending "up"
        tx_thread_sleep(30);

        // Button released
        touch_report->state = 0x00; // Set Tip Switch bit to 0

        status = _ux_device_class_hid_event_set(_hid_instance, (UX_SLAVE_CLASS_HID_EVENT *)&touch_event);
        if (status != UX_SUCCESS)
        {
          printf("Failed to send USB HID report\n");
        }
      }
    }
  }

  /* USER CODE END app_ux_device_thread_entry */
}

/* USER CODE BEGIN 1 */
VOID USBD_HID_Activate(VOID *hid_instance)
{
  _hid_instance = (UX_SLAVE_CLASS_HID *)hid_instance;
  printf("USB HID Activated\n");
}

VOID USBD_HID_Deactivate(VOID *hid_instance)
{
  _hid_instance = UX_NULL;
  printf("USB HID Deactivated\n");
}
/* USER CODE END 1 */
