#include "updata1.h"
#include "updata0.h"
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
#include "sys_cfg.h"
#include "core.h"

UINT16 get_missed_sn_r(UINT8 *data, UINT8 offset)
{
	UINT16 sn = 0;
	if(offset <= MAX_FAILED_PKG_NUM)
	{
		memcpy(&sn, data+offset*2, sizeof(sn));
	}
	return (sn & MASK_OF_PKG_SN);
}

UINT8 check_failed_pkg_r(UINT16 sn, UINT8 *buf, UINT8 failed_num)
{
	UINT16 failed_sn = 0;
	INT32 i = 0;
	UINT8 ret = 0;

	for(i = 0; i < failed_num; i++)
	{
		failed_sn = get_missed_sn_r(buf, i);
		if(sn == failed_sn)
		{
			ret = 1;
			break;
		}
	}

	return ret;
}

UINT16 m1_init_data(UINT32 addr, UINT32 len, updata_table_t *table)
{
	INT32 left_len = len;
	UINT32 cur_addr = addr;
	UINT8 ff = 1;
	UINT8 last_id[4] = {0};
	UINT16 num = 0;
	UINT8 cur_id[4] = {0};
	INT32 i,j;
	UINT16 sn;
	
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;
	table->max_esl_num = TABLE_BUF_SIZE / sizeof(mode1_esl_t);
	
	pdebug("m1_init_data\r\n");
	
	table->pkg_num = 0;
	while(left_len > 0)
	{
		pdebug("%d,", left_len);
		
		if(get_one_data(cur_addr, cur_id, NULL, NULL, NULL, 0) == 0)
		{
			perr("m1_init_data() get data!\r\n");
			break;
		}
		
		if((ff==1) || (memcmp(cur_id, last_id, 4) != 0)) // a new id
		{
			ff = 0;
			memcpy(last_id, cur_id, 4);
			memcpy(pESL[num].esl_id, cur_id, 4);
			pESL[num].first_pkg_addr = cur_addr;
			pESL[num].failed_pkg_offset= 0;
			pdebug("\r\ntable %d: 0x%02X-0x%02X-0x%02X-0x%02X, first addr = 0x%08X.\r\n", num, \
					pESL[num].esl_id[0], pESL[num].esl_id[1], pESL[num].esl_id[2], pESL[num].esl_id[3], \
					pESL[num].first_pkg_addr);
			pESL[num].ack = 0;
			pESL[num].total_pkg_num = 0;
			pESL[num].failed_pkg_num = 0;
			pESL[num].sleep_flag = 2;
			num++;
			table->esl_num++;
		}
		
		pESL[num-1].total_pkg_num += 1;	
		pESL[num-1].failed_pkg_num += 1;
		table->pkg_num += 1;
		
		cur_addr += SIZE_ESL_DATA_SINGLE;
		left_len -= SIZE_ESL_DATA_SINGLE;
		
		if(num > table->max_esl_num)
		{
			perr("m1_init_data() esl num(d%) > max(%d)!\r\n", num, table->max_esl_num);
			break;
		}
	}
	
	/* init failed pkg sn */
	pESL = (mode1_esl_t *)table->data;
	for(i = 0; i < table->esl_num; i++)
	{
		memset(pESL[i].failed_pkg, 0, MAX_FAILED_PKG_NUM*2);
		for(j = 0; (j<MAX_FAILED_PKG_NUM)&&(j < pESL[i].total_pkg_num); j++)
		{
			sn = get_pkg_sn_f(pESL[i].first_pkg_addr+j*SIZE_ESL_DATA_SINGLE, 7);
			memcpy(pESL[i].failed_pkg+j*2, &sn, sizeof(sn));
		}
	}
	
	pdebug("\r\n");
		
	return num;
}

//extern INT32 search_pkg_sn_times;
//extern UINT16 search_pkg_history[40];
//extern UINT32 search_addr_history[40];
//extern INT32 search_end_pkg;

//extern UINT8 search_first_pkg[32];
//extern UINT16 search_pkg_history_o[40];

