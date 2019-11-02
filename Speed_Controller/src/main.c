#include "stm32f10x.h"

#include "ADC/ADC.h"
#include "Encoder/encoder.h"
#include "CAN.h"
#include "virtual_com.h"
#include "Timer/Timer.h"

#define DRIVE_CONTROL_ID 0x400
#define BATTERY_FULL_MSG 0x622
#define BATT_BASE 0x620

#define ADC_MAX 0xFFF				// TODO: find out what the actual ADC_MAX is
#define ADC_MIN 0                   // TODO: find out what the actual ADC_MIN is

#define TRUE 1
#define FALSE 0

#define DEBUG_STATE FALSE
#define READ_BATTERY_CHARGE FALSE
#define SEND_CAN_MSG FALSE

#define FORWARD 0

union {
	float float_var;
	uint8_t chars[4];
} u;

union {
	float float_var;
	uint8_t chars[4];
} v;

CAN_msg_t CAN_drive;

uint8_t reverse_toggle = 0;
uint8_t old_reverse_toggle = 0;

/**
 * When called it updates the CAN_drive and then sends the CAN_drive to the motor controller
 * Takes Current (0.000 - 1.000) you want the car to be at, and Velocity(any number in m/s) you want the car to be at
 * Note: NEVER pass in a negative velocity, the forwards and backwards is taken care of with the switch on the dashboard and this function.
 */
void sendMotorCommand(float curr, float vel)
{
	
	//Set velocity based on input and the reverse toggle, and also sets the current from the input
	if(reverse_toggle == FORWARD) 
	{
		v.float_var = vel;
	} else
	{
		v.float_var = -(vel);
	}
	
	u.float_var = curr;
	
	#if DEBUG_STATE
		SendString(" Direction: ");
		if(v.float_var > 0) { SendString("Forwards"); }
		if(v.float_var < 0) { SendString("Backwards"); }
	#endif
	
	//Set current
	CAN_drive.data[4] = u.chars[0];
	CAN_drive.data[5] = u.chars[1];
	CAN_drive.data[6] = u.chars[2];
	CAN_drive.data[7] = u.chars[3];
	
	//set velocity
	CAN_drive.data[0] = v.chars[0];
	CAN_drive.data[1] = v.chars[1];
	CAN_drive.data[2] = v.chars[2];
	CAN_drive.data[3] = v.chars[3];
	
    #if SEND_CAN_MSG
	    CANSend(&CAN_drive);
    #endif

	RestartTimer();
}

/**
 * When called this will change the direction that the car will be going
 */
void sendReverseToggle() {
	//Toggles the dirrection of the car, regardless of that direction
	v.float_var = -(v.float_var);

	CAN_drive.data[0] = v.chars[0];
	CAN_drive.data[1] = v.chars[1];
	CAN_drive.data[2] = v.chars[2];
	CAN_drive.data[3] = v.chars[3];

	#if DEBUG_STATE
		SendString("Toggle reverse, vel= ");
		SendInt(v.float_var);
		SendString("Direction: ");
		SendInt(reverse_toggle);
	#endif
	
	#if SEND_CAN_MSG
	    CANSend(&CAN_drive);
    #endif

	RestartTimer();
}

