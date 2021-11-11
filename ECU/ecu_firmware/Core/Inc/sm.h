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

//-------------------------------------------
// Other Lib Include Files
//-------------------------------------------

//-------------------------------------------
// Constant Definitions & Macros
//-------------------------------------------

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
	SM_eStateMaxNum
} SM_eState;



//-------------------------------------------
// Global Function Prototypes
//-------------------------------------------

extern void SM_Init(void);
extern void SM_Update(void);
extern SM_eState SM_GetCurrentState(void);

#endif /* INC_SM_H_ */