static void m1_transmit(updata_table_t *table, UINT8 timer)
{
	INT32 i = 0, j = 0, k = 0, t = 0;
	UINT8 id[4] = {0};
	UINT8 ch = 0;
	UINT8 len = 0;
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	INT32 left_pkg_num = 0;
	INT32 dummy_us = 0;
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;
	UINT16 tsn = 0;
	UINT32 taddr = 0;
	UINT8 f = 0;
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);
	
	for(i = 0; i < table->esl_num; i++)
	{
		left_pkg_num += pESL[i].failed_pkg_num;
	}
	pdebug("m1_transmit pkg_num %d, timer %d\r\n", left_pkg_num, timer);
	i = 0;
	j = 0;
	while(left_pkg_num > 0)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_transmit quit\r\n");
			break;
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			pdebug("timeout.\r\n");
			break;
		}
				
		if((pESL[i].failed_pkg_num != 0) && (j < pESL[i].failed_pkg_num))
		{
			if(pESL[i].failed_pkg_num == pESL[i].total_pkg_num)
			{
				taddr = pESL[i].first_pkg_addr + j*SIZE_ESL_DATA_SINGLE;
				pdebug("send 0x%02X-0x%02X-0x%02X-0x%02X pkg %d, ch=%d, len=%d\r\n", id[0], id[1], id[2], id[3], j, ch, len);
			}
//			else if((pESL[i].failed_pkg_num>=MAX_FAILED_PKG_NUM) && (pESL[i].failed_pkg_num<pESL[i].total_pkg_num)) // >= 10 && < total_pkg_num
//			{
//				taddr = pESL[i].first_pkg_addr + (pESL[i].failed_pkg_offset+j)*SIZE_ESL_DATA_SINGLE;
//				pdebug("send misss 0x%02X-0x%02X-0x%02X-0x%02X pkg %d, ch=%d, len=%d\r\n", id[0], id[1], id[2], id[3], j, ch, len);
//			}
			else // > 0 && < 10
			{
				tsn = get_missed_sn_r(pESL[i].failed_pkg, j);
				
				//for debug
//				search_pkg_sn_times = 0;
//				memset(search_pkg_history, 0, sizeof(search_pkg_history));
//				memset(search_addr_history, 0, sizeof(search_addr_history));
//				search_end_pkg = 0;
//				memset(search_pkg_history_o, 0, sizeof(search_pkg_history_o));
//				memset(search_first_pkg, 0, sizeof(search_first_pkg));
				
				taddr = get_pkg_addr_bsearch(pESL[i].first_pkg_addr, pESL[i].total_pkg_num, tsn, 7);
				if(taddr == 0)
				{
					perr("m1t can't find %02X-%02X-%02X-%02X pkg %d 0x%08X, 0x%08X, %d\r\n", 
						pESL[i].esl_id[0],pESL[i].esl_id[1],pESL[i].esl_id[2],pESL[i].esl_id[3],
						tsn, taddr, pESL[i].first_pkg_addr, pESL[i].total_pkg_num);
					
					//for debug
//					perr("search end pkg: %d\r\n", search_end_pkg);
//					perrhex((UINT8 *)search_pkg_history, search_pkg_sn_times*2);
//					perrhex((UINT8 *)search_addr_history, search_pkg_sn_times*4);
//					perrhex((UINT8 *)search_pkg_history_o, search_pkg_sn_times*2);
//					perrhex((UINT8 *)search_first_pkg, 32);
					
					left_pkg_num -= pESL[i].failed_pkg_num-j-1;
					pESL[i].failed_pkg_num = 0;
					pESL[i].ack = 0x5F;
					
					goto user_continue;
				}			
//				pESL[i].failed_pkg_offset = (taddr-pESL[i].first_pkg_addr)/SIZE_ESL_DATA_SINGLE + 1;
				pdebug("send miss 0x%02X-0x%02X-0x%02X-0x%02X pkg %d, ch=%d, len=%d\r\n", id[0], id[1], id[2], id[3], tsn, ch, len);
			}

 			if(get_one_data(taddr, id, &ch, &len, data, sizeof(data)) == 0)
			{
				perr("m1_transmit() get data!\r\n");
				goto user_continue;
			}
			
			pdebughex(data, len);
			
			if(send_without_wait(id, data, len, ch, 6000) == 0)
			{
				perr("m1_transmit() send data!\r\n");
				wait(1000);
			}
		user_continue:
			left_pkg_num--;
			k++;
			t++;
		}
		
		f = 0;
		i++;
		if(i == table->esl_num)
		{
			i = 0;
			j++;
			
			if((dummy_us=(table->tx_interval*1000-k*table->tx_duration)) >= 0)
			{
				dummy(table, dummy_us);
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
		if((dummy_us=(table->tx_interval*1000-k*table->tx_duration)) >= 0)
		{
			dummy(table, dummy_us);
			f = 1;
		}
		else
		{
			dummy(table, table->tx_duration*2);
		}
	}
	
	wait(2000);
}

