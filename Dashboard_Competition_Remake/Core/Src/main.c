/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
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
#include "can.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD.h"
#include <time.h>


/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

#define TRUE 1
#define FALSE 0

#define MC_BASE 0x500
#define BATT_BASE 0x620
#define ARR_BASE 0x700
#define MCB_BASE 0x400
#define LV_BASE 0x450
#define FAULTS 0x622

#define CRUISE_TARGET 234 // Placeholder values
#define REGEN 543 // Placeholder values

#define FILTER_LEN 9

#define NUM_PAGES 4
#define PAGE_0 0
#define PAGE_1 1
#define PAGE_2 2
#define PAGE_3 3

#define TIMEOUT 10

#define GETBIT(var, bit)	(((var) >> (bit)) & 1) // gives bit position

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

CAN_FilterTypeDef CAN_filter0;
CAN_FilterTypeDef CAN_filter1;

CAN_RxHeaderTypeDef CAN_rx_header;
uint8_t CAN_rx_data[8];

union {
	float float_var;
	uint8_t chars[4];
}vel;

union {
	float float_var;
	uint8_t chars[4];
}cur;

union {
	float float_var;
	uint8_t chars[4];
} u;

uint8_t recent_warnings[4]; // [LV Warn, HV Warn, LT Warn, HT Warn]

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/**
 * Initialize Dashboard LED lights
 */
void InitLEDs(void)
{

	RCC->APB2ENR |= 0x1UL << 2; 		//Initialize clock for GPIOA, if it hasn't been initialized yet
	GPIOA->CRL &= 0;
	GPIOA->CRH &= 0;
	GPIOA->CRL |= 0x33330033UL;			//Set pins A1, A4, A5, A6, A7 to be Push-Pull Output, 50Mhz
	GPIOA->CRH |= 0x30033333UL;			//SetBar pins A8, A9, A10 to be Push-Pull Output, 50Mhz

	RCC->APB2ENR |= 0x1;
	AFIO->MAPR |= 0x2 << 24;
	//GPIOA->BSRR = 0xFFFF;

	//GPIOA->BSRR = 0x1 << 11;

	GPIOA->BRR = 0xFFFF;

}

/**
 * Called when warning CAN message received
 * Updates recent_warnings array with latest warnings
 */
void parse_warnings(void)
{
	uint8_t temp_byte = CAN_rx_data[1]; // Contains bits 8-15, 15 14 13 12 11 10 9 8
	uint8_t low_voltage_warning = GETBIT(temp_byte, 6);
	uint8_t high_voltage_warning = GETBIT(temp_byte, 7);
	uint8_t low_temperature_warning = GETBIT(temp_byte, 8);

	temp_byte = CAN_rx_data[2]; // Contains bits 16-23, 23 22 21 20 19 18 17 16
	uint8_t high_temperature_warning = GETBIT(temp_byte, 0);

	/* Update Warnings */
	recent_warnings[0] = low_voltage_warning;
	recent_warnings[1] = high_voltage_warning;
	recent_warnings[2] = low_temperature_warning;
	recent_warnings[3] = high_temperature_warning;
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  int32_t tempInt32;
  uint8_t c = 0;
  uint8_t d = 0;

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /*
   * Page Layout
   * Page 0: Main Page
   * Page 1: Warnings
   * Page 2: Current Summary
   * Page 3: Pack Summary (Voltage + Temperature)
   */

  // Page initialized to Page 0
  uint8_t current_page = 0;

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_CAN_Init();
  /* USER CODE BEGIN 2 */

  CanFilterSetup();
  HAL_CAN_Start(&hcan);

  // Commented Clock Setup as it is already done in the ioc
  //Setup System Clock C
  // RCC->APB2ENR &= 0;
  // RCC->APB2ENR |= 0x1UL << 4;

  // Commented Pinout setup as it is already done in the ioc
  //Setup Pins C5 - C12 as OUTPUT
  // GPIOC->CRL &= 0;
  // GPIOC->CRH &= 0;
  // GPIOC->CRL |= 0x33333333UL; //Initialise C0 to C7
  // GPIOC->CRH |= 0x33333UL; //Initialise C8 to C12

  //Set Pin initial values
  GPIOC->BSRR = 0x1UL << 0;	 // C0 HIGH
  GPIOC->BSRR = 0x1UL << 1;	 // C1 HIGH
  GPIOC->BSRR = 0x1UL << 2;	 // C2 HIGH
  GPIOC->BRR = 0x1UL << 11;	 // C11 LOW
  GPIOC->BSRR = 0x1UL << 12;   //C12 HIGH

  ScreenSetup();

  // Check if this is required!
  InitLEDs();

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 1);
		HAL_Delay(1000);
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, 0);
		HAL_Delay(1000);

	// Check if message is available
	if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) != 0)
	{
		// Populate CAN header and data variables (CAN_rx_header/data is updated respectively)
		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &CAN_rx_header, CAN_rx_data);

		uint16_t received_CAN_ID = (uint16_t) CAN_rx_header.StdId;
