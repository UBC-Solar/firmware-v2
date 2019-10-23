/**
 * Interface with CAN drivers
 * Requires CSMIS RTOS to be enabled in project
 * Designed to be non-blocking and interrupt/event based
 */
#ifndef CAN_NEW_H
#define CAN_NEW_H

#include <string.h>

#include "stm32f10x.h"
#include "cmsis_os.h"
#include "RTE_Components.h"
#include "Driver_CAN.h"

typedef struct
{
	uint32_t id;
	uint8_t  data[8];
	uint8_t  len;
} CAN_Message;

/** 
 * Initializes the CAN Framework 
 */
void CAN_Initialize(void);

/**
 * Allows messages with specified filter to be received 
 * @Param allowed_id: message id to allow through filter 
 * @Returns: nothing
 */
void CAN_SetFilter(uint32_t allowed_id);

/** 
 * Allows messages with any the specified filters to be received
 * @Param allowed_ids: message ids to allow through filter
 * @Returns: nothing
 */
void CAN_SetFilters(uint32_t* allowed_ids, uint8_t length);

/** 
 * USE CAN_QueueMessage INSTEAD UNLESS ABSOLUTELY NECESSARY
 * Sends message with the specified data and id
 * Allows up to 8 bytes to be sent
 * @Param msg_tx: data to be transmitted
 * @Returns: nothing
 */
void CAN_SendMessage(CAN_Message* msg_tx);
 
/**
 * Yields thread execution until a CAN message has been received 
 * Function passed will be called with the received message 
 * Not Available to be called in ISR
 * @Param fp: callback function on message 
 * @Returns: nothing 
 */
void CAN_PerformOnMessage(void (*fp)(CAN_Message* msg));

/** 
 * Queues up CAN message to be sent when available 
 * Safe to be called within ISR 
 * @Param msg_tx: data to be transmitted
 * @Returns: nothing
 */
void CAN_QueueMessage(CAN_Message* msg_tx);

#endif //CAN_NEW_H