int main(void)
{
	
	CANInit(CAN_500KBPS);
	EncoderInit();
	ADCInit();
	TimerInit(200);
	VirtualComInit();
	
	//Setup of RegenToggle
	RCC->APB2ENR |= (0x1 << 4); //Clock C enable
	GPIOC->CRH &= ~(0xF); //Reset C8
	GPIOC->CRH |= (0x4); //Enable C8 as Input
	
	GPIOC->CRL &= ~(0xF << 24); //C6
	GPIOC->CRL |= (0x4 << 24); //C6

	//CAN receive setup
	CAN_msg_t CAN_rx_msg;
	
	//CAN motor command setup
	CAN_drive.len = 8;
	CAN_drive.id = DRIVE_CONTROL_ID + 1;
	
	sendMotorCommand(0, 0);
	
	volatile uint16_t encoder_reading;
	volatile uint16_t regen_reading;

	uint16_t old_encoder_reading = 0x0000;
	uint16_t old_regen_reading = 0x0000;
	
	uint8_t battery_percent = 0x00;
	uint8_t regen_toggle = 0x00;

	while(1) 	
	{
		//Gets all new values
		encoder_reading = EncoderRead();
		regen_reading = (ReadADC() >> 6) << 6;
		regen_toggle = ((GPIOC->IDR >> 8) & 0x1);
		reverse_toggle = ((GPIOC->IDR >> 6) & 0x1);
		
		if(CANMsgAvail())
		{
			CANReceive(&CAN_rx_msg);
			
			if(CAN_rx_msg.id == BATT_BASE + 6)
			{
                #if READ_BATTERY_CHARGE
				    battery_percent = (int8_t) CAN_rx_msg.data[0];   //TODO: Confirm battery percent value in Elithion manual
                #else
                    battery_percent = 70;
                #endif
			}
		}

		if(reverse_toggle != old_reverse_toggle)
		{
			sendReverseToggle();
		}

        #if DEBUG_STATE
            //SendString(" rgn tgl: ");
            //SendInt(regen_toggle);

            //SendString(" bat prc: ");
            //SendInt(battery_percent);

            //SendString(" old rgn: ");
            //SendInt(old_regen_reading);

            SendString(" new rgn: ");
            SendInt(regen_reading);

            //SendString(" old enc: ");
            //SendInt(old_encoder_reading);

            SendString(" new enc: ");
            SendInt(encoder_reading);
        #endif

		//If Regen is on and battery is below 98%, then regenerative braking is enabled
		if(regen_toggle == TRUE && battery_percent < 98)
		{

            /*
                regen zero        //regen NOT altered //encoder zero      //encoder altered           --> send drive msg                                     
                regen zero        //regen NOT altered //encoder NOT zero  //encoder altered           --> send drive msg  
                regen zero        //regen altered     //encoder NOT zero  //encoder altered           --> send drive msg to go back to accelerating
                regen zero        //regen altered     //encoder NOT zero  //encoder NOT altered       --> send drive msg to go back to accelerating

                regen zero        //regen altered     //encoder zero      //encoder altered           --> send regen msg to set regen to zero
                regen zero        //regen altered     //encoder zero      //encoder NOT altered       --> send regen msg to set regen to zero  
                regen NOT zero    //regen altered     //encoder zero      //encoder altered           --> send regen msg                                      
                regen NOT zero    //regen altered     //encoder NOT zero  //encoder altered           --> send regen msg
                regen NOT zero    //regen altered     //encoder zero      //encoder NOT altered       --> send regen msg                                      
                regen NOT zero    //regen altered     //encoder NOT zero  //encoder NOT altered       --> send regen msg                                      

                regen zero        //regen NOT altered //encoder zero      //encoder NOT altered       --> don't send any msg; wait for timout or update     
                regen zero        //regen NOT altered //encoder NOT zero  //encoder NOT altered       --> don't send any msg; wait for timout or update 
                regen NOT zero    //regen NOT altered //encoder zero      //encoder altered           --> don't send any msg; wait for timeout or update      
                regen NOT zero    //regen NOT altered //encoder zero      //encoder NOT altered       --> don't send any msg; wait for timeout or update 
                regen NOT zero    //regen NOT altered //encoder NOT zero  //encoder altered           --> don't send any msg; wait for timeout or update      
                regen NOT zero    //regen NOT altered //encoder NOT zero  //encoder NOT altered       --> don't send any msg; wait for timout or update       
            */

            //a NON altered state

            #if DEBUG_STATE
                SendString(" ,rgn en ");
            #endif
            
            //send a new regen message
			if( (old_regen_reading != regen_reading) && (regen_reading != 0 || (regen_reading == 0 && encoder_reading == 0)))
			{
                
                #if DEBUG_STATE
                    SendString(" ,new rgn ");
                #endif

				//sends knob percentage and velocity
				sendMotorCommand((float) regen_reading/ADC_MAX, 0.000);

			}
            //The encoder reading has changed, or no regen is applied at the moment, send a new drive message
			else if( (old_encoder_reading != encoder_reading && regen_reading == old_regen_reading && regen_reading == 0) || 
					 (regen_reading == 0 && regen_reading != old_regen_reading && encoder_reading != 0) ) 
			{

                #if DEBUG_STATE
                    SendString(" ,new drv ");
                #endif
            
				//Sends pedal percentage and velocity
				sendMotorCommand((float) encoder_reading/(PEDAL_MAX - PEDAL_MIN), 100.000);

			}
            
		} 
		else 
		{
            #if DEBUG_STATE
                SendString(" ,rgn den ");
            #endif

			//If the encoder count changed, send new drive CAN message
			if(old_encoder_reading != encoder_reading)
			{
                #if DEBUG_STATE
                    SendString(" ,new drv ");
                #endif

				sendMotorCommand((float) encoder_reading/(PEDAL_MAX - PEDAL_MIN), 100.000);
			}
		 }
		
		//If a timeout occured, send the previously sent CAN drive message
		if(timeoutFlag == TRUE)
		{	
            #if DEBUG_STATE
                SendString(" ,timout ");
            #endif

            #if SEND_CAN_MSG
			    CANSend(&CAN_drive);
            #endif
			
			timeoutFlag = FALSE;
			RestartTimer();
		}
		
		old_regen_reading = regen_reading;
		old_encoder_reading = encoder_reading;
		old_reverse_toggle = reverse_toggle;
		
		#if DEBUG_STATE
			SendLine();	
		#endif
		
	}
	
}
