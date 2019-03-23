#include "stm32f10x.h"

#include "LCD/LCD.h"
#include "CAN.h"
#include "XBee/XBee.h"
#include "virtual_com.h"

#define TRUE 1
#define FALSE 0

#define MC_BASE 0x200
#define ARR_BASE 0x700

uint8_t canTxReady = FALSE;
uint8_t canRxReady = FALSE;
uint8_t screen_state = HIGH;
uint8_t changeScreen = FALSE;

CAN_msg_t CAN_rx_msg;  // Holds receiving CAN messages
CAN_msg_t CAN_tx_msg;  // Holds transmitted CAN messages


void EXTI1_IRQHandler(void)
{
		//If interrupt line 1 is not masked AND a trigger arrives on line 1
    if ((EXTI->IMR & EXTI_IMR_MR1) && (EXTI->PR & EXTI_PR_PR1))
    {					
				//Set or Reset LED A10, depending on its initial state
				if ( (GPIOA->ODR >> 10) & 0x1UL)
				{
					GPIOA->BSRR = 0x1UL << 10;
				}
				else
				{
					GPIOA->BRR = 0x1UL << 10;
				}
				
				//Clear the trigger bit
				EXTI->PR |= EXTI_PR_PR4;
				
    }
}

void EXTI2_IRQHandler(void)
{
		//If interrupt line 2 is not masked AND a trigger arrives on line 2
    if ((EXTI->IMR & EXTI_IMR_MR2) && (EXTI->PR & EXTI_PR_PR2))
    {					
				//Set or Reset LED A11, depending on its initial state
				if ( (GPIOA->ODR >> 11) & 0x1UL)
				{
					GPIOA->BSRR = 0x1UL << 11;
				}
				else
				{
					GPIOA->BRR = 0x1UL << 11;
				}

				//Clear the trigger bit
				EXTI->PR |= EXTI_PR_PR2;
				
    }
}

void EXTI3_IRQHandler(void)
{
		//If interrupt line 3 is not masked AND a trigger arrives on line 3
    if ((EXTI->IMR & EXTI_IMR_MR1) && (EXTI->PR & EXTI_PR_PR1))
    {					
				//Set or Reset LED A13, depending on its initial state
				if ( (GPIOA->ODR >> 13) & 0x1UL)
				{
					GPIOA->BSRR = 0x1UL << 13;
				}
				else
				{
					GPIOA->BRR = 0x1UL << 11;
				}
				
				//Clear the trigger bit
				EXTI->PR |= EXTI_PR_PR4;
				
    }
}

/*
void EXTI4_IRQHandler(void)
{
		//If interrupt line 13 is not masked AND a trigger arrives on line 13
    if ((EXTI->IMR & EXTI_IMR_MR4) && (EXTI->PR & EXTI_PR_PR4))
    {					
				//Set a flag to change screens
				changeScreen = TRUE;

				//Clear the trigger bit
				EXTI->PR |= EXTI_PR_PR4;
				
    }
}
*/

//Interrupt Handler to receive a new message
void USB_LP_CAN1_RX0_IRQHandler(void)
{
	canRxReady = TRUE;
}

//Interrupt Handler to transmit a new message
void USB_HP_CAN1_TX_IRQHandler(void)
{
	canTxReady = TRUE;
}


void InitLEDs(void)
{
	
	RCC->APB2ENR |= 0x1UL << 2; 		//Initialize clock for GPIOA, if it hasn't been initialized yet
	GPIOA->CRL |= 0x33333330UL;			//Set all pins from A1 to A7 to be Push-Pull Output, 50Mhz
	GPIOA->CRH |= 0x00333333UL;			//SetBar all pins from A8 to A13 to be Push-Pull Output, 50Mhc
	
	//TODO: Confirm that all LCDs are working by blinking once
	
	RCC->APB2ENR |= 0x1UL << 3;			//Initialize clock for GPIOB, if it hasn't been initialized yet
	RCC->APB2ENR  |= 0x1UL;					//ENABLE Alternate Function I/O Clock
	
	AFIO->EXTICR[0] |= 0x1110UL;
	EXTI->IMR |= 0xEUL;
	EXTI->FTSR |= 0xEUL;
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	
	//Setup Button Interrupt for changing screens at B4
	/*
  AFIO->EXTICR[1] |= 0x1UL;				//Select PB[4] pin as source input for external interrupt
  EXTI->IMR |= 0x1UL << 4;					//Unmask interrupt request from line 4, enables interrupt line
  EXTI->FTSR |= 0x1UL << 4;				//ENABLE falling trigger for line 4 
  NVIC_EnableIRQ(EXTI4_IRQn);		//enable interrupt for changing screens
	*/
}

