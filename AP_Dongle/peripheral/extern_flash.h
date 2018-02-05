
#ifndef  EXTERN_FLASH_H
#define  EXTERN_FLASH_H

#include <ti/drivers/NVS.h>
#include <ti/drivers/nvs/NVSSPI25X.h>
#include "datatype.h"

extern NVS_Handle nvsHandle;
extern NVS_Attrs regionAttrs;
extern NVS_Params nvsParams;

void init_nvs_spi_flash(void);
void extern_flash_open(void);
void extern_flash_close(void);
void segment_erase(WORD seg_addr);
void segment_write(WORD addr, WORD data, WORD len);
void segment_read(WORD addr, WORD buf, WORD len);

#endif
