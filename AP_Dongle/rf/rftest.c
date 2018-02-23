#include <hw_rf.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "rftest.h"
#include "timer.h"
#include "bsp.h"
#include "debug.h"
#include "crc16.h"
#include "corefunc.h"

#define BER_DEBUG

static UINT8 actor = 0;
static UINT8 ber_enable = 1;
static UINT8 power_enable = 1;
static UINT8 fc_enable = 1;

static INT32 test_rounds = 0;

static UINT8 tx_id[4] = {0x59,0x01,0x23,0x45};
static UINT16 tx_datarate = 500;
static UINT8 tx_channel = 168;
static UINT8 tx_power = 168;

static UINT8 rx_id[4] = {0x50,0x12,0x34,0x56};
static UINT16 rx_datarate = 500;
static UINT8 rx_channel = 168;
static UINT8 rx_power = 168;

/*
** local setting
*/
#define TEST_DATA_LEN	26

UINT8 rx_buf[TEST_DATA_LEN] = {0};
UINT8 tx_buf[TEST_DATA_LEN] = {0};

static INT32 ber_rx(void)
{
	INT32 ret = 0;
	UINT8 timer = 0;
//
//	if((timer=TIM_Open(100, 100, 0)) == 0)
//	{
//		ret = -1;
//		goto done;
//	}
//
//	RFC_SetPower(rx_power);
//
//	while(!TIM_CheckTimeout(timer))
//	{
//		RFC_SetChannel(tx_channel);
//		RFC_SetDataRate(tx_datarate);
//		RFC_WriteID(rx_id);
//		RFC_SetFifoLen(TEST_DATA_LEN);
//		memset(rx_buf, 0, TEST_DATA_LEN);
//
//		RFC_Cmd(RFC_CMD_RX);
//		while(RFC_CheckTXOrRX())
//		{
//			if(TIM_CheckTimeout(timer))
//			{
//				break;
//			}
//		}
//		if(TIM_CheckTimeout(timer))
//		{
//			break;
//		}
//		RFC_Cmd(RFC_CMD_PLL);
//
//		if(!RFC_CheckCRC())
//		{
//#ifdef BER_DEBUG
//			printf("crc err\r\n");
//#endif
//			continue;
//		}
//
//#ifdef BER_DEBUG
//		printf("recvd\r\n");
//#else
//		BSP_Delay1MS(1);
//#endif
//		RFC_ReadFifo(rx_buf, TEST_DATA_LEN);
//
//		RFC_SetChannel(rx_channel);
//		RFC_SetDataRate(rx_datarate);
//		RFC_WriteID(tx_id);
//		RFC_SetFifoLen(TEST_DATA_LEN);
//		RFC_WriteFifo(rx_buf, TEST_DATA_LEN);
//
//		RFC_Cmd(RFC_CMD_TX);
//		while(RFC_CheckTXOrRX())
//		{
//			if(TIM_CheckTimeout(timer))
//			{
//				break;
//			}
//		}
//		if(TIM_CheckTimeout(timer))
//		{
//			break;
//		}
//
//		RFC_Cmd(RFC_CMD_PLL);
//	}
//
//done:
//	TIM_Close(timer);
//	RFC_Cmd(RFC_CMD_STBY);
	return ret;
}

float ber = 0.0;
INT32 ber_err_bit = 0;

void set_random_buf(UINT8 *dst, INT32 len)
{
	INT32 i;
	
	dst[0] = 1; // ber ctrl
	
	for(i = 1; i < len; i++)
	{
		dst[i] = rand();
	}
}

INT32 get_err_bit(UINT8 *str1, UINT8 *str2, UINT8 count)
{
	UINT8 i, a;
	INT32 n = 0;
	
	for (i = 0; i < count; i++)
	{
		a = (*(str1+i))^( *(str2+i));
		while(a != 0)
		{
			n++;
			a = a & (a-1);
		}
	}
	
	return n;		
}

UINT8 rssi_f = 0, rssi = 0, rssi_max = 0, rssi_avg = 0 , rssi_min = 0;

void ber_get_rssi(void)
{	
//	rssi = RFC_GetRssi();

#ifdef BER_DEBUG
	printf("rssi=%d\r\n", rssi);
#endif
	
	if(rssi_f == 0)
	{
		rssi_f = 1;
		rssi_min = rssi;
		rssi_avg = rssi;
		rssi_max = rssi;
	}
	else
	{
		rssi_min = rssi < rssi_min ? rssi : rssi_min;
		rssi_max = rssi > rssi_max ? rssi : rssi_max;
		rssi_avg = (rssi_avg + rssi) / 2;
	}	
}

INT16 fc_f = 0, fc = 0, fc_max = 0, fc_avg = 0, fc_min = 0;

