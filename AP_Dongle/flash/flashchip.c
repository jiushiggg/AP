/*
**  flash_cmd.c
** 
** 20120605 GS create
**  
**
**
*/

#include "flashchip.h"
#include "flashchipif.h"

// Flash control register mask define
// status register
#define    FLASH_WIP_MASK         0x01
#define    FLASH_LDSO_MASK        0x02
#define    FLASH_QE_MASK          0x40
// security register
#define    FLASH_OTPLOCK_MASK     0x03
#define    FLASH_4BYTE_MASK       0x04
#define    FLASH_WPSEL_MASK       0x80
// other
#define    BLOCK_PROTECT_MASK     0xff
#define    BLOCK_LOCK_MASK        0x01


/*--- insert your MCU information ---*/
#define    CLK_PERIOD                 14  // unit: ns
#define    Min_Cycle_Per_Inst         1  // cycle count of one instruction
#define    One_Loop_Inst              3  // instruction count of one loop (estimate)

/*
  Flash Timing Information Define
  (The following information could get from device specification)
*/
#define    FlashSize        0x400000       // 4 MB
#define    CE_period        26041667       // tCE /  ( CLK_PERIOD * Min_Cycle_Per_Inst *One_Loop_Inst)
#define    tW               100000000      // 100ms
#define    tDP              10000          // 10us
#define    tBP              300000         // 300us
#define    tPP              5000000        // 5ms
#define    tSE              300000000      // 300ms
#define    tBE              2000000000     // 2s
#define    tVSL             200000         // 200us
#define    tPUW             10000000       // 10ms
#define    tWSR             tBP

// Flash information define
#define    WriteStatusRegCycleTime     tW / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    PageProgramCycleTime        tPP / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    SectorEraseCycleTime        tSE / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    BlockEraseCycleTime         tBE / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    ChipEraseCycleTime          CE_period
#define    FlashFullAccessTime         tPUW / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    ByteProgramCycleTime        tBP / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)
#define    WriteSecuRegCycleTime       tWSR / (CLK_PERIOD * Min_Cycle_Per_Inst * One_Loop_Inst)


/*** MX25 series command hex code definition ***/
//ID comands
#define    FLASH_CMD_RDID      0x9F    //RDID (Read Identification)
#define    FLASH_CMD_RES       0xAB    //RES (Read Electronic ID)
#define    FLASH_CMD_REMS      0x90    //REMS (Read Electronic & Device ID)

//Register comands
#define    FLASH_CMD_WRSR      0x01    //WRSR (Write Status Register)
#define    FLASH_CMD_RDSR      0x05    //RDSR (Read Status Register)
#define    FLASH_CMD_WRSCUR    0x2F    //WRSCUR (Write Security Register)
#define    FLASH_CMD_RDSCUR    0x2B    //RDSCUR (Read Security Register)

//READ comands
#define    FLASH_CMD_READ        0x03    //READ (1 x I/O)
#define    FLASH_CMD_FASTREAD    0x0B    //FAST READ (Fast read data)
#define    FLASH_CMD_DREAD       0x3B    //DREAD (1In/2 Out fast read)

//Program comands
#define    FLASH_CMD_WREN     0x06    //WREN (Write Enable)
#define    FLASH_CMD_WRDI     0x04    //WRDI (Write Disable)
#define    FLASH_CMD_PP       0x02    //PP (page program)

//Erase comands
#define    FLASH_CMD_SE       0x20    //SE (Sector Erase)
#define    FLASH_CMD_BE       0xD8    //BE (Block Erase)
#define    FLASH_CMD_CE       0x60    //CE (Chip Erase) hex code: 60 or C7

//Mode setting comands
#define    FLASH_CMD_DP       0xB9    //DP (Deep Power Down)
#define    FLASH_CMD_RDP      0xAB    //RDP (Release form Deep Power Down)
#define    FLASH_CMD_ENSO     0xB1    //ENSO (Enter Secured OTP)
#define    FLASH_CMD_EXSO     0xC1    //EXSO  (Exit Secured OTP)

/*
** basic fucntions
*/
void InsertDummyCycle(unsigned char dummy_cycle)
{
	unsigned char i;

	for(i = 0; i < dummy_cycle/8; i++)
	{
		FCI_SendByte(0);
	}
}

/*
** utility fucntions
*/
void WaitFlashWarmUp(void)
{
	unsigned int time_cnt = FlashFullAccessTime;

	while(time_cnt > 0)
    {
        time_cnt--;
    }
}

unsigned char IsFlashBusy(void)
{
    unsigned char  gDataBuffer;

    CMD_RDSR(&gDataBuffer);

    if((gDataBuffer & FLASH_WIP_MASK)  == FLASH_WIP_MASK)
	{
        return 1;
    }
	else
	{
        return 0;
	}
}

