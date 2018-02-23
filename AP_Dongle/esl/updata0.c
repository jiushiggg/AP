#include "updata0.h"
#include "updata1.h"
#include "frame1.h"
#include "timer.h"
#include "bsp.h"
#include "flash.h"
#include "debug.h"
#include "rf.h"
#include "data.h"
#include "common.h"
#include <string.h>
#include "sleep.h"
#include "core.h"

#pragma pack(1)
typedef struct {
	UINT8  esl_id[4];
	UINT8  ack;
	UINT8  sleep_flag;
	UINT32 first_pkg_addr;
	UINT16 total_pkg_num;
	UINT8  failed_pkg_num;
} mode0_esl_t;
#pragma pack(1)

void dummy(updata_table_t *table, INT32 nus)
{
	if(table->frame1_addr == 0)
	{
		pdebug("dummy %d us.\r\n", nus);
		BSP_Delay100US(nus/100+1);
	}
	else
	{
		INT32 frame1_num = nus/table->tx_duration;
		pdebug("dummy %d frame1.\r\n", frame1_num);
		frame1_dummy(table->frame1_addr, table->frame1_len, (UINT32*)&table->frame1_offset, frame1_num);
	}
}

UINT16 init_data(UINT32 addr, UINT32 len, updata_table_t *table)
{
	INT32 left_len = len;
	UINT32 cur_addr = addr;
	UINT8 ff = 1;
	UINT8 last_id[4] = {0};
	UINT16 num = 0;
	UINT8 cur_id[4] = {0};
	UINT32 i;
	
	mode0_esl_t *pESL = (mode0_esl_t *)table->data;
	
//	table->data_addr = addr;
	table->max_esl_num = TABLE_BUF_SIZE / sizeof(mode0_esl_t);
	
	table->pkg_num = 0;
	while(left_len > 0)
	{
		pdebug("init_data() left len = %d.\r\n", left_len);
		
		if(get_one_data(cur_addr, cur_id, NULL, NULL, NULL, 0) == 0)
		{
			perr("init_data() get data!\r\n");
			break;
		}
		
		if((ff==1) || (memcmp(cur_id, last_id, 4) != 0)) // a new id
		{
			ff = 0;
			memcpy(last_id, cur_id, 4);
			memcpy(pESL[num].esl_id, cur_id, 4);
			pESL[num].first_pkg_addr = cur_addr;
			pdebug("table %d: 0x%02X-0x%02X-0x%02X-0x%02X, first addr = 0x%08X.\r\n", \
					num, pESL[num].esl_id[0], pESL[num].esl_id[1], \
					pESL[num].esl_id[2], pESL[num].esl_id[3], \
					pESL[num].first_pkg_addr);
			pESL[num].ack = 0;
			pESL[num].total_pkg_num = 0;
			pESL[num].failed_pkg_num = 0;
			pESL[num].sleep_flag = 2;
			num++;
			table->esl_num++;
		}
		
		pESL[num-1].total_pkg_num += 1;
		table->pkg_num += 1;
		
		cur_addr += SIZE_ESL_DATA_SINGLE;
		left_len -= SIZE_ESL_DATA_SINGLE;
		
		if(num >= table->max_esl_num)
		{
			perr("m0_init_data() esl num(d%) > max(%d)!\r\n", num, table->max_esl_num);
			break;
		}
	}
	
	for(i = 0; i < num; i++)
	{
		pESL[i].failed_pkg_num = pESL[i].total_pkg_num;
	}
		
	return num;
}

