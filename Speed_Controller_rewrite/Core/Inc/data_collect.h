#ifndef __DATA_COLLECT_H
#define __DATA_COLLECT_H

#include "stm32f1xx_hal.h"

#define DATA_COLLECT_NUM_ADCS 1
#define DATA_COLLECT_TOTAL_NUM_ANALOG_CHANNELS 1

#define ADC1_RESULTS_STORED 0x1
// #define ADC2_RESULTS_STORED 0x2

void DataCollect_PrepareHardware(ADC_HandleTypeDef *hadc1);
void DataCollect_Start(TIM_HandleTypeDef *triggerTimerHandle);
void DataCollect_Stop(TIM_HandleTypeDef *triggerTimerHandle);
int  DataCollect_Poll(void);
void DataCollect_Get(uint32_t buffer[DATA_COLLECT_TOTAL_NUM_ANALOG_CHANNELS]);

#endif // __DATA_COLLECT_H
