/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : usbd_custom_hid_if.c
 * @version        : v3.0_Cube
 * @brief          : USB Device Custom HID interface file.
 ******************************************************************************
 * @attention
 *
 * Copyright (c) 2025 STMicroelectronics.
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
#include "usbd_custom_hid_if.h"

/* USER CODE BEGIN INCLUDE */

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
 * @brief Usb device.
 * @{
 */

/** @addtogroup USBD_CUSTOM_HID
 * @{
 */

/** @defgroup USBD_CUSTOM_HID_Private_TypesDefinitions USBD_CUSTOM_HID_Private_TypesDefinitions
 * @brief Private types.
 * @{
 */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_Defines USBD_CUSTOM_HID_Private_Defines
 * @brief Private defines.
 * @{
 */

/* USER CODE BEGIN PRIVATE_DEFINES */
#define DESC_1
/* USER CODE END PRIVATE_DEFINES */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_Macros USBD_CUSTOM_HID_Private_Macros
 * @brief Private macros.
 * @{
 */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_Variables USBD_CUSTOM_HID_Private_Variables
 * @brief Private variables.
 * @{
 */

/** Usb HID report descriptor. */
__ALIGN_BEGIN static uint8_t CUSTOM_HID_ReportDesc_FS[USBD_CUSTOM_HID_REPORT_DESC_SIZE] __ALIGN_END = {
/* USER CODE BEGIN 0 */
// 0x05, 0x0D,       // Usage Page (Digitizer)
// 0x09, 0x04,       // Usage (Touch Screen)
// 0xA1, 0x01,       // Collection (Application)

// 0x85, 0x03,       //   Report ID (3)
// 0x09, 0x22,       //   Usage (Finger)
// 0xA1, 0x02,       //   Collection (Logical)
// 0x09, 0x42,       //     Usage (Tip Switch)
// 0x14,             //     Logical Minimum
// 0x25, 0x01,       //     Logical Maximum (1)
// 0x75, 0x01,       //     Report Size (1)
// 0x95, 0x01,       //     Report Count (1)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x09, 0x32,       //     Usage (In Range)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x09, 0x47,       //     Usage (0x47)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x95, 0x05,       //     Report Count (5)
// 0x81, 0x03,       //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x75, 0x08,       //     Report Size (8)
// 0x09, 0x51,       //     Usage (0x51)
// 0x95, 0x01,       //     Report Count (1)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x05, 0x01,       //     Usage Page (Generic Desktop Ctrls)
// 0x26, 0x75, 0x10, //     Logical Maximum (4213)
// 0x09, 0x30,       //     Usage (X)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x09, 0x31,       //     Usage (Y)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0xC0,             //   End Collection
// 0x05, 0x0D,       //   Usage Page (Digitizer)
// 0x09, 0x22,       //   Usage (Finger)
// 0xA1, 0x02,       //   Collection (Logical)
// 0x09, 0x42,       //     Usage (Tip Switch)
// 0x25, 0x01,       //     Logical Maximum (1)
// 0x75, 0x01,       //     Report Size (1)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x09, 0x32,       //     Usage (In Range)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x09, 0x47,       //     Usage (0x47)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x95, 0x05,       //     Report Count (5)
// 0x81, 0x03,       //     Input (Const,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x75, 0x08,       //     Report Size (8)
// 0x95, 0x01,       //     Report Count (1)
// 0x09, 0x51,       //     Usage (0x51)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x05, 0x01,       //     Usage Page (Generic Desktop Ctrls)
// 0x26, 0x75, 0x10, //     Logical Maximum (4213)
// 0x09, 0x30,       //     Usage (X)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x09, 0x31,       //     Usage (Y)
// 0x81, 0x02,       //     Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0xC0,             //   End Collection
// 0x05, 0x0D,       //   Usage Page (Digitizer)
// 0x09, 0x54,       //   Usage (0x54)
// 0x75, 0x08,       //   Report Size (8)
// 0x25, 0x08,       //   Logical Maximum (8)
// 0x81, 0x02,       //   Input (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position)
// 0x09, 0x55,       //   Usage (0x55)
// 0xB1, 0x02,       //   Feature (Data,Var,Abs,No Wrap,Linear,Preferred State,No Null Position,Non-volatile)

// 120 bytes
// 0x00,
#ifdef DESC_1
    // TOUCHSCRREN = 0
    0x05,
    0x01, // USAGE_PAGE (Generic Desktop)
    0x09,
    0x01, // USAGE (Pointer)
    0xa1,
    0x01, // COLLECTION (Application)

    // declare a pointer collection
    0x05,
    0x01, //   USAGE_PAGE (Generic Desktop)
    0x09,
    0x01, //   USAGE (Pointer)
    0xa1,
    0x00, //   COLLECTION (Physical)

    // Declare a button
    0x05,
    0x09, //     USAGE_PAGE (Button)
    0x19,
    0x01, //     USAGE_MINIMUM (Button 1)
    0x29,
    0x01, //     USAGE_MAXIMUM (Button 1)
    0x15,
    0x00, //     LOGICAL_MINIMUM (0)
    0x25,
    0x01, //     LOGICAL_MAXIMUM (1)
    0x35,
    0x00, //     PHYSICAL_MINIMUM (0)
    0x45,
    0x01, //     PHYSICAL_MAXIMUM (1)
    0x66,
    0x00,
    0x00, //     UNIT (None)
    0x75,
    0x01, //     REPORT_SIZE (1)
    0x95,
    0x01, //     REPORT_COUNT (1)
    0x81,
    0x62, //     INPUT (Data,Var,Abs)

    // Declare the remaining 7 bits of the first data byte as constant -> the driver will ignore them
    0x75,
    0x01, //     REPORT_SIZE (1)
    0x95,
    0x07, //     REPORT_COUNT (7)
    0x81,
    0x01, //     INPUT (Cnst,Ary,Abs)

    // Define absolute X and Y coordinates of 16 bit each (percent values multiplied with 100)
    0x05,
    0x01, //     Usage Page (Generic Desktop)
    0x09,
    0x30, //     Usage (X)
    0x09,
    0x31, //     Usage (Y)
    0x16,
    0x00,
    0x00, //     Logical Minimum (0)
    0x26,
    0x10,
    0x27, //     Logical Maximum (10000)
    0x36,
    0x00,
    0x00, //     Physical Minimum (0)
    0x46,
    0x10,
    0x27, //     Physical Maximum (10000)
    0x66,
    0x00,
    0x00, //     UNIT (None)
    0x75,
    0x10, //     Report Size (16),
    0x95,
    0x02, //     Report Count (2),
    0x81,
    0x62, //     Input (Data,Var,Abs)

    0xc0, //   END_COLLECTION
#elif defined DESC_2
    // TOCUHSCREEN = 1

    0x05,
    0x0d, // USAGE_PAGE (Digitizer)
    0x09,
    0x02, // USAGE (Pen)
    0xa1,
    0x01, // COLLECTION (Application)

    // declare a finger collection
    0x09,
    0x20, //   Usage (Stylus)
    0xA1,
    0x00, //   Collection (Physical)

    // Declare a finger touch (finger up/down)
    0x09,
    0x42, //     Usage (Tip Switch)
    0x09,
    0x32, //     USAGE (In Range)
    0x15,
    0x00, //     LOGICAL_MINIMUM (0)
    0x25,
    0x01, //     LOGICAL_MAXIMUM (1)
    0x75,
    0x01, //     REPORT_SIZE (1)
    0x95,
    0x02, //     REPORT_COUNT (2)
    0x81,
    0x02, //     INPUT (Data,Var,Abs)

    // Declare the remaining 6 bits of the first data byte as constant -> the driver will ignore them
    0x75,
    0x01, //     REPORT_SIZE (1)
    0x95,
    0x06, //     REPORT_COUNT (6)
    0x81,
    0x01, //     INPUT (Cnst,Ary,Abs)

    // Define absolute X and Y coordinates of 16 bit each (percent values multiplied with 100)
    // http://www.usb.org/developers/hidpage/Hut1_12v2.pdf
    // Chapter 16.2 says: "In the Stylus collection a Pointer physical collection will contain the axes reported by the
    // stylus."
    0x05,
    0x01, //     Usage Page (Generic Desktop)
    0x09,
    0x01, //     Usage (Pointer)
    0xA1,
    0x00, //     Collection (Physical)
    0x09,
    0x30, //        Usage (X)
    0x09,
    0x31, //        Usage (Y)
    0x16,
    0x00,
    0x00, //        Logical Minimum (0)
    0x26,
    0x10,
    0x27, //        Logical Maximum (10000)
    0x36,
    0x00,
    0x00, //        Physical Minimum (0)
    0x46,
    0x10,
    0x27, //        Physical Maximum (10000)
    0x66,
    0x00,
    0x00, //        UNIT (None)
    0x75,
    0x10, //        Report Size (16),
    0x95,
    0x02, //        Report Count (2),
    0x81,
    0x02, //        Input (Data,Var,Abs)
    0xc0, //     END_COLLECTION

    0xc0, //   END_COLLECTION

#endif
    /* USER CODE END 0 */
    0xC0 /*     END_COLLECTION	             */
};

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Exported_Variables USBD_CUSTOM_HID_Exported_Variables
 * @brief Public variables.
 * @{
 */
extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */
/**
 * @}
 */

/** @defgroup USBD_CUSTOM_HID_Private_FunctionPrototypes USBD_CUSTOM_HID_Private_FunctionPrototypes
 * @brief Private functions declaration.
 * @{
 */

static int8_t CUSTOM_HID_Init_FS(void);
static int8_t CUSTOM_HID_DeInit_FS(void);
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state);

/**
 * @}
 */

USBD_CUSTOM_HID_ItfTypeDef USBD_CustomHID_fops_FS = {
    CUSTOM_HID_ReportDesc_FS, CUSTOM_HID_Init_FS, CUSTOM_HID_DeInit_FS, CUSTOM_HID_OutEvent_FS};

/** @defgroup USBD_CUSTOM_HID_Private_Functions USBD_CUSTOM_HID_Private_Functions
 * @brief Private functions.
 * @{
 */

/* Private functions ---------------------------------------------------------*/

/**
 * @brief  Initializes the CUSTOM HID media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CUSTOM_HID_Init_FS(void)
{
    /* USER CODE BEGIN 4 */
    return (USBD_OK);
    /* USER CODE END 4 */
}

/**
 * @brief  DeInitializes the CUSTOM HID media low layer
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CUSTOM_HID_DeInit_FS(void)
{
    /* USER CODE BEGIN 5 */
    return (USBD_OK);
    /* USER CODE END 5 */
}

/**
 * @brief  Manage the CUSTOM HID class events
 * @param  event_idx: Event index
 * @param  state: Event state
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
static int8_t CUSTOM_HID_OutEvent_FS(uint8_t event_idx, uint8_t state)
{
    /* USER CODE BEGIN 6 */
    UNUSED(event_idx);
    UNUSED(state);

    /* Start next USB packet transfer once data processing is completed */
    USBD_CUSTOM_HID_ReceivePacket(&hUsbDeviceFS);

    return (USBD_OK);
    /* USER CODE END 6 */
}

/* USER CODE BEGIN 7 */
/**
 * @brief  Send the report to the Host
 * @param  report: The report to be sent
 * @param  len: The report length
 * @retval USBD_OK if all operations are OK else USBD_FAIL
 */
/*
static int8_t USBD_CUSTOM_HID_SendReport_FS(uint8_t *report, uint16_t len)
{
  return USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, report, len);
}
*/
/* USER CODE END 7 */

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */
/**
 * @}
 */

/**
 * @}
 */

/**
 * @}
 */
