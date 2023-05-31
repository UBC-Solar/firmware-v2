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

#define FILTER_LEN 9

#define NUM_PAGES 4
#define PAGE_0 0
#define PAGE_1 1
#define PAGE_2 2
#define PAGE_3 3

#define TIMEOUT 10

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

  uint16_t CAN_messages_by_page = {{4, // Num of items
		  	  	  	  	  	  	    0x624, // SOC
  	  	  	  	  	  	  	  	  	// cruise,
		  	  	  	  	  	  	  	0x503 // Vehicle Velocity
									// regen % (new message required)
  	  	  	  	  	  	  	  	  	}, // Page 0: Main
  	  	  	  	  	  	  	  	  {1}, // Page 1: Warnings
  	  	  	  	  	  	  	  	  {3, // Num of items
  	  	  	  	  	  	  	  	   0x501 // Motor current
  	  	  	  	  	  	  	  	   // array current
  	  	  	  	  	  	  	  	   // low voltage current
  	  	  	  	  	  	  	  	  }, // Page 2: Current Summary
  	  	  	  	  	  	  	  	  {2, // num of items
  	  	  	  	  	  	  	  	    0x625, // pack temperature
  	  	  	  	  	  	  	  		0x623 // voltage
  	  	  	  	  	  	  	  	  }}; // Page 3: Pack Summary

  // Page initialized to Page 0
  uint8_t current_page = 0;

  // Timers
  time_t startTime;
  time_t currentTime;

  // Set start time
  time(&startTime);

  uint8_t check_time = FALSE;

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
//	if (HAL_CAN_GetRxFifoFillLevel(&hcan, CAN_RX_FIFO0) != 0)
	if (1)
	{
		// Populate CAN header and data variables (CAN_rx_header/data is updated respectively)
//		HAL_CAN_GetRxMessage(&hcan, CAN_RX_FIFO0, &CAN_rx_header, CAN_rx_data);

//		uint16_t curr_CAN_ID = (uint16_t) CAN_rx_header.StdID;

		// Check which page the can message belongs to
//		message_on_current_page = FALSE;
//		uint8_t curr_msg_page = 0xFF; // will change if the message is found

//		for (int i = 0; i < NUM_PAGES; i++) {
//			// Obtain length of sub array (first element of sub array is the length)
//			uint16_t subarr_len = CAN_messages_by_page[i][0];
//			for (int j = 1; j < subarr_len; j++) {
//				// Check if the message is found on the current page
//				if ((uint16_t) curr_CAN_ID == CAN_messages_by_page[i][j]) {
//					message_on_current_page = TRUE;
//					curr_msg_page = i;
//					break;
//				}
//			}
//			if (message_on_current_page) {
//				// message is found
//				break;
//			}
//		}

		/* Add Check for CAN message that is incoming to change the page
		 * This CAN message comes from the MCB
		 * The current_page is simply incremented by +1.
		 * if (current_page + 1 == NUM_PAGES) set current_page = 0
		 */


		// Switch by page
		// If parsed message is not on the current page, it is ignored
		switch(current_page)
		{
			case PAGE_0: // main page
				UpdateScreenTitles(PAGE_0);
//				switch(curr_CAN_ID)
//				{
//					case (BATT_BASE + 4): // SOC
//						UpdateScreenParameter(SOC_DATA_XPOS, SOC_DATA_YPOS, (int8_t) CAN_rx_data[0], 0);
//						// Add UNIT "%"
//						break;
//					// case CRUISE_TARGET:
//					case (MC_BASE + 3): // Vehicle Velocity
//						u.chars[0] = CAN_rx_data[4];
//						u.chars[1] = CAN_rx_data[5];
//						u.chars[2] = CAN_rx_data[6];
//						u.chars[3] = CAN_rx_data[7];
//
//						u.float_var = u.float_var * -3.6;
//						tempInt32 = (int32_t) u.float_var;
//
//						if (u.float_var < 0)
//						{
//							u.float_var = u.float_var * -1;
//						}
//
//						UpdateScreenParameter(SPEED_DATA_XPOS, SPEED_DATA_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10);
//						break;
//					//case REGEN_%:
//					default:
//						// Can message read is not part of the current page, Ignore.
//						break;
//
//				}
				break;
//			case PAGE_1: // warnings
//				UpdateScreenTitles(PAGE_1);
//				if (check_time == FALSE) {
//					// start timer
//					// Only needs this condition in PAGE_1 because we have to go by PAGE_1 to get to other pages
//					time(&startTime);
//					check_time = TRUE;
//				} else {
//					// check_time == TRUE
//					time(&currentTime);
//					if (currentTime - startTime > TIMEOUT ) {
//						current_page = PAGE_0; // TIMEOUT has occurred --> Reset to Main Page
//						check_time = FALSE;
//					}
//				}
//				switch(curr_CAN_ID)
//				{
//					default:
//						// Can message read is not part of the current page, Ignore.
//						break;
//				}
//
//				break;
//			case PAGE_2: // Current Summary
//				UpdateScreenTitles(PAGE_2);
//				if (check_time == TRUE) {
//					time(&currentTime);
//					if (currentTime - startTime > TIMEOUT ) {
//						current_page = PAGE_0; // TIMEOUT has occurred --> Reset to Main Page
//						check_time = FALSE;
//					} else {
//						time(&startTime); // reset timer
//					}
//				}
//				switch(curr_CAN_ID)
//				{
//					default:
//						// Can message read is not part of the current page, Ignore.
//						break;
//				}
//				break;
//			case PAGE_3: // Pack Summary (Voltage + Temperature)
//				UpdateScreenTitles(PAGE_3);
//				if (check_time == TRUE) {
//					time(&currentTime);
//					if (currentTime - startTime > TIMEOUT ) {
//						current_page = PAGE_0; // TIMEOUT has occurred --> Reset to Main Page
//						check_time = FALSE;
//					} else {
//						time(&startTime); // reset timer
//					}
//				}
//				switch(curr_CAN_ID)
//				{
//					default:
//						// Can message read is not part of the current page, Ignore.
//						break;
//				}
//				break;
			default:
				// Ignore CAN message as it is not on the current page
				// Can add a print statement here for debugging
				break;
		}




		// Switch statement based on CAN ID - E.g. BATT_BASE = 620
		switch(CAN_rx_header.StdId)
		{

			//Battery: Pack Voltage(For LCD Display). Values are unsigned 16-bit integers in Volts (V). Period: 1s
			//Battery: Minimum Cell Voltage (For LCD Display). Values are unsigned 8-bit integers in 100mv intervals. Period: 1s
			//Battery: Maximum Cell Voltage (For LCD Display). Values are unsigned 8-bit integers in 100mv intervals. Period: 1s
			case BATT_BASE + 3:
				UpdateScreenParameter(BATTERY_VOLTAGE_XPOS, BATTERY_VOLTAGE_YPOS, (uint16_t) (CAN_rx_data[1] | CAN_rx_data[0] << 8), 0);

				UpdateScreenParameter(BATTERY_MINVOLT_XPOS, BATTERY_MINVOLT_YPOS, (uint8_t) CAN_rx_data[2] / 10, (uint8_t) CAN_rx_data[2] % 10);

				UpdateScreenParameter(BATTERY_MAXVOLT_XPOS, BATTERY_MAXVOLT_YPOS, (uint8_t) CAN_rx_data[4] / 10, (uint8_t) CAN_rx_data[4] % 10);

				//XBeeTransmitCan(&CAN_rx_msg);
				break;

			//Battery: Pack Current(For LCD Display). Values are signed 16-bit integers in Amperes (A). Period: 1s
			case BATT_BASE + 4:

				UpdateScreenParameter(BATTERY_CURRENT_XPOS, BATTERY_CURRENT_YPOS, (int16_t) (CAN_rx_data[1] | CAN_rx_data[0] << 8), 0);

				//XBeeTransmitCan(&CAN_rx_msg);
				break;

			//Battery: Pack Maximum Temperature (For LCD Display). Values are signed 8-bit integers in Celsius (C). Period: 1s
			case BATT_BASE + 7:
				UpdateScreenParameter(BATTERY_MAXTEMP_XPOS, BATTERY_MAXTEMP_YPOS, (int8_t) CAN_rx_data[4], 0);

				//XBeeTransmitCan(&CAN_rx_msg);
				break;

			//Battery: State of Charge (For LCD Display). Values are unsigned 8-bit integers. Period: 1s
			case BATT_BASE + 6:

				UpdateScreenParameter(BATTERY_CHARGE_XPOS, BATTERY_CHARGE_YPOS, (int8_t) CAN_rx_data[0], 0);

				//This one is different; it is used to set a battery percentage bar
				//SetBar(0xFF & CAN_rx_data[0], 100, CHARGE_BAR_YPOS);

				//XBeeTransmitCan(&CAN_rx_msg);
				break;

			//NOT FULLY IMPLEMENTED
			case BATT_BASE + 30:

				UpdateScreenParameter(BATTERY_SUPPVOLT_XPOS, BATTERY_SUPPVOLT_YPOS, 0, 0);

				break;

			//Motor Drive Unit: Speed (For LCD Display). Values are IEEE 32-bit floating point in m/s. Period: 200ms
			case MC_BASE + 3:

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

				UpdateScreenParameter(MOTOR_SPEED_XPOS, MOTOR_SPEED_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );

				//send the CAN message once every second
				if (d == 5)
				{
					//XBeeTransmitCan(&CAN_rx_msg);
					d = 0;
				}
				d++;

				break;

			//Motor Drive Unit: Temperature (For LCD Display). Values are IEEE 32-bit floating point in Celsius (C). Period: 1s
			case MC_BASE + 0xB:


				u.chars[0] = CAN_rx_data[0];
				u.chars[1] = CAN_rx_data[1];
				u.chars[2] = CAN_rx_data[2];
				u.chars[3] = CAN_rx_data[3];

				tempInt32 = (int32_t) u.float_var;

				while(u.float_var < 0)
				{
					u.float_var = u.float_var * -1;
				}

				//XBeeTransmitCan(&CAN_rx_msg);

				UpdateScreenParameter(MOTOR_TEMP_XPOS, MOTOR_TEMP_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
				break;

			//Motor Drive Unit: Current (For LCD Display). Values are IEEE 32-bit floating point in Amperes(A). Period: 200ms
			case MC_BASE + 2:

				u.chars[0] = CAN_rx_data[4];
				u.chars[1] = CAN_rx_data[5];
				u.chars[2] = CAN_rx_data[6];
				u.chars[3] = CAN_rx_data[7];

				tempInt32 = (int32_t) u.float_var;

				if (u.float_var < 0)
				{
					u.float_var = u.float_var * -1;
				}

				//Send the CAN message once every second
				if (c == 5)
				{
					//XBeeTransmitCan(&CAN_rx_msg);
					c = 0;
				}
				c++;

				UpdateScreenParameter(MOTOR_CURRENT_XPOS, MOTOR_CURRENT_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
				break;

			//DEPRECATED
			//Array: Maximum Temperature (For LCD Display). Values are 16-bit unsigned integer in Celsius(C). Period: 1s
			//case ARR_BASE + 2:
			//
			//	tempInt32 = CAN_rx_data[6] << 8 | CAN_rx_data[7];
			//
			//	UpdateScreenParameter(ARRAY_MAXTEMP_XPOS, ARRAY_MAXTEMP_YPOS, tempInt32, (uint32_t) (tempInt32 / 10) % 10);
			//
			//	XBeeTransmitCan(&CAN_rx_msg);
			//
			//	break;

			//Battery: Faults, Battery High and Battery Low (For Dashboard Indicator)
			case BATT_BASE + 2:

				//Turn on LEDs if the appropriate battery fault exists.

				//Battery Low:	A0
				//Battery Full: A1
				//Battery Communications Fault: A4
				//Battery charge over-current: A6
				//Battery discharge over-current: A7
				//Battery over-temperature: A8
				//Battery under-voltage: A9
				//Battery over-voltage: A10

				GPIOA->BSRR = (CAN_rx_data[6] & 0x3) || \
							( ( (CAN_rx_data[5] >> 2) & 0x1) << 4 ) || \
							( ( (CAN_rx_data[5] >> 4) & 0x1F) << 6);

				/*
				//A10: Check if the high voltage bit is set, meaning battery is full
				if ( (CAN_rx_data[6] >> 1) & 0x1)
				{
					//Reset pp;in A10, turn on LED
					GPIOA->BSRR = 0x1 << 10;
				}

				//A9: Check if the low voltage bit is set, meaning battery is low
				if ( (CAN_rx_data[6] ) & 0x1 )
				{
					//Reset pin A9, turn on LED
					GPIOA->BSRR = 0x1 << 9;
				}

				//A1: battery over-temperature fault
				if ( ( CAN_rx_data[5] >> 5 ) & 0x1 )
				{
					//Reset pin A1, turn on LED
					GPIOA->BSRR = 0x1 << 1;
				}

				//A4: battery discharge over-current fault
				if ( ( CAN_rx_data[5] >> 4 ) & 0x1 )
				{
					//Reset pin A4, turn on LED
					GPIOA->BSRR = 0x1 << 4;
				}

				//A6: battery charge over-current fault
				if ( ( CAN_rx_data[5] >> 3 ) & 0x1 )
				{
					//Reset pin A6, turn on LED
					GPIOA->BSRR = 0x1 << 6;
				}

				//A8: battery over-voltage fault
				if ( ( CAN_rx_data[5] >> 7 ) & 0x1 )
				{
					//Reset pin A8, turn on LED
					GPIOA->BSRR = 0x1 << 8;
				}

				//A7: battery under-voltage fault
				if ( ( CAN_rx_data[5] >> 6 ) & 0x1 )
				{
					//Reset pin A7, turn on LED
					GPIOA->BSRR = 0x1 << 7;
				}

				//A11: Communications Fault
				if ( (CAN_rx_data[5] >> 2) & 0x1)
				{
					//Reset pin A11: turn on LED
					GPIOA->BSRR = 0x1 << 11;
				}

				//XBeeTransmitCan(&newCanMsg);
				*/

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
