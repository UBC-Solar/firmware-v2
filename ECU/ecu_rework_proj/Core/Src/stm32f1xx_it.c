/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    stm32f1xx_it.c
  * @brief   Interrupt Service Routines.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "stm32f1xx_it.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "sm.h"
#include "supp.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN TD */

/* USER CODE END TD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define TIM2_DEBOUNCE_TIME_MS 				(50u)
#define TIM2_PERIOD_TIME_MS 				(50u)
#define PC_FLAG_SET							(1u)
#define PC_FLAG_RESET						(0u)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN PV */

//
// Digital input debounce states
//
typedef enum digin_eDebounceState
{
	digin_MaybeDown = 0u,
	digin_Down,
	digin_MaybeUp,
	digin_Up,
}digin_eDebounceState;

static volatile digin_eDebounceState diginLLIMState = digin_MaybeDown;
static volatile digin_eDebounceState diginLLIMNextState = digin_MaybeDown;
static volatile uint8_t diginLLIMDebounceTim = 0u;

static volatile digin_eDebounceState diginHLIMState = digin_MaybeDown;
static volatile digin_eDebounceState diginHLIMNextState = digin_MaybeDown;
static volatile uint8_t diginHLIMDebounceTim = 0u;

static volatile uint8_t pcFlag = PC_FLAG_RESET;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN PFP */
static uint8_t digin_LLIMIsHigh(void);
static uint8_t digin_HLIMIsHigh(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/* External variables --------------------------------------------------------*/
extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
/* USER CODE BEGIN EV */

volatile uint8_t itDbCounter;//Debounce counter

/* USER CODE END EV */

/******************************************************************************/
/*           Cortex-M3 Processor Interruption and Exception Handlers          */
/******************************************************************************/
/**
  * @brief This function handles Non maskable interrupt.
  */
void NMI_Handler(void)
{
  /* USER CODE BEGIN NonMaskableInt_IRQn 0 */

  /* USER CODE END NonMaskableInt_IRQn 0 */
  /* USER CODE BEGIN NonMaskableInt_IRQn 1 */
  while (1)
  {
  }
  /* USER CODE END NonMaskableInt_IRQn 1 */
}

/**
  * @brief This function handles Hard fault interrupt.
  */
void HardFault_Handler(void)
{
  /* USER CODE BEGIN HardFault_IRQn 0 */

  /* USER CODE END HardFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_HardFault_IRQn 0 */
    /* USER CODE END W1_HardFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Memory management fault.
  */
void MemManage_Handler(void)
{
  /* USER CODE BEGIN MemoryManagement_IRQn 0 */

  /* USER CODE END MemoryManagement_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_MemoryManagement_IRQn 0 */
    /* USER CODE END W1_MemoryManagement_IRQn 0 */
  }
}

/**
  * @brief This function handles Prefetch fault, memory access fault.
  */
void BusFault_Handler(void)
{
  /* USER CODE BEGIN BusFault_IRQn 0 */

  /* USER CODE END BusFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_BusFault_IRQn 0 */
    /* USER CODE END W1_BusFault_IRQn 0 */
  }
}

/**
  * @brief This function handles Undefined instruction or illegal state.
  */
void UsageFault_Handler(void)
{
  /* USER CODE BEGIN UsageFault_IRQn 0 */

  /* USER CODE END UsageFault_IRQn 0 */
  while (1)
  {
    /* USER CODE BEGIN W1_UsageFault_IRQn 0 */
    /* USER CODE END W1_UsageFault_IRQn 0 */
  }
}

/**
  * @brief This function handles System service call via SWI instruction.
  */
void SVC_Handler(void)
{
  /* USER CODE BEGIN SVCall_IRQn 0 */

  /* USER CODE END SVCall_IRQn 0 */
  /* USER CODE BEGIN SVCall_IRQn 1 */

  /* USER CODE END SVCall_IRQn 1 */
}

/**
  * @brief This function handles Debug monitor.
  */
void DebugMon_Handler(void)
{
  /* USER CODE BEGIN DebugMonitor_IRQn 0 */

  /* USER CODE END DebugMonitor_IRQn 0 */
  /* USER CODE BEGIN DebugMonitor_IRQn 1 */

  /* USER CODE END DebugMonitor_IRQn 1 */
}

/**
  * @brief This function handles Pendable request for system service.
  */
void PendSV_Handler(void)
{
  /* USER CODE BEGIN PendSV_IRQn 0 */

  /* USER CODE END PendSV_IRQn 0 */
  /* USER CODE BEGIN PendSV_IRQn 1 */

  /* USER CODE END PendSV_IRQn 1 */
}

/**
  * @brief This function handles System tick timer.
  */
void SysTick_Handler(void)
{
  /* USER CODE BEGIN SysTick_IRQn 0 */

  /* USER CODE END SysTick_IRQn 0 */
  HAL_IncTick();
  /* USER CODE BEGIN SysTick_IRQn 1 */

  /* USER CODE END SysTick_IRQn 1 */
}

/******************************************************************************/
/* STM32F1xx Peripheral Interrupt Handlers                                    */
/* Add here the Interrupt Handlers for the used peripherals.                  */
/* For the available peripheral interrupt handler names,                      */
/* please refer to the startup file (startup_stm32f1xx.s).                    */
/******************************************************************************/