static INT32 fisrt_transmit_round(updata_table_t *table, UINT8 timer)
{
	INT32 ret = table->pkg_num;
	INT32 i = 0, j = 0, k = 0, t = 0;
	UINT8 id[4] = {0};
	UINT8 ch = 0;
	UINT8 len = 0;
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	INT32 left_pkg_num = table->pkg_num;
	INT32 delay_us = 0;
	mode0_esl_t *pESL = (mode0_esl_t *)table->data;
	UINT8 f = 0;
	
	pdebug("transmit_round(), timer %d.\r\n", timer);
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);
	
	while(left_pkg_num > 0)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("fisrt_transmit_round quit\r\n");
			break;
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			pdebug("transmit_round, timer timeout.\r\n");
			ret = -1;
			break;
		}
				
		if(pESL[i].total_pkg_num > j)
		{
			if(get_one_data(pESL[i].first_pkg_addr+j*SIZE_ESL_DATA_SINGLE, id, &ch, &len, data, sizeof(data)) == 0)
			{
				perr("transmit_round() get data.\r\n");
				ret = -2;
				break;
			}
						
			pdebug("transmit_round %d/%d 0x%02X-0x%02X-0x%02X-0x%02X, ch=%d,len=%d.\r\n", \
					i+1, j+1, id[0], id[1], id[2], id[3], ch, len);
			pdebughex(data, len);
						
			if(send_without_wait(id, data, len, ch, 6000) == 0)
			{
				perr("transmit_round() send data.\r\n");
				wait(1000);
			}
							
			left_pkg_num--;
			k++;
			t++;
		}
		
		f = 0;	
		if((++i)==table->esl_num)
		{
			i = 0;
			j++;
			
			if((delay_us=(table->tx_interval*1000-k*table->tx_duration)) >= 0)
			{
				dummy(table, delay_us);
				f = 1;
			}
			k = 0;
		}	
		if((t%50==0) && (f==0))
		{
			dummy(table, table->tx_duration);
		}
	}

	wait(2000);
		
	return ret;
}

static UINT8 query_miss_slot = 0;
UINT8 first_pkg_data[SIZE_ESL_DATA_BUF] = {0};

static INT32 query_miss_round(updata_table_t *table, UINT8 timer)
{
	INT32 ret = 0;
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	INT32 i;
	UINT32 deal_timeout = table->deal_duration*1000;
	UINT8 channel = 0;
	mode0_esl_t *pESL = (mode0_esl_t *)table->data;
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);

	pdebug("mode0_query_miss_round, timer is %d.\r\n", timer);
	
	for(i = 0; i < table->esl_num; i++, query_miss_slot++)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("query_miss_round quit\r\n");
			break;
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			pdebug("mode0_query_miss_round, timer timeout.\r\n");
			break;
		}
		
		if(pESL[i].failed_pkg_num == 0)
		{
			continue;
		}
		
		if(query_miss_slot == 0)
		{
			query_miss_slot = 1;
		}
		
//		channel = g3_get_channel(pESL[i].first_pkg_addr);
		get_one_data(pESL[i].first_pkg_addr, NULL, &channel, NULL, first_pkg_data, sizeof(first_pkg_data));
		pdebug("query miss esl 0x%02x-0x%02x-0x%02x-0x%02x, txbps=%d, rxbps=%d, ch=%d, timeout=%d.\r\n", \
				pESL[i].esl_id[0], pESL[i].esl_id[1], \
				pESL[i].esl_id[2], pESL[i].esl_id[3], \
				table->tx_datarate, table->rx_datarate, channel, deal_timeout);
		set_datarate(table->tx_datarate);
		memset(data, 0, sizeof(data));
		g3_make_link_query(pESL[i].esl_id, get_pkg_sn_f(pESL[i].first_pkg_addr+(pESL[i].total_pkg_num-1)*32, 7), \
							query_miss_slot, first_pkg_data, data, sizeof(data));
		send_data(pESL[i].esl_id, data, sizeof(data), channel, 2000);
		set_datarate(table->rx_datarate);	
		memset(data, 0, sizeof(data));
		if(recv_data(table->master_id, data, sizeof(data), channel, deal_timeout) == 0)
		{
			pdebug("recv timeout.\r\n");
			continue;
		}		
		pdebug("recv:");
		pdebughex(data, sizeof(data));
		ret++;
		
		if(g3_check_link_query(pESL[i].esl_id, pESL[i].total_pkg_num, query_miss_slot, first_pkg_data, data, sizeof(data)) == 0)
		{
			pdebug("data check error.\r\n");
			continue;
		}
		
		if(data[2] == 0) // miss 0 pkg, tx successfully
		{
			pESL[i].failed_pkg_num = 0;
			pESL[i].ack = data[4];
		}
		else
		{
			pESL[i].failed_pkg_num = pESL[i].total_pkg_num;
		}
	}

	dummy(table, table->tx_duration*2);
	
	wait(2000);
		
	return ret;
}

