#include "CAN_new.h"

// CAN Interface configuration -------------------------------------------------

#define  CAN_RX_MAILBOX_SIZE    4
#define  CAN_TX_MAILBOX_SIZE    4
#define  CAN_TX_MAX_THREADS     1

#define  CAN_CONTROLLER         1       // CAN Controller number (Use 1 or 2)
#define  CAN_LOOPBACK           1       // 0 = no loopback (normal), 1 = loopback (tx linked to rx)
#define  CAN_BITRATE_NOMINAL    500000  // Nominal bitrate (bit/s)

//------------------------------------------------------------------------------
// defines the pointer to the access struct of the CAN driver
#define _CAN_Driver_(n)         Driver_CAN##n
#define  CAN_Driver_(n)        _CAN_Driver_(n)
extern   ARM_DRIVER_CAN         CAN_Driver_(CAN_CONTROLLER);
#define  ptrCAN               (&CAN_Driver_(CAN_CONTROLLER))

// Receiver object to intilize driver
uint32_t                        rx_obj_idx = 0xFFFFFFFFU;
ARM_CAN_MSG_INFO                rx_msg_info;

// Mailbox for non-blocking operaiton 
osMailQDef(CAN_RXMailbox, CAN_RX_MAILBOX_SIZE, CAN_Message);
osMailQId CAN_RXMailbox_id; 

// Transmission object to intilize driver
uint32_t                        tx_obj_idx = 0xFFFFFFFFU;
ARM_CAN_MSG_INFO                tx_msg_info;

// Mailbox for non-blocking operation 
osMailQDef(CAN_TXMailbox, CAN_TX_MAILBOX_SIZE, CAN_Message);
osMailQId CAN_TXMailbox_id; 

// Thread for transmission processing 
void CAN_TXThread(void const *argument);

osThreadDef(CAN_TXThread, osPriorityAboveNormal, CAN_TX_MAX_THREADS, 0);
osThreadId CAN_TXThread_id;


enum CAN_Error
{
	DRIVER_INIT_FAIL,
	POWER_UP_FAIL, 
	FILTER_SET_FAIL,
	INIT_MODE_FAIL,
	NORMAL_MODE_FAIL,
	UNSUPPORTED_BIT_RATE_FAIL, 
	BIT_RATE_SET_FAIL,
	OBJECT_ALLOC_FAIL,
	OBJECT_CONFIG_FAIL, 
	RX_UNSUPPORTED_FAIL,
	RX_MAILBOX_CREATE_FAIL,
	RX_MESSAGE_QUEUE_FAIL,
	TX_UNSUPPORTED_FAIL,
	TX_THREAD_CREATE_FAIL,
	TX_MAILBOX_CREATE_FAIL,
	TX_MESSAGE_QUEUE_FAIL,
	TX_SEND_FAIL,
	LOOPBACK_UNSUPPORTED_FAIL, 
	LOOPBACK_SET_FAIL,
	UNKNOWN_SIGNAL_UNIT_EVENT,
	UNKNOWN_SIGNAL_OBJECT_EVENT,
	UNKNOWN
};


/** 
 * Error handler for debugging, remove if unnecessary
 * Check stack to see error message
 * @Param error: error message corresponding with fail 
 * @Returns: nothing 
 */
