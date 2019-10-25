#include "stm32f10x.h"

#include "ADC/ADC.h"
#include "Encoder/encoder.h"
#include "CAN.h"
#include "virtual_com.h"
#include "Timer/Timer.h"
#include "virtual_com.h"

#define DRIVE_CONTROL_ID 0x400
#define BATTERY_FULL_MSG 0x622
#define BATT_BASE 0x620

#define ADC_MAX 255					// TODO: find out what the actual ADC_MAX is

#define REGENTOGGLE 1				// TODO: Add input for REGENTOGGLE
#define TRUE 1
#define FALSE 0

union {
	float float_var;
	uint8_t chars[4];
} u;

union {
	float float_var;
	uint8_t chars[4];
} v;


void sendMotorCommand(CAN_msg_t CAN, float curr, float vel)
{
	v.float_var = vel;
	u.float_var = curr;
	
	//Set current
	CAN.data[4] = u.chars[0];
	CAN.data[5] = u.chars[1];
	CAN.data[6] = u.chars[2];
	CAN.data[7] = u.chars[3];
	
	//set velocity
	CAN.data[0] = v.chars[0];
	CAN.data[1] = v.chars[1];
	CAN.data[2] = v.chars[2];
	CAN.data[3] = v.chars[3];
	
	CANSend(&CAN);
	
	RestartTimer();
}

int main(void)
{
	
	CANInit(CAN_500KBPS);
	EncoderInit();
	ADCInit();
	TimerInit(200);
	VirtualComInit();
	
	//CAN recive setup
	CAN_msg_t CAN_rx_msg;
  CAN_msg_t CAN_tx_msg;
	
	//CAM motor command setup
	CAN_msg_t CAN_drive;
	CAN_drive.len = 8;
	CAN_drive.id = DRIVE_CONTROL_ID + 1;
	
	volatile uint16_t encoder_reading;
	volatile uint16_t adc_reading;

	uint16_t old_encoder_reading = 0x0000;
	uint8_t batteryPercent = 0x00;

	while(1) 
	{
		
		//Gets all new values
		encoder_reading = EncoderRead();
		adc_reading = ReadADC();
		
		//Updates the battery percentage
		if (CANMsgAvail())
		{
			CANReceive(&CAN_rx_msg);
			
			if(CAN_rx_msg.id == BATT_BASE + 6) {
				batteryPercent = (int8_t) CAN_rx_msg.data[0];
			}
		}
		
		
		//SendInt(encoder_reading);
		//SendLine();
		
		//Checks if Regen is on and battery is beloew 98%
		if(REGENTOGGLE && batteryPercent < 98)
		{
			//IChecks for regen knob not at 0
			if(adc_reading != 0) // TODO: Check for change in adc knob so we dont flood CAN, TODO: Add min for adc knob
			{
				//sends knob percentage and velocity
				sendMotorCommand(CAN_drive, adc_reading/ADC_MAX, 0.000);
			}
			else if(old_encoder_reading != encoder_reading) 
			{
				//Sends peddle percentage and velocity
				sendMotorCommand(CAN_drive, encoder_reading/PEDAL_MAX, 100.000);
			}
		
		} else 
		{
				//If the encoder count changed, send new drive CAN message and restart timer
				if(old_encoder_reading != encoder_reading)
				{
					sendMotorCommand(CAN_drive, encoder_reading/PEDAL_MAX, 100.000);
				}
		 }
		
		//If a timeout occured, send the previously sent CAN drive message
		 if(timeoutFlag == TRUE) //TODO: I dont think this is going to work
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