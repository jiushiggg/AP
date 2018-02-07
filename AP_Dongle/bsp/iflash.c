#include "iflash.h"
#include <stm32f10x.h>
#include <string.h>
#include "debug.h"

void IFLASH_Lock(void)
{
	FLASH_Lock();
}

void IFLASH_Unlock(void)
{
	FLASH_Unlock();
}

INT32 IFLASH_Erase(UINT32 first_page_addr, INT32 page_num)
{
	INT32 i = 0;
	
	for(i = 0; i < page_num; i++)
	{
		if(FLASH_ErasePage(first_page_addr+(i*IFLASH_PAGE_SIZE)) != FLASH_COMPLETE)
		{
			break;
		}
	}
	
	if(i >= page_num)
	{
		return page_num;
	}
	else
	{
		return 0;
	}
}

INT32 IFLASH_Write(UINT32 addr, UINT8 *src, INT32 len)
{
	UINT16 halfword = 0;
	INT32 left_len = len;
	UINT8 *ptr = src;
	UINT32 add = addr;
	INT32 write_len = 0;
	
	while(left_len > 0)
	{
//		pdebug("iflash write: addr=0x%08X, left=%d, total=%d\r\n", add, left_len, len);
		
		if(left_len % 2 == 0)
		{
			write_len = 2;
			memcpy(&halfword, ptr, sizeof(halfword));
		}
		else
		{
			write_len = 1;
			halfword = *ptr;
		}
		
		if(FLASH_ProgramHalfWord(add, halfword) != FLASH_COMPLETE)
		{
//			pdebug("x\r\n");
			break;
		}
	
		ptr += write_len;
		add += write_len;
		left_len -= write_len;
	}
	
	if(left_len > 0)
	{
		return 0;
	}
	else
	{
		return len;
	}
}

INT32 IFLASH_Read(UINT32 addr, UINT8 *dst, INT32 len)
{
	memcpy(dst, (UINT8 *)addr, len);
	return len;
}













