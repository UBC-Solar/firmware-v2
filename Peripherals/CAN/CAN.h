
/**
 * This is the header file for the CAN driver.
 *
 * The CAN controller configured will have no ID filters, and the
 * bit rate is set to 400KBS.
 *
 * Polling will be required by the user, since the functions executed when CAN
 * messages are received are too complex and long, and will not be suitable to
 * be placed in a interrupt handler (the alternative is to have the interrupt
 * set a valid bit and poll that bit in the main loop. Unfortunately, clearing
 * the interrupt mask means setting the pending number of CAN messages to 0,
 * which means, depending on the rate of messages being received, some messages
 * will be dropped.)
 *
 * Note: Please define the following fields in the main file
 * (or any file that will include this header file):
 * - CAN_msg_t CAN_rx_msg
 * - CAN_msg_t CAN_tx_msg
 */

#ifndef CAN_H
#define CAN_H

#include "stm32f103xb.h"

enum BITRATE
{
	CAN_50KBPS,
	CAN_100KBPS,
	CAN_125KBPS,
	CAN_250KBPS,
	CAN_500KBPS,
	CAN_1000KBPS
};

typedef struct
{
	uint16_t id;
	uint8_t data[8];
	uint8_t len;
} CAN_msg_t;

typedef struct
{
	uint8_t TS2; // Time segment 1
	uint8_t TS1; // Time segment 2
	uint8_t BRP; // Baud rate prescaler
} CAN_bit_timing_config_t;

void CANInit(enum BITRATE bitrate);
void CANReceive(CAN_msg_t *CAN_rx_msg);
void CANSend(CAN_msg_t *CAN_tx_msg);
int CANSetFilter(uint16_t id);
int CANSetFilters(uint16_t *ids, uint8_t num);
uint8_t CANMsgAvail(void);

#endif /* CAN_H */