void ber_get_fc(void)
{
//	fc = RFC_GetFC();
#ifdef BER_DEBUG
	printf("fc=%d\r\n", fc);
#endif	
	if(fc_f == 0)
	{
		fc_f = 1;
		fc_min = fc;
		fc_max = fc;
		fc_avg = fc;
	}
	else
	{
		fc_min = fc < fc_min ? fc : fc_min;
		fc_max = fc > fc_max ? fc : fc_max;
		fc_avg = (fc_avg + fc) / 2;
	}
}

static INT32 make_tx_result(UINT8 *dst, INT32 size)
{
	INT32 ret = 0;
	UINT8 *ptr = dst;
	
	if(size < 16)
	{
		ret = -1;
		goto done;
	}

	if(ber_enable == 1)
	{
		ptr[0] = 1;
		memcpy(ptr+1, &ber, 4);
	}
	
	if(power_enable)
	{
		ptr[5] = 1;
		ptr[6] = rssi_min;
		ptr[7] = rssi_avg;
		ptr[8] = rssi_max;
	}
	
	if(fc_enable)
	{
		ptr[9] = 1;
		memcpy(ptr+10, &fc_min, 2);
		memcpy(ptr+12, &fc_avg, 2);
		memcpy(ptr+14, &fc_max, 2);
	}
	
	ret = 16;
	
	done:
	return ret;
}

static INT32 ber_make_result(UINT8* dst, INT32 size)
{
	UINT16 crc = 0;
	UINT32 result_ber = ber * 10000;

	memset(dst, 0, size);
	dst[0] = 99; // ctrl of ber result
	
	if(ber_enable == 1)
	{
		dst[1] = 1;
		memcpy(&dst[2], &result_ber, sizeof(result_ber));
	}

	if(power_enable)
	{
		dst[6] = 1;
		dst[7] = rssi_min;
		dst[8] = rssi_avg;
		dst[9] = rssi_max;
	}
	
	if(fc_enable)
	{
		dst[10] = 1;
		memcpy(&dst[11], &fc_min, 2);
		memcpy(&dst[13], &fc_avg, 2);
		memcpy(&dst[15], &fc_max, 2);
	}

	crc = CRC16_CaculateStepByStep(0, dst, size-2);
	crc = CRC16_CaculateStepByStep(crc, rx_id, 4);
	
	memcpy(dst+size-2, &crc, 2);
	
	return size;
}

static INT32 ber_check_result_ack(UINT8* ack, INT32 len)
{
	UINT16 crc1 = 0, crc2 = 0;
	INT32 ret = 0;

	memcpy(&crc1, ack+len-2, 2);

	crc2 = CRC16_CaculateStepByStep(0, ack, len-2);
	crc2 = CRC16_CaculateStepByStep(crc2, rx_id, 4);

	if(crc1 != crc2)
	{
		ret = -1;
		goto done;
	}

	if(ack[0] != 100)
	{
		ret = -2;
		goto done;
	}

	ret = 1;

done:
	return ret;
}

static INT32 ber_tx_result(void)
{
	UINT8 result_buf[26] = {0};
	UINT8 ack_buf[26] = {0};
	INT32 retry = 5;
	INT32 ret = 0;
	
//	ber_make_result(result_buf, sizeof(result_buf));
//	while((retry--) >= 0)
//	{
//		RFC_SetDataRate(tx_datarate);
//		if(send_data(rx_id, result_buf, sizeof(result_buf), tx_channel, 10000) != sizeof(result_buf))
//		{
//			continue;
//		}
//
//		RFC_SetDataRate(rx_datarate);
//		if(recv_data(tx_id, ack_buf, sizeof(ack_buf), rx_channel, 30000) != sizeof(ack_buf))
//		{
//			continue;
//		}
//
//		if(ber_check_result_ack(ack_buf, sizeof(ack_buf)) == 1)
//		{
//			ret = 1;
//			break;
//		}
//	}
//
//	pprint("ber tx result ack: ");
//	phex(ack_buf, sizeof(ack_buf));
//
//	set_cmd_stby();
	return ret;
}

