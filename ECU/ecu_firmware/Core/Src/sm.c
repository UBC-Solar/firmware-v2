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

#define SM_STATE_DELAY_TIME			(1000u)
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
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state2 handler function: start precharge and close negative terminal
//
static void sm_State2Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, NEG_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(SM_STATE_DELAY_TIME);
}


//
// state3 handler function: close dcdc and wait till it stablizes
//
static void sm_State3Handler(void)
{
	HAL_GPIO_WritePin(GPIOB, DCDC_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state4 handler function: swap power source from aux battery to dcdc
//
static void sm_State4Handler(void)
{
	HAL_GPIO_WritePin(GPIOB, SWAP_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state5 handler function: close FAN power mosfet
//
static void sm_State5Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, FAN_OUT1_Pin, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOB, FAN_OUT2_Pin, GPIO_PIN_SET);
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state6 handler function: close llim to activate motor
//
static void sm_State6Handler(void)
{
	HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state7 handler function: end precharge sequence
//
static void sm_State7Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, FLT_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, SUPP_LOW_Pin, GPIO_PIN_RESET);
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state8 handler function: close hlim for mppt
//
static void sm_State8Handler(void)
{
	HAL_GPIO_WritePin(GPIOA, HLIM_OUT_Pin, GPIO_PIN_SET);
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state9 handler function: adc reading and can message handling
//
static void sm_State9Handler(void)
{
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state10 handler function: open all terminals and indicate fault in a forever loop
//
static void sm_State10Handler(void)
{
	__disable_irq();
	HAL_GPIO_WritePin(GPIOB, SWAP_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, HLIM_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, NEG_OUT_Pin, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOB, DCDC_OUT_Pin, GPIO_PIN_RESET);
	for(;;)
	{
		HAL_GPIO_WritePin(GPIOB, FLT_OUT_Pin, GPIO_PIN_SET);
		HAL_Delay(SM_STATE_DELAY_TIME);
		HAL_GPIO_WritePin(GPIOB, FLT_OUT_Pin, GPIO_PIN_RESET);
		HAL_Delay(SM_STATE_DELAY_TIME);
	}
}

//
// fault-checking function: goes into fault based on flt_in and estop_in signal
//
static void sm_FltCheck(void)
{
	GPIO_PinState st1 = HAL_GPIO_ReadPin(GPIOA, FLT_IN_Pin);
	GPIO_PinState st2 =	HAL_GPIO_ReadPin(GPIOB, ESTOP_IN_Pin);
	if (st1 == GPIO_PIN_SET || st2 == GPIO_PIN_RESET)
	{
		stateVal = (uint8_t)SM_eState10;
	}
}

//
// state updating function: increments until state 9 and stays there unless fault occurs
//
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
