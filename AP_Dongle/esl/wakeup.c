#include "wakeup.h"
#include "flash.h"
#include "timer.h"
#include "rf.h"
#include "debug.h"
#include "data.h"
#include "bsp.h"
#include "core.h"

INT32 wakeup_start(UINT32 addr, UINT32 len, UINT8 type)
{
	INT32 ret = 0;
	UINT8 timer = 0;
	UINT32 duration_ms = 0;
	UINT16 timer_count = 0;
	
	UINT8 id[4] = {0};
	UINT8 data[26] = {0};
	UINT8 data_len = 0;
	UINT8 power = 0;
	UINT8 channel = 0;
	UINT16 datarate = 0;
	UINT8 duration = 0;
	UINT8 slot_duration = 0;
	UINT8 ctrl = 0;
	UINT16 interval = 0;
	UINT8 mode = 0;

	pdebug("wkup addr=0x%08X, len=%d\r\n", addr, len);

	if((addr==0) || (len==0))
	{
		ret = -1;
		goto done;
	}
	
	if(g3_get_wkup_para(addr, &datarate, &power, &duration, &slot_duration, &mode) == 0)
	{
		perr("g3_wkup() get para from flash.\r\n");
		ret = -2;
		goto done;
	}
	
	interval = g3_get_wkup_interval(addr);
	if(interval == 0)
	{
		interval = 30;
	}
	
	pdebug("wkup para: datarate=%d, power=%d, duration=%d, slot_duration=%d, interval=%d\r\n", \
			datarate, power, duration, slot_duration, interval);
	
	if(duration == 0)
	{
		pdebug("warning: wkup duration is 0\r\n");
		goto done;
	}
	
	if(get_one_data(addr+OFFSET_WKUP_DATA, id, &channel, &data_len, data, sizeof(data)) == 0)
	{
		perr("g3_wkup() get data from flash.\r\n");
		ret = -3;
		goto done;
	}
	
	pdebug("wkup data: id:0x%02X-0x%02X-0x%02X-0x%02X, channel=%d, len=%d, data=", \
			id[0], id[1], id[2], id[3], channel, data_len);
	pdebughex(data, data_len);

	ctrl = data[0];
		
	set_datarate(datarate);
	set_power(power);
	
	if(mode == 1)
	{
		duration_ms = duration * 10;
	}
	else
	{
		duration_ms = duration * 1000 - 500;
	}
	
	if((timer=TIM_Open(slot_duration, duration_ms/slot_duration, 1)) == 0)
	{
		perr("g3_wkup() open timer.\r\n");
		ret = -4;
		goto done;
	}
		
	while(!TIM_CheckTimeout(timer))
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("g3_wkup quit\r\n");
			break;
		}
		
		if(type == 0) // 0 is default
		{
			timer_count = TIM_GetCount(timer);
			
			if(ctrl == 0xAA)
			{
				data[0] = ctrl;
			}
			else
			{
				data[0] = (ctrl&0xE0) | ((timer_count >> 8) & 0x1f);
			}
			
			data[1] = timer_count & 0xff;
		}

		if(send_data(id, data, data_len, channel, 5000) != data_len)
		{
			perr("g3_wkup() send data.\r\n");
			ret = -5;
			break;
		}
		
		BSP_Delay10US(interval);
	}
	
	TIM_Close(timer);
	
	ret = 1;
done:
	return ret;
}

INT32 wakeup_get_loop_times(UINT32 addr)
{
	return g3_get_wkup_loop_times(addr);
}