static void Error_Handler(enum CAN_Error error)
{
	switch (error)
	{
		case DRIVER_INIT_FAIL:
			while (1) ;
		case POWER_UP_FAIL:
			while (1) ;
		case FILTER_SET_FAIL:
			while (1) ;
		case INIT_MODE_FAIL:
			while (1) ;
		case NORMAL_MODE_FAIL:
			while (1) ;
		case UNSUPPORTED_BIT_RATE_FAIL: 
			while (1) ;
		case BIT_RATE_SET_FAIL:
			while (1) ;
		case OBJECT_ALLOC_FAIL:
			while (1) ;
		case OBJECT_CONFIG_FAIL: 
			while (1) ;
		case RX_UNSUPPORTED_FAIL:
			while (1) ;
		case RX_MAILBOX_CREATE_FAIL: 
			while (1) ;
		case RX_MESSAGE_QUEUE_FAIL:
			while (1) ;
		case TX_UNSUPPORTED_FAIL:
			while (1) ;
		case TX_THREAD_CREATE_FAIL:
			while (1) ; 
		case TX_MAILBOX_CREATE_FAIL:
			while (1) ;
		case TX_MESSAGE_QUEUE_FAIL:
			while (1) ;
		case TX_SEND_FAIL:
			while (1) ;
		case LOOPBACK_UNSUPPORTED_FAIL: 
			while (1) ;
		case LOOPBACK_SET_FAIL:
			while (1) ;
		case UNKNOWN_SIGNAL_UNIT_EVENT:
			while (1) ;
		case UNKNOWN_SIGNAL_OBJECT_EVENT:
			while (1) ;
		case UNKNOWN:
			while (1) ;
		default: 
			while (1) ;
	}
}

/**
 * DO NOT CHANGE THE FUNCTION NAME - function name is internally registered ISR callback
 * Callback evoked when the CAN driver changes states 
 * Handlers for state changes should be placed within this callback
 * 
 * @Param event: unit event that occurred during driver operation
 *     Possible Events: ARM_CAN_EVENT_UNIT_INACTIVE - Unit entered Inactive state
 *                      ARM_CAN_EVENT_UNIT_ACTIVE   - Unit entered Error Active state
 *                      ARM_CAN_EVENT_UNIT_WARNING  - Unit entered Error Warning state (one or both error counters >= 96)
 *                      ARM_CAN_EVENT_UNIT_PASSIVE  - Unit entered Error Passive state
 *                      ARM_CAN_EVENT_UNIT_BUS_OFF  - Unit entered Bus-off state
 * @Returns: nothing
 */
void CAN_SignalUnitEvent(uint32_t event)
{
  switch (event)
	{
		case ARM_CAN_EVENT_UNIT_INACTIVE: 
			break; 
		
		case ARM_CAN_EVENT_UNIT_ACTIVE:
			break;
		
		case ARM_CAN_EVENT_UNIT_WARNING:
			break;
		
		case ARM_CAN_EVENT_UNIT_PASSIVE:
			break;
		
		case ARM_CAN_EVENT_UNIT_BUS_OFF:
			break;
		
		default: 
			Error_Handler(UNKNOWN_SIGNAL_UNIT_EVENT);
  }
}

/**
 * DO NOT CHANGE THE FUNCTION NAME - function name is internally registered ISR callback
 * Callback evoked when any registered CAN object has an event
 * Handlers for different events should be placed within this callback
 * 
 * @Param obj_idx: index of the message object
 * @Param event: object event that occurred during driver operation
 *     Possible Events: ARM_CAN_EVENT_SEND_COMPLETE   - Message was sent successfully
 *                      ARM_CAN_EVENT_RECEIVE         - Message was received successfully
 *                      ARM_CAN_EVENT_RECEIVE_OVERRUN - Message was overwritten before it was read
 *
 * @Returns: nothing
 */
void CAN_SignalObjectEvent(uint32_t obj_idx, uint32_t event)
{
	CAN_Message* msg_rx;
	uint8_t msg_len; 

	switch (event)
	{
		case ARM_CAN_EVENT_SEND_COMPLETE: 
			break; 
		
		case ARM_CAN_EVENT_RECEIVE:
		{
			if (obj_idx == rx_obj_idx)
			{
				// This will not wait for slot to free up as it runs in ISR
				msg_rx = (CAN_Message*)osMailAlloc(CAN_RXMailbox_id, 0);
				if (!msg_rx)
				{
					Error_Handler(RX_MESSAGE_QUEUE_FAIL);
				}
				
				memset(msg_rx, 0U, 8U); 
				msg_len = ptrCAN->MessageRead(rx_obj_idx, &rx_msg_info, msg_rx->data, 8U);
				msg_rx->len = msg_len; 
				msg_rx->id = rx_msg_info.id;
				
				if (osMailPut(CAN_RXMailbox_id, msg_rx) != osOK)
				{
					Error_Handler(RX_MESSAGE_QUEUE_FAIL);
				}
			}
		}
		break; 
		
		case ARM_CAN_EVENT_RECEIVE_OVERRUN: 
			break; 
		
		default: 
			Error_Handler(UNKNOWN_SIGNAL_OBJECT_EVENT);
	}
}

