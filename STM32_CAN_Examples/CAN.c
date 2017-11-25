/*----------------------------------------------------------------------------
 * Name:    Can.c
 * Purpose: CAN interface for STM32
 * Version: V1.01
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
#include "CAN.h"                                  // STM32 CAN adaption layer

CAN_msg       CAN_TxMsg;                          // CAN messge for sending
CAN_msg       CAN_RxMsg;                          // CAN message for receiving                                

unsigned int  CAN_TxRdy = 0;                      // CAN HW ready to transmit a message
unsigned int  CAN_RxRdy = 0;                      // CAN HW received a message

/*----------------------------------------------------------------------------
  setup CAN interface
 *----------------------------------------------------------------------------*/
void CAN_setup (void)  {
  unsigned int brp = stm32_GetPCLK1();

  RCC->APB1ENR |= RCC_APB1ENR_CANEN;              // enable clock for CAN

                                                  // Note: MCBSTM32 uses PB8 and PB9 for CAN
  RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;             // enable clock for Alternate Function
  AFIO->MAPR   &= 0xFFFF9FFF;                     // reset CAN remap
  AFIO->MAPR   |= 0x00004000;                     //   set CAN remap, use PB8, PB9
												  
  RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;             // enable clock for GPIO B
  GPIOB->CRH &= ~(0x0F<<0);
  GPIOB->CRH |=  (0x08<<0);                       // CAN RX pin PB.8 input push pull 
  
  GPIOB->CRH &= ~(0x0F<<4);
  GPIOB->CRH |=  (0x0B<<4);                       // CAN TX pin PB.9 alternate output push pull 

  NVIC->ISER[0] |= (1 << (USB_HP_CAN_TX_IRQChannel  & 0x1F));// enable interrupt
  NVIC->ISER[0] |= (1 << (USB_LP_CAN_RX0_IRQChannel & 0x1F));// enable interrupt

  CAN->MCR = (CAN_MCR_NART | CAN_MCR_INRQ);       // init mode, disable auto. retransmission
                                                  // Note: only FIFO 0, transmit mailbox 0 used
  CAN->IER = (CAN_IER_FMPIE0 | CAN_IER_TMEIE);    // FIFO 0 msg pending, Transmit mbx empty

  /* Note: this calculations fit for PCLK1 = 36MHz */
  brp  = (brp / 18) / 500000;                     // baudrate is set to 500k bit/s
                                                                          
  /* set BTR register so that sample point is at about 72% bit time from bit start */
  /* TSEG1 = 12, TSEG2 = 5, SJW = 4 => 1 CAN bit = 18 TQ, sample at 72%    */
  CAN->BTR &= ~(((        0x03) << 24) | ((        0x07) << 20) | ((         0x0F) << 16) | (          0x1FF)); 
  CAN->BTR |=  ((((4-1) & 0x03) << 24) | (((5-1) & 0x07) << 20) | (((12-1) & 0x0F) << 16) | ((brp-1) & 0x1FF));
}


/*----------------------------------------------------------------------------
  leave initialisation mode
 *----------------------------------------------------------------------------*/
void CAN_start (void)  {

  CAN->MCR &= ~CAN_MCR_INRQ;                      // normal operating mode, reset INRQ
  while (CAN->MSR & CAN_MCR_INRQ);

}

/*----------------------------------------------------------------------------
  set the testmode
 *----------------------------------------------------------------------------*/
void CAN_testmode (unsigned int testmode) {

  CAN->BTR &= ~(CAN_BTR_SILM | CAN_BTR_LBKM);     // set testmode
  CAN->BTR |=  (testmode & (CAN_BTR_SILM | CAN_BTR_LBKM));
}

/*----------------------------------------------------------------------------
  check if transmit mailbox is empty
 *----------------------------------------------------------------------------*/
void CAN_waitReady (void)  {

  while ((CAN->TSR & CAN_TSR_TME0) == 0);         // Transmit mailbox 0 is empty
  CAN_TxRdy = 1;
 
}

/*----------------------------------------------------------------------------
  wite a message to CAN peripheral and transmit it
 *----------------------------------------------------------------------------*/
void CAN_wrMsg (CAN_msg *msg)  {

  CAN->sTxMailBox[0].TIR  = (unsigned int)0;      // Reset TIR register
                                                  // Setup identifier information
  if (msg->format == STANDARD_FORMAT)  {          //    Standard ID
      CAN->sTxMailBox[0].TIR |= (unsigned int)(msg->id << 21) | CAN_ID_STD;
  }  else  {                                      // Extended ID
      CAN->sTxMailBox[0].TIR |= (unsigned int)(msg->id <<  3) | CAN_ID_EXT;
  }
                                                  // Setup type information
  if (msg->type == DATA_FRAME)  {                 // DATA FRAME
      CAN->sTxMailBox[0].TIR |= CAN_RTR_DATA;
  }
  else {                                          // REMOTE FRAME
      CAN->sTxMailBox[0].TIR |= CAN_RTR_REMOTE;
  }
                                                  // Setup data bytes
  CAN->sTxMailBox[0].TDLR = (((unsigned int)msg->data[3] << 24) | 
                             ((unsigned int)msg->data[2] << 16) |
                             ((unsigned int)msg->data[1] <<  8) | 
                             ((unsigned int)msg->data[0])        );
  CAN->sTxMailBox[0].TDHR = (((unsigned int)msg->data[7] << 24) | 
                             ((unsigned int)msg->data[6] << 16) |
                             ((unsigned int)msg->data[5] <<  8) |
                             ((unsigned int)msg->data[4])        );
                                                  // Setup length
  CAN->sTxMailBox[0].TDTR &= ~CAN_TDTxR_DLC;
  CAN->sTxMailBox[0].TDTR |=  (msg->len & CAN_TDTxR_DLC);

  CAN->IER |= CAN_IER_TMEIE;                      // enable  TME interrupt 
  CAN->sTxMailBox[0].TIR |=  CAN_TIxR_TXRQ;       // transmit message
}

