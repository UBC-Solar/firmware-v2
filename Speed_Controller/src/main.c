#include "stm32f103xb.h"

#include "clock_init/clock_init.h"
#include "ADC/ADC.h"
#include "Encoder/encoder.h"
#include "Timer/Timer.h"
#include "CAN.h"
#include "virtual_com.h"

#define DRIVE_CONTROL_ID 0x400
#define BATTERY_FULL_MSG 0x622
#define BATT_BASE 0x620

#define ADC_MAX 0xFFF // TODO: find out what the actual ADC_MAX is
#define ADC_MIN 0	  // TODO: find out what the actual ADC_MIN is

#define TRUE 1
#define FALSE 0

#define PRINT_DEBUG TRUE
#define READ_BATTERY_CHARGE FALSE
#define SEND_CAN_MSG TRUE

#define FORWARD 0

static union
{
	float float_var;
	uint8_t byte[4];
} current_setpoint;

static union
{
	float float_var;
	uint8_t byte[4];
} velocity_setpoint;

static CAN_msg_t CAN_drive;

static uint8_t reverse_toggle = 0;
static uint8_t old_reverse_toggle = 0;

static void sendMotorCommand(float curr, float vel);
static void sendReverseToggle(void);

int main(void)
{
	// SetSysClockTo36Hsi();
	SetSysClockTo36HseBypass();

	CANInit(CAN_500KBPS);

	EncoderInit();
	ADCInit();
	TimerInit(200);
	VirtualComInit(BAUD_115200);

	SendString("RESET RESET RESET RESET RESET RESET \r\n\n\n\n\n");

	// Setup regen toggle
	RCC->APB2ENR |= RCC_APB2ENR_IOPCEN; 				// Enable GPIO port C clock
	GPIOC->CRH &= ~(GPIO_CRH_CNF8 | GPIO_CRH_MODE8); 	// Reset pin C8 config
	GPIOC->CRH |= (GPIO_CRH_CNF8_0);					// Enable pin C8 as input

	// Setup reverse toggle
	GPIOC->CRL &= ~(GPIO_CRL_CNF6 | GPIO_CRL_MODE6);	// Reset pin C6 config
	GPIOC->CRL |= (GPIO_CRH_CNF8_0);					// Enable pin C6 as input

	// CAN receive setup
	CAN_msg_t CAN_rx_msg;

	// CAN motor command setup
	CAN_drive.len = 8;
	CAN_drive.id = DRIVE_CONTROL_ID + 1;

	sendMotorCommand(0, 0);

	volatile uint16_t encoder_reading;
	volatile uint16_t regen_reading;
#if PRINT_DEBUG
	volatile uint16_t encoder_timer_count;
#endif /* PRINT_DEBUG */

	uint16_t old_encoder_reading = 0x0000;
	uint16_t old_regen_reading = 0x0000;

	uint8_t battery_percent = 0x00;
	uint8_t regen_toggle = 0x00;

	while (1)
	{
		// Gets all new values
#if PRINT_DEBUG
		encoder_timer_count = TIM1->CNT;
#endif /* PRINT_DEBUG */
		encoder_reading = EncoderRead();
		regen_reading = (ReadADC() >> 6) << 6;
		regen_toggle = (GPIOC->IDR >> GPIO_IDR_IDR8_Pos) & 0x1;
		reverse_toggle = (GPIOC->IDR >> GPIO_IDR_IDR6_Pos) & 0x1;

		if (CANMsgAvail())
		{
			CANReceive(&CAN_rx_msg);

			if (CAN_rx_msg.id == BATT_BASE + 6)
			{
#if READ_BATTERY_CHARGE
				battery_percent = (int8_t)CAN_rx_msg.data[0]; // TODO: Confirm battery percent value in Elithion manual
#else /* !READ_BATTERY_CHARGE */
				battery_percent = 70;
#endif /* READ_BATTERY_CHARGE */
			}
		}

		if (reverse_toggle != old_reverse_toggle)
		{
			sendReverseToggle();
		}

#if PRINT_DEBUG
		// SendString(" rgn tgl: ");
		// SendInt(regen_toggle);

		// SendString(" bat prc: ");
		// SendInt(battery_percent);

		// SendString(" old rgn: ");
		// SendInt(old_regen_reading);

		SendString(" new rgn: ");
		SendInt(regen_reading);

		// SendString(" old enc: ");
		// SendInt(old_encoder_reading);

		SendString(" new enc: ");
		SendInt(encoder_reading);
		SendString(" new cnt: ");
		SendInt(encoder_timer_count);
#endif /* PRINT_DEBUG */

		// If Regen is on and battery is below 98%, then regenerative braking is enabled
		if (regen_toggle == TRUE && battery_percent < 98)
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

			// a NON altered state

#if PRINT_DEBUG
			SendString(", rgn en ");
#endif /* PRINT_DEBUG */

			// send a new regen message
			if ((old_regen_reading != regen_reading) && (regen_reading != 0 || (regen_reading == 0 && encoder_reading == 0)))
			{

#if PRINT_DEBUG
				SendString(", new rgn ");
#endif /* PRINT_DEBUG */

				// sends knob percentage and velocity
				sendMotorCommand((float)regen_reading / ADC_MAX, 0.000);
			}
			// The encoder reading has changed, or no regen is applied at the moment, send a new drive message
			else if ((old_encoder_reading != encoder_reading && regen_reading == old_regen_reading && regen_reading == 0) ||
					 (regen_reading == 0 && regen_reading != old_regen_reading && encoder_reading != 0))
			{

#if PRINT_DEBUG
				SendString(", new drv ");
#endif /* PRINT_DEBUG */

				// Sends pedal percentage and velocity
				sendMotorCommand((float)encoder_reading / (PEDAL_MAX - PEDAL_MIN), 100.000);
			}
		}
		else
		{
#if PRINT_DEBUG
			SendString(", rgn den ");
#endif /* PRINT_DEBUG */

			// If the encoder count changed, send new drive CAN message
			if (old_encoder_reading != encoder_reading)
			{
#if PRINT_DEBUG
				SendString(", new drv ");
#endif /* PRINT_DEBUG */

				sendMotorCommand((float)encoder_reading / (PEDAL_MAX - PEDAL_MIN), 100.000);
			}
		}

		// If a timeout occured, send the previously sent CAN drive message
		if (GetTimeoutFlag() == TRUE)
		{
#if PRINT_DEBUG
			SendString(", timout ");
#endif /* PRINT_DEBUG */

#if SEND_CAN_MSG
			CANSend(&CAN_drive);
#endif /* SEND_CAN_MSG */

			ClearTimeoutFlag();
			RestartTimer();
		}

		old_regen_reading = regen_reading;
		old_encoder_reading = encoder_reading;
		old_reverse_toggle = reverse_toggle;

#if PRINT_DEBUG
		SendLine();
#endif /* PRINT_DEBUG */
	}
}


