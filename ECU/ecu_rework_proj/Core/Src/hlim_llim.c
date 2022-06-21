//-------------------------------------------
// File: hlim_llim.c
// Brief: HLIM and LLIM State Machines
// Authors: Yuqi Fu and Andrew Hanlon
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
#include "main.h"
#include "sm.h"

//-------------------------------------------
// Constant Definitions & Macros
//-------------------------------------------
#define TIM2_DEBOUNCE_TIME_MS (50u)
#define TIM2_PERIOD_TIME_MS (50u)
#define PC_FLAG_SET (1u)
#define PC_FLAG_RESET (0u)

//-------------------------------------------
// Local Typedefs
//-------------------------------------------
// Digital input debounce states
typedef enum digin_eDebounceState
{
    digin_MaybeDown = 0u,
    digin_Down,
    digin_MaybeUp,
    digin_Up,
} digin_eDebounceState;

//-------------------------------------------
// Local Variables
//-------------------------------------------

//-------------------------------------------
// Local function prototypes
//-------------------------------------------
static volatile uint8_t digin_LLIMIsHigh(void);
static volatile uint8_t digin_HLIMIsHigh(void);

//-------------------------------------------
// Global function definitions
//-------------------------------------------

void runLLIMStateMachine(void)
{
    static volatile digin_eDebounceState diginLLIMState = digin_MaybeDown;
    static volatile uint8_t diginLLIMDebounceTim = 0u;
    static volatile uint8_t pcFlag = PC_FLAG_RESET;

    volatile digin_eDebounceState diginLLIMNextState = diginLLIMState;

    switch (diginLLIMState)
    {
    case digin_MaybeDown:
        if (digin_LLIMIsHigh())
        {
            diginLLIMNextState = digin_MaybeUp;
        }
        else
        {
            if (diginLLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
            {
                diginLLIMDebounceTim += TIM2_PERIOD_TIME_MS;
            }
            else
            {
                diginLLIMNextState = digin_Down;
            }
        }
        break;

    case digin_Down:
        if (digin_LLIMIsHigh())
        {
            diginLLIMNextState = digin_MaybeUp;
        }
        else
        {
            pcFlag = PC_FLAG_SET;
            HAL_GPIO_WritePin(GPIOB, LLIM_OUT_Pin, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOA, PC_OUT_Pin, GPIO_PIN_RESET);
        }
        break;

    case digin_MaybeUp:
        if (digin_LLIMIsHigh())
        {
            if (diginLLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
            {
                diginLLIMDebounceTim += TIM2_PERIOD_TIME_MS;
            }
            else
            {
                diginLLIMNextState = digin_Up;
            }
        }
        else
        {
            diginLLIMNextState = digin_MaybeDown;
        }
        break;

    case digin_Up:
        if (digin_LLIMIsHigh())
        {
            if (pcFlag == PC_FLAG_SET)
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

    if (diginLLIMNextState != diginLLIMState)
    {
        diginLLIMDebounceTim = 0u;
        diginLLIMState = diginLLIMNextState;
    }
}

void runHLIMStateMachine(void)
{
    static volatile digin_eDebounceState diginHLIMState = digin_MaybeDown;
    static volatile uint8_t diginHLIMDebounceTim = 0u;

    volatile digin_eDebounceState diginHLIMNextState = diginHLIMState;

    switch (diginHLIMState)
    {
    case digin_MaybeDown:
        if (digin_HLIMIsHigh())
        {
            diginHLIMNextState = digin_MaybeUp;
        }
        else
        {
            if (diginHLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
            {
                diginHLIMDebounceTim += TIM2_PERIOD_TIME_MS;
            }
            else
            {
                diginHLIMNextState = digin_Down;
            }
        }
        break;

    case digin_Down:
        if (digin_HLIMIsHigh())
        {
            diginHLIMNextState = digin_MaybeUp;
        }
        else
        {
            HAL_GPIO_WritePin(GPIOA, HLIM_OUT_Pin, GPIO_PIN_RESET);
        }
        break;

    case digin_MaybeUp:
        if (digin_HLIMIsHigh())
        {
            if (diginHLIMDebounceTim < TIM2_DEBOUNCE_TIME_MS)
            {
                diginHLIMDebounceTim += TIM2_PERIOD_TIME_MS;
            }
            else
            {
                diginHLIMNextState = digin_Up;
            }
        }
        else
        {
            diginHLIMNextState = digin_MaybeDown;
        }
        break;

    case digin_Up:
        if (digin_HLIMIsHigh())
        {
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

    if (diginHLIMNextState != diginHLIMState)
    {
        diginHLIMDebounceTim = 0u;
        diginHLIMState = diginHLIMNextState;
    }
}

//-------------------------------------------
// Local function definitions
//-------------------------------------------
static volatile uint8_t digin_LLIMIsHigh(void)
{
    return (GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA, LLIM_IN_Pin));
}

static volatile uint8_t digin_HLIMIsHigh(void)
{
    return (GPIO_PIN_SET == HAL_GPIO_ReadPin(GPIOA, HLIM_IN_Pin));
}