unsigned char WaitFlashReady(unsigned int ExpectTime)
{
    unsigned int temp = 0;
	
    while(IsFlashBusy())
    {
        if(temp > ExpectTime)
        {
            return 0;
        }
        temp++;
    }

	return 1;
}

void SendFlashAddr(unsigned int flash_address)
{
    unsigned char tmp0, tmp1, tmp2;
	unsigned int tmp;

	tmp = flash_address;
	tmp0 = tmp;
	tmp1 = tmp >> 8;
	tmp2 = tmp >> 16;
	
    FCI_SendByte(tmp2);
    FCI_SendByte(tmp1);
    FCI_SendByte(tmp0);
}

/*
**  cmd functions
*/

/*
 * Arguments:      Identification, 32 bit buffer to store id
 * Description:    The RDID instruction is to read the manufacturer ID
 *                 of 1-byte and followed by Device ID of 2-byte.
 */
unsigned int CMD_RDID(void)
{
    unsigned int id = 0;
    unsigned int temp = 0;
    unsigned char  gDataBuffer[3];

    // Chip select go low to start a flash command
    FCI_Enable();

    // Send command
    FCI_SendByte(FLASH_CMD_RDID);

    // Get manufacturer identification, device identification
    gDataBuffer[0] = FCI_ReadByte();
    gDataBuffer[1] = FCI_ReadByte();
    gDataBuffer[2] = FCI_ReadByte();

    // Chip select go high to end a command
    FCI_Disable();

    // Store identification
    temp =  gDataBuffer[0];
    temp =  (temp << 8) | gDataBuffer[1];
    id =  (temp << 8) | gDataBuffer[2];

	return id;
}

/*
 * Arguments:      ElectricIdentification, 8 bit buffer to store electric id
 * Description:    The RES instruction is to read the Device
 *                 electric identification of 1-byte.
 */
unsigned char CMD_RES(void)
{
	unsigned char eid = 0;

    // Chip select go low to start a flash command
    FCI_Enable();

    // Send flash command and insert dummy cycle
    FCI_SendByte(FLASH_CMD_RES);
    InsertDummyCycle(24);

    // Get electric identification
    eid = FCI_ReadByte();

    // Chip select go high to end a flash command
    FCI_Disable();

	return eid;
}

/*
 * Arguments:      REMS_Identification, 16 bit buffer to store id
 *                 fsptr, pointer of flash status structure
 * Description:    The REMS instruction is to read the Device
 *                 manufacturer ID and electric ID of 1-byte.
 */
void CMD_REMS(unsigned short *REMS_Identification, FlashStatus *fsptr )
{
    unsigned char  gDataBuffer[2] = {0};

    // Chip select go low to start a flash command
    FCI_Enable();

    // Send flash command and insert dummy cycle ( if need )
    // ArrangeOpt = 0x00 will output the manufacturer's ID first
    //            = 0x01 will output electric ID first
    FCI_SendByte(FLASH_CMD_REMS);
    InsertDummyCycle(16);
    FCI_SendByte(fsptr->ArrangeOpt);

    // Get ID
    gDataBuffer[0] = FCI_ReadByte();
    gDataBuffer[1] = FCI_ReadByte();

    // Store identification informaion
    *REMS_Identification = (gDataBuffer[0] << 8) | gDataBuffer[1];

    // Chip select go high to end a flash command
    FCI_Disable();
}


/*
 * Register  Command
 */

/*
 * Arguments:      StatusReg, 8 bit buffer to store status register value
 * Description:    The RDSR instruction is for reading Status Register Bits.
 */
void CMD_RDSR(unsigned char *StatusReg)
{
    unsigned char  DataBuffer;

    // Chip select go low to start a flash command
    FCI_Enable();

    // Send command
    FCI_SendByte(FLASH_CMD_RDSR);
    DataBuffer = FCI_ReadByte();

    // Chip select go high to end a flash command
    FCI_Disable();

    *StatusReg = DataBuffer;
}

/*
 * Function:       CMD_WRSR
 * Arguments:      UpdateValue, 8 bit status register value to updata
 * Description:    The WRSR instruction is for changing the values of
 *                 Status Register Bits
 * Return Message: FlashIsBusy, FlashTimeOut, FlashOperationSuccess
 */
