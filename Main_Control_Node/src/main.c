#include "stm32f10x.h"

#include "LCD/LCD.h"
#include "CAN.h"
#include "XBee/XBee.h"
#include "virtual_com.h"

#define TRUE 1
#define FALSE 0

#define MC_BASE 0x200
#define ARR_BASE 0x700

/**
 * Initialize Dashboard LED lights
 */
void InitLEDs(void)
{
	
	RCC->APB2ENR |= 0x1UL << 2; 		//Initialize clock for GPIOA, if it hasn't been initialized yet
	GPIOA->CRL &= 0;
	GPIOA->CRH &= 0;
	GPIOA->CRL |= 0x33330030UL;			//Set pins A1, A4, A5, A6, A7 to be Push-Pull Output, 50Mhz
	GPIOA->CRH |= 0x00003333UL;			//SetBar pins A8, A9, A10 to be Push-Pull Output, 50Mhz
	GPIOA->BSRR = 0xFFFF;
	GPIOA->BRR = 0x1 << 5;
	
}

/**
 * Main procedure
 */
int main(void)
{
	
	float tempFloat;
	int32_t tempInt32;
	CAN_msg_t newCanMsg;
	uint8_t c;
	
	InitialiseLCDPins();
	CANInit();
	ScreenSetup();
	InitLEDs();
	VirtualComInit();
	XBeeInit();
	
	while(1)
	{
		//If a message can be received,		
		if (CANMsgAvail())
		{
			
			CANReceive(&CAN_rx_msg);		//Receive the msg currently in buffer
			//Check the CAN STID against several known IDs
			//Several need to be parsed, especially the ones designated for LCD and dashboard
			
			switch(CAN_rx_msg.id)
			{
				
				//Battery: Pack Voltage(For LCD Display)
				case 0x623:
					UpdateScreenParameter(XPOS_0, YPOS_3, 0xFFFF & (CAN_rx_msg.data[1] | CAN_rx_msg.data[0] << 8), 0);
									
					newCanMsg.id = 0x623;
					for (c = 0; c < 8; c++)
					{
						newCanMsg.data[c] = CAN_rx_msg.data[c];
					}
					newCanMsg.len = 8;
					XBeeTransmitCan(&newCanMsg);								
					break;
				
				//Battery: Pack Current(For LCD Display)
				case 0x624:
					
					UpdateScreenParameter(XPOS_0, YPOS_0, 0xFFFF & (CAN_rx_msg.data[1] | CAN_rx_msg.data[0] << 8), 0);
					
					newCanMsg.id = 0x624;					
					for (c = 0; c < 8; c++)
					{
						newCanMsg.data[c] = CAN_rx_msg.data[c];
					}
					newCanMsg.len = 8;
					XBeeTransmitCan(&newCanMsg);							
					break;
				
				//Battery: Pack Maximum Temperature (For LCD Display)
				case 0x627:
					UpdateScreenParameter(XPOS_20, YPOS_6, 0xFFFF & CAN_rx_msg.data[4], 0);
				
					newCanMsg.id = 0x627;					
					for (c = 0; c < 8; c++)
					{
						newCanMsg.data[c] = CAN_rx_msg.data[c];
					}
					newCanMsg.len = 8;
					XBeeTransmitCan(&newCanMsg);	
					break;
					
				//Battery: State of Charge (For LCD Display)
				case 0x626:
					
					//This one is different; it is used to set a battery percentage bar
					SetBar(0xFF & CAN_rx_msg.data[0], 128, YPOS_12);
				
					newCanMsg.id = 0x624;					
					for (c = 0; c < 8; c++)
					{
						newCanMsg.data[c] = CAN_rx_msg.data[c];
					}
					newCanMsg.len = 8;
					XBeeTransmitCan(&newCanMsg);	
					break;
				
				//Motor Drive Unit: Speed (For LCD Display) 200ms
				case MC_BASE + 3:
					
					tempFloat = 0xFFFF & (CAN_rx_msg.data[7] | CAN_rx_msg.data[6] << 8 | CAN_rx_msg.data[5] << 16 | CAN_rx_msg.data[4] << 24);			
					tempFloat = tempFloat * 3.6; //Conversion to KPH
					tempInt32 = (int32_t) tempFloat;					
					while (tempFloat > 1)
					{
						tempFloat = tempFloat / 10;
					}
					UpdateScreenParameter(XPOS_0, YPOS_9, tempInt32, (uint8_t) tempFloat * 10);
					break;
				
				//Motor Drive Unit: Temperature (For LCD Display) 1s
				case MC_BASE + 1:			
					tempFloat = 0xFFFF & (CAN_rx_msg.data[3] | CAN_rx_msg.data[2] << 8 | CAN_rx_msg.data[1] << 16 | CAN_rx_msg.data[0] << 24);
					tempInt32 = (int32_t) tempFloat;					
					while(tempFloat > 1)
					{
						tempFloat = tempFloat / 10;
					}					
					UpdateScreenParameter(XPOS_20, YPOS_3, tempInt32, (uint8_t) tempFloat * 10);
					break;
					
				//Motor Drive Unit: Current (For LCD Display) 200ms
				case MC_BASE + 2:
					
					tempFloat = 0xFFFF & (CAN_rx_msg.data[3] | CAN_rx_msg.data[2] << 8 | CAN_rx_msg.data[1] << 16 | CAN_rx_msg.data[0] << 24);
					tempInt32 = (int32_t) tempFloat;					
					while(tempFloat > 1)
					{
						tempFloat = tempFloat / 10;
					}					
					UpdateScreenParameter(XPOS_0, YPOS_6, tempInt32, (uint8_t) tempFloat * 10);
					break;
					
				//Array: Maximum Temperature (For LCD Display)
				case ARR_BASE:
					//TODO: Waiting for array team to build their interface
					
					UpdateScreenParameter(XPOS_20, YPOS_0, 0, 0);
					break;
				
				//Battery: Faults, Battery High and Battery Low (For Dashboard Indicator)
				case 0x622:
					
					//A10: Check if the high voltage bit is set, meaning battery is full
					if ( (CAN_rx_msg.data[6] >> 1) & 0x1)
					{
						//Reset pin A10, turn on LED
						GPIOA->BRR = 0x1 << 10;
					}
					
					//A9: Check if the low voltage bit is set, meaning battery is low
					if ( (CAN_rx_msg.data[6] ) & 0x1 )
					{
						//Reset pin A9, turn on LED
						GPIOA->BRR = 0x1 << 9;
					}
					
					//A1: battery over-temperature fault
					if ( ( CAN_rx_msg.data[5] >> 5 ) & 0x1 )
					{
						//Reset pin A1, turn on LED
						GPIOA->BRR = 0x1 << 1;
					}
					
					//A4: battery discharge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 4 ) & 0x1 )
					{
						//Reset pin A4, turn on LED
						GPIOA->BRR = 0x1 << 4;
					}
										
					//A6: battery charge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 3 ) & 0x1 )
					{
						//Reset pin A6, turn on LED
						GPIOA->BRR = 0x1 << 6;
					}
					
					//A8: battery over-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 7 ) & 0x1 )
					{
						//Reset pin A8, turn on LED
						GPIOA->BRR = 0x1 << 8;
					}
					
					//A7: battery under-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 6 ) & 0x1 )
					{
						//Reset pin A7, turn on LED
						GPIOA->BRR = 0x1 << 7;
					}
					
					//A11: Communications Fault
					if ( (CAN_rx_msg.data[5] >> 2) & 0x1)
					{
						//Reset pin A11: turn on LED
						GPIOA->BRR = 0x1 << 11;
					}
					
					newCanMsg.id = 0x622;					
					for (c = 0; c < 8; c++)
					{
						newCanMsg.data[c] = CAN_rx_msg.data[c];
					}
					newCanMsg.len = 8;
					XBeeTransmitCan(&newCanMsg);
									
					break;
							
			}		
		}
		
	}
	
	return 0;
}