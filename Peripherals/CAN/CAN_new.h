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

void CAN_Initialize(void);
void CAN_SetFilter(uint32_t allowed_id);
void CAN_SetFilters(uint32_t* allowed_ids, uint8_t length);
void CAN_PerformOnMessage(void (*fp)(CAN_Message* msg));
void CAN_SendMessage(CAN_Message* msg_tx);
void CAN_QueueMessage(CAN_Message* msg_tx);

#endif //CAN_NEW_H