int main(void){
	
	float tempFloat;
	int32_t tempInt32;
	
	InitLEDs();
	ScreenSetup();
	InitialiseLCDPins();
	
	while(1){
		/*
		if (changeScreen)
		{	
			if (screen_state == HIGH)
			{
				screen_state = LOW;
				SecondScreen();
			}
			else 
			{
				screen_state = HIGH;
				FirstScreen();
			}
		}
		*/
						
	}
	
	//If a message can be receivved,
	if (canRxReady)
	{
			CANReceive(&CAN_rx_msg);		//Receive the msg currently in buffer
			
			//Check the CAN STID against several known IDs
			//Several need to be parsed, especially the ones designated for LCD and dashboard
			switch(CAN_rx_msg.id)
			{
				//Battery: Pack Voltage(For LCD Display)
				case 0x623:
					UpdateScreenParameter(XPOS_0, YPOS_3, 0xFF & (CAN_rx_msg.data[0] | CAN_rx_msg.data[1] << 8), 0);
					break;
				
				//Battery: Pack Current(For LCD Display)
				case 0x624:
					UpdateScreenParameter(XPOS_0, YPOS_0, 0xFF & (CAN_rx_msg.data[0] | CAN_rx_msg.data[1] << 8), 0);
					break;
				
				//Battery: Pack Maximum Temperature (For LCD Display)
				case 0x627:
					UpdateScreenParameter(XPOS_0, YPOS_6, 0xFF & CAN_rx_msg.data[4], 0);
					break;
					
				//Battery: State of Charge (For LCD Display)
				case 0x626:
					//This one is different; it is used to set a battery percentage bar
					SetBar(0xFF & CAN_rx_msg.data[0], 100, YPOS_12);
					break;
				
				//Motor Drive Unit: Speed (For LCD Display) 200ms
				case MC_BASE + 3:
					
					//tempFloat = 0xFFFF & (CAN_rx_msg.data[4] | CAN_rx_msg.data[5] << 8 | CAN_rx_msg.data[6] << 16 | CAN_rx_msg.data[7] << 24);			
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
					tempFloat = 0xFFFF & (CAN_rx_msg.data[0] | CAN_rx_msg.data[1] << 8 | CAN_rx_msg.data[2] << 16 | CAN_rx_msg.data[3] << 24);
					tempInt32 = (int32_t) tempFloat;					
					while(tempFloat > 1)
					{
						tempFloat = tempFloat / 10;
					}					
					UpdateScreenParameter(XPOS_20, YPOS_0, tempInt32, (uint8_t) tempFloat * 10);
					break;
				
				//Motor Drive Unit: Current (For LCD Display) 200ms
				case MC_BASE + 2:
						
					tempFloat = 0xFFFF & (CAN_rx_msg.data[4] | CAN_rx_msg.data[1] << 8 | CAN_rx_msg.data[2] << 16 | CAN_rx_msg.data[3] << 24);
					tempInt32 = (int32_t) tempFloat;					
					while(tempFloat > 1)
					{
						tempFloat = tempFloat / 10;
					}					
					UpdateScreenParameter(XPOS_20, YPOS_6, tempInt32, (uint8_t) tempFloat * 10);
					break;
					
				//Array: Maximum Temperature (For LCD Display)
				case ARR_BASE:
					//TODO: Waiting for array team to build their interface
					
					UpdateScreenParameter(XPOS_20, YPOS_0, 0, 0);
					break;
				
				//Battery: Faults, Battery High and Battery Low (For Dashboard Indicator)
				case 0x622:
					
					//Check if the high voltage bit is set, meaning battery is full
					if ( (CAN_rx_msg.data[6] >> 1) & 0x1)
					{
						//Set LED indicator A9
						GPIOA->BSRR = 0x1 << 9;
					}
					else
					{
						//Reset LED indicator A9
						GPIOA->BRR = 0x1 << 9;
					}
					
					//Check if the low voltage bit is set, meaning battery is low
					if ( CAN_rx_msg.data[6] & 0x1 )
					{
						//Set LED indicator A8
						GPIOA->BSRR = 0x1 << 8;
					}
					else
					{
						GPIOA-> BRR = 0x1 << 8;
					}
					
					//Check if a general battery fault has occurred
					if ( CAN_rx_msg.data[0] & 0x1 )
					{
						//Set LED indicator A1
						GPIOA->BSRR = 0x1;
					}
					else
					{
						//Reset LED indicator A1
						GPIOA->BRR = 0x1;
					}
					
					
					//battery over-temperature fault
					if ( ( CAN_rx_msg.data[5] >> 5 ) & 0x1 )
					{
						//Set LED indicator A2
						GPIOA->BSRR = 0x1 << 2;
					}
					else
					{
						//Reset LED indicator A2
						GPIOA->BRR = 0x1 << 2;
					}
					
					//battery discharge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 4 ) & 0x1 )
					{
						//Set LED indicator A3
						GPIOA->BSRR = 0x1 << 3;
					}
					else
					{
						//Reset an LED indicator here
						GPIOA->BRR = 0x1 << 3;
					}
					
					//battery charge over-current fault
					if ( ( CAN_rx_msg.data[5] >> 3 ) & 0x1 )
					{
						//Set LED indicator A4
						GPIOA->BSRR = 0x1 << 4;
					}
					else
					{
						//Reset LED indicator A4
						GPIOA->BRR = 0x1 << 4;
					}

					//battery over-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 7 ) & 0x1 )
					{
						//Set LED indicator A7
						GPIOA->BSRR = 0x1 << 7;
					}
					else
					{
						//Reset LED indicator A7
						GPIOA->BRR = 0x1 << 7;
					}
					
					//battery under-voltage fault
					if ( ( CAN_rx_msg.data[5] >> 6 ) & 0x1 )
					{
						//Set LED indicator A6
						GPIOA->BSRR = 0x1 << 6;
					}
					else
					{
						//Reset LED indicator A7
						GPIOA->BRR = 0x1 << 6;
					}
					
					break;
							
			}
			
	}
	
	//If a message can be transmitted,
	if (canTxReady)
	{
			CANSend(&CAN_tx_msg);
	}
	
	
	
	
}


 

