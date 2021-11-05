/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define SW_LOCK_OUT_Pin GPIO_PIN_2
#define SW_LOCK_OUT_GPIO_Port GPIOA
#define FAN_OUT1_Pin GPIO_PIN_3
#define FAN_OUT1_GPIO_Port GPIOA
#define HLIM_OUT_Pin GPIO_PIN_4
#define HLIM_OUT_GPIO_Port GPIOA
#define HLIM_IN_Pin GPIO_PIN_5
#define HLIM_IN_GPIO_Port GPIOA
#define HLIM_IN_EXTI_IRQn EXTI9_5_IRQn
#define LLIM_IN_Pin GPIO_PIN_6
#define LLIM_IN_GPIO_Port GPIOA
#define LLIM_IN_EXTI_IRQn EXTI9_5_IRQn
#define FLT_IN_Pin GPIO_PIN_7
#define FLT_IN_GPIO_Port GPIOA
#define FAN_OUT2_Pin GPIO_PIN_12
#define FAN_OUT2_GPIO_Port GPIOB
#define FLT_OUT_Pin GPIO_PIN_13
#define FLT_OUT_GPIO_Port GPIOB
#define LLIM_OUT_Pin GPIO_PIN_14
#define LLIM_OUT_GPIO_Port GPIOB
#define ESTOP_IN_Pin GPIO_PIN_15
#define ESTOP_IN_GPIO_Port GPIOB
#define PC_OUT_Pin GPIO_PIN_8
#define PC_OUT_GPIO_Port GPIOA
#define NEG_OUT_Pin GPIO_PIN_6
#define NEG_OUT_GPIO_Port GPIOB
#define DCDC_OUT_Pin GPIO_PIN_7
#define DCDC_OUT_GPIO_Port GPIOB
#define SWAP_OUT_Pin GPIO_PIN_8
#define SWAP_OUT_GPIO_Port GPIOB
#define SUPP_LOW_Pin GPIO_PIN_9
#define SUPP_LOW_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