static INT32 ber_tx(void)
{
	INT32 ret = 0;
	UINT8 crc_error = 0;
	INT32 left_rounds = test_rounds;
	INT32 timeout = 10;
//
//	/* reset para */
//	rssi_f = 0;
//	fc_f = 0;
//	ber_err_bit = 0;
//	fc = 0; fc_max = 0; fc_avg = 0; fc_min = 0;
//	rssi = 0; rssi_max = 0; rssi_avg = 0; rssi_min = 0;
//
//	RFC_SetPower(tx_power);
//
//	while((left_rounds--) > 0)
//	{
//		crc_error = 0;
//
//		RFC_SetChannel(tx_channel);
//		RFC_SetDataRate(tx_datarate);
//		RFC_WriteID(rx_id);
//		RFC_SetFifoLen(TEST_DATA_LEN);
//		set_random_buf(tx_buf, TEST_DATA_LEN);
//		RFC_WriteFifo(tx_buf, TEST_DATA_LEN);
//
//		RFC_Cmd(RFC_CMD_TX);
//		timeout = 100;
//		while(RFC_CheckTXOrRX())
//		{
//			if(timeout <= 0)
//			{
//				break;
//			}
//			BSP_Delay100US(1);
//			timeout--;
//		}
//		if(timeout <= 0)
//		{
//#ifdef BER_DEBUG
//			printf("tx to\r\n");
//#endif
//			crc_error = 1;
//			goto cal_ber;
//		}
//
//		RFC_Cmd(RFC_CMD_PLL);
//
//		RFC_SetChannel(rx_channel);
//		RFC_SetDataRate(rx_datarate);
//		RFC_WriteID(tx_id);
//		RFC_SetFifoLen(TEST_DATA_LEN);
//		memset(rx_buf, 0, TEST_DATA_LEN);
//
//		RFC_Cmd(RFC_CMD_RX);
//		timeout = 200;
//		while(RFC_CheckTXOrRX())
//		{
//			if(timeout <= 0)
//			{
//				break;
//			}
//			BSP_Delay100US(1);
//			timeout--;
//		}
//		if(timeout <= 0)
//		{
//#ifdef BER_DEBUG
//			printf("rx to\r\n");
//#endif
//			crc_error = 1;
//			goto cal_ber;
//		}
//
//		/* get rssi and fc */
//		ber_get_rssi();
//		ber_get_fc();
//
//		if(!RFC_CheckCRC())
//		{
//#ifdef BER_DEBUG
//			printf("crc e\r\n");
//#endif
//			crc_error = 1;
//			goto cal_ber;
//		}
//		RFC_ReadFifo(rx_buf, TEST_DATA_LEN);
//#ifdef BER_DEBUG
//		printf("recved\r\n");
//#endif
//cal_ber:
//
//		RFC_Cmd(RFC_CMD_PLL);
//
//		if(crc_error == 0)
//		{
//			ber_err_bit += get_err_bit(rx_buf, tx_buf, TEST_DATA_LEN);
//		}
//		else
//		{
//			ber_err_bit += TEST_DATA_LEN * 8;
//		}
//#ifdef BER_DEBUG
//		printf("err_bit = %d\r\n", ber_err_bit);
//#endif
//	}
//
//	ber = (float)ber_err_bit/(float)(test_rounds*TEST_DATA_LEN*8);
//
//	RFC_Cmd(RFC_CMD_STBY);
	return ret;
}

INT32 rft_check_ber_data(UINT8 *src, INT32 len)
{
	actor = src[0];
	ber_enable = src[1];
	power_enable = src[2];
	fc_enable = src[3];
	
	memcpy(&test_rounds, src+4, 4);
	
	memcpy(tx_id, src+8, 4);
	memcpy(&tx_datarate, src+12, 2);
	tx_channel = src[14];
	tx_power = src[15];
	
	memcpy(rx_id, src+16, 4);
	memcpy(&rx_datarate, src+20, 2);
	rx_channel = src[22];
	rx_power = src[23];

	return actor;
}

void print_ber_para(void)
{
	printf("****** BER PARA *******\r\n");
	printf("actor = %d, ber_e = %d, power_e = %d, fc_e = %d, rounds = %d.\r\n", \
			actor, ber_enable, power_enable, fc_enable, test_rounds);
	printf("txid = %02X-%02X-%02X-%02X, txdatarate = %d, tx_channel = %d, tx_power = %d\r\n", \
			tx_id[0], tx_id[1], tx_id[2], tx_id[3], tx_datarate, tx_channel, tx_power);
	printf("rxid = %02X-%02X-%02X-%02X, rxdatarate = %d, rx_channel = %d, rx_power = %d\r\n", \
			rx_id[0], rx_id[1], rx_id[2], rx_id[3], rx_datarate, rx_channel, rx_power);
	printf("**** BER PARA END *****\r\n");
}

void print_ber_result(void)
{
	printf("****** BER RESULT *******\r\n");

	printf("ber = %f\r\n", ber);
	printf("rssi_min = %d, rssi_avg = %d, rssi_max = %d\r\n", rssi_min, rssi_avg, rssi_max);
	printf("fc_min = %d, fc_avg = %d, fc_max = %d\r\n", fc_min, fc_avg, fc_max);
	
	printf("**** BER RESULT END *****\r\n");
}