ReturnMsg CMD_WRSR( unsigned char UpdateValue )
{

    // Check flash is busy or not
    if(IsFlashBusy())    return FlashIsBusy;

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    FCI_Enable();

    // Send command and update value
    FCI_SendByte(FLASH_CMD_WRSR);
    FCI_SendByte(UpdateValue);

    // Chip select go high to end a flash command
    FCI_Disable();

    if(WaitFlashReady(WriteStatusRegCycleTime))
        return FlashOperationSuccess;
    else
        return FlashTimeOut;

}

/*
 * Arguments:      SecurityReg, 8 bit buffer to store security register value
 * Description:    The RDSCUR instruction is for reading the value of
 *                 Security Register bits.
 */
void CMD_RDSCUR(unsigned char *SecurityReg)
{
    unsigned char  gDataBuffer;

    // Chip select go low to start a flash command
    FCI_Enable();

    //Send command
    FCI_SendByte(FLASH_CMD_RDSCUR);
    gDataBuffer = FCI_ReadByte();

    // Chip select go high to end a flash command
    FCI_Disable();

    *SecurityReg = gDataBuffer;
}

/*
 * Function:       CMD_WRSCUR
 * Arguments:      None.
 * Description:   The WRSCUR instruction is for changing the values of
 *                      Security Register Bits.
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashWriteRegFailed,
 *                 FlashTimeOut
 */
ReturnMsg CMD_WRSCUR(void)
{
    unsigned char  gDataBuffer;

    // Check flash is busy or not
    if(IsFlashBusy())    return FlashIsBusy;

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    FCI_Enable();

    // Write WRSCUR command
    FCI_SendByte(FLASH_CMD_WRSCUR);

    // Chip select go high to end a flash command
    FCI_Disable();

    if(WaitFlashReady(WriteSecuRegCycleTime))
	{
        CMD_RDSCUR(&gDataBuffer);

        // Check security register LDSO bit
        if( (gDataBuffer & FLASH_LDSO_MASK) == FLASH_LDSO_MASK )
                return FlashOperationSuccess;
        else
                return FlashWriteRegFailed;
    }
    else
    {
        return FlashTimeOut;
    }
}

/*
 * Read Command
 */

/*
 * Function:       CMD_READ
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The READ instruction is for reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_READ(unsigned int flash_address, unsigned char *target_address, unsigned int byte_length )
{
    unsigned int index;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Chip select go low to start a flash command
    FCI_Enable();

    // Write READ command and address
    FCI_SendByte(FLASH_CMD_READ);
    SendFlashAddr(flash_address);

    // Set a loop to read data into buffer
    for(index = 0; index < byte_length; index++)
    {
        // Read data one byte at a time
        *(target_address + index) = FCI_ReadByte();
    }

    // Chip select go high to end a flash command
    FCI_Disable();

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_DREAD
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The DREAD instruction enable double throughput of Serial
 *                 Flash in read mode
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_DREAD( unsigned int flash_address, unsigned char *target_address, unsigned int byte_length )
{
    unsigned int index;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Chip select go low to start a flash command
    FCI_Enable();

    // Write 2-I/O Read command and address
    FCI_SendByte(FLASH_CMD_DREAD);
    SendFlashAddr(flash_address);
    InsertDummyCycle(8);                    // Wait 8 dummy cycle

    // Set a loop to read data into data buffer
    for(index = 0; index < byte_length; index++)
    {
        *(target_address + index) = FCI_ReadByte();
    }

    // Chip select go high to end a flash command
    FCI_Disable();

    return FlashOperationSuccess;
}


/*
 * Function:       CMD_FASTREAD
 * Arguments:      flash_address, 32 bit flash memory address
 *                 target_address, buffer address to store returned data
 *                 byte_length, length of returned data in byte unit
 * Description:    The FASTREAD instruction is for quickly reading data out.
 * Return Message: FlashAddressInvalid, FlashOperationSuccess
 */
ReturnMsg CMD_FASTREAD( unsigned int flash_address, unsigned char *target_address, unsigned int byte_length )
{
    unsigned int index;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Chip select go low to start a flash command
    FCI_Enable();

    // Write Fast Read command, address and dummy cycle
    FCI_SendByte(FLASH_CMD_FASTREAD);
    SendFlashAddr(flash_address);
    InsertDummyCycle(8);          // Wait dummy cycle

    // Set a loop to read data into data buffer
    for(index = 0; index < byte_length; index++)
    {
        *(target_address + index) = FCI_ReadByte();
    }

    // Chip select go high to end a flash command
    FCI_Disable();

    return FlashOperationSuccess;
}


/*
 * Program Command
 */

/*
 * Description:    The WREN instruction is for setting
 *                 Write Enable Latch (WEL) bit.
 */
void CMD_WREN(void)
{
    FCI_Enable();
    FCI_SendByte(FLASH_CMD_WREN);
    FCI_Disable();
}

