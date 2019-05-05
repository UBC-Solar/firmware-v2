#include "stm32f10x.h"
#include "CAN.h"

#define CAN_MSG_SIZE (uint8_t) 17;
#define CAN_DATA_SIZE 8;

/**
 * Initializes the STM32 UART on GPIO B10 and B11 to a baudrate of 9600
 */
void XBeeInit(void);
/**
 * Transmits a CAN message via the previously initialized STM32 UART
 * Passes: An instance of the can_msg_t data type
 */
void XBeeTransmitCan(CAN_msg_t* can_tx_msg);
/**
 * Outputs a character in 2 bytes according to its ASCII value
 * For example, 'A' is referenced in ASCII as 0x41, so 4 and 1 are sent
 * as individual bytes.
 * 'z' is referenced in ASCII as 0x7A, so 7 and A are sent in individual
 * bytes
 */
void XBeeSendChar(char c);
/**
 * Outputs the raw character written here
 * 	If the parameter is 'A', output to console 'A'
 * 
 * Passes: a character (1 byte) that will appear on a serial monitor
 */
void XBeeSendByte(char c);