static UINT8 query_miss_slot = 0;
static 	UINT8 first_pkg_data[SIZE_ESL_DATA_BUF] = {0};

static INT32 m1_query_miss(updata_table_t *table, UINT8 timer)
{
	INT32 ret = 0;
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	UINT8 rxbuf[SIZE_ESL_DATA_BUF] = {0};
	INT32 i;
	UINT32 deal_timeout = table->deal_duration*1000;
	UINT8 channel = 0;
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);

	pdebug("m1_query_miss, timer is %d.\r\n", timer);
	
	for(i = 0; i < table->esl_num; i++, query_miss_slot++)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_query_miss quit\r\n");
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
		
		set_datarate(table->tx_datarate);
//		channel = g3_get_channel(pESL[i].first_pkg_addr);
		get_one_data(pESL[i].first_pkg_addr, NULL, &channel, NULL, first_pkg_data, sizeof(first_pkg_data));
		pdebug("query miss esl 0x%02x-0x%02x-0x%02x-0x%02x, txbps=%d, rxbps=%d, ch=%d, timeout=%d.\r\n", \
				pESL[i].esl_id[0], pESL[i].esl_id[1], pESL[i].esl_id[2], pESL[i].esl_id[3], \
				table->tx_datarate, table->rx_datarate, channel, deal_timeout);
		memset(data, 0, sizeof(data));
		g3_make_link_query(pESL[i].esl_id, get_pkg_sn_f(pESL[i].first_pkg_addr+(pESL[i].total_pkg_num-1)*32, 7), \
								query_miss_slot, first_pkg_data, data, sizeof(data));
#ifdef DEBUG_CTRL_OF_OSD
		if((data[4]&0xe0) != 0x40)
		{
			perr("OSD CTRL:%02X-%02X-%02X-%02X\r\n", pESL[i].esl_id[0], pESL[i].esl_id[1], pESL[i].esl_id[2], pESL[i].esl_id[3]);
			perrhex(data, sizeof(data));
		}
#endif
		send_data(pESL[i].esl_id, data, sizeof(data), channel, 2000);
		set_datarate(table->rx_datarate);
		memset(rxbuf, 0, sizeof(rxbuf));
		if(recv_data(table->master_id, rxbuf, sizeof(rxbuf), channel, deal_timeout) == 0)
		{
			pdebug("recv timeout.\r\n");
			continue;
		}		
		pdebug("recv:");
		pdebughex(rxbuf, sizeof(rxbuf));
		ret++;
		
		if(g3_check_link_query(pESL[i].esl_id, pESL[i].total_pkg_num, query_miss_slot, first_pkg_data, rxbuf, sizeof(rxbuf)) == 0)
		{
			perr("data check error.\r\n");
			continue;
		}
	
		memcpy((UINT8 *)&pESL[i].failed_pkg_num, &rxbuf[2], sizeof(pESL[i].failed_pkg_num)); // get failed pkg sn
		memcpy((UINT8 *)pESL[i].failed_pkg, &rxbuf[4], MAX_FAILED_PKG_NUM*2);
		//memset((UINT8 *)pESL[i].failed_pkg, 0 , MAX_FAILED_PKG_NUM*2);
		pESL[i].failed_pkg_num &= MASK_OF_PKG_SN;
		if(pESL[i].failed_pkg_num == 0) // miss 0 pkg, tx successfully
		{
			//pESL[i].failed_pkg_num = 0;
			pESL[i].ack = rxbuf[4];
#ifdef DEBUG_5
			if(pESL[i].ack != 0x40)
			{
				perr("%02X-%02X-%02X-%02X: %02X\r\n", 
						pESL[i].esl_id[0], pESL[i].esl_id[1], pESL[i].esl_id[2], pESL[i].esl_id[3], pESL[i].ack);
				perr("q: ");
				perrhex(data, sizeof(data));
				perr("r: ");
				perrhex(rxbuf, sizeof(rxbuf));
			}
			else
			{
				pinfo("%02X-%02X-%02X-%02X: 0x40\r\n", 
						pESL[i].esl_id[0], pESL[i].esl_id[1], pESL[i].esl_id[2], pESL[i].esl_id[3]);			
			}
#endif
		}
		else if(pESL[i].failed_pkg_num == pESL[i].total_pkg_num)
		{
			pESL[i].failed_pkg_offset = 0;
		}
		else // > 0 && < total_pkg_num
		{
			pESL[i].failed_pkg_num = pESL[i].failed_pkg_num <= MAX_FAILED_PKG_NUM ? pESL[i].failed_pkg_num : MAX_FAILED_PKG_NUM;
		}
	
		pdebug("failed pkg num = %d\r\n", pESL[i].failed_pkg_num);
	}

	dummy(table, table->tx_duration*2);

	wait(2000);
		
	return ret;
}

