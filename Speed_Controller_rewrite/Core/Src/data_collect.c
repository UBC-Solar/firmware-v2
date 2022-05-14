#include "data_collect.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>

// =============================================================================
// Private Defines and Type Definitions
// =============================================================================
#define NUM_ADCS DATA_COLLECT_NUM_ADCS // Declared for convenience
#define BUF_LENGTH_PER_CHANNEL 200

#define ADC1_NUM_ANALOG_CHANNELS 2
#define ADC1_BUF_LENGTH (BUF_LENGTH_PER_CHANNEL * ADC1_NUM_ANALOG_CHANNELS)
// #define ADC2_NUM_ANALOG_CHANNELS 0
// #define ADC2_BUF_LENGTH (BUF_LENGTH_PER_CHANNEL * ADC2_NUM_ANALOG_CHANNELS)

#define MAX_NUM_ANALOG_CHANNELS 2

typedef enum {
    FIRST_HALF = 0,
    SECOND_HALF = 1
} TransferHalf_t;

typedef enum {
    ADC1_ID = 0,
    ADC2_ID = 1,
    ADC_ID_MAX
} AdcId_t;

// =============================================================================
// Global Variables
// =============================================================================

ADC_HandleTypeDef *adcHandles[NUM_ADCS];

// Circular buffers for DMA transfers from ADCs
static volatile uint16_t adc1_buf[ADC1_BUF_LENGTH] = {0};
// static volatile uint16_t adc2_buf[ADC2_BUF_LENGTH] = {0};

static volatile uint32_t storedResults[DATA_COLLECT_TOTAL_NUM_ANALOG_CHANNELS] = {0};
static volatile uint32_t storedFlags = 0;

// =============================================================================
// Private Function Prototypes
// =============================================================================
static AdcId_t GetAdcId(ADC_HandleTypeDef *hadc);
static void ProcessReadings(TransferHalf_t half, volatile uint16_t *adc_buf,
                            uint32_t results[MAX_NUM_ANALOG_CHANNELS], uint32_t num_analog_channels);
static void StoreResults(uint32_t results[], AdcId_t adcId);

// =============================================================================
// Public Function Definitions
// =============================================================================

void DataCollect_PrepareHardware(ADC_HandleTypeDef *hadc1/*, ADC_HandleTypeDef *hadc2*/)
{
    adcHandles[0] = hadc1;
    // adcHandles[1] = hadc2;

    if (HAL_ADC_Start_DMA(hadc1, (uint32_t *) adc1_buf, ADC1_BUF_LENGTH) != HAL_OK)
    {
        Error_Handler();
    }
    // if (HAL_ADC_Start_DMA(hadc2, (uint32_t *) adc2_buf, ADC2_BUF_LENGTH) != HAL_OK)
    // {
    //     Error_Handler();
    // }

    storedFlags = 0;
}

void DataCollect_Start(TIM_HandleTypeDef *triggerTimerHandle)
{
    HAL_TIM_Base_Start(triggerTimerHandle);
}

void DataCollect_Stop(TIM_HandleTypeDef *triggerTimerHandle)
{
    HAL_TIM_Base_Stop(triggerTimerHandle);
}

int DataCollect_Poll(void)
{
    return storedFlags;
}

void DataCollect_Get(uint32_t buffer[DATA_COLLECT_TOTAL_NUM_ANALOG_CHANNELS])
{
    __disable_irq();
    for(int i = 0; i < DATA_COLLECT_TOTAL_NUM_ANALOG_CHANNELS; i++)
    {
        buffer[i] = storedResults[i];
    }

    storedFlags = 0;
    __enable_irq();
}

// =============================================================================
// Interrupt Callback Definitions
// =============================================================================

// Conversion half complete DMA interrupt callback
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc)
{
    uint32_t results[MAX_NUM_ANALOG_CHANNELS];
    AdcId_t adcId = GetAdcId(hadc);

    switch (adcId)
    {
        case ADC1_ID:
            ProcessReadings(FIRST_HALF, adc1_buf, results, ADC1_NUM_ANALOG_CHANNELS);
            break;
        // case ADC2_ID:
        //     ProcessReadings(FIRST_HALF, adc2_buf, results, ADC2_NUM_ANALOG_CHANNELS);
        //     break;
        default:
            Error_Handler();
    }

    StoreResults(results, adcId);
}

// Conversion complete DMA interrupt callback
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc)
{
    uint32_t results[MAX_NUM_ANALOG_CHANNELS];
    AdcId_t adcId = GetAdcId(hadc);

    switch (adcId)
    {
        case ADC1_ID:
            ProcessReadings(SECOND_HALF, adc1_buf, results, ADC1_NUM_ANALOG_CHANNELS);
            break;
        // case ADC2_ID:
        //     ProcessReadings(SECOND_HALF, adc2_buf, results, ADC2_NUM_ANALOG_CHANNELS);
        //     break;
        default:
            Error_Handler();
    }

    StoreResults(results, adcId);
}

void HAL_ADC_ErrorCallback(ADC_HandleTypeDef *hadc)
{
    printf("\nDMA TRANSFER ERROR\n");
    Error_Handler();
}

// =============================================================================
// Private Function Definitions
// =============================================================================

static AdcId_t GetAdcId(ADC_HandleTypeDef *hadc)
{
    if (hadc == adcHandles[0])
        return ADC1_ID;
    // if (hadc == adcHandles[1])
    //     return ADC2_ID;

    return ADC_ID_MAX;
}

static void ProcessReadings(TransferHalf_t half, volatile uint16_t *adc_buf,
                            uint32_t results[MAX_NUM_ANALOG_CHANNELS], uint32_t num_analog_channels)
{
    uint32_t sum[MAX_NUM_ANALOG_CHANNELS] = {0};
    uint32_t sample_num;
    uint32_t sample_start_index;
    uint32_t limit;

    if (half == FIRST_HALF)
    {
        sample_num = 0;
        limit = BUF_LENGTH_PER_CHANNEL / 2;
    }
    else
    {
        sample_num = BUF_LENGTH_PER_CHANNEL / 2;
        limit = BUF_LENGTH_PER_CHANNEL;
    }

    // Sum the samples
    for (; sample_num < limit; sample_num++)
    {
        sample_start_index = sample_num * num_analog_channels;
        for (int channel = 0; channel < num_analog_channels; channel++)
        {
            sum[channel] += adc_buf[sample_start_index + channel];
        }
    }

    for (int channel = 0; channel < num_analog_channels; channel++)
    {
        // Uncomment if average should be taken instead
        // results[channel] = ((float)sum[channel]) / (BUF_LENGTH_PER_CHANNEL / 2);

        results[channel] = sum[channel];
    }
}

static void StoreResults(uint32_t results[], AdcId_t adcId)
{
    // Mapping of inputs to thermistor indices is defined here
    switch (adcId)
    {
        case ADC1_ID:
            storedResults[0] = results[0];
            storedFlags |= ADC1_RESULTS_STORED;
            break;
        // case ADC2_ID:
        //     storedResults[4] = results[0];
        //     storedResults[5] = results[1];
        //     storedFlags |= ADC2_RESULTS_STORED;
        //     break;
        default:
            Error_Handler();
            break;
    }
}