INT32 rft_ber(UINT8 *ack_buf, INT32 size)
{
	INT32 ret = 0;
	
//#ifdef BER_DEBUG
//	print_ber_para();
//#endif
//
//	enter_txrx();
//
//	if(actor == 0) // gold board
//	{
//		ber_tx();
//		ret = ber_tx_result();
//#ifdef BER_DEBUG
//		printf("ber tx result return %d\r\n", ret);
//#endif
//		ret = make_tx_result(ack_buf, size);
//#ifdef BER_DEBUG
//		print_ber_result();
//#endif
//	}
//	else //test board
//	{
//		ber_rx();
//	}
//
//	exit_txrx();
	
	return ret;
}

UINT8 gFTRummanTestChannel = 128;
UINT8 gFTRummanTestPower = 2;

void rft_tx_null(void)
{
//	RFC_SetPower(gFTRummanTestPower);
//	RFC_RumanTest(gFTRummanTestChannel);
}

#define SCAN_BG_DEBUG

INT32 rft_scan_bg(UINT8 *src, INT32 srclen, UINT8 *dst, INT32 dstsize)
{
	INT32 ret = 0;
//	float factor = (float)src[3]/100;
//	int scantimes = 0;
//
//	memcpy(&scantimes, &src[4], sizeof(scantimes));
//
//	enter_txrx();
//
//	dst[1] = RFC_CalcBgRssi(src[0], src[1], src[2], src[8], factor, scantimes, &dst[2]);
//	dst[0] = src[0];
//	ret = 3;
//#ifdef SCAN_BG_DEBUG
//	printf("BG scan channel: %d, rssi = %d, dutycycle = %f\r\n", dst[0], dst[1], (float)dst[2]/100);
//#endif
//
//	exit_txrx();
	return ret;
}

#define OFFSET_OF_ACTOR_IN_RFTXRX_CMD		0
#define OFFSET_OF_ID_IN_RFTXRX_CMD			1
#define OFFSET_OF_CHANNEL_IN_RFTXRX_CMD		5
#define OFFSET_OF_BPS_IN_RFTXRX_CMD			6
#define OFFSET_OF_POWER_IN_RFTXRX_CMD		8
#define OFFSET_OF_TIMEOUT_IN_RFTXRX_CMD		9
#define OFFSET_OF_DATALEN_IN_RFTXRX_CMD		13
#define OFFSET_OF_DATA_IN_RFTXRX_CMD		14

INT32 rf_txrx(UINT8 *cmd_buf, INT32 cmd_len, UINT8 *ack_buf, INT32 ack_buf_size)
{
	UINT16 bps = 0;
	UINT8 data_len = *(cmd_buf+OFFSET_OF_DATALEN_IN_RFTXRX_CMD);
	INT32 timeout = 0;
	INT32 ret = 0;
	UINT8 rx_buf[64] = {0};
//
//	//phex(cmd_buf, cmd_len);
//	memcpy(&bps, cmd_buf+OFFSET_OF_BPS_IN_RFTXRX_CMD, sizeof(bps));
//	memcpy(&timeout, cmd_buf+OFFSET_OF_TIMEOUT_IN_RFTXRX_CMD, sizeof(timeout));
//	timeout = timeout*1000;
//
//	set_datarate(bps);
//	if(*(cmd_buf+OFFSET_OF_ACTOR_IN_RFTXRX_CMD) != 0) //tx
//	{
//		printf("actor tx %d timeout is %d\r\n", data_len, timeout);
//		set_power(*(cmd_buf+OFFSET_OF_POWER_IN_RFTXRX_CMD));
//		if(send_data(cmd_buf+OFFSET_OF_ID_IN_RFTXRX_CMD, \
//					cmd_buf+OFFSET_OF_DATA_IN_RFTXRX_CMD, data_len, \
//					*(cmd_buf+OFFSET_OF_CHANNEL_IN_RFTXRX_CMD), timeout) == data_len)
//		{
//			ret = Core_MakeCmdBuf(0x10f0, NULL, 0, ack_buf, ack_buf_size);
//		}
//		else
//		{
//			ret = Core_MakeCmdBuf(0x10f5, NULL, 0, ack_buf, ack_buf_size);
//		}
//	}
//	else //rx
//	{
//		printf("actor rx %d, timeout is %d\r\n", data_len, timeout);
//		if(recv_data(cmd_buf+OFFSET_OF_ID_IN_RFTXRX_CMD, \
//					rx_buf, data_len, *(cmd_buf+OFFSET_OF_CHANNEL_IN_RFTXRX_CMD),\
//					timeout)  == data_len)
//		{
//			*(rx_buf+data_len) = get_rssi();
//			ret = Core_MakeCmdBuf(0x10f0, rx_buf, data_len+1, ack_buf, ack_buf_size);
//		}
//		else
//		{
//			ret = Core_MakeCmdBuf(0x10f5, NULL, 0, ack_buf, ack_buf_size);
//		}
//	}
	
	return ret;
}

