#include "stm32f10x.h"

#include "LCD/LCD.h"
#include "CAN_new.h"
#include "XBee/XBee.h"
#include "virtual_com.h"

#define TRUE 1
#define FALSE 0

CAN_msg_t CAN_rx_msg;

union {
	float float_var;
	uint8_t chars[4];
} u;

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
	GPIOA->BRR = 0xFFFF;
	
}

void testerbester(CAN_Message* rx_msg)
{
	SendString("Message Received");
}
/**
 * Main procedure
 */
int main(void)
{
	
	//int32_t tempInt32;
	//CAN_msg_t newCanMsg;
	//uint8_t c = 0;
	//uint8_t d = 0;
	
	osKernelInitialize();
	
	//InitialiseLCDPins();
	//CANInit(CAN_1000KBPS);
	//ScreenSetup();
	//InitLEDs();
	VirtualComInit();
	//XBeeInit();
	CAN_Initialize();
	CAN_SetFilter(100);
	
	osKernelStart();
	

	
	
	
#if 0
	while(1)
	{
		//If a message can be received,		
		if (CANMsgAvail())
		{
			
			CANReceive(&CAN_rx_msg);		//Receive the msg currently in buffer
			//Check the CAN ID against several known IDs
			//Several need to be parsed, especially the ones designated for LCD and dashboard
			
			switch(CAN_rx_msg.id)
			{
				
				//Battery: Pack Voltage(For LCD Display). Values are unsigned 16-bit integers in Volts (V). Period: 1s
				case BATT_BASE + 3:
					UpdateScreenParameter(BATTERY_VOLTAGE_XPOS, BATTERY_VOLTAGE_YPOS, (uint16_t) (CAN_rx_msg.data[1] | CAN_rx_msg.data[0] << 8), 0);
									
					XBeeQueueCan(&CAN_rx_msg);								
					break;
				
				//Battery: Pack Current(For LCD Display). Values are signed 16-bit integers in Amperes (A). Period: 1s
				case BATT_BASE + 4:
					
					UpdateScreenParameter(BATTERY_CURRENT_XPOS, BATTERY_CURRENT_YPOS, (int16_t) (CAN_rx_msg.data[1] | CAN_rx_msg.data[0] << 8), 0);
					
					XBeeQueueCan(&CAN_rx_msg);							
					break;
				
				//Battery: Pack Maximum Temperature (For LCD Display). Values are signed 8-bit integers in Celsius (C). Period: 1s
				case BATT_BASE + 7:
					UpdateScreenParameter(BATTERY_MAXTEMP_XPOS, BATTERY_MAXTEMP_YPOS, (int8_t) CAN_rx_msg.data[4], 0);
				
					XBeeQueueCan(&CAN_rx_msg);	
					break;
					
				//Battery: State of Charge (For LCD Display). Values are unsigned 8-bit integers. Period: 1s
				case BATT_BASE + 6:
					
					//This one is different; it is used to set a battery percentage bar
					SetBar(0xFF & CAN_rx_msg.data[0], 100, CHARGE_BAR_YPOS);
				
					XBeeQueueCan(&CAN_rx_msg);	
					break;
				
				//Motor Drive Unit: Speed (For LCD Display). Values are IEEE 32-bit floating point in m/s. Period: 200ms
				case MC_BASE + 3:
					
					u.chars[0] = CAN_rx_msg.data[4];
					u.chars[1] = CAN_rx_msg.data[5];
					u.chars[2] = CAN_rx_msg.data[6];
					u.chars[3] = CAN_rx_msg.data[7];
				
					u.float_var = u.float_var * 3.6;
					tempInt32 = (int32_t) u.float_var;
				
					if (u.float_var < 0)
					{
						u.float_var = u.float_var * -1;
					}
							
					UpdateScreenParameter(MOTOR_SPEED_XPOS, MOTOR_SPEED_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
					
					//send the CAN message once every second
					if (d == 5)
					{
						XBeeQueueCan(&CAN_rx_msg);
						d = 0;
					}
					d++;
					
					break;
				
				//Motor Drive Unit: Temperature (For LCD Display). Values are IEEE 32-bit floating point in Celsius (C). Period: 1s
				case MC_BASE + 0x11:
					
					u.chars[0] = CAN_rx_msg.data[0];
					u.chars[1] = CAN_rx_msg.data[1];
					u.chars[2] = CAN_rx_msg.data[2];
					u.chars[3] = CAN_rx_msg.data[3];
					
					tempInt32 = (int32_t) u.float_var;
				
					while(u.float_var < 0)
					{
						u.float_var = u.float_var * -1;
					}
					
					XBeeQueueCan(&CAN_rx_msg);
					
					UpdateScreenParameter(MOTOR_TEMP_XPOS, MOTOR_TEMP_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
					break;
					
				//Motor Drive Unit: Current (For LCD Display). Values are IEEE 32-bit floating point in Amperes(A). Period: 200ms
				case MC_BASE + 2:
					
					u.chars[0] = CAN_rx_msg.data[4];
					u.chars[1] = CAN_rx_msg.data[5];
					u.chars[2] = CAN_rx_msg.data[6];
					u.chars[3] = CAN_rx_msg.data[7];
					
					tempInt32 = (int32_t) u.float_var;
				
					if (u.float_var < 0)
					{
						u.float_var = u.float_var * -1;
					}
					
					//Send the CAN message once every second
					if (c == 5)
					{
						XBeeQueueCan(&CAN_rx_msg);
						c = 0;
					}
					c++;
					
					UpdateScreenParameter(MOTOR_CURRENT_XPOS, MOTOR_CURRENT_YPOS, tempInt32, ((uint32_t) (u.float_var * 10)) % 10 );
					break;
					
					//Array: Maximum Temperature (For LCD Display). Values are 16-bit unsigned integer in Celsius(C). Period: 1s
				case ARR_BASE:
					//TODO: Waiting for array team to build their interface
					
					UpdateScreenParameter(ARRAY_MAXTEMP_XPOS, ARRAY_MAXTEMP_YPOS, 0, 0);
				
					XBeeQueueCan(&CAN_rx_msg);
				
					break;
				
				//Battery: Faults, Battery High and Battery Low (For Dashboard Indicator)
				case 0x622:
					
					//A10: Check if the high voltage bit is set, meaning battery is full
					if ( (CAN_rx_msg.data[6] >> 1) & 0x1)
					{
						//Reset pp;in A10, turn on LED
						GPIOA->BSRR = 0x1 << 10;
					}
					
					//A9: Check if the low voltage bit is set, meaning battery is low
					if ( (CAN_rx_msg.data[6] ) & 0x1 )
					{
						//Reset pin A9, turn on LED
						GPIOA->BSRR = 0x1 << 9;
					}
					
					//A1: battery over-temperature fault
					if ( ( CAN_rx_msg.data[5] >> 5 ) & 0x1 )
					{
						//Reset pin A1, turn on LED
						GPIOA->BSRR = 0x1 << 1;
					}
					
					//A4: battery discharge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 4 ) & 0x1 )
					{
						//Reset pin A4, turn on LED
						GPIOA->BSRR = 0x1 << 4;
					}
										
					//A6: battery charge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 3 ) & 0x1 )
					{
						//Reset pin A6, turn on LED
						GPIOA->BSRR = 0x1 << 6;
					}
					
					//A8: battery over-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 7 ) & 0x1 )
					{
						//Reset pin A8, turn on LED
						GPIOA->BSRR = 0x1 << 8;
					}
					
					//A7: battery under-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 6 ) & 0x1 )
					{
						//Reset pin A7, turn on LED
						GPIOA->BSRR = 0x1 << 7;
					}
					
					//A11: Communications Fault
					if ( (CAN_rx_msg.data[5] >> 2) & 0x1)
					{
						//Reset pin A11: turn on LED
						GPIOA->BSRR = 0x1 << 11;
					}
					
					XBeeQueueCan(&newCanMsg);
									
					break;
							
			}		
		}
		
	}
#endif
}
