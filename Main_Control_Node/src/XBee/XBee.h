#include "stm32f10x.h"

#define CAN_MSG_SIZE (uint8_t) 17;
#define CAN_DATA_SIZE 8;

typedef struct
{
	uint8_t 	id;
	uint8_t 	data[8];
	uint8_t 	len;	
} can_msg_t;


void uart_init(void);
void RTCinit(void);
void XBeeTransmitCan(can_msg_t* can_tx_msg);
void XBeeSendChar(char c);
void XBeeSendByte(char c);
void speedTest(void);