/*
 * Description:    The WRDI instruction is to reset
 *                 Write Enable Latch (WEL) bit.
 */
void CMD_WRDI(void)
{
    FCI_Enable();
    FCI_SendByte(FLASH_CMD_WRDI);
    FCI_Disable();
}

/*
 * Function:       CMD_PP
 * Arguments:      flash_address, 32 bit flash memory address
 *                 source_address, buffer address of source data to program
 *                 byte_length, byte length of data to programm
 * Description:    The PP instruction is for programming
 *                 the memory to be "0".
 *                 The device only accept the last 256 byte ( or 32 byte ) to program.
 *                 If the page address ( flash_address[7:0] ) reach 0xFF, it will
 *                 program next at 0x00 of the same page.
 *                 Some products have smaller page size ( 32 byte )
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_PP(unsigned int flash_address, unsigned char *source_address, unsigned int byte_length )
{
    unsigned int index;

    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy() )    return FlashIsBusy;

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    FCI_Enable();

    // Write Page Program command
    FCI_SendByte(FLASH_CMD_PP);
    SendFlashAddr(flash_address);

    // Set a loop to down load whole page data into flash's buffer
    // Note: only last 256 byte ( or 32 byte ) will be programmed
    for(index = 0; index < byte_length; index++)
    {
        FCI_SendByte( *(source_address + index));
    }

    // Chip select go high to end a flash command
    FCI_Disable();

    if(WaitFlashReady(PageProgramCycleTime))
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Erase Command
 */

/*
 * Function:       CMD_SE
 * Arguments:      flash_address, 32 bit flash memory address
 * Description:    The SE instruction is for erasing the data
 *                 of the chosen sector (4KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_SE(unsigned int flash_address)
{
    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy() )    return FlashIsBusy;


    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    FCI_Enable();

    //Write Sector Erase command = 0x20;
    FCI_SendByte(FLASH_CMD_SE);
    SendFlashAddr(flash_address);

    // Chip select go high to end a flash command
    FCI_Disable();

    if(WaitFlashReady(SectorEraseCycleTime))
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Description:    The BE instruction is for erasing the data
 *                 of the chosen sector (64KB) to be "1".
 * Return Message: FlashAddressInvalid, FlashIsBusy, FlashOperationSuccess,
 *                 FlashTimeOut
 */
ReturnMsg CMD_BE(unsigned int flash_address)
{
    // Check flash address
    if( flash_address > FlashSize ) return FlashAddressInvalid;

    // Check flash is busy or not
    if( IsFlashBusy() )    return FlashIsBusy;

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    FCI_Enable();

    //Write Block Erase command = 0xD8;
    FCI_SendByte(FLASH_CMD_BE);
    SendFlashAddr(flash_address);

    // Chip select go high to end a flash command
    FCI_Disable();

    if( WaitFlashReady( BlockEraseCycleTime ) )
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}

/*
 * Description:    The CE instruction is for erasing the data
 *                        of the whole chip to be "1".
 * Return Message: FlashIsBusy, FlashOperationSuccess, FlashTimeOut
 */
ReturnMsg CMD_CE(void)
{
    // Check flash is busy or not
    if( IsFlashBusy() )    return FlashIsBusy;

    // Setting Write Enable Latch bit
    CMD_WREN();

    // Chip select go low to start a flash command
    FCI_Enable();

    //Write Chip Erase command = 0x60;
    FCI_SendByte(FLASH_CMD_CE);

    // Chip select go high to end a flash command
    FCI_Disable();

    if(WaitFlashReady(ChipEraseCycleTime))
        return FlashOperationSuccess;
    else
        return FlashTimeOut;
}


/*
 * Mode setting Command
 */

/*
 * Description:    The DP instruction is for setting the
 *                 device on the minimizing the power consumption.
 */
void CMD_DP(void)
{
    FCI_Enable();
    FCI_SendByte(FLASH_CMD_DP);
    FCI_Disable();
}

/*
 * Description:    The Release from RDP instruction is
 *                 putting the device in the Stand-by Power mode.
 */
void CMD_RDP(void)
{
    FCI_Enable();
    FCI_SendByte(FLASH_CMD_RDP);
    FCI_Disable();
}

/*
 * Description:    The ENSO instruction is for entering the secured OTP mode.
 */
void CMD_ENSO(void)
{
    FCI_Enable();
    FCI_SendByte(FLASH_CMD_ENSO);
    FCI_Disable();
}

/*
 * Description:    The EXSO instruction is for exiting the secured OTP mode.
 */
void CMD_EXSO(void)
{
    FCI_Enable();
    FCI_SendByte(FLASH_CMD_EXSO);
    FCI_Disable();
}