INT32 m1_send_sleep(updata_table_t *table, UINT8 timer)
{
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	UINT32 i;
	INT32 ret = 0;
	UINT8 channel = 0;
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;
	
	pdebug("mode1_send_sleep(), timer: %d.\r\n", timer);
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);
	
	for(i = 0; i < table->esl_num; i++)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_send_sleep quit\r\n");
			break;
		}
		
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}
		
		if((pESL[i].failed_pkg_num==0) && (pESL[i].sleep_flag>0))
		{
			pdebug("sleep 0x%02X-0x%02X-0x%02X-0x%02X\r\n", \
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

INT32 m1_sleep_all(updata_table_t *table)
{
	UINT8 data[SIZE_ESL_DATA_BUF] = {0};
	UINT32 i;
	INT32 ret = 0;
	UINT8 channel = 0;
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;
	
	pdebug("m1_sleep_all\r\n");
	
	set_datarate(table->tx_datarate);
	set_power(table->tx_power);
	
	for(i = 0; i < table->esl_num; i++)
	{
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_sleep_all quit\r\n");
			break;
		}
		
		pdebug("sleep 0x%02X-0x%02X-0x%02X-0x%02X\r\n", \
				pESL[i].esl_id[0], pESL[i].esl_id[1], \
				pESL[i].esl_id[2], pESL[i].esl_id[3]);
		make_sleep_data(pESL[i].esl_id, table->id_x_ctrl, data, sizeof(data));
		get_one_data(pESL[i].first_pkg_addr, NULL, &channel, NULL, NULL, 0);
		send_data(pESL[i].esl_id, data, sizeof(data), channel, 6000);
		ret++;
	}
	
	return ret;
}

UINT8 m1_updata_loop(updata_table_t *table)
{
	UINT8 ret = 0;
	UINT8 timer = 0;
	UINT16 timeout = table->esl_work_duration * 10 * 85 / 100;
	UINT16 leftime = 0;
		
	pdebug("m1_updata_loop().\r\nfirst rount timeout is %d.\r\n", timeout);	
	if((timer=TIM_Open(100, timeout, TIMER_UP_CNT)) == ALL_TIMER_ACTIVE)
	{
		goto done;
	}
	m1_transmit(table, timer);
	leftime = timeout - TIM_GetCount(timer);
	TIM_Close(timer);
	if(Core_GetQuitStatus() == 1)
	{
		pdebug("m1_updata_loop quit\r\n");
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
		m1_query_miss(table, timer);
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_updata_loop quit1\r\n");
			break;
		}
		
		m1_send_sleep(table, timer);
		if(table->ok_esl_num == table->esl_num)
		{
			break;
		}
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_updata_loop quit2\r\n");
			break;
		}	
		
		m1_transmit(table, timer);
		if(TIM_CheckTimeout(timer) == 1)
		{
			break;
		}
		if(Core_GetQuitStatus() == 1)
		{
			pdebug("m1_updata_loop quit3\r\n");
			break;
		}		
	}
	
	TIM_Close(timer);

	m1_sleep_all(table);
		
	ret = 1;
	
done:	
	return ret;
}

