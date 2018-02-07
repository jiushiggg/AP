#include "event.h"
#include <stdio.h>
#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/BIOS.h>
//static volatile UINT32 event_flag = 0;


static Event_Handle protocol_eventHandle;

UINT32 Event_Get(void)
{
    return Event_getPostedEvents(protocol_eventHandle);
}

void Event_Set(UINT32 event)
{
    Event_post(protocol_eventHandle, event);
}

void Event_Clear(UINT32 event)
{
//    event_flag ^= event;
}

void Event_Reset(void)
{
    Event_Params protocol_eventParam;
    Event_Struct protocol_eventStruct;

    Event_Params_init(&protocol_eventParam);
    Event_construct(&protocol_eventStruct, &protocol_eventParam);
    protocol_eventHandle = Event_handle(&protocol_eventStruct);
}

UINT32 Event_GetStatus(void)
{
    return Event_getPostedEvents(protocol_eventHandle);
}

UINT32 Event_PentCore(void)
{
    return Event_pend(protocol_eventHandle, 0, EVENT_ALL, BIOS_WAIT_FOREVER);
}