/**
  * @brief This function handles ADC1 and ADC2 global interrupts.
  */
void ADC1_2_IRQHandler(void)
{
  /* USER CODE BEGIN ADC1_2_IRQn 0 */
  SUPP_UpdateAdcFilter(HAL_ADC_GetValue(&hadc1));
  /* USER CODE END ADC1_2_IRQn 0 */
  HAL_ADC_IRQHandler(&hadc1);
  /* USER CODE BEGIN ADC1_2_IRQn 1 */

  /* USER CODE END ADC1_2_IRQn 1 */
}

/**
  * @brief This function handles TIM2 global interrupt.
  */
void TIM2_IRQHandler(void)
{
 /* USER CODE BEGIN TIM2_IRQn 0 */
  diginLLIMState = diginLLIMNextState;
  switch(diginLLIMState)
  {
	  case digin_MaybeDown:
		  if(digin_LLIMIsHigh())
		  {
			  diginLLIMNextState = digin_MaybeUp;
		  }
		  else
		  {
			  if(diginLLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
			  {
				  diginLLIMDebounceTim += TIM2_PERIOD_TIME_MS;
				  diginLLIMNextState = digin_MaybeDown;
			  }
			  else
			  {
				  diginLLIMDebounceTim = 0u;
				  diginLLIMNextState = digin_Down;
			  }
		  }
		  break;

	  case digin_Down:
		  if(digin_LLIMIsHigh())
		  {
			  diginLLIMNextState = digin_MaybeUp;
		  }
		  else
		  {
			  diginLLIMNextState = digin_Down;
			  pcFlag = PC_FLAG_SET;
			  HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_RESET);
			  HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_RESET);
		  }
		  break;


	  case digin_MaybeUp:
		  if(digin_LLIMIsHigh())
		  {
			  if(diginLLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
			  {
				  diginLLIMDebounceTim += TIM2_PERIOD_TIME_MS;
				  diginLLIMNextState = digin_MaybeUp;
			  }
			  else
			  {
				  diginLLIMDebounceTim = 0u;
				  diginLLIMNextState = digin_Up;
			  }
		  }
		  else
		  {
			  diginLLIMNextState = digin_MaybeDown;
		  }
		  break;

	  case digin_Up:
		  if(digin_LLIMIsHigh())
		  {
			  diginLLIMNextState = digin_Up;
			  if(pcFlag == PC_FLAG_SET)
			  {
				  pcFlag = PC_FLAG_RESET;
				  SM_SetStatusFlag(SM_STATUS_LLIM_HIGH_FLAG);
			  }

		  }
		  else
		  {
			  diginLLIMNextState = digin_MaybeDown;
		  }
		  break;

	  default:
		  /* Do Nothing */
		  break;

  }

  diginHLIMState = diginHLIMNextState;

  switch(diginHLIMState)
  {
  case digin_MaybeDown:
	  if(digin_HLIMIsHigh())
	  {
		  diginHLIMNextState = digin_MaybeUp;
	  }
	  else
	  {
		  if(diginHLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
		  {
			  diginHLIMDebounceTim += TIM2_PERIOD_TIME_MS;
			  diginHLIMNextState = digin_MaybeDown;
		  }
		  else
		  {
			  diginHLIMDebounceTim = 0u;
			  diginHLIMNextState = digin_Down;
		  }
	  }
	  break;

  case digin_Down:
	  if(digin_HLIMIsHigh())
	  {
		  diginHLIMNextState = digin_MaybeUp;
	  }
	  else
	  {
		  diginHLIMNextState = digin_Down;
		  HAL_GPIO_WritePin(GPIOA, HLIM_OUT_Pin, GPIO_PIN_RESET);
	  }
	  break;

  case digin_MaybeUp:
	  if(digin_HLIMIsHigh())
	  {
		  if(diginHLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
		  {
			  diginHLIMDebounceTim += TIM2_PERIOD_TIME_MS;
			  diginHLIMNextState = digin_MaybeUp;
		  }
		  else
		  {
			  diginHLIMDebounceTim = 0u;
			  diginHLIMNextState = digin_Up;
		  }
	  }
	  else
	  {
		  diginHLIMNextState = digin_MaybeDown;
	  }
	  break;

  case digin_Up:
	  if(digin_HLIMIsHigh())
	  {
		  diginHLIMNextState = digin_Up;
		  HAL_GPIO_WritePin(GPIOA, HLIM_OUT_Pin, GPIO_PIN_SET);
	  }
	  else
	  {
		  diginHLIMNextState = digin_MaybeDown;
	  }
	  break;

  default:
	  /* Do Nothing */
	  break;

  }

  /* USER CODE END TIM2_IRQn 0 */
  HAL_TIM_IRQHandler(&htim2);
  /* USER CODE BEGIN TIM2_IRQn 1 */
  /* USER CODE END TIM2_IRQn 1 */
}

/* USER CODE BEGIN 1 */

static uint8_t digin_LLIMIsHigh(void)
{
	return (GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA, LLIM_IN_Pin));
}

static uint8_t digin_HLIMIsHigh(void)
{
	return (GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA, HLIM_IN_Pin));
}
/* USER CODE END 1 */
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
