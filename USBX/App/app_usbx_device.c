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
#include "touch.h"
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

  custom_hid_parameter.ux_slave_class_hid_instance_activate = TOUCH_UsbHidActivate;
  custom_hid_parameter.ux_slave_class_hid_instance_deactivate = TOUCH_UsbHidDeactivate;

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
  ret = TOUCH_Init(memory_ptr);
  if (ret != UX_SUCCESS)
  {
    return ret;
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
  TOUCH_Thread(thread_input);
  /* USER CODE END app_ux_device_thread_entry */
}

/* USER CODE BEGIN 1 */

/* USER CODE END 1 */