void m1_make_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset)
{	
	UINT32 offset = 0;
	INT32 i, j;
	UINT8 ack = 0;
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;
	UINT16 last_failed_sn = 0;
	UINT16 sn = 0;

	pdebug("m1_make_ack\r\n");
	
	*ack_len = table->pkg_num*5 + 4 + 19;
	*ack_addr = Flash_Malloc(*ack_len);	
	if(*ack_addr == MALLOC_FAIL)
	{
		perr("m1_make_ack malloc flash addr = 0x%08X, len = %d!\r\n", \
				*ack_addr, *ack_len);
		*ack_len = 0;
		goto done;
	}
	else
	{
		pdebug("m1_make_ack malloc flash addr = 0x%08X, len = %d, success.\r\n", \
				*ack_addr, *ack_len);
	}
	
	offset = *ack_addr;
	if(g3_set_ack_para(offset, table->sid, 0x0900, table->pkg_num*5+11, 0, table->pkg_num) == 0)
	{
		perr("m1_make_ack set ack para!\r\n");
		*ack_len = 0;
		goto done;
	}
	offset += 21;

	for(i = 0; i < table->esl_num; i++)
	{	
		for(j = 0; j < pESL[i].total_pkg_num; j++)
		{
			if(pESL[i].failed_pkg_num == 0)
			{
				//ack = pESL[i].ack == 0x40 ? 1 : pESL[i].ack; // old ack
				ack = pESL[i].ack == 1 ? 2 : pESL[i].ack; // new ack
				
//				if(pESL[i].ack == 0x40)
//				{
//					ack = 1;
//				}
//				else
//				{
//					ack = pESL[i].ack;
//				}
			}
			else if(pESL[i].failed_pkg_num == pESL[i].total_pkg_num)
			{
				ack = 0;
			}
			else if(pESL[i].failed_pkg_num >= MAX_FAILED_PKG_NUM)
			{
				last_failed_sn = get_missed_sn_r(pESL[i].failed_pkg, (MAX_FAILED_PKG_NUM-1));
				sn = get_pkg_sn_f(pESL[i].first_pkg_addr+j*SIZE_ESL_DATA_SINGLE, sn_offset);
				if((sn==0) || (sn>last_failed_sn))
				{
					ack = 0;
				}
				else if(check_failed_pkg_r(sn, pESL[i].failed_pkg, MAX_FAILED_PKG_NUM) == 1)
				{
					ack = 0;
				}
				else
				{
					//ack = 1; //old ack
					ack = 0x40; //new ack
				}			
			}
			else
			{
				sn = get_pkg_sn_f(pESL[i].first_pkg_addr+j*SIZE_ESL_DATA_SINGLE, sn_offset);
				if(sn == 0)
				{
					ack = 0;
				}
				else if(check_failed_pkg_r(sn, pESL[i].failed_pkg, pESL[i].failed_pkg_num) == 1)
				{
					ack = 0;
				}
				else
				{
					//ack = 1; //old ack
					ack = 0x40; //new ack
				}
			}

			if(g3_set_ack(offset, pESL[i].esl_id, ack) == 0)
			{
				perr("m1_make_ack set ack!\r\n");
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
		perr("m1_make_ack set crc!\r\n");
		*ack_len = 0;
	}
	
done:	
	return;
}

void m1_make_new_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset)
{	
	UINT32 offset = 0;
	INT32 i;
	UINT8 ack = 0;
	mode1_esl_t *pESL = (mode1_esl_t *)table->data;

	pdebug("m1_make_new_ack\r\n");
	// one ack : 4B ID + 1B ACK + 20B DETAIL, other: 4B sid, 2B cmd, 4B cmdlen, 1B para, 8B rev, 2B num, 2B crc
	*ack_len = table->esl_num*(4+1+20) + 4 + 19; 
	*ack_addr = Flash_Malloc(*ack_len);	
	if(*ack_addr == MALLOC_FAIL)
	{
		perr("m1_make_new_ack malloc flash addr = 0x%08X, len = %d!\r\n", \
				*ack_addr, *ack_len);
		*ack_len = 0;
		goto done;
	}
	else
	{
		pdebug("m1_make_new_ack malloc flash addr = 0x%08X, len = %d, success.\r\n", \
				*ack_addr, *ack_len);
	}
	
	offset = *ack_addr;
	if(g3_set_ack_para(offset, table->sid, CMD_ACK_NEW, table->esl_num*(4+1+20)+11, 0, table->esl_num) == 0)
	{
		perr("m1_make_new_ack set ack para!\r\n");
		*ack_len = 0;
		goto done;
	}
	offset += 21;

	for(i = 0; i < table->esl_num; i++)
	{	
		if(pESL[i].failed_pkg_num == 0) //通讯上，所有包都发成功
		{
			ack = pESL[i].ack == 1 ? 2 : pESL[i].ack; // new ack
		}
		else //通讯上，有丢包，丢包术小于10
		{
			ack = 0;
		}

		if(g3_set_new_ack(offset, pESL[i].esl_id, ack, pESL[i].failed_pkg, MAX_FAILED_PKG_NUM*2) == 0)
		{
			perr("m1_make_new_ack set ack!\r\n");
			*ack_len = 0;
			goto done;
		}
		else
		{
			offset += 4+1+20;
		}
	}
	
	if(g3_set_ack_crc(*ack_addr, *ack_len-2) == 0)
	{
		perr("m1_make_new_ack set crc!\r\n");
		*ack_len = 0;
	}
	
done:	
	return;
}