//		uint16_t received_CAN_ID = 0x503;

		/* Check for CAN message that is incoming to change the page
		 * This CAN message comes from the MCB
		 * The current_page is simply incremented by +1.
		 * if (current_page + 1 == NUM_PAGES) set current_page = 0
		 */

		uint8_t button_pressed = FALSE;
		uint8_t cruise_enabled = FALSE;

		if (received_CAN_ID == MCB_BASE)
		{
			/* Check for Button Press
			 * Bit 0 = button press event
			 * Bit 1 = cruise enabled
			 */
			uint8_t mcb_data = CAN_rx_data[0];

			/* For Cruise Enabled:
			 * - If Enabled, display the cruise velocity on page 1
			 * - If Disabled, display "OFF" instead of speed
			 */

			if (mcb_data == 1) { // binary 01
				button_pressed = TRUE;
			} else if (mcb_data == 3) { // binary 11
				button_pressed = TRUE;
				cruise_enabled = TRUE;
			} else if (mcb_data == 2){ // binary 10
				cruise_enabled = TRUE;
			} // any other cases, don't need to reassign.
		}

		if (button_pressed) {
			current_page = current_page + 1; // Increment page
			if (current_page == NUM_PAGES) current_page = 0; // Reset to 0 if changing from last page
			button_pressed = FALSE; // Set back to False
		}

		// Switch by page
		// If parsed message is not on the current page, it is ignored
		switch(current_page)
		{
			case PAGE_0: // main page
				UpdateScreenTitles(PAGE_0);
				switch(received_CAN_ID)
				{
					case (BATT_BASE + 4): // SOC
//						UpdateScreenParameter(SOC_DATA_XPOS, SOC_DATA_YPOS, (int8_t) 100, 0, FALSE);
						UpdateScreenParameter(SOC_DATA_XPOS, SOC_DATA_YPOS, (int8_t) CAN_rx_data[0], 0, FALSE);
						break;
					case CRUISE_TARGET: // Change MACRO
						UpdateScreenParameter(CRUISE_DATA_XPOS, CRUISE_DATA_YPOS, (int8_t) 80, 5, TRUE);
						// Add data parameter
						break;
					case (MC_BASE + 3): // Vehicle Velocity
						u.chars[0] = CAN_rx_data[4];
						u.chars[1] = CAN_rx_data[5];
						u.chars[2] = CAN_rx_data[6];
						u.chars[3] = CAN_rx_data[7];

						u.float_var = u.float_var * -3.6;
						tempInt32 = (int32_t) u.float_var;

						if (u.float_var < 0)
						{
							u.float_var = u.float_var * -1;
						}
//						UpdateScreenParameter(SPEED_DATA_XPOS, SPEED_DATA_YPOS, -45, 7, TRUE);
						UpdateScreenParameter(SPEED_DATA_XPOS, SPEED_DATA_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10, TRUE);
						break;
					case REGEN: // Change MACRO
						if (cruise_enabled) UpdateScreenParameter(REGEN_DATA_XPOS, REGEN_DATA_YPOS, 50, 0, FALSE);
						else {
							OutputString("     ", REGEN_DATA_XPOS, REGEN_DATA_YPOS); // Clear
							OutputString("OFF", REGEN_DATA_XPOS, REGEN_DATA_YPOS); // Write "OFF"
						}
						break;
					case FAULTS: ;
						/* ADD recent Warning on bottom line of page 0 */
						parse_warnings();
						/* Add code to update page 0 bottom line */

					default:
						// CAN message read is not part of the current page, Ignore.
						break;

				}
				break;
			case PAGE_1: // warnings
				UpdateScreenTitles(PAGE_1);
				switch(received_CAN_ID)
				{
					case (FAULTS): ; // need semicolon as have declaration after colon :
						/* Check which warning message
						 * Bit 13 = Low Voltage Warning
						 * Bit 14 = High Voltage Warning
						 * Bit 15 = Low Temperature Warning
						 * Bit 16 = High Temperature Warning
						 */

						parse_warnings();

						if (recent_warnings[0]) { // Low Voltage Warning
							OutputString("     ", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Clear
							OutputString("YES", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Write "YES"
						} else {
							OutputString("     ", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Clear
							OutputString("---", LV_WARN_DATA_XPOS, LV_WARN_DATA_YPOS); // Write "---"
						}
						if (recent_warnings[1]) { // High Voltage Warning
							OutputString("     ", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Clear
							OutputString("YES", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Write "YES"
						} else {
							OutputString("     ", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Clear
							OutputString("---", HV_WARN_DATA_XPOS, HV_WARN_DATA_YPOS); // Write "---"
						}
						if (recent_warnings[2]) { // Low Temperature Warning
							OutputString("     ", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Clear
							OutputString("YES", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Write "YES"
						} else {
							OutputString("     ", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Clear
							OutputString("---", LT_WARN_DATA_XPOS, LT_WARN_DATA_YPOS); // Write "---"
						}
						if (recent_warnings[3]) { // High Temperature Warning
							OutputString("     ", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Clear
							OutputString("YES", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Write "YES"
						} else {
							OutputString("     ", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Clear
							OutputString("---", HT_WARN_DATA_XPOS, HT_WARN_DATA_YPOS); // Write "---"
						}

						break;
					default:
						// CAN message read is not part of the current page, Ignore.
						break;
				}

				break;
			case PAGE_2: // Current Summary
				UpdateScreenTitles(PAGE_2);
				switch(received_CAN_ID)
				{
					case (MCB_BASE + 1): // Motor Current
						// Upper 4 bytes of data
						u.chars[0] = CAN_rx_data[4];
						u.chars[1] = CAN_rx_data[5];
						u.chars[2] = CAN_rx_data[6];
						u.chars[3] = CAN_rx_data[7];
						tempInt32 = (int32_t) u.float_var;
						UpdateScreenParameter(MOTOR_CURRENT_DATA_XPOS, MOTOR_CURRENT_DATA_YPOS, tempInt32, 0, FALSE); // percentage of max current
						break;
					case (ARR_BASE + 2): // Array Current
						// Using Sensor 2 Data
						// Upper 4 bytes of data
						u.chars[0] = CAN_rx_data[4];
						u.chars[1] = CAN_rx_data[5];
						u.chars[2] = CAN_rx_data[6];
						u.chars[3] = CAN_rx_data[7];
						tempInt32 = (int32_t) u.float_var;
						UpdateScreenParameter(ARRAY_CURRENT_DATA_XPOS, ARRAY_CURRENT_DATA_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10, TRUE); // Float
						break;
					case (LV_BASE): // Array Current
						// Using Sensor 2 Data
						// 2nd lowest byte
						UpdateScreenParameter(ARRAY_CURRENT_DATA_XPOS, ARRAY_CURRENT_DATA_YPOS, CAN_rx_data[1], 0, FALSE);
						break;
					case (MCB_BASE + 2): // Bus Current
						// Upper 4 bytes of data
						u.chars[0] = CAN_rx_data[4];
						u.chars[1] = CAN_rx_data[5];
						u.chars[2] = CAN_rx_data[6];
						u.chars[3] = CAN_rx_data[7];
						tempInt32 = (int32_t) u.float_var;
						UpdateScreenParameter(BUS_CURRENT_DATA_XPOS, BUS_CURRENT_DATA_YPOS, tempInt32, 0, FALSE); // percentage of max current
						break;
					default:
						// CAN message read is not part of the current page, Ignore.
						break;
				}
				break;
			case PAGE_3: // Pack Summary (Voltage + Temperature)
				UpdateScreenTitles(PAGE_3);
				switch(received_CAN_ID)
				{
					default:
						// CAN message read is not part of the current page, Ignore.
						break;
				}
				break;
			default:
				// Should never reach here.
				break;
		}

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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
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