/**
 * Update the CAN_drive and then sends the CAN_drive to the motor controller
 * 
 * @param curr 	Current (0.000 - 1.000) you want the car to be at
 * @param vel  	Velocity (any number in m/s) you want the car to be at
 * 
 * @note NEVER pass in a negative velocity, the forwards and backwards is taken care of with
 * the switch on the dashboard and this function.
 */
static void sendMotorCommand(float curr, float vel)
{
	// Set velocity based on input and the reverse toggle, and also sets the current from the input
	if (reverse_toggle == FORWARD)
	{
		velocity_setpoint.float_var = vel;
	}
	else
	{
		velocity_setpoint.float_var = -(vel);
	}

	current_setpoint.float_var = curr;

#if PRINT_DEBUG
	SendString(", Direction: ");
	if (velocity_setpoint.float_var > 0)
	{
		SendString("Forwards");
	}
	if (velocity_setpoint.float_var < 0)
	{
		SendString("Backwards");
	}
#endif /* PRINT_DEBUG */

	// set velocity
	CAN_drive.data[0] = velocity_setpoint.byte[0];
	CAN_drive.data[1] = velocity_setpoint.byte[1];
	CAN_drive.data[2] = velocity_setpoint.byte[2];
	CAN_drive.data[3] = velocity_setpoint.byte[3];

	// Set current
	CAN_drive.data[4] = current_setpoint.byte[0];
	CAN_drive.data[5] = current_setpoint.byte[1];
	CAN_drive.data[6] = current_setpoint.byte[2];
	CAN_drive.data[7] = current_setpoint.byte[3];

#if SEND_CAN_MSG
	CANSend(&CAN_drive);
#endif /* SEND_CAN_MSG */

	RestartTimer();
}

/**
 * Change the direction that the car will be going
 */
static void sendReverseToggle()
{
	// Toggles the dirrection of the car, regardless of that direction
	velocity_setpoint.float_var = -(velocity_setpoint.float_var);

	CAN_drive.data[0] = velocity_setpoint.byte[0];
	CAN_drive.data[1] = velocity_setpoint.byte[1];
	CAN_drive.data[2] = velocity_setpoint.byte[2];
	CAN_drive.data[3] = velocity_setpoint.byte[3];

#if PRINT_DEBUG
	SendString(", Toggle reverse");
#endif /* PRINT_DEBUG */

#if SEND_CAN_MSG
	CANSend(&CAN_drive);
#endif /* SEND_CAN_MSG */

	RestartTimer();
}
