#include "stm32f10x.h"

#include "LCD/LCD.h"
#include "CAN_new.h"
#include "CAN_common.h"
#include "XBee/XBee.h"
#include "virtual_com.h"

#define  MOTOR_SPEED_SCALING            3.6
#define  MOTOR_SPEED_TRANSMIT_QUANTA    5
#define  MOTOR_CURRENT_TRANSMIT_QUANTA  5

union Mem_Space_4_Byte {
	float     to_float;
	uint8_t   to_uint8_t_array[4];
	int32_t   to_int32_t; 
};

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

static inline void CAN_Message_Battery_Error_Handler(CAN_Message* CAN_msg_p)
{
	//A10: Check if the high voltage bit is set, meaning battery is full
	if ((CAN_msg_p->data[6] >> 1) & 0x1)
	{
		//Reset pp;in A10, turn on LED
		GPIOA->BSRR = 0x1 << 10;
	}
					
	//A9: Check if the low voltage bit is set, meaning battery is low
	if ((CAN_msg_p->data[6]) & 0x1)
	{
		//Reset pin A9, turn on LED
		GPIOA->BSRR = 0x1 << 9;
	}
					
	//A1: battery over-temperature fault
	if ((CAN_msg_p->data[5] >> 5) & 0x1)
	{
		//Reset pin A1, turn on LED
		GPIOA->BSRR = 0x1 << 1;
	}
					
	//A4: battery discharge over-current fault
	if ((CAN_msg_p->data[5] >> 4) & 0x1)
	{
		//Reset pin A4, turn on LED
		GPIOA->BSRR = 0x1 << 4;
	}
										
	//A6: battery charge over-current fault
	if ((CAN_msg_p->data[5] >> 3) & 0x1)
	{
		//Reset pin A6, turn on LED
		GPIOA->BSRR = 0x1 << 6;
	}
					
	//A8: battery over-voltage fault
	if ((CAN_msg_p->data[5] >> 7) & 0x1)
	{
		//Reset pin A8, turn on LED
		GPIOA->BSRR = 0x1 << 8;
	}
					
	//A7: battery under-voltage fault
	if ((CAN_msg_p->data[5] >> 6) & 0x1)
	{
		//Reset pin A7, turn on LED
		GPIOA->BSRR = 0x1 << 7;
	}
					
	//A11: Communications Fault
	if ((CAN_msg_p->data[5] >> 2) & 0x1)
	{
		//Reset pin A11: turn on LED
		GPIOA->BSRR = 0x1 << 11;
	}
}

