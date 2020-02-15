#include "stm32f10x.h"
#include "CAN.h"
#include "virtual_com.h"
#include "XBee.h"
#include "RTC.h"

int main(void)
{
	CANInit(CAN_500KBPS);
	XBeeInit();
	RTCinit();
	VirtualComInit();
	
	//CAN setup
	CAN_msg_t CAN_rx_msg;
	
	while(1)
	{
		if(CANMsgAvail())
		{
			CANReceive(&CAN_rx_msg);
			XBeeTransmitCan(&CAN_rx_msg);
			SendString("CAN");
			SendLine();
		}
	}
	
}