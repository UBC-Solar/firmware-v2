/**
 * Function implementations for enabling and using CAN messaging
 */

#include "CAN.h"
#include "virtual_com.h"

// For non-connectivity line MCU models, there are 14 filter banks, and
// each bank is configured to hold 4 IDs; 14 * 4 = 56
#define MAX_NUM_FILTERS 56

const CAN_bit_timing_config_t can_configs[6] = {
	{2, 13, 45}, // CAN_50KBPS
	{2, 15, 20}, // CAN_100KBPS
	{2, 13, 18}, // CAN_125KBPS
	{2, 13, 9},	 // CAN_250KBPS
	{2, 15, 4},	 // CAN_500KBPS
	{2, 15, 2}	 // CAN_1000KBP
};

/**
 * Initializes the CAN controller with specified bit rate
 *
 * @param bitrate Specified bitrate. If this value is not one of the defined constants, bit rate will be defaulted to 125KBS
 */
void CANInit(enum BITRATE bitrate)
{
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;			  // Enable AFIO clock
	AFIO->MAPR &= ~AFIO_MAPR_CAN_REMAP;			  // reset CAN remap
	AFIO->MAPR |= 0x2 << AFIO_MAPR_CAN_REMAP_Pos; // Set CAN remap, use PB8, PB9

	// Configure PB8 for CAN RX and PB9 for CAN TX
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN; // Enable GPIOB clock
	GPIOB->CRH &= ~(GPIO_CRH_CNF9 | GPIO_CRH_MODE9 | GPIO_CRH_CNF8 | GPIO_CRH_MODE8);
	GPIOB->CRH |= (0x2 << GPIO_CRH_CNF9_Pos) | (0x3 << GPIO_CRH_MODE9_Pos) | // Alternate function output push-pull, fastest speed
				  (0x2 << GPIO_CRH_CNF8_Pos) | (0x0 << GPIO_CRH_MODE8_Pos);	 // Input

	RCC->APB1ENR |= RCC_APB1ENR_CAN1EN; // Enable CAN clock

	// Enable auto bus management, disable automatic retransmission, and set CAN to initialization mode
	// Note: SLEEP and DBF (debug freeze) bits are also cleared
	CAN1->MCR = CAN_MCR_ABOM | CAN_MCR_NART | CAN_MCR_INRQ;

	// Set bit rates
	CAN1->BTR &= ~(CAN_BTR_SJW | CAN_BTR_TS2 | CAN_BTR_TS1 | CAN_BTR_BRP);
	CAN1->BTR |= (((can_configs[bitrate].TS2 - 1) & 0x7) << CAN_BTR_TS2_Pos) |	// Time segment 2
				 (((can_configs[bitrate].TS1 - 1) & 0xF) << CAN_BTR_TS1_Pos) |	// Time segment 1
				 (((can_configs[bitrate].BRP - 1) & 0x1FF) << CAN_BTR_BRP_Pos); // Baud rate prescaler

	// Configure  default values
	CAN1->FMR |= CAN_FMR_FINIT; // Set to filter initialization mode
	CAN1->FMR |= 0x1C << CAN_FMR_CAN2SB_Pos; // Assign all filters to CAN1
	CAN1->FA1R &= ~CAN_FA1R_FACT0;			 // Deactivate filter 0
	CAN1->FS1R |= CAN_FS1R_FSC0;			 // Set filter 0 to single 32-bit scale configuration

	// Set filter bank 0 filters to 0
	CAN1->sFilterRegister[0].FR1 = 0x0UL;
	CAN1->sFilterRegister[0].FR2 = 0x0UL;

	CAN1->FM1R &= ~CAN_FM1R_FBM0; // Set filter to mask mode

	CAN1->FFA1R &= ~CAN_FFA1R_FFA0; // Apply filter to FIFO 0
	CAN1->FA1R |= CAN_FA1R_FACT0;	// Activate filter 0

	CAN1->FMR &= ~CAN_FMR_FINIT; // Exit filter initialization mode
	CAN1->MCR &= ~CAN_MCR_INRQ;	 // Set CAN to normal mode

	while (CAN1->MSR & 0x1UL); // Wait for CAN to enter normal mode
}

/**
 * Configures the next available CAN receive filter to pass the given ID
 * 
 * @param id The ID to accept
 * 
 * @returns 0 if successful, -1 if all of the filters are in use
 */
int CANSetFilter(uint16_t id)
{
	static uint32_t filterID = 0;

	if (filterID == MAX_NUM_FILTERS)
	{
		return -1;
	}

	CAN1->FMR |= CAN_FMR_FINIT; // Enter filter initialization mode

	// Configure the next available filter bank
	// Note that the leftshifts by 5 and 21 are to align the 11 bit ID to the highest 11 bits of each 16 bit word
	// Once a filter bank is initialized, no slot should be left empty (implies an open filter for ID 0)
	switch (filterID % 4)
	{
	case 0:
		// if we need another filter bank, initialize it
		CAN1->FA1R |= 0x1UL << (filterID / 4);    // Activate filter
		CAN1->FM1R |= 0x1UL << (filterID / 4);    // Put filter in identifier list mode
		CAN1->FS1R &= ~(0x1UL << (filterID / 4)); // Put filter in dual 16-bit scale configuration

		// Assign the new ID to all 4 slots in bank
		CAN1->sFilterRegister[filterID / 4].FR1 = (id << 5) | (id << 21);
		CAN1->sFilterRegister[filterID / 4].FR2 = (id << 5) | (id << 21);
		break;
	case 1:
		// Write the new ID to 2nd slot
		CAN1->sFilterRegister[filterID / 4].FR1 &= 0x0000FFFF;
		CAN1->sFilterRegister[filterID / 4].FR1 |= id << 21;
		break;
	case 2:
		// Write the new ID to 3rd and 4th slots
		CAN1->sFilterRegister[filterID / 4].FR2 = (id << 5) | (id << 21);
		break;
	case 3:
		// Write the new ID to the 4th slot
		CAN1->sFilterRegister[filterID / 4].FR2 &= 0x0000FFFF;
		CAN1->sFilterRegister[filterID / 4].FR2 |= id << 21;
		break;
	}
	filterID++;
	CAN1->FMR &= ~CAN_FMR_FINIT; // Exit filter initialization mode

	return 0;
}

