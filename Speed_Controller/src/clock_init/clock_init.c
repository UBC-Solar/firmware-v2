#include "clock_init.h"

#define HSE_STARTUP_TIMEOUT 16000 // cycles for roughly 2ms? (0.002s * 8000000 Hz)

/**
 * Sets System clock frequency to 24MHz and configure HCLK, PCLK2
 * and PCLK1 prescalers.
 * 
 * This function should be used only after reset.
 */
void SetSysClockTo36Hsi(void)
{
    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/

    /* Flash wait state */
    // Must be set depending on the system clock frequency
    // 0 wait states - 0 < SYSCLK <= 24 MHz (default)
    // 1 wait state  - 24 < SYSCLK <= 48 MHz
    // 2 wait states - 48 < SYSCLK <= 72 MHz
    FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
    FLASH->ACR |= (uint32_t)(0x1 << FLASH_ACR_LATENCY_Pos);

    /* HCLK = SYSCLK (HCLK is for AHB domain; 72MHz max) */
    RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

    /* PCLK2 = HCLK (PCLK2 is for APB2 domain; 72MHz max) */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;

    /* PCLK1 = HCLK (PCLK1 is for APB1 domain; 36MHz max) */
    RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;

    /*  PLL configuration: PLLCLK = HSI / 2 * 9 = 36 MHz */
    RCC->CFGR &= (uint32_t)((uint32_t) ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                                         RCC_CFGR_PLLMULL));
    RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLMULL9);

    /* Enable PLL */
    RCC->CR |= RCC_CR_PLLON;

    /* Wait till PLL is ready */
    while ((RCC->CR & RCC_CR_PLLRDY) == 0);

    /* Select PLL as system clock source */
    RCC->CFGR &= (uint32_t)((uint32_t) ~(RCC_CFGR_SW));
    RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

    /* Wait till PLL is used as system clock source */
    while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL);

    SystemCoreClockUpdate();
}

/**
 * Sets System clock frequency to 36MHz and configure HCLK, PCLK2
 * and PCLK1 prescalers.
 * 
 * This function should be used only after reset, and requires an external clock signal.
 */
void SetSysClockTo36HseBypass(void)
{
    __IO uint32_t StartUpCounter = 0, HSEStatus = 0;

    /* SYSCLK, HCLK, PCLK2 and PCLK1 configuration ---------------------------*/
    /* Enable HSE in BYPASS mode (no crystal, oscillator bypassed by an external clock signal) */
    /* On a Nucleo board, this external clock signal comes from the built-in ST Link */
    RCC->CR |= (uint32_t)(RCC_CR_HSEBYP | RCC_CR_HSEON);

    /* Wait till HSE is ready and if Time out is reached exit */
    do
    {
        HSEStatus = RCC->CR & RCC_CR_HSERDY;
        StartUpCounter++;
    } while ((HSEStatus == 0) && (StartUpCounter != HSE_STARTUP_TIMEOUT));

    if ((RCC->CR & RCC_CR_HSERDY) != 0)
    {
        HSEStatus = (uint32_t)0x01;
    }
    else
    {
        HSEStatus = (uint32_t)0x00;
    }

    if (HSEStatus == (uint32_t)0x01)
    {
        /* Flash wait state */
        // Must be set depending on the system clock frequency
        // 0 wait states - 0 < SYSCLK <= 24 MHz (default)
        // 1 wait state  - 24 < SYSCLK <= 48 MHz
        // 2 wait states - 48 < SYSCLK <= 72 MHz
        FLASH->ACR &= (uint32_t)((uint32_t)~FLASH_ACR_LATENCY);
        FLASH->ACR |= (uint32_t)(0x1 << FLASH_ACR_LATENCY_Pos);

        /* HCLK = SYSCLK (HCLK is for AHB domain; 72MHz max) */
        RCC->CFGR |= (uint32_t)RCC_CFGR_HPRE_DIV1;

        /* PCLK2 = HCLK (PCLK2 is for APB2 domain; 72MHz max) */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE2_DIV1;

        /* PCLK1 = HCLK (PCLK1 is for APB1 domain; 36MHz max) */
        RCC->CFGR |= (uint32_t)RCC_CFGR_PPRE1_DIV1;

#ifdef STM32F10X_CL
        /* Configure PLLs ------------------------------------------------------*/
        /* PLL2 configuration: PLL2CLK = (HSE / 5) * 8 = 40 MHz */
        /* PREDIV1 configuration: PREDIV1CLK = PLL2 / 5 = 8 MHz */

        RCC->CFGR2 &= (uint32_t) ~(RCC_CFGR2_PREDIV2 | RCC_CFGR2_PLL2MUL |
                                   RCC_CFGR2_PREDIV1 | RCC_CFGR2_PREDIV1SRC);
        RCC->CFGR2 |= (uint32_t)(RCC_CFGR2_PREDIV2_DIV5 | RCC_CFGR2_PLL2MUL8 |
                                 RCC_CFGR2_PREDIV1SRC_PLL2 | RCC_CFGR2_PREDIV1_DIV5);

        /* Enable PLL2 */
        RCC->CR |= RCC_CR_PLL2ON;
        /* Wait till PLL2 is ready */
        while ((RCC->CR & RCC_CR_PLL2RDY) == 0)
        {
        }

        /* PLL configuration: PLLCLK = PREDIV1 * 9 = 72 MHz */
        RCC->CFGR &= (uint32_t) ~(RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL);
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE_PREDIV1 | RCC_CFGR_PLLSRC_PREDIV1 |
                                RCC_CFGR_PLLMULL9);
#else
        /*  PLL configuration: PLLCLK = HSE / 2 * 9 = 36 MHz */
        RCC->CFGR &= (uint32_t)((uint32_t) ~(RCC_CFGR_PLLSRC | RCC_CFGR_PLLXTPRE |
                                             RCC_CFGR_PLLMULL));
        RCC->CFGR |= (uint32_t)(RCC_CFGR_PLLXTPRE | RCC_CFGR_PLLSRC | RCC_CFGR_PLLMULL9);
#endif /* STM32F10X_CL */

        /* Enable PLL */
        RCC->CR |= RCC_CR_PLLON;

        /* Wait till PLL is ready */
        while ((RCC->CR & RCC_CR_PLLRDY) == 0);

        /* Select PLL as system clock source */
        RCC->CFGR &= (uint32_t)((uint32_t) ~(RCC_CFGR_SW));
        RCC->CFGR |= (uint32_t)RCC_CFGR_SW_PLL;

        /* Wait till PLL is used as system clock source */
        while ((RCC->CFGR & (uint32_t)RCC_CFGR_SWS) != (uint32_t)RCC_CFGR_SWS_PLL);
    }
    else
    { /* If HSE fails to start-up, the application will have wrong clock
         configuration. User can add here some code to deal with this error */
    }

    SystemCoreClockUpdate();
}
