
#include "extern_flash.h"
#include "datatype.h"
#include "Board.h"


#define  EF_BLOCK_SIZE   ((UINT32)0x1000)   //4k
#define  DMA_MAX_BUF    1024
static NVS_Handle nvsHandle;

void init_nvs_spi_flash(void)
{
//    NVS_Attrs regionAttrs;
    NVS_Params nvsParams;

    NVS_init();
    NVS_Params_init(&nvsParams);
    nvsHandle = NVS_open(Board_NVS1, &nvsParams);
    if(!nvsHandle)
    {
        while(1);
    }
}

void extern_flash_open(void)
{
    NVS_Params nvsParams;
    nvsHandle = NVS_open(Board_NVS1, &nvsParams);
}
void extern_flash_close(void)
{
    NVS_close(nvsHandle);           //flash½øÈësleep×´Ì¬
}
ReturnMsg CMD_SE(WORD seg_addr)
{
    NVS_erase(nvsHandle, seg_addr, EF_BLOCK_SIZE);
    return FlashOperationSuccess;
}

ReturnMsg CMD_PP(WORD addr, WORD data, WORD len)
{
    uint32_t tmp_len = 0;
    while(len > 0){
        if (len > DMA_MAX_BUF){
            tmp_len = DMA_MAX_BUF;
            len -= DMA_MAX_BUF;
        }else {
            tmp_len = len;
            len = 0;
        }
        NVS_write(nvsHandle, addr, (void *)data, tmp_len, NVS_WRITE_POST_VERIFY);
        data += tmp_len;
        addr += tmp_len;
    }
    return FlashOperationSuccess;
}

ReturnMsg CMD_FASTREAD(WORD addr, WORD buf, WORD len)
{
    uint32_t tmp_len = 0;
    while(len > 0){
        if (len > DMA_MAX_BUF){
            tmp_len = DMA_MAX_BUF;
            len -= DMA_MAX_BUF;
        }else {
            tmp_len = len;
            len = 0;
        }
        NVS_read(nvsHandle, addr, (void *)buf,tmp_len);
        buf += tmp_len;
        addr += tmp_len;
    }

    return FlashOperationSuccess;
}

UINT32 CMD_RDID(void)
{
//    const uint8_t wbuf[] = { BLS_CODE_MDID, 0xFF, 0xFF, 0x00 };
//
//    extFlashSelect();
//
//    int ret = Spi_write(wbuf, sizeof(wbuf));
//    if (ret)
//    {
//        extFlashDeselect();
//        return false;
//    }
//
//    ret = Spi_read(infoBuf, sizeof(infoBuf));
//    extFlashDeselect();
//
//    return ret == 0;
    return FlashID;
}

//while (1) {
//    segment_erase(0);
//    memset(buf, 1, sizeof(buf));
//    segment_write(0, (WORD)buf, sizeof(buf));
//    sleep(time);
//    memset(buf, 0, sizeof(buf));
//    segment_r(0, (WORD)buf, sizeof(buf));
////        GPIO_toggle(Board_GPIO_LED0);
//    while(1);
//}
