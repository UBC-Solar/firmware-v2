//-------------------------------------------
//File: sm.h
//Brief: Header for state machine module
//Author: Yuqi Fu
//-------------------------------------------

#ifndef INC_SM_H_
#define INC_SM_H_


//-------------------------------------------
// Copyright 2021 UBC Solar, all rights reserved
//-------------------------------------------

//-------------------------------------------
// Standard Lib Include Files
//-------------------------------------------
#include <stdint.h>
//-------------------------------------------
// Other Lib Include Files
//-------------------------------------------

//-------------------------------------------
// Constant Definitions & Macros
//-------------------------------------------
#define SM_STATUS_FLT_FLAG					((uint8_t)(1u << 0u))
#define SM_STATUS_HLIM_HIGH_FLAG			((uint8_t)(1u << 1u))
#define SM_STATUS_LLIM_HIGH_FLAG			((uint8_t)(1u << 2u))
//-------------------------------------------
// Local Variables
//-------------------------------------------

//-------------------------------------------
// Global Variables
//-------------------------------------------
typedef enum
{
	SM_eState1 = 0u,
	SM_eState2,
	SM_eState3,
	SM_eState4,
	SM_eState5,
	SM_eState6,
	SM_eState7,
	SM_eState8,
	SM_eState9,
	SM_eState10,
	SM_eState11,
	SM_eStateMaxNum
} SM_eState;



//-------------------------------------------
// Global Function Prototypes
//-------------------------------------------

extern void SM_Init(void);
extern void SM_Update(void);
extern void SM_SetStatusFlag(uint8_t flag);
extern void SM_ClearStatusFlag(uint8_t flag);

#endif /* INC_SM_H_ */
