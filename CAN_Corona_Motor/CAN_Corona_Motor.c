/* UBC Solar Corona Motor/Motor Controller Code
	Date: 1/13/18
	Purpose:
	- To monitor and/or control the speed, torque, input voltage, etc., of the M2096D-III motor via the pins on hte M2096C motor controller and other hardware
	- To communicate data collected to the driver, pit crew, and the rest of the electrical system in the car via CAN Bus
	Inputs:
	-
	Outputs:
	-
*/

#include <stdio.h>
#include <stdlib.h>
#include <ubcsolar_can_ids.h>
#include <SPI.h>
#include <mcp_can.h>
#include <mcp_can_dfs.h>

#define CAN_SS 9			// Replace with actual value later
#define BUS_SPEED CAN_125KBPS

MCP_CAN CAN(CAN_SS);

int canSSOffset = 0;

void CAN_setup (void){
	// Initialize CAN bus serial communications
	CAN_INIT: 
		if (CAN_OK == CAN.begin(BUS_SPEED)) {
			printf("CAN BUS Shield init okay\n");
		}
		else {
			printf("CAN BUS Shield init fail\n");
			printf("Init CAN BUS Shield again with SS pin\n");
			printf(CAN_SS + canSSOffset);
		
			delay(100);

			canSSOffset ^= 1;
			CAN = MCP_CAN(CAN_SS + canSSOffset);
			ljmp CAN_INIT;
		}
}

void main (void){
	CAN_setup();
}

