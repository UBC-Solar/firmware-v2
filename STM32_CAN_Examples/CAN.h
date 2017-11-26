/*----------------------------------------------------------------------------
 * Name:    CAN.h
 * Purpose: CAN interface for STM32
 * Version: V1.00
 *----------------------------------------------------------------------------
 * This file is part of the uVision/ARM development tools.
 * This software may only be used under the terms of a valid, current,
 * end user licence from KEIL for a compatible version of KEIL software
 * development tools. Nothing else gives you the right to use this software.
 *
 * Copyright (c) 2005-2007 Keil Software. All rights reserved.
 *----------------------------------------------------------------------------*/

#ifndef _CAN_H_
#define _CAN_H_

#define STANDARD_FORMAT  0
#define EXTENDED_FORMAT  1

#define DATA_FRAME       0
#define REMOTE_FRAME     1

typedef struct  {
  unsigned int   id;                 // 29 bit identifier
  unsigned char  data[8];            // Data field
  unsigned char  len;                // Length of data field in bytes
  unsigned char  format;             // 0 - STANDARD, 1- EXTENDED IDENTIFIER
  unsigned char  type;               // 0 - DATA FRAME, 1 - REMOTE FRAME
} CAN_msg;

/* Functions defined in module CAN.c */
void CAN_setup         (void);
void CAN_init          (void);
void CAN_start         (void);
void CAN_waitReady     (void);
void CAN_wrMsg         (CAN_msg *msg);
void CAN_rdMsg         (CAN_msg *msg);
void CAN_wrFilter      (unsigned int id, unsigned char filter_type);

void CAN_testmode      (unsigned int testmode);

extern CAN_msg       CAN_TxMsg;      // CAN messge for sending
extern CAN_msg       CAN_RxMsg;      // CAN message for receiving                                
extern unsigned int  CAN_TxRdy;      // CAN HW ready to transmit a message
extern unsigned int  CAN_RxRdy;      // CAN HW received a message

#endif // _CAN_H_


