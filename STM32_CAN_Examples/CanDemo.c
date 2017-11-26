/*----------------------------------------------------------------------------
 * Name:    CanDemo.c
 * Purpose: CAN example for STM32
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/

#include <stm32f10x_lib.h>                        // STM32F10x Library Definitions
#include "STM32_Reg.h"                            // STM32 register and bit Definitions
#include "STM32_Init.h"                           // STM32 Initialization
#include "LCD.h"                                  // LCD function prototypes
#include "CAN.h"                                  // STM32 CAN adaption layer


unsigned int val_Tx = 0, val_Rx = 0;              // Global variables used for display

char hex_chars[16] = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'};


/*------------------------------------------------------------------------------
  convert one byte to string in hexadecimal notation
 *------------------------------------------------------------------------------*/
void Hex_Str (unsigned char hex, char *str) {
  *str++ = '0';
  *str++ = 'x';
  *str++ = hex_chars[hex >>  4];
  *str++ = hex_chars[hex & 0xF];
}


/*----------------------------------------------------------------------------
  insert a delay time.
 *----------------------------------------------------------------------------*/
void delay(unsigned int nCount)	{

  for(; nCount != 0; nCount--);
}


/*------------------------------------------------------------------------------
  Initialises the Analog/Digital converter
  PA1 (ADC Channel1) is used as analog input
 *------------------------------------------------------------------------------*/
void adc_Init (void) {

  RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;             // enable periperal clock for ADC1

  ADC1->SQR1  = 0x00000000;                       // only one conversion
  ADC1->SMPR2 = 0x00000028;                       // set sample time channel1 (55,5 cycles)
  ADC1->SQR3  = 0x00000001;                       // set channel1 as 1st conversion

  ADC1->CR1   = 0x00000100;                       // use independant mode, SCAN mode
  ADC1->CR2   = 0x000E0003;                       // data align right, cont conversion
                                                  // EXTSEL = SWSTART
                                                  // enable ADC, no external Trigger
  ADC1->CR2  |=  0x00500000;                      // start SW conversion
}

/*------------------------------------------------------------------------------
  read a converted value from the Analog/Digital converter
 *------------------------------------------------------------------------------*/
unsigned char adc_Get (void) {
  unsigned short val;

  while(!(ADC1->SR & ADC_SR_EOC));                // whait til end of conversion
  val = ADC1->DR & ADC_DR_DATA;                   // Get conversion result
  val = (val >> 4) & 0xFF;                        // Scale analog value to 8 bits
	
  return (val);
}

/*----------------------------------------------------------------------------
  display transmit and receieve values
 *---------------------------------------------------------------------------*/
void val_display (void) {
  static char disp_buf[] = "Tx:    , Rx:    ";    // display string

  Hex_Str(val_Tx, &disp_buf[ 3]);                 // display Tx and Rx values to LCD disp
  Hex_Str(val_Rx, &disp_buf[12]);
  set_cursor (0, 1);
  lcd_print  ((char *)disp_buf);                  // print string to LCD

  delay (1000000);                                // Wait wait a while
}


/*----------------------------------------------------------------------------
  initialize CAN interface
 *----------------------------------------------------------------------------*/
void can_Init (void) {

  CAN_setup ();                                   // setup CAN interface
  CAN_wrFilter (33, STANDARD_FORMAT);             // Enable reception of messages

  /* COMMENT THE LINE BELOW TO ENABLE DEVICE TO PARTICIPATE IN CAN NETWORK   */
  CAN_testmode(CAN_BTR_SILM | CAN_BTR_LBKM);      // Loopback, Silent Mode (self-test)

  CAN_start ();                                   // leave init mode

  CAN_waitReady ();                               // wait til mbx is empty
}



/*----------------------------------------------------------------------------
  MAIN function
 *----------------------------------------------------------------------------*/
int main (void)  {
  int i;

  stm32_Init ();                                  // STM32 setup
  adc_Init ();                                    // initialise A/D converter
  can_Init ();                                    // initialise CAN interface

  lcd_init  ();                                   // initialise LCD
  lcd_clear ();
  lcd_print ("MCBSTM32 CanDemo");                 // Display string on LCD display
  set_cursor (0, 1);                              // Set cursor position on LCD display
  lcd_print ("  www.keil.com  ");
  delay (4500000);                                // Wait for initial display (~5s)

  lcd_clear ();
  lcd_print ("CAN at 500kbit/s");

  CAN_TxMsg.id = 33;                              // initialise message to send
  for (i = 0; i < 8; i++) CAN_TxMsg.data[i] = 0;
  CAN_TxMsg.len = 1;
  CAN_TxMsg.format = STANDARD_FORMAT;
  CAN_TxMsg.type = DATA_FRAME;

  while (1) {                                     // Loop forever
    if (CAN_TxRdy) {
      CAN_TxRdy = 0;

      CAN_TxMsg.data[0] = adc_Get ();             // data[0] field = ADC value
      CAN_wrMsg (&CAN_TxMsg);                     // transmit message
      val_Tx = CAN_TxMsg.data[0];
    }

    delay (10000);                                // Wait a while to receive the message

    if (CAN_RxRdy) {
      CAN_RxRdy = 0;

      val_Rx = CAN_RxMsg.data[0];
    }

    val_display ();                               // display TX and RX values

  } // end while
} // end main

