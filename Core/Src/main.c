/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
#include "main.h"
#include "dma.h"
#include "memorymap.h"
#include "sai.h"
#include "usb_device.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
typedef struct
{
    uint8_t reportId; // Report ID = 0x03 (3)
                      // Collection: CA:TouchScreen CP:Finger
    uint8_t DIG_TouchScreenFingerTipSwitch : 1; // Usage 0x000D0042: Tip Switch, Value = 0 to 1
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t DIG_TouchScreenFingerInRange : 1; // Usage 0x000D0032: In Range, Value = 0 to 1
    uint8_t DIG_TouchScreenFingerConfidence : 1; // Usage 0x000D0047: Confidence, Value = 0 to 1
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint8_t : 1; // Pad
    uint16_t GD_TouchScreenFingerX; // Usage 0x00010030: X, Value = 0 to 32767
    uint16_t GD_TouchScreenFingerY; // Usage 0x00010031: Y, Value = 0 to 32767
    uint16_t DIG_TouchScreenFingerWidth; // Usage 0x000D0048: Width, Value = 0 to 32767
                                         // Usage 0x000D0049 Height, Value = 0 to 32767 <-- Ignored: REPORT_COUNT (1) is
                                         // too small
    uint16_t pad_8; // Pad
    uint16_t DIG_TouchScreenFingerContactIdentifier[2]; // Usage 0x000D0051: Contact Identifier, Value = 0 to 32767
} TouchScreenReport_t;

// typedef struct
// {
//     uint8_t reportId; // bit 0 = finger up/down, bit 1 = finger in range
//     uint8_t

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

COM_InitTypeDef BspCOMInit;

/* USER CODE BEGIN PV */
static volatile uint8_t _usb_connected = 0;

TouchScreenReport_t touch_screen_report;
extern USBD_HandleTypeDef hUsbDeviceFS;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
void PeriphCommonClock_Config(void);
/* USER CODE BEGIN PFP */
extern uint8_t USBD_CUSTOM_HID_SendReport(USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);

void USB_ConnectionCallback(uint8_t state)
{
    _usb_connected = state;
    if (state)
    {
        BSP_LED_On(LED_GREEN);
    }
    else
    {
        BSP_LED_Off(LED_GREEN);
    }
}

