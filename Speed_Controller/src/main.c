#include "stm32f10x.h"

#include "ADC/ADC.h"
#include "Encoder/encoder.h"
#include "CAN.h"
#include "virtual_com.h"
#include "Timer/Timer.h"
#include "virtual_com.h"

#define DRIVE_CONTROL_ID 0x500
#define BATTERY_FULL_MSG 0x622

union {
	float float_var;
	uint8_t chars[4];
} u;

union {
	float float_var;
	uint8_t chars[4];
} v;

int main(void){
	
	
	CANInit(CAN_500KBPS);
	CANSetFilter(0);
	//CANSetFilter(BATTERY_FULL_MSG);
	EncoderInit();
	ADCInit();
	TimerInit(25);
	VirtualComInit();

	CAN_msg_t CAN_drive;   
	CAN_msg_t CAN_regen;   
	
	CAN_drive.len = 8;
	CAN_regen.len = 8;
	
	CAN_drive.id = DRIVE_CONTROL_ID + 1;
	CAN_regen.id = DRIVE_CONTROL_ID + 1;
	
	v.float_var = 100.0;
	
	//set velocity to 100 for drive messages
	CAN_drive.data[0] = v.chars[0];
	CAN_drive.data[1] = v.chars[1];
	CAN_drive.data[2] = v.chars[2];
	CAN_drive.data[3] = v.chars[3];

	//set velocity to 0 for regen messages
	CAN_regen.data[0] = 0;
	CAN_regen.data[1] = 0;
	CAN_regen.data[2] = 0;
	CAN_regen.data[3] = 0;
	
	//set initial 
	CAN_drive.data[4] = 0;
	CAN_drive.data[5] = 0;
	CAN_drive.data[6] = 0;
	CAN_drive.data[7] = 0;
	
	CAN_regen.data[4] = 0;
	CAN_regen.data[5] = 0;
	CAN_regen.data[6] = 0;
	CAN_regen.data[7] = 0;
	
		
	RCC->APB2ENR |= 0x1 << 4;
	GPIOC->CRH &= ~(0xF);
	GPIOC->CRL &= ~(0xF << 24);
	GPIOC->CRH |= 0x4;
	GPIOC->CRL |= 0x4 << 24;
	
	
	RCC->APB2ENR |= 0x1 << 2;
	GPIOA->CRL &= ~(0xFF << 20);
	GPIOA->CRL |= 0x33 << 20;
	
	GPIOA->BRR = 0x1 << 5;
	GPIOA->BRR = 0x1 << 6;
	
	volatile uint16_t encoder_reading;
	volatile uint16_t ADC_reading;
	uint16_t old_ADC_reading = 0x000;
	uint16_t old_encoder_reading = 0x000;
	uint8_t regen_enabled;
	uint8_t current_direction;
	uint8_t previous_direction = 1;		//assume the vehicle always starts as moving forward
	uint16_t encoderZeroCount = 0;
	CAN_msg_t CAN_rx_msg;
	
	while(1) 
	{
		
		encoder_reading = EncoderRead();// / 10;
		//encoder_reading = encoder_reading * 10;
		
		ADC_reading = ReadADC(); /// 50;
		//ADC_reading = ADC_reading * 50;
		
		regen_enabled = (GPIOC->IDR >> 8) & 0x1UL;
		current_direction = (GPIOC->IDR >> 6) & 0x1UL;
		
		
		//SendString("    REGEN: ");
		//SendInt(ADC_reading);
		//SendString("    CURRENT: ");
		//SendInt(TIM1->CNT);
		//     SendLine();
		/*
		SendString("    REGEN ENABLED: ");
		SendInt(regen_enabled);
		SendString("    CURRENT DIRECTION: ");
		SendInt(current_direction);
		*/
		
		if (encoder_reading == 0)
		{
			encoderZeroCount++;
			
			if(encoderZeroCount > 10000)
			{
				GPIOA->BSRR = 0x1 << 6;
				encoderZeroCount = 0;
			}
		}
		else
		{
			encoderZeroCount = 0;
			GPIOA->BRR = 0x1 << 6;
		}
		
		
		if (current_direction != previous_direction)
		{
			v.float_var = v.float_var * -1;
			
			CAN_drive.data[0] = v.chars[0];
			CAN_drive.data[1] = v.chars[1];
			CAN_drive.data[2] = v.chars[2];
			CAN_drive.data[3] = v.chars[3];			
			
			previous_direction = current_direction;
		}
		
		if (CANMsgAvail())
		{
			CANReceive(&CAN_rx_msg);
			
			if (CAN_rx_msg.id == BATTERY_FULL_MSG)
			{
				regen_enabled = regen_enabled & (CAN_rx_msg.data[6] >> 1) & 0x1;
			}
		}
		
		//If regen is enabled and the ADC for regen changed and the battery is not full, regen must be enabled
		//	Send new regen CAN message and restart timer
		if( regen_enabled && (old_ADC_reading != ADC_reading))
		{
		
			StopTimer();
			GPIOA->BRR = 0x1 << 6;
			//SendString("NRG");

			u.float_var = (float)((float) ADC_reading / ADC_ZERO_THRESHOLD);
			
			SendInt(ADC_reading);
			SendLine();
			
			CAN_regen.data[4] = u.chars[0];
			CAN_regen.data[5] = u.chars[1];
			CAN_regen.data[6] = u.chars[2];
			CAN_regen.data[7] = u.chars[3];
			
			GPIOA->BSRR = 0x1 << 5;
			CANSend(&CAN_regen);
			GPIOA->BRR = 0x1 << 5;
			
			//old_ADC_reading = ADC_reading;
			//old_encoder_reading = encoder_reading;
			
			//SendLine();
			
			RestartTimer();
			
		}		
		//Ff the encoder count changed, send new drive CAN message and restart timer unless 
		//	
		else if(old_encoder_reading != encoder_reading && (ADC_reading == 0 || !regen_enabled))
		{
			
			StopTimer();
			GPIOA->BRR = 0x1 << 6;
			//SendString("N DRV");
			
			u.float_var = (float)(2*((float) encoder_reading/PEDAL_MAX) - ((float) encoder_reading/PEDAL_MAX)*((float) encoder_reading/PEDAL_MAX));
			
			//SendInt(u.float_var*100);
			//SendLine();
			//u.float_var = 0.2;
			CAN_drive.data[4] = u.chars[0];
			CAN_drive.data[5] = u.chars[1];
			CAN_drive.data[6] = u.chars[2];
			CAN_drive.data[7] = u.chars[3];
			
			GPIOA->BSRR = 0x1 << 5;
			CANSend(&CAN_drive);
			GPIOA->BRR = 0x1 << 5;
			
			//old_ADC_reading = ADC_reading;
			//old_encoder_reading = encoder_reading;
		
			RestartTimer();
		
		}		
		//If a timeout occured, send the previously sent CAN drive message
		else if(timeoutFlag == TRUE)
		{	
			
			//If the driver is holding regen at a positive value, regen is enabled and the battery is not full
			//	Send the previous regen message
			if(ADC_reading > 10 && regen_enabled)
			{	
			//	SendString("O RGN");
				//SendString("   ");
				//SendInt(ADC_reading);
				//SendLine();
				GPIOA->BSRR = 0x1 << 6;
				GPIOA->BSRR = 0x1 << 5;
				CANSend(&CAN_regen);
				GPIOA->BRR = 0x1 << 5;
				//SendLine();
			}
			//If the driver is not braking, send the previous CAN drive message.
			else
			{	
				//SendString("O DRV");
				GPIOA->BRR = 0x1 << 6;
				GPIOA->BSRR = 0x1 << 5;
				CANSend (&CAN_drive);
				GPIOA->BRR = 0x1 << 5;
			}
			
			timeoutFlag = FALSE;
			
			//old_ADC_reading = ADC_reading;
			//old_encoder_reading = encoder_reading;
			
			RestartTimer();
		}
		
			old_ADC_reading = ADC_reading;
			old_encoder_reading = encoder_reading;		
		
		//otherwise, nothing is to be done - nothing has changed and nothing is due
		
		//SendLine();
		
			/*
			u.float_var = 0.2;
			CAN_drive.data[4] = u.chars[0];
			CAN_drive.data[5] = u.chars[1];
			CAN_drive.data[6] = u.chars[2];
			CAN_drive.data[7] = u.chars[3];
			
			GPIOA->BSRR = 0x1 << 5;
			CANSend(&CAN_drive);
			GPIOA->BRR = 0x1 << 5;
			
			for (volatile int j = 0; j < 10000; j++)
			{}
			*/
	}
	
}