/** 
 * Initializes the CAN Framework 
 */ 
void CAN_Initialize(void) 
{
	ARM_CAN_CAPABILITIES     can_cap;
	ARM_CAN_OBJ_CAPABILITIES can_obj_cap;
	uint32_t                 count;
	uint32_t                 num_objects;
	uint32_t                 clock; 
	int32_t                  status;
	
	CAN_RXMailbox_id = osMailCreate(osMailQ(CAN_RXMailbox), NULL);
	if (!CAN_RXMailbox_id) Error_Handler(RX_MAILBOX_CREATE_FAIL);

	CAN_TXMailbox_id = osMailCreate(osMailQ(CAN_TXMailbox), NULL);
	if (!CAN_TXMailbox_id) Error_Handler(TX_MAILBOX_CREATE_FAIL); 
	
	CAN_TXThread_id = osThreadCreate(osThread(CAN_TXThread), NULL);
	if (!CAN_TXThread_id) Error_Handler(TX_THREAD_CREATE_FAIL);
	
	can_cap = ptrCAN->GetCapabilities();                                          // Get CAN driver capabilities
	num_objects = can_cap.num_objects;                                            // Number of receive/transmit objects
	
	// Initialization
	status = ptrCAN->Initialize(CAN_SignalUnitEvent, CAN_SignalObjectEvent);
	if (status != ARM_DRIVER_OK) Error_Handler(DRIVER_INIT_FAIL); 

	// Power up controller
	status = ptrCAN->PowerControl(ARM_POWER_FULL);
	if (status != ARM_DRIVER_OK) Error_Handler(POWER_UP_FAIL); 

	// Set mode to initialization
	status = ptrCAN->SetMode(ARM_CAN_MODE_INITIALIZATION);
	if (status != ARM_DRIVER_OK) Error_Handler(INIT_MODE_FAIL); 

	// Set CAN bit rate
	clock = ptrCAN->GetClock();                                                   // Get CAN base clock
	if ((clock % (8U*CAN_BITRATE_NOMINAL)) == 0U)                                 // If CAN base clock is divisible by 8 * nominal bitrate without remainder
	{
		status = ptrCAN->SetBitrate    (ARM_CAN_BITRATE_NOMINAL,                    // Set nominal bitrate
                                    CAN_BITRATE_NOMINAL,                        // Set nominal bitrate to configured constant value
                                    ARM_CAN_BIT_PROP_SEG  (5U) |                // Set propagation segment to 5 time quanta
                                    ARM_CAN_BIT_PHASE_SEG1(1U) |                // Set phase segment 1 to 1 time quantum (sample point at 87.5% of bit time)
                                    ARM_CAN_BIT_PHASE_SEG2(1U) |                // Set phase segment 2 to 1 time quantum (total bit is 8 time quanta long)
                                    ARM_CAN_BIT_SJW       (1U));                // Resynchronization jump width is same as phase segment 2
	}
	else if ((clock % (10U*CAN_BITRATE_NOMINAL)) == 0U)                           // If CAN base clock is divisible by 10 * nominal bitrate without remainder
	{
		status = ptrCAN->SetBitrate    (ARM_CAN_BITRATE_NOMINAL,                    // Set nominal bitrate
                                    CAN_BITRATE_NOMINAL,                        // Set nominal bitrate to configured constant value
                                    ARM_CAN_BIT_PROP_SEG  (7U) |                // Set propagation segment to 7 time quanta
                                    ARM_CAN_BIT_PHASE_SEG1(1U) |                // Set phase segment 1 to 1 time quantum (sample point at 90% of bit time)
                                    ARM_CAN_BIT_PHASE_SEG2(1U) |                // Set phase segment 2 to 1 time quantum (total bit is 10 time quanta long)
                                    ARM_CAN_BIT_SJW       (1U));                // Resynchronization jump width is same as phase segment 2
	}
	else if ((clock % (12U*CAN_BITRATE_NOMINAL)) == 0U)                           // If CAN base clock is divisible by 12 * nominal bitrate without remainder
	{
		status = ptrCAN->SetBitrate    (ARM_CAN_BITRATE_NOMINAL,                    // Set nominal bitrate
                                    CAN_BITRATE_NOMINAL,                        // Set nominal bitrate to configured constant value
                                    ARM_CAN_BIT_PROP_SEG  (7U) |                // Set propagation segment to 7 time quanta
                                    ARM_CAN_BIT_PHASE_SEG1(2U) |                // Set phase segment 1 to 2 time quantum (sample point at 83.3% of bit time)
                                    ARM_CAN_BIT_PHASE_SEG2(2U) |                // Set phase segment 2 to 2 time quantum (total bit is 12 time quanta long)
                                    ARM_CAN_BIT_SJW       (2U));                // Resynchronization jump width is same as phase segment 2
	}
	else Error_Handler(UNSUPPORTED_BIT_RATE_FAIL); 
	if (status != ARM_DRIVER_OK) Error_Handler(BIT_RATE_SET_FAIL);

	// Reserve RX/TX Objects
	for (count = 0U; count < num_objects; ++count)
	{                                                                             // Find first available object for receive and transmit
		can_obj_cap = ptrCAN->ObjectGetCapabilities(count);                         // Get object capabilities
		
		if ((rx_obj_idx == 0xFFFFFFFFU) && (can_obj_cap.rx == 1U))
		{ 
			rx_obj_idx = count;
		}
		
		else if ((tx_obj_idx == 0xFFFFFFFFU) && (can_obj_cap.tx == 1U))
		{ 
			tx_obj_idx = count;
			break;
		}
	}
	
	// Configure receive object
	if (rx_obj_idx == 0xFFFFFFFFU) Error_Handler(RX_UNSUPPORTED_FAIL);
	status = ptrCAN->ObjectConfigure(rx_obj_idx, ARM_CAN_OBJ_RX);
	if (status != ARM_DRIVER_OK) Error_Handler(OBJECT_CONFIG_FAIL);
	
	// Configure transmit object
	if (tx_obj_idx == 0xFFFFFFFFU) Error_Handler(TX_UNSUPPORTED_FAIL);; 
	status = ptrCAN->ObjectConfigure(tx_obj_idx, ARM_CAN_OBJ_TX);
	if (status != ARM_DRIVER_OK) Error_Handler(OBJECT_CONFIG_FAIL);
	
#if CAN_LOOPBACK
	// Loopback mode used for testing
	if (can_cap.external_loopback != 1U) Error_Handler(LOOPBACK_UNSUPPORTED_FAIL);
	status = ptrCAN->SetMode (ARM_CAN_MODE_LOOPBACK_EXTERNAL);
	if (status != ARM_DRIVER_OK) Error_Handler(LOOPBACK_SET_FAIL);
#else
	// Activate normal operation mode
	status = ptrCAN->SetMode (ARM_CAN_MODE_NORMAL);                              
	if (status != ARM_DRIVER_OK) Error_Handler(NORMAL_MODE_FAIL);
#endif //CAN_LOOPBACK
}

