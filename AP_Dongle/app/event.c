#include <ti/sysbios/knl/Event.h>
#include <ti/sysbios/knl/Semaphore.h>
#include <ti/sysbios/BIOS.h>
#include "event.h"

static Event_Handle protocol_eventHandle;
static Event_Struct protocol_eventStruct;

static Event_Handle communicateEventHandle;
static Event_Struct communicateEventStruct;


static Semaphore_Struct  recSemStruct;
static Semaphore_Handle  recSemHandle;


void Semphore_init(void)
{
    Semaphore_Params  recSemParam;
    Semaphore_Params_init(&recSemParam);
    recSemParam.mode = ti_sysbios_knl_Semaphore_Mode_BINARY;
    Semaphore_construct(&recSemStruct, 0, &recSemParam);
    recSemHandle = Semaphore_handle(&recSemStruct);
}
void Device_Recv_pend(UINT32 timeout)
{
    Semaphore_pend(recSemHandle, timeout);
}
void Device_Recv_post(void)
{
    Semaphore_post(recSemHandle);
}



void Event_init(void)
{
    Event_Params eventParams;
    Event_Params_init(&eventParams);

    Event_construct(&protocol_eventStruct, &eventParams);
    protocol_eventHandle = Event_handle(&protocol_eventStruct);


    Event_construct(&communicateEventStruct, &eventParams);
    communicateEventHandle = Event_handle(&communicateEventStruct);
}

UINT32 Event_Get(void)
{
    return Event_getPostedEvents(protocol_eventHandle);
}


UINT32 Event_communicateGet(void)
{
    return Event_getPostedEvents(communicateEventHandle);
}

void Event_Set(UINT32 event)
{
    Event_post(protocol_eventHandle, event);
}
void Event_communicateSet(UINT32 event)
{
    Event_post(communicateEventHandle, event);
}



void Event_Clear(UINT32 event)
{
//    event_flag ^= event;
}
void Event_communicateClear(UINT32 event)
{

}

UINT32 Event_GetStatus(void)
{
    return Event_getPostedEvents(protocol_eventHandle);
}
UINT32 Event_communicateGetStatus(void)
{
    return Event_getPostedEvents(communicateEventHandle);
}

UINT32 Event_PendCore(void)
{
    return Event_pend(protocol_eventHandle, 0, EVENT_ALL, BIOS_WAIT_FOREVER);

}

UINT32 Event_Pendcommunicate(void)
{
    return Event_pend(communicateEventHandle, 0, EVENT_ALL, BIOS_WAIT_FOREVER);
}