/*----------------------------------------------------------------------------
  read a message from CAN peripheral and release it
 *----------------------------------------------------------------------------*/
void CAN_rdMsg (CAN_msg *msg)  {
                                                  // Read identifier information
  if ((CAN->sFIFOMailBox[0].RIR & CAN_ID_EXT) == 0) { // Standard ID
    msg->format = STANDARD_FORMAT;
    msg->id     = (u32)0x000007FF & (CAN->sFIFOMailBox[0].RIR >> 21);
  }  else  {                                          // Extended ID
    msg->format = EXTENDED_FORMAT;
    msg->id     = (u32)0x0003FFFF & (CAN->sFIFOMailBox[0].RIR >> 3);
  }
                                                  // Read type information
  if ((CAN->sFIFOMailBox[0].RIR & CAN_RTR_REMOTE) == 0) {
    msg->type =   DATA_FRAME;                     // DATA   FRAME
  }  else  {
    msg->type = REMOTE_FRAME;                     // REMOTE FRAME
  }
                                                  // Read length (number of received bytes)
  msg->len = (unsigned char)0x0000000F & CAN->sFIFOMailBox[0].RDTR;
                                                  // Read data bytes
  msg->data[0] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR);
  msg->data[1] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 8);
  msg->data[2] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 16);
  msg->data[3] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDLR >> 24);

  msg->data[4] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR);
  msg->data[5] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 8);
  msg->data[6] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 16);
  msg->data[7] = (unsigned int)0x000000FF & (CAN->sFIFOMailBox[0].RDHR >> 24);

  CAN->RF0R |= CAN_RF0R_RFOM0;                    // Release FIFO 0 output mailbox
}


void CAN_wrFilter (unsigned int id, unsigned char format)  {
  static unsigned short CAN_filterIdx = 0;
         unsigned int   CAN_msgId     = 0;
  
  if (CAN_filterIdx > 13) {                       // check if Filter Memory is full
    return;
  }
                                                  // Setup identifier information
  if (format == STANDARD_FORMAT)  {               // Standard ID
      CAN_msgId  |= (unsigned int)(id << 21) | CAN_ID_STD;
  }  else  {                                      // Extended ID
      CAN_msgId  |= (unsigned int)(id <<  3) | CAN_ID_EXT;
  }

  CAN->FMR  |=  CAN_FMR_FINIT;                    // set Initialisation mode for filter banks
  CAN->FA1R &=  ~(unsigned int)(1 << CAN_filterIdx); // deactivate filter

                                                  // initialize filter   
  CAN->FS1R |= (unsigned int)(1 << CAN_filterIdx);// set 32-bit scale configuration
  CAN->FM1R |= (unsigned int)(1 << CAN_filterIdx);// set 2 32-bit identifier list mode

  CAN->sFilterRegister[CAN_filterIdx].FR1 = CAN_msgId; //  32-bit identifier
  CAN->sFilterRegister[CAN_filterIdx].FR2 = CAN_msgId; //  32-bit identifier
    													   
  CAN->FFA1R &= ~(unsigned int)(1 << CAN_filterIdx);  // assign filter to FIFO 0
  CAN->FA1R  |=  (unsigned int)(1 << CAN_filterIdx);  // activate filter

  CAN->FMR &= ~CAN_FMR_FINIT;                     // reset Initialisation mode for filter banks

  CAN_filterIdx += 1;                             // increase filter index
}

/*----------------------------------------------------------------------------
  CAN transmit interrupt handler
 *----------------------------------------------------------------------------*/
void USB_HP_CAN_TX_IRQHandler (void) {

  if (CAN->TSR & CAN_TSR_RQCP0) {                 // request completed mbx 0
    CAN->TSR |= CAN_TSR_RQCP0;                    // reset request complete mbx 0
    CAN->IER &= ~CAN_IER_TMEIE;                   // disable  TME interrupt
	
	CAN_TxRdy = 1; 
  }
}

/*----------------------------------------------------------------------------
  CAN receive interrupt handler
 *----------------------------------------------------------------------------*/
void USB_LP_CAN_RX0_IRQHandler (void) {

  if (CAN->RF0R & CAN_RF0R_FMP0) {			      // message pending ?
	CAN_rdMsg (&CAN_RxMsg);                       // read the message

    CAN_RxRdy = 1;                                // set receive flag
  }
}


