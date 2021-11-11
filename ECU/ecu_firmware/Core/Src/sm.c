//-------------------------------------------
//File: sm.c
//Brief: Finite State Machine Implementation
//Author: Yuqi Fu
//-------------------------------------------

//-------------------------------------------
// Copyright 2021 UBC Solar, all rights reserved
//-------------------------------------------

//-------------------------------------------
// Standard Lib Include Files
//-------------------------------------------

//-------------------------------------------
// Other Lib Include Files
//-------------------------------------------
#include "sm.h"
#include "stm32f1xx_hal.h"
#include "main.h"
//-------------------------------------------
// Constant Definitions & Macros
//-------------------------------------------

//-------------------------------------------
// Local Variables
//-------------------------------------------

//look-up table for all state handlers
static void (*sm_pfaStateHandler[SM_eStateMaxNum])(void);

//state number storage
static uint8_t stateVal;

//-------------------------------------------
// Global Variables
//-------------------------------------------

//-------------------------------------------
// Local Function Prototypes
//-------------------------------------------
static void sm_State1Handler(void);
static void sm_State2Handler(void);
static void sm_State3Handler(void);
static void sm_State4Handler(void);
static void sm_State5Handler(void);
static void sm_State6Handler(void);
static void sm_State7Handler(void);
static void sm_State8Handler(void);
static void sm_State9Handler(void);
static void sm_State10Handler(void);
//-------------------------------------------
// Local Function Implementations
//-------------------------------------------

//
// state1 handler function: sets fault and low power supply indication
//

static void sm_State1Handler(void)
{
	HAL_GPIO_WritePin(GPIOB, FLT_OUT_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, SUPP_LOW_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

static void sm_State2Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, NEG_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

static void sm_State3Handler(void)
{
	HAL_GPIO_WritePin(GPIOB, DCDC_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

static void sm_State4Handler(void)
{
	HAL_GPIO_WritePin(GPIOB, SWAP_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

static void sm_State5Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, FAN_OUT1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, FAN_OUT2_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

static void sm_State6Handler(void)
{
	HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

static void sm_State7Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, FLT_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, SUPP_LOW_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);
}

static void sm_State8Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, HLIM_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(1000);
}

static void sm_State9Handler(void)
{
	HAL_Delay(1000);
}

static void sm_State10Handler(void)
{
	Error_Handler();
}

static void sm_UpdateStateNum(void)
{
	stateVal++;
	if (stateVal > SM_eState9)
	{
		stateVal = SM_eState9;
	}
}

//-------------------------------------------
// Global Function Implementations
//-------------------------------------------

void SM_Init(void)
{
	stateVal = (uint8_t)SM_eState1;
	sm_pfaStateHandler[SM_eState1] = &sm_State1Handler;
	sm_pfaStateHandler[SM_eState2] = &sm_State2Handler;
	sm_pfaStateHandler[SM_eState3] = &sm_State3Handler;
	sm_pfaStateHandler[SM_eState4] = &sm_State4Handler;
	sm_pfaStateHandler[SM_eState5] = &sm_State5Handler;
	sm_pfaStateHandler[SM_eState6] = &sm_State6Handler;
	sm_pfaStateHandler[SM_eState7] = &sm_State7Handler;
	sm_pfaStateHandler[SM_eState8] = &sm_State8Handler;
	sm_pfaStateHandler[SM_eState9] = &sm_State9Handler;
	sm_pfaStateHandler[SM_eState10] = &sm_State10Handler;
}