/**
 * Allows messages with specified filter to be received
 * Filter set will be a 11-bit standard ID filter 
 * @Param allowed_id: message id to allow through filter 
 * @Returns: nothing
 */
void CAN_SetFilter(uint32_t allowed_id)
{
	// Uses Standard 11-bit ID to match with the old CAN driver, can change in future if necessary
	if (ptrCAN->ObjectSetFilter(rx_obj_idx, ARM_CAN_FILTER_ID_EXACT_ADD, ARM_CAN_STANDARD_ID(allowed_id), 0) != ARM_DRIVER_OK)
	{
		Error_Handler(FILTER_SET_FAIL);
	}
}

/** 
 * Allows messages with any the specified filters to be received
 * Filters set will be 11-bit standard ID filters
 * @Param allowed_ids: message ids to allow through filter
 * @Returns: nothing
 */
void CAN_SetFilters(uint32_t* allowed_ids, uint8_t length)
{
	for (uint8_t i = 0; i < length; ++i)
	{
		CAN_SetFilter(allowed_ids[i]);
	}
}

/**
 * Yields thread execution until a CAN message has been received 
 * Function passed will be called with the received message 
 * Not Available to be called in ISR
 * @Param fp: callback function on message 
 * @Returns: nothing 
 */
void CAN_PerformOnMessage(void (*fp)(CAN_Message* msg))
{
	osEvent      evt;
	CAN_Message* msg_rx;
	
	evt = osMailGet(CAN_RXMailbox_id, osWaitForever);
	if (evt.status == osEventMail)
	{
		msg_rx = (CAN_Message*)evt.value.p;
		fp(msg_rx);
		osMailFree(CAN_RXMailbox_id, msg_rx);
	}
}