static INT32 send_sleep_round(updata_table_t *table, UINT8 timer)
{
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	UINT32 i;
	INT32 ret = 0;
	UINT8 channel = 0;
	mode0_esl_t *pESL = (mode0_esl_t *)table->data;
	
	pdebug("mode0_send_sleep_round(), timer is %d.\r\n", timer);
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);
	
	for(i = 0; i < table->esl_num; i++)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("send_sleep_round quit\r\n");
			break;
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}
		
		if((pESL[i].failed_pkg_num==0) && (pESL[i].sleep_flag>0))
		{
			pdebug("sleep esl: 0x%02X-0x%02X-0x%02X-0x%02X.\r\n", \
					pESL[i].esl_id[0], pESL[i].esl_id[1], \
					pESL[i].esl_id[2], pESL[i].esl_id[3]);
			pESL[i].sleep_flag--;
			make_sleep_data(pESL[i].esl_id, table->id_x_ctrl, data, sizeof(data));
			get_one_data(pESL[i].first_pkg_addr, NULL, &channel, NULL, NULL, 0);
			send_data(pESL[i].esl_id, data, sizeof(data), channel, 6000);
			ret++;
		}
	}
	
	table->ok_esl_num += ret;
	return ret;
}

static INT32 send_sleep_all(updata_table_t *table)
{
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	UINT32 i;
	INT32 ret = 0;
	UINT8 channel = 0;
	mode0_esl_t *pESL = (mode0_esl_t *)table->data;
	
	pdebug("send_sleep_all\r\n");
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);
	
	for(i = 0; i < table->esl_num; i++)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("send_sleep_all quit\r\n");
			break;
		}
		
		pdebug("sleeep esl: 0x%02X-0x%02X-0x%02X-0x%02X.\r\n", \
				pESL[i].esl_id[0], pESL[i].esl_id[1], \
				pESL[i].esl_id[2], pESL[i].esl_id[3]);
		make_sleep_data(pESL[i].esl_id, table->id_x_ctrl, data, sizeof(data));
		get_one_data(pESL[i].first_pkg_addr, NULL, &channel, NULL, NULL, 0);
		send_data(pESL[i].esl_id, data, sizeof(data), channel, 2000);
		ret++;
	}
	
	return ret;
}

static INT32 send_miss_round(updata_table_t *table, UINT8 timer)
{
	INT32 ret = table->pkg_num;
	INT32 i = 0, j = 0, k = 0, t = 0;
	UINT8 id[4] = {0};
	UINT8 ch = 0;
	UINT8 len = 0;
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	INT32 left_esl_num = 0;
	INT32 left_pkg_num = 0;
	INT32 delay_us = 0;
	mode0_esl_t *pESL = (mode0_esl_t *)table->data;
	UINT8 f = 0;
	
	pdebug("mode0_send_miss_round(), timer is %d.\r\n", timer);
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);
	
	for(i = 0; i < table->esl_num; i++)
	{
		if(pESL[i].failed_pkg_num != 0) 
		{
			left_esl_num++;
			left_pkg_num += pESL[i].total_pkg_num;
		}
	}
	pdebug("need to be sent miss esl_num=%d, pkg_num=%d.\r\n", left_esl_num, left_pkg_num);
	
	i = 0;
	while(left_pkg_num > 0)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("send_miss_round quit\r\n");
			break;
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			pdebug("timeout.\r\n");
			break;
		}
				
		if((pESL[i].failed_pkg_num != 0) && (pESL[i].total_pkg_num > j))
		{
			if(get_one_data(pESL[i].first_pkg_addr+j*SIZE_ESL_DATA_SINGLE, id, &ch, &len, data, sizeof(data)) == 0)
			{
				perr("mode0_send_miss_round() get data!\r\n");
				break;
			}
			
			pdebug("send miss %d/%d 0x%02X-0x%02X-0x%02X-0x%02X, ch=%d,len=%d.\r\n", \
					i+1, j+1, id[0], id[1], id[2], id[3], ch, len);
			pdebughex(data, len);
			
			if(send_without_wait(id, data, len, ch, 6000) == 0)
			{
				perr("mode0_send_miss_round() send data!\r\n");
				wait(1000);
			}
			
			left_pkg_num--;
			k++;
			t++;
		}

		f = 0;	
		if((++i)==table->esl_num)
		{
			i = 0;
			j++;
			
			if((delay_us=(table->tx_interval*1000-k*table->tx_duration)) >= 0)
			{
				dummy(table, delay_us);
				f = 1;
			}
			k = 0;
		}	
		if((t%50==0) && (f==0))
		{
			dummy(table, table->tx_duration);
		}
	}
	
	if((i != table->esl_num) && (f == 0))
	{
		if((delay_us=(table->tx_interval*1000-k*table->tx_duration)) >= 0)
		{
			dummy(table, delay_us);
			f = 1;
		}
		else
		{
			dummy(table, table->tx_duration*2);
		}
	}	
	wait(2000);	
	return ret;
}