static void CAN_Message_Handler(CAN_Message* CAN_msg_p) 
{
	// Conversion space shared across all function calls 
	// Change to non-static if multiple threads for message handling 
	static union Mem_Space_4_Byte  var_conv                     = {0};
	static uint8_t                 motor_speed_message_ticks    = 0; 
	static uint8_t                 motor_current_message_ticks  = 0; 
	
	switch(CAN_msg_p->id)
	{
		case CAN_ID_MOTOR_BASE + CAN_ID_MOTOR_SPEED_OFFSET:
		{
			float     motor_speed; 
			int32_t   motor_speed_display;
			uint32_t  motor_speed_decimal_display;
			
			var_conv.to_uint8_t_array[0] = CAN_msg_p->data[4];
			var_conv.to_uint8_t_array[1] = CAN_msg_p->data[5];
			var_conv.to_uint8_t_array[2] = CAN_msg_p->data[6];
			var_conv.to_uint8_t_array[3] = CAN_msg_p->data[7];
		
			motor_speed = var_conv.to_float * MOTOR_SPEED_SCALING;
			motor_speed_display = (int32_t) motor_speed; 
			motor_speed = motor_speed < 0 ? -1 * motor_speed : motor_speed; 
			motor_speed_decimal_display = ((uint32_t) (motor_speed * 10)) % 10; 
			
			UpdateScreenParameter(MOTOR_SPEED_XPOS, MOTOR_SPEED_YPOS, motor_speed_display, motor_speed_decimal_display);
					
			//Send the CAN message once every second
			if (++motor_speed_message_ticks == MOTOR_SPEED_TRANSMIT_QUANTA)
			{
				XBeeQueueCan(CAN_msg_p);
				motor_speed_message_ticks = 0;
			}
			break;
		} 
				
		case CAN_ID_MOTOR_BASE + CAN_ID_MOTOR_TEMPERATURE_OFFSET:
		{
			float     motor_temperature; 
			int32_t   motor_temperature_display; 
			uint32_t  motor_temperature_decimal_display;
			
			var_conv.to_uint8_t_array[0] = CAN_msg_p->data[0];
			var_conv.to_uint8_t_array[1] = CAN_msg_p->data[1];
			var_conv.to_uint8_t_array[2] = CAN_msg_p->data[2];
			var_conv.to_uint8_t_array[3] = CAN_msg_p->data[3];
			
			motor_temperature = var_conv.to_float;  
			motor_temperature_display = (int32_t) motor_temperature;
			motor_temperature = motor_temperature < 0 ? -1 * motor_temperature : motor_temperature; 
			motor_temperature_decimal_display = ((uint32_t) (motor_temperature * 10)) % 10; 

			UpdateScreenParameter(MOTOR_TEMP_XPOS, MOTOR_TEMP_YPOS, motor_temperature_display, motor_temperature_decimal_display);
			XBeeQueueCan(CAN_msg_p);
			break;
		}
		
		case CAN_ID_MOTOR_BASE + CAN_ID_MOTOR_CURRENT_OFFSET:
		{
			float     motor_current; 
			int32_t   motor_current_display; 
			uint32_t  motor_current_decimal_display;
			
			var_conv.to_uint8_t_array[0] = CAN_msg_p->data[4];
			var_conv.to_uint8_t_array[1] = CAN_msg_p->data[5];
			var_conv.to_uint8_t_array[2] = CAN_msg_p->data[6];
			var_conv.to_uint8_t_array[3] = CAN_msg_p->data[7];
			
			motor_current = var_conv.to_float;
			motor_current_display = (int32_t) motor_current; 
			motor_current = motor_current < 0 ? -1 * motor_current : motor_current; 
			motor_current_decimal_display = ((uint32_t) (motor_current * 10)) % 10; 
			
			UpdateScreenParameter(MOTOR_CURRENT_XPOS, MOTOR_CURRENT_YPOS, motor_current_display, motor_current_decimal_display);

			//Send the CAN message once every second
			if (++motor_current_message_ticks == MOTOR_CURRENT_TRANSMIT_QUANTA)
			{
				XBeeQueueCan(CAN_msg_p);
				motor_current_message_ticks = 0;
			}
			break;
		}
					
		case CAN_ID_BATTERY_BASE + CAN_ID_BATTERY_VOLTAGE_OFFSET:
		{
			uint16_t battery_voltage = (uint16_t) (CAN_msg_p->data[1] | CAN_msg_p->data[0] << 8);
			UpdateScreenParameter(BATTERY_VOLTAGE_XPOS, BATTERY_VOLTAGE_YPOS, battery_voltage, 0);
			XBeeQueueCan(CAN_msg_p);
			break;
		}
		
		case CAN_ID_BATTERY_BASE + CAN_ID_BATTERY_CURRENT_OFFSET:
		{
			uint16_t battery_current = (int16_t) (CAN_msg_p->data[1] | CAN_msg_p->data[0] << 8);
			UpdateScreenParameter(BATTERY_CURRENT_XPOS, BATTERY_CURRENT_YPOS, battery_current, 0);
			XBeeQueueCan(CAN_msg_p);
			break;
		}
				
		case CAN_ID_BATTERY_BASE + CAN_ID_BATTERY_TEMPERATURE_OFFSET:
		{
			int8_t battery_temperature = (int8_t) CAN_msg_p->data[4];
			UpdateScreenParameter(BATTERY_MAXTEMP_XPOS, BATTERY_MAXTEMP_YPOS, battery_temperature, 0);
			XBeeQueueCan(CAN_msg_p);	
			break;
		}
					
		case CAN_ID_BATTERY_BASE + CAN_ID_BATTERY_CHARGE_OFFSET:
		{
			//This one is different; it is used to set a battery percentage bar
			SetBar(0xFF & CAN_msg_p->data[0], 100, CHARGE_BAR_YPOS);				
			XBeeQueueCan(CAN_msg_p);	
			break;
		}
		
		case CAN_ID_BATTERY_BASE + CAN_ID_BATTERY_ERROR_OFFSET:
		{
			CAN_Message_Battery_Error_Handler(CAN_msg_p);
			XBeeQueueCan(CAN_msg_p);				
			break;
							
		}	
		
		//Array: Maximum Temperature (For LCD Display). Values are 16-bit unsigned integer in Celsius(C). Period: 1s
		case CAN_ID_ARRAY_BASE:
		{
			//TODO: Waiting for array team to build their interface
			UpdateScreenParameter(ARRAY_MAXTEMP_XPOS, ARRAY_MAXTEMP_YPOS, 0, 0);
			XBeeQueueCan(CAN_msg_p);
			break;
		}
	}
}

int main(void)
{
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

	while (1)
	{
		CAN_PerformOnMessage(CAN_Message_Handler);
	}
}