/** 
 * USE CAN_QueueMessage INSTEAD UNLESS ABSOLUTELY NECESSARY
 * Sends message with the specified data and id
 * Allows up to 8 bytes to be sent
 * @Param msg_tx: data to be transmitted
 * @Returns: nothing
 */
void CAN_SendMessage(CAN_Message* msg_tx)
{
	// Set message send parameters here in tx_msg_info struct
	memset(&tx_msg_info, 0U, sizeof(ARM_CAN_MSG_INFO));
	tx_msg_info.id = ARM_CAN_STANDARD_ID(msg_tx->id);
	if (ptrCAN->MessageSend(tx_obj_idx, &tx_msg_info, msg_tx->data, msg_tx->len) != msg_tx->len)
	{
		Error_Handler(TX_SEND_FAIL);
	}
}

/** 
 * Queues up CAN message to be sent when available 
 * Safe to be called within ISR 
 * @Param msg_tx: data to be transmitted
 * @Returns: nothing
 */
void CAN_QueueMessage(CAN_Message* msg_tx)
{
	CAN_Message* msg_tx_copy; 

	msg_tx_copy = (CAN_Message*)osMailAlloc(CAN_TXMailbox_id, 0U);
	if (!msg_tx_copy)
	{
		Error_Handler(TX_MESSAGE_QUEUE_FAIL);
	}
	
	msg_tx_copy->id = msg_tx->id; 
	msg_tx_copy->len = msg_tx->len;
	memset(msg_tx_copy->data, 0U, 8U);
	memcpy(msg_tx_copy->data, msg_tx->data, msg_tx_copy->len);  
	
	if (osMailPut(CAN_TXMailbox_id, msg_tx_copy) != osOK) 
	{
		Error_Handler(TX_MESSAGE_QUEUE_FAIL);
	}
}

/**
 * Waits until a message is present in TX mailbox and sends it after
 * @TODO: implement message retries and mailbox timeouts 
 * 
 */
void CAN_TXThread(void const *argument)
{
	osEvent      evt;
	CAN_Message* msg_tx;
	
	while (1)
	{
		evt = osMailGet(CAN_TXMailbox_id, osWaitForever);
		if (evt.status == osEventMail)
		{
			msg_tx = (CAN_Message*)evt.value.p;
			CAN_SendMessage(msg_tx);
			osMailFree(CAN_TXMailbox_id, msg_tx);
		}
  }
}