// int _write(int le, char *ptr, int len)
// {
//     // This function is called by printf to send data to the UART
//     // It sends 'len' bytes from 'ptr' to the UART
//     HAL_UART_Transmit(COM1_UART, (uint8_t *)ptr, len, HAL_MAX_DELAY);
//     // Return the number of bytes written
//     return len;
// }

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* Configure the peripherals common clocks */
  PeriphCommonClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_USB_Device_Init();
  MX_SAI1_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Initialize leds */
  BSP_LED_Init(LED_BLUE);
  BSP_LED_Init(LED_GREEN);
  BSP_LED_Init(LED_RED);

  /* Initialize User push-button without interrupt mode. */
  BSP_PB_Init(BUTTON_SW1, BUTTON_MODE_GPIO);
  BSP_PB_Init(BUTTON_SW2, BUTTON_MODE_GPIO);
  BSP_PB_Init(BUTTON_SW3, BUTTON_MODE_GPIO);

  /* Initialize COM1 port (115200, 8 bits (7-bit data + 1 stop bit), no parity */
  BspCOMInit.BaudRate   = 115200;
  BspCOMInit.WordLength = COM_WORDLENGTH_8B;
  BspCOMInit.StopBits   = COM_STOPBITS_1;
  BspCOMInit.Parity     = COM_PARITY_NONE;
  BspCOMInit.HwFlowCtl  = COM_HWCONTROL_NONE;
  if (BSP_COM_Init(COM1, &BspCOMInit) != BSP_ERROR_NONE)
  {
    Error_Handler();
  }

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
    printf("Waiting for USB connection...\r\n");
    // while (_usb_connected == 0)
    {
        /* Wait for USB connection */
        HAL_Delay(100);
    }
    printf("USB connected!\n");

    touch_screen_report.reportId = 0x03;
    touch_screen_report.DIG_TouchScreenFingerTipSwitch = 1;
    touch_screen_report.DIG_TouchScreenFingerInRange = 1;
    touch_screen_report.DIG_TouchScreenFingerConfidence = 1;
    touch_screen_report.DIG_TouchScreenFingerContactIdentifier[0] = 1;
    touch_screen_report.GD_TouchScreenFingerX = 100;
    touch_screen_report.GD_TouchScreenFingerY = 100;
    // touch_screen_report.DIG_TouchScreenContactCount = 1;

    while (1)
    {
        // Wait for button press, then send a touch report
        if (BSP_PB_GetState(BUTTON_SW1) == GPIO_PIN_RESET)
        {
            // Light up LED
            BSP_LED_On(LED_BLUE);
            HAL_Delay(10); // Debounce delay
            BSP_LED_Off(LED_BLUE);

            static uint8_t counter = 0;
            counter = (counter + 10) % 100; // Increment counter by 10, wrap around at 100

            // Send the touch report
            // USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, (uint8_t *)&touch_screen_report, sizeof(touch_screen_report));
            uint8_t x_coord_percentage = counter;
            uint8_t y_coord_percentage = counter;

            printf("Button pressed, sending touch report: X=%d%%, Y=%d%%\n", x_coord_percentage, y_coord_percentage);

            uint8_t buff[5];
            // Report ID
            buff[0] = 0x03;
            // LSB of X coordinate percentage * 100 (0... 10000)
            buff[1] = (x_coord_percentage * 100) & 0xFF; // X coordinate LSB
            // MSB of X coordinate percentage * 100 (0... 10000)
            buff[2] = (x_coord_percentage * 100) >> 8; //
            // LSB of Y coordinate percentage * 100 (0... 10000)
            buff[3] = (y_coord_percentage * 100) & 0xFF; // Y coordinate LSB
            // MSB of Y coordinate percentage * 100 (0... 10000)
            buff[4] = (y_coord_percentage * 100) >> 8; // Y coordinate MSB
            USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, buff, sizeof(buff));

            HAL_Delay(100); // Debounce delay
            buff[0] = 0x02;
            USBD_CUSTOM_HID_SendReport(&hUsbDeviceFS, buff, sizeof(buff));
            HAL_Delay(1000); // Debounce delay
            // Turn off LED
            // BSP_LED_Off(LED_BLUE);
        }

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSE
                              |RCC_OSCILLATORTYPE_MSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.MSIState = RCC_MSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSICalibrationValue = RCC_MSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.MSIClockRange = RCC_MSIRANGE_6;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_MSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 32;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure the SYSCLKSource, HCLK, PCLK1 and PCLK2 clocks dividers
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK4|RCC_CLOCKTYPE_HCLK2
                              |RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.AHBCLK2Divider = RCC_SYSCLK_DIV2;
  RCC_ClkInitStruct.AHBCLK4Divider = RCC_SYSCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }

  /** Enable MSI Auto calibration
  */
  HAL_RCCEx_EnableMSIPLLMode();
}

/**
  * @brief Peripherals Common Clock Configuration
  * @retval None
  */
void PeriphCommonClock_Config(void)
{
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the peripherals clock
  */
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SMPS|RCC_PERIPHCLK_SAI1
                              |RCC_PERIPHCLK_USB;
  PeriphClkInitStruct.PLLSAI1.PLLN = 24;
  PeriphClkInitStruct.PLLSAI1.PLLP = RCC_PLLP_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLQ = RCC_PLLQ_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLR = RCC_PLLR_DIV2;
  PeriphClkInitStruct.PLLSAI1.PLLSAI1ClockOut = RCC_PLLSAI1_SAI1CLK|RCC_PLLSAI1_USBCLK;
  PeriphClkInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI1;
  PeriphClkInitStruct.UsbClockSelection = RCC_USBCLKSOURCE_PLLSAI1;
  PeriphClkInitStruct.SmpsClockSelection = RCC_SMPSCLKSOURCE_HSI;
  PeriphClkInitStruct.SmpsDivSelection = RCC_SMPSCLKDIV_RANGE0;

  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN Smps */

  /* USER CODE END Smps */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
    /* User can add his own implementation to report the HAL error return state */
    __disable_irq();
    while (1)
    {
    }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
    /* User can add his own implementation to report the file name and line number,
       ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