UINT8 updata_loop(updata_table_t *table)
{
	UINT8 ret = 0;
	UINT8 timer = 0;
	UINT16 timeout = table->esl_work_duration * 10 * 85 / 100;
	UINT16 leftime = 0;
	
	pdebug("updata_loop\r\nfirst rount timeout is %d.\r\n", timeout);	
	if((timer=TIM_Open(100, timeout, TIMER_UP_CNT)) == ALL_TIMER_ACTIVE)
	{
		goto done;
	}
	fisrt_transmit_round(table, timer);
	leftime = timeout - TIM_GetCount(timer);
	TIM_Close(timer);
	if(Core_GetQuitStatus() == 1)
	{
		pdebug("updata_loop quit\r\n");
		goto done;
	}
	
	timeout = table->esl_work_duration * 10 * 15 / 100 + leftime;
	pdebug("first round left %d ms.\r\nsecond timeout is %d.\r\n", leftime, timeout);
	if((timer=TIM_Open(100, timeout, TIMER_UP_CNT)) == ALL_TIMER_ACTIVE)
	{
		goto done;
	}
	while(1)
	{
		query_miss_round(table, timer);
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}
		send_sleep_round(table, timer);
		if(table->ok_esl_num == table->esl_num)
		{
			break;
		}
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}		
		send_miss_round(table, timer);
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}	
	}
	
	TIM_Close(timer);

	send_sleep_all(table);

	ret = 1;
	
done:	
	return ret;
}

void make_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len)
{	
	UINT32 offset = 0;
	INT32 i, j;
	UINT8 ack = 0;
	mode0_esl_t *pESL = (mode0_esl_t *)table->data;
	
	*ack_len = table->pkg_num*5 + 4 + 19;
	*ack_addr = Flash_Malloc(*ack_len);
	
	pdebug("make ack\r\n");
	
	if(*ack_addr == MALLOC_FAIL)
	{
		perr("make ack malloc flash addr = 0x%08X, len = %d!\r\n", \
				*ack_addr, *ack_len);
		*ack_len = 0;
		goto done;
	}
	else
	{
		pdebug("malloc flash addr = 0x%08X, len = %d, success.\r\n", \
				*ack_addr, *ack_len);
	}
	
	offset = *ack_addr;
	if(g3_set_ack_para(offset, table->sid, 0x0900, table->pkg_num*5+11, 0, table->pkg_num) == 0)
	{
		perr("make ack set ack para!\r\n");
		*ack_len = 0;
		goto done;
	}
	offset += 21;

	for(i = 0; i < table->esl_num; i++)
	{
		if(pESL[i].failed_pkg_num == 0)
		{
			//ack = pESL[i].ack == 0x40 ? 1 : pESL[i].ack; // old ack
			ack = pESL[i].ack == 1 ? 2 : pESL[i].ack; // new ack
//			if(pESL[i].ack == 0x40)
//			{
//				ack = 1;
//			}
//			else
//			{
//				ack = pESL[i].ack;
//			}
		}
		else
		{
			ack = 0;
		}
		
		for(j = 0; j < pESL[i].total_pkg_num; j++)
		{
			if(g3_set_ack(offset, pESL[i].esl_id, ack) == 0)
			{
				perr("make ack set ack!\r\n");
				*ack_len = 0;
				goto done;
			}
			else
			{
				offset += 5;
			}
		}
	}
	
	if(g3_set_ack_crc(*ack_addr, *ack_len-2) == 0)
	{
		perr("make ack set crc!\r\n");
		*ack_len = 0;
	}
	
done:	
	return;
}