/**
 * Configures the CAN receive filters to pass the given IDs
 * 
 * This function should only be called once
 * 
 * @param ids Array of IDs to accept
 * @param num Size of ids array
 * 
 * @returns 0 if successful, -1 if too many IDs are given, -2 if function has been called before
 */
int CANSetFilters(uint16_t *ids, uint8_t num)
{
	static int filtersConfigured = 0;

	// Only run once
	if (filtersConfigured) return -2;
	filtersConfigured = 1;

	if (num > MAX_NUM_FILTERS) return -1;

	for (int i = 0; i < num; i++)
	{
		CANSetFilter(ids[i]);
	}

	return 0;
}

/**
 * Decodes CAN messages from the data registers and populates a
 * CAN message struct with the data fields
 *
 * @preconditions A valid CAN message is received
 * @param CAN_rx_msg CAN message struct that will be populated
 */
void CANReceive(CAN_msg_t *CAN_rx_msg)
{
	CAN_rx_msg->id = (CAN1->sFIFOMailBox[0].RIR >> CAN_RI0R_STID_Pos) & 0x7FFUL; // Get 11-bit standard msg ID
	CAN_rx_msg->len = CAN1->sFIFOMailBox[0].RDTR & 0xFUL;						 // Get 4-bit msg length

	CAN_rx_msg->data[0] = 0xFFUL & CAN1->sFIFOMailBox[0].RDLR;
	CAN_rx_msg->data[1] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 8);
	CAN_rx_msg->data[2] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 16);
	CAN_rx_msg->data[3] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDLR >> 24);
	CAN_rx_msg->data[4] = 0xFFUL & CAN1->sFIFOMailBox[0].RDHR;
	CAN_rx_msg->data[5] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 8);
	CAN_rx_msg->data[6] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 16);
	CAN_rx_msg->data[7] = 0xFFUL & (CAN1->sFIFOMailBox[0].RDHR >> 24);

	CAN1->RF0R |= CAN_RF0R_RFOM0; // Release the RX FIFO 0 output mailbox
}

/**
 * Encodes CAN messages using the CAN message struct and populates the
 * data registers with the sent
 *
 * @preconditions A valid CAN message is received
 * @param CAN_rx_msg CAN message struct that will be populated
 */
void CANSend(CAN_msg_t *CAN_tx_msg)
{
	volatile int count = 0;

	CAN1->sTxMailBox[0].TIR = (CAN_tx_msg->id) << CAN_TI0R_STID_Pos; // Set msg ID

	// Set msg length
	CAN1->sTxMailBox[0].TDTR &= ~CAN_TDT0R_DLC;
	CAN1->sTxMailBox[0].TDTR |= CAN_tx_msg->len & 0xF;

	CAN1->sTxMailBox[0].TDHR = (((uint32_t)CAN_tx_msg->data[7] << 24) |
								((uint32_t)CAN_tx_msg->data[6] << 16) |
								((uint32_t)CAN_tx_msg->data[5] << 8) |
								((uint32_t)CAN_tx_msg->data[4]));
	CAN1->sTxMailBox[0].TDLR = (((uint32_t)CAN_tx_msg->data[3] << 24) |
								((uint32_t)CAN_tx_msg->data[2] << 16) |
								((uint32_t)CAN_tx_msg->data[1] << 8) |
								((uint32_t)CAN_tx_msg->data[0]));

	CAN1->sTxMailBox[0].TIR |= CAN_TI0R_TXRQ; // Request transmission
	// Wait for transmit mailbox to be empty, or timeout
	while (CAN1->sTxMailBox[0].TIR & CAN_TI0R_TXRQ && count++ < 1000000);

	if (!(CAN1->sTxMailBox[0].TIR & CAN_TI0R_TXRQ)) // If transmit mailbox is empty (ie. transmission successful)
	{
		return;
	}

	// Sends error log to screen
	while (CAN1->sTxMailBox[0].TIR & CAN_TI0R_TXRQ) // While transmit mailbox is not empty
	{
		SendInt(CAN1->ESR);
		SendLine();
		SendInt(CAN1->MSR);
		SendLine();
		SendInt(CAN1->TSR);
		SendLine();
		SendLine();
	}
}

/**
 * Returns the number ofCAN messages available (pending in the RX FIFO)
 *
 * @returns Count of pending CAN messages in the RX FIFO (0-3)
 */
uint8_t CANMsgAvail(void)
{
	return (CAN1->RF0R & CAN_RF0R_RFOM0) >> CAN_RF0R_RFOM0_Pos;
}
