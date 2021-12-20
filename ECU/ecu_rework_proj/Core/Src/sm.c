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
#include "supp.h"
//-------------------------------------------
// Constant Definitions & Macros
//-------------------------------------------

#define SM_STATE_DELAY_TIME			(1000u)
#define SM_FLT_MAX_COUNTER_VAL		(3u)
#define SM_PC_DELAY_TIME_1          (4500u)
#define SM_PC_DELAY_TIME_2          (500u)
#define SM_ADCVAL_LOWER_MASK		((uint16_t)0xff)
#define SM_ADCVAL_SHIFT_VAL			(8u)
#define SM_ADCVAL_UPPER_MASK		((uint16_t)(0xff << SM_ADCVAL_SHIFT_VAL))

//-------------------------------------------
// Local Variables
//-------------------------------------------

//look-up table for all state handlers
static void (*sm_pfaStateHandler[SM_eStateMaxNum])(void);

//state number storage
static volatile uint8_t sm_stateVal;

//status flags for flt, hlim high, and llim high
static volatile uint8_t sm_statusFlags;

CAN_TxHeaderTypeDef zTxHeader =
{
	.DLC = 2u,
	.IDE = CAN_ID_STD,
	.RTR = CAN_RTR_DATA,
	.StdId = 0x446
};


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
static void sm_FltCheck(void);
static uint8_t sm_IsStatusFlagged(uint8_t flag);
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
	//start timer to check for hlim and llim inputs
	HAL_TIM_Base_Start_IT(&htim2);
	HAL_Delay(SM_STATE_DELAY_TIME);
}

//
// state9 handler function: adc reading and can message handling
//
static void sm_State9Handler(void)
{
	static uint16_t reading = 0u;
	static uint8_t aTxData[8] = {0u};
	static uint32_t txMailBox;

	//read ADC value and send via can
	reading = SUPP_GetAdcVal();
	aTxData[0u] =  reading & SM_ADCVAL_LOWER_MASK;
	aTxData[1u] =  (reading & SM_ADCVAL_UPPER_MASK) >> SM_ADCVAL_SHIFT_VAL;
	HAL_CAN_AddTxMessage(&hcan, &zTxHeader, aTxData, &txMailBox);
}

//
// State10: LLIM High Event handling
//
static void sm_State10Handler(void)
{
	if (sm_IsStatusFlagged(SM_STATUS_LLIM_HIGH_FLAG))
	{
		HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_RESET);
		HAL_Delay(SM_PC_DELAY_TIME_1);
		HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_SET);
		HAL_Delay(SM_PC_DELAY_TIME_1);
		HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_SET);
		HAL_Delay(SM_PC_DELAY_TIME_2);
		HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_RESET);
		SM_ClearStatusFlag(SM_STATUS_LLIM_HIGH_FLAG);
	}
}

//
// state10 handler function: open all terminals and indicate fault in a forever loop
//
static void sm_State11Handler(void)
{
	if(sm_IsStatusFlagged(SM_STATUS_FLT_FLAG))
	{
		__disable_irq();
		HAL_GPIO_WritePin(GPIOB, SWAP_OUT_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, HLIM_OUT_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, NEG_OUT_Pin, GPIO_PIN_RESET);
		HAL_GPIO_WritePin(GPIOB, DCDC_OUT_Pin, GPIO_PIN_RESET);

		//toggle fault indicator led
		for(;;)
		{
			HAL_GPIO_WritePin(GPIOB, FLT_OUT_Pin, GPIO_PIN_SET);
			HAL_Delay(SM_STATE_DELAY_TIME);
			HAL_GPIO_WritePin(GPIOB, FLT_OUT_Pin, GPIO_PIN_RESET);
			HAL_Delay(SM_STATE_DELAY_TIME);
		}
	}
}

//
// fault-checking function: goes into fault based on flt_in and estop_in signal
//
static void sm_FltCheck(void)
{
	static uint8_t faultCounter1 = 0u;
	static uint8_t faultCounter2 = 0u;
	GPIO_PinState st1 = HAL_GPIO_ReadPin(GPIOA, FLT_IN_Pin);
	GPIO_PinState st2 =	HAL_GPIO_ReadPin(GPIOB, ESTOP_IN_Pin);

	//something's wrong when estop_in or flt_in is pulled to gnd
	if (st1 == GPIO_PIN_RESET)
	{
		//do some de-bouncing, give it three chances
		if(faultCounter1 >= SM_FLT_MAX_COUNTER_VAL)
		{
			SM_SetStatusFlag(SM_STATUS_FLT_FLAG);
		}
		else
		{
			faultCounter1++;
		}
	}
	else
	{
		faultCounter1 = 0u;
	}

	if (st2 == GPIO_PIN_RESET)
	{
		//do some de-bouncing, give it three chances
		if(faultCounter2 >= SM_FLT_MAX_COUNTER_VAL)
		{
			SM_SetStatusFlag(SM_STATUS_FLT_FLAG);
		}
		else
		{
			faultCounter2++;
		}
	}
	else
	{
		faultCounter2 = 0u;
	}
}

//
// Checks whether a status (FLT, HLIM High, LLIM High) has been flagged
//
static uint8_t sm_IsStatusFlagged(uint8_t flag)
{
	return (flag & sm_statusFlags);
}


//-------------------------------------------
// Global Function Implementations
//-------------------------------------------

//
// Initialize state machine module
//
void SM_Init(void)
{
	sm_stateVal = (uint8_t)SM_eState1;
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
	sm_pfaStateHandler[SM_eState11] = &sm_State11Handler;
}

//
// State machine update states
//
void SM_Update(void)
{

	//calling from function pointer array
	(void)sm_pfaStateHandler[sm_stateVal]();
	//fault checking
	sm_FltCheck();
	sm_stateVal++;

	//keep calling state handlers 9, 10, and 11
	if (SM_eStateMaxNum <= sm_stateVal)
	{
		sm_stateVal = SM_eState9;
	}

}

void SM_SetStatusFlag(uint8_t flag)
{
	sm_statusFlags |= flag;
}

void SM_ClearStatusFlag(uint8_t flag)
{
	sm_statusFlags &= ~flag;
}

