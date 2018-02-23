#include "updatabdc.h"
#include "frame1.h"
#include "timer.h"
#include "debug.h"
#include "rf.h"
#include "data.h"
#include "common.h"
#include "core.h"

//mode 0: 11111222223333334444444......xxxxxxx
static void _bdc_transmit_mode0(updata_table_t *table, UINT8 timer)
{
	UINT8 id[4] = {0};
	UINT8 ch = 0;
	UINT8 len = 0;
	UINT8 data[64] = {0};
	INT32 i, j, k;
	UINT32 txaddr = table->updata_addr+OFFSET_DATA_DATA;

	i = j = 0;
	k = table->tx_interval*1000/table->tx_duration+1;
	
	while(1)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("_bdc_transmit_mode0 Core_GetQuitStatus()=1\r\n");
			break;		
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			pdebug("_bdc_transmit_mode0 timeout.\r\n");
			break;
		}
		
		if(j == 0)
		{
			get_one_data(txaddr, id, &ch, &len, data, sizeof(data));
		}
		send_without_wait(id, data, len, ch, 2000);
		j++;
		if(j >= k)
		{
			txaddr += sizeof(id)+sizeof(ch)+sizeof(len)+len;
			j = 0;
			i++;
		}
		
		if(i >= table->num)
		{
			txaddr = table->updata_addr+OFFSET_DATA_DATA;
			i = 0;
		}
	}
	
	wait(2000);
}

//mode 1: 12345....x12345....x12345....x.......
static void _bdc_transmit_mode1(updata_table_t *table, UINT8 timer)
{
	UINT8 id[4] = {0};
	UINT8 ch = 0;
	UINT8 len = 0;
	UINT8 data[64] = {0};
	INT32 i = 0;
	UINT32 txaddr = table->updata_addr+OFFSET_DATA_DATA;

	while(1)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("_bdc_transmit_mode1 Core_GetQuitStatus()=1\r\n");
			break;		
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			pdebug("_bdc_transmit_mode1(%d) timeout\r\n", timer);
			break;
		}
		
		if((i >= table->num) || (i == 0))
		{
			txaddr = table->updata_addr+OFFSET_DATA_DATA;
			i = 0;
		}
		
		get_one_data(txaddr, id, &ch, &len, data, sizeof(data));
		send_without_wait(id, data, len, ch, 2000);
		txaddr += sizeof(id)+sizeof(ch)+sizeof(len)+len;
		i++;
	}
	
	wait(2000);
}

UINT8 bdc_updata_loop(updata_table_t *table)
{
	UINT8 ret = 0;
	UINT8 timer = 0;
	UINT16 timeout = table->esl_work_duration * 10;
		
	if((timer=TIM_Open(100, timeout, TIMER_UP_CNT)) == ALL_TIMER_ACTIVE)
	{
		perr("bdc_updata_loop open timer\r\n");
		goto done;
	}
	pdebug("bdc_updata_loop, timer %d timeout is %d\r\n", timer, timeout);

	set_datarate(table->tx_datarate);
	set_power(table->tx_power);

	if(table->mode == 1)
	{
		_bdc_transmit_mode1(table, timer);
	}
	else //0 and default mode
	{
		_bdc_transmit_mode0(table, timer);
	}
	
	TIM_Close(timer);
		
	ret = 1;
	
done:
	pdebug("bdc_updata_loop end\r\n");
	return ret;
}
