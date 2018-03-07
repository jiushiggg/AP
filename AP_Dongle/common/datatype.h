/*
** datatype.h
**
** 201205301048 GS create
**
*/


#ifndef COMMON_DATATYPE_H_
#define COMMON_DATATYPE_H_

//#include "ti/sysbios/knl/Mailbox.h"
typedef unsigned char       UINT8;
typedef signed char         INT8;
typedef unsigned short      UINT16;
typedef signed short        INT16;
typedef unsigned int        UINT32;
typedef signed int          INT32;

typedef unsigned long int        WORD;

typedef unsigned char       BOOL;
#define TRUE                1
#define FALSE               0

#ifndef NULL
#define NULL                ((void *)0)
#endif


//typedef struct MsgObj {
//    short unsigned int     id;
//    char    val;
//} MsgObj;
//
//
//typedef struct MailboxMsgObj {
//    Mailbox_MbxElem  elem;      /* Mailbox header        */
//    MsgObj           obj;       /* Application's mailbox */
//} MailboxMsgObj;
#define COREMEM_SIZE    9000//(1024*14)
extern UINT8 coremem[COREMEM_SIZE];
#endif /* COMMON_DATATYPE_H_ */
