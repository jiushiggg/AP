#ifndef _FLASH_CMD_H_
#define _FLASH_CMD_H_
#if 0
#include "datatype.h"

// Return Message
typedef enum {
    FlashOperationSuccess,
    FlashWriteRegFailed,
    FlashTimeOut,
    FlashIsBusy,
    FlashQuadNotEnable,
    FlashAddressInvalid,
}ReturnMsg;

// Flash status structure define

/* Mode Register:
 * Bit  Description
 * -------------------------
 *  7   RYBY enable
 *  6   Reserved
 *  5   Reserved
 *  4   Reserved
 *  3   Reserved
 *  2   Reserved
 *  1   Parallel mode enable
 *  0   QPI mode enable
 */

typedef struct{
    UINT8  ModeReg;
    UINT8  ArrangeOpt;
} FlashStatus;

/*
  Flash ID, Information Define
  (The following information could get from device specification)
*/
#define    FlashID          0xc22015
#define		FlashID_GD		0xc84015
#define    ElectronicID     0x14
#define    RESID0           0xc214
#define    RESID1           0x14c2

UINT32 CMD_RDID(void);
UINT8 CMD_RES(void);
void CMD_REMS(UINT16 *REMS_Identification, FlashStatus *fsptr);
void CMD_RDSR(UINT8 *StatusReg);
ReturnMsg CMD_WRSR(UINT8 UpdateValue);
void CMD_RDSCUR(UINT8 *SecurityReg);
ReturnMsg CMD_WRSCUR(void);
ReturnMsg CMD_READ(UINT32 flash_address, UINT8 *target_address, UINT32 byte_length);
ReturnMsg CMD_DREAD(UINT32 flash_address, UINT8 *target_address, UINT32 byte_length);
ReturnMsg CMD_FASTREAD( UINT32 flash_address, UINT8 *target_address, UINT32 byte_length);
void CMD_WREN(void);
void CMD_WRDI(void);
ReturnMsg CMD_PP(UINT32 flash_address, UINT8 *source_address, UINT32 byte_length);
ReturnMsg CMD_SE(UINT32 flash_address);
ReturnMsg CMD_BE(UINT32 flash_address);
ReturnMsg CMD_CE(void);
void CMD_DP(void);
void CMD_RDP(void);
void CMD_ENSO(void);
void CMD_EXSO(void);
#endif
#endif

