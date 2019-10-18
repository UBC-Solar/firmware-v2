#include "stm32f10x.h"

#include "ADC/ADC.h"
#include "Encoder/encoder.h"
#include "CAN.h"
#include "virtual_com.h"
#include "Timer/Timer.h"
#include "virtual_com.h"

#define DRIVE_CONTROL_ID 0x400
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
	EncoderInit();
	TimerInit(200);
	VirtualComInit();
	CAN_msg_t CAN_drive;     
	
	CAN_drive.len = 8;
	
	CAN_drive.id = DRIVE_CONTROL_ID + 1;
	
	v.float_var = 100.0;
	u.float_var = 0.0;
	
	//set velocity to 100 for drive messages
	CAN_drive.data[0] = v.chars[0];
	CAN_drive.data[1] = v.chars[1];
	CAN_drive.data[2] = v.chars[2];
	CAN_drive.data[3] = v.chars[3];
	
	//set initial 
	CAN_drive.data[4] = u.chars[0];
	CAN_drive.data[5] = u.chars[1];
	CAN_drive.data[6] = u.chars[2];
	CAN_drive.data[7] = u.chars[3];
	
	volatile uint16_t encoder_reading;
	uint16_t old_encoder_reading = 0x0000;
	
	CAN_msg_t testCAN;
	
	while(1) 
	{
		
		/*
		if (CANMsgAvail())
		{
			CANReceive(&testCAN);
			
			
			if (testCAN.id > 0x500 && testCAN.id < 0x600)
			{
				SendInt(testCAN.id);
				
				SendLine();
			}
				
		}
		*/
		
		
		
		encoder_reading = EncoderRead();
		
		//SendInt(encoder_reading);
		//SendLine();
		
		
		//If the encoder count changed, send new drive CAN message and restart timer
		if(old_encoder_reading != encoder_reading)
		{

			
				RestartTimer();
			}		
			//if encoder count changed, send new drive CAN message and restart timer
			else if(old_encoder_reading != encoder_reading && (ADC_reading == 0 || !regen_enabled))
			{
				StopTimer();
			
			//Use a parabolic scaling
			//u.float_var = (float)(2*((float) encoder_reading/PEDAL_MAX) - ((float) encoder_reading/PEDAL_MAX)*((float) encoder_reading/PEDAL_MAX));		
			u.float_var = (float) ((float) encoder_reading/PEDAL_MAX);
				
			CAN_drive.data[4] = u.chars[0];
			CAN_drive.data[5] = u.chars[1];
			CAN_drive.data[6] = u.chars[2];
			CAN_drive.data[7] = u.chars[3];
			
			CANSend(&CAN_drive);
			//SendInt(DRIVE_CONTROL_ID + 1);
			//SendLine();
		
			RestartTimer();
		
		}		
		//If a timeout occured, send the previously sent CAN drive message
		else if(timeoutFlag == TRUE)
		{	
			
			CANSend (&CAN_drive);
			//SendInt(DRIVE_CONTROL_ID + 1);
			//SendLine();
			
			
			timeoutFlag = FALSE;
			RestartTimer();
		}
		
		old_encoder_reading = encoder_reading;		

	}
	
}
