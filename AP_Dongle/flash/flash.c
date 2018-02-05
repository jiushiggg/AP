#include "flashchip.h"
#include "flash.h"
#include <stdio.h>

#define FLASH_SECTOR_NUM  	1024
#define FLASH_SECTOR_SIZE	4096
#define FLASH_PAGE_SIZE  	256

#define DATA_SECTER_START  50
#define DATA_SECTER_END    499

static UINT32 _sector = DATA_SECTER_START;

//#define FLASH_DBG

BOOL Flash_SetErrorSector(UINT16 sector)
{
	UINT8 flag = SECTOR_ERR;
	return Flash_Write(sector, &flag, sizeof(flag));
}

UINT8 Flash_GetSectorStatus(UINT16 sector)
{
	UINT8 status = 0;
	
	if(!Flash_Read(sector, &status, sizeof(status)))
	{
		status = 0;
	}
	
	return status;
}

#define FLASH_CHECK_ERR		0
#define FLASH_CHECK_OK		1
#define FLASH_CHECK_NEW		2

UINT8 Flash_Check(void)
{
	UINT8 ret = FLASH_CHECK_ERR;
	UINT8 flag = 0;
	UINT8 _buf[FLASH_SECTOR_NUM] = {0};
	UINT32 i                                                                                 , j;

	if (Flash_Read(FLASH_BASE_ADDR, &flag, sizeof(flag)))
	{
		if (flag == FLASH_USE_FLAG)
		{
			//�Ѿ�ʹ�ù���flash����黵sector�����
			if (Flash_Read(FLASH_BASE_ADDR, _buf, sizeof(_buf)))
			{
				j = 0;
				for (i = DATA_SECTER_START; i < DATA_SECTER_END + 1; i++)
				{
					if (_buf[i] == SECTOR_ERR)
					{
						j++;
					}
				}

				//����sector��С��sector������һ�룬�糬��������Ϊflash��
				if (j < ((DATA_SECTER_END-DATA_SECTER_START+1)/2))
				{
					ret = FLASH_CHECK_OK;
				}
			}
		} 
		else
		{
			//��flash��дuse��־��������һ��sector��Ϊ��sector index
			if (CMD_SE(FLASH_BASE_ADDR) == FlashOperationSuccess)
			{
				flag = FLASH_USE_FLAG;
				if (CMD_PP(FLASH_BASE_ADDR, &flag, sizeof(flag)) == FlashOperationSuccess)
				{
					ret = FLASH_CHECK_NEW;
				}
			} 
		}
	}

	return ret;
}

void Flash_SoftReset(void)
{
	_sector = DATA_SECTER_START;
}

UINT8 Flash_Init(void)
{
	UINT8 ret = 0;
	int id = 0;
	//��ʼ��IO
//	MX25L3206E_IOInit();
//	InitialGpio();
	
	//��ȡ
	id = CMD_RDID();
	if ((id!=FlashID) && (id!=FlashID_GD))
	{
#ifdef	FLASH_DBG
		printf("FI RDID FAIL.\r\n");
#endif
		return FLASH_INIT_ERR_IO;
	}

	ret = Flash_Check();
	if (ret == FLASH_CHECK_ERR)
	{
#ifdef	FLASH_DBG
		printf("FI FC FAIL.\r\n");
#endif
		return FLASH_INIT_ERR_CK;
	}
	else if (ret == FLASH_CHECK_NEW)
	{
#ifdef	FLASH_DBG
		printf("FI FC NEW.\r\n");
#endif
	}
	//ϵͳ���������õ�ǰsectorλ��
	_sector = DATA_SECTER_START;
#ifdef	FLASH_DBG
	printf("FI OK.\r\n");
#endif
	
	return FLASH_INIT_OK;
}

UINT32 Flash_Malloc(UINT32 size)
{
	UINT32 sector_num = 0;
	UINT16 i, j;
	UINT8 flag = 0;
	UINT32 ret = MALLOC_FAIL;
	UINT32 find_start_sector;

	if((size > 0) && (size < ((DATA_SECTER_END-DATA_SECTER_START+1)*FLASH_SECTOR_SIZE)))
	{
		//������Ҫ�����sector����
		sector_num = size / FLASH_SECTOR_SIZE;
		if ((size % FLASH_SECTOR_SIZE) != 0)
		{
			sector_num += 1;
		}

		find_start_sector = _sector;
find_loop:
		//�ڵ�ǰ��ַ��ʼ����sector��ֱ���ҵ�������sector_num��sector
		j = 0;
		for (i = find_start_sector; i < DATA_SECTER_END; i++)
		{
			if (CMD_SE(i*FLASH_SECTOR_SIZE) == FlashOperationSuccess)
			{
				j += 1;
			}
			else
			{
				Flash_SetErrorSector(i);
				j = 0;
			}

			if (j == sector_num)
			{
				//�ɹ��ҵ������ò��ҳɹ���־flag����¼�´�malloc����ʼsector
				flag = 1;
				_sector = i + 1;
				ret = (i + 1 - sector_num) * FLASH_SECTOR_SIZE;
				break;
			}
		}

		if (flag == 0)
		{
			//��һ�ֵ�
			find_start_sector = DATA_SECTER_START;
			flag = 2;
			goto find_loop;
		}
	}

	return ret;
}

BOOL Flash_Write(UINT32 addr, UINT8* src, UINT16 len)
{
	BOOL ret = FALSE;
	UINT16 left_len = len;
	UINT32 w_addr = addr;
	UINT8 *ptr = src;
	UINT16 w_len = 0;

	if((w_addr>= ((DATA_SECTER_END+1)*FLASH_SECTOR_SIZE)) || (w_addr < (DATA_SECTER_START*FLASH_SECTOR_SIZE)))
	{
		goto done;
	}
	
	//�ж�addr�Ƿ��ڿ�д��Χ��
	if (w_addr < (_sector*FLASH_SECTOR_SIZE))
	{
		while(left_len > 0)
		{
			if (w_addr%FLASH_PAGE_SIZE != 0)
			{
				//����Ŀ�ĵ�ַû��page��������
				w_len = FLASH_PAGE_SIZE - (w_addr%FLASH_PAGE_SIZE);

				if (w_len > left_len)
				{
					w_len = left_len;				
				}
			} 
			else
			{
				//Ŀ�ĵ�ַpage����
				if (left_len > FLASH_PAGE_SIZE)
				{
					w_len = FLASH_PAGE_SIZE;
				} 
				else
				{
					w_len = left_len;
				}
			}

			if (CMD_PP(w_addr, ptr, w_len) == FlashOperationSuccess)
			{
				w_addr += w_len;
				addr += w_len;
				ptr += w_len;
				left_len -= w_len;
				w_len = 0;
			} 
			else
			{
				break;
			}
		}
	}

	if (left_len == 0)
	{
		ret = TRUE;
	}

done:
	return ret;
}

BOOL Flash_Read(UINT32 addr, UINT8* dst, UINT16 len)
{
	CMD_FASTREAD(addr, dst, len);
	return TRUE;
}

