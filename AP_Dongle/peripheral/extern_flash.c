
#include "extern_flash.h"
#include "datatype.h"
#include "Board.h"


#define  EF_BLOCK_SIZE   ((UINT32)0x1000)   //4k
NVS_Handle nvsHandle;
NVS_Attrs regionAttrs;
NVS_Params nvsParams;



void init_nvs_spi_flash(void)
{
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
    nvsHandle = NVS_open(Board_NVS1, &nvsParams);
}
void extern_flash_close(void)
{
    NVS_close(nvsHandle);           //flash½øÈësleep×´Ì¬
}
void segment_erase(WORD seg_addr)
{
    NVS_erase(nvsHandle, seg_addr, EF_BLOCK_SIZE);

}

void segment_write(WORD addr, WORD data, WORD len)
{
    NVS_write(nvsHandle, addr, (void *)data, len,  NVS_WRITE_POST_VERIFY);
}

void segment_read(WORD addr, WORD buf, WORD len)
{
    NVS_read(nvsHandle, addr, (void *) buf,len);
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
