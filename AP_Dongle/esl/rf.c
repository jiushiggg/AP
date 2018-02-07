#include "rf.h"
#include "bsp.h"
#include "crc16.h"
#include "debug.h"

/*
** timeout unit is us
*/

void enter_txrx()
{
//	RFC_RecoverFromRumanTest();
//	set_cmd_pll();
}

void exit_txrx()
{
//	set_cmd_stby();
}

UINT8 send_data(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout) //timeout unit: 1us
{
//	RFC_SetChannel(ch);
//	RFC_WriteID(id);
//	RFC_SetFifoLen(len);
//	RFC_WriteFifo(data, len);
//
//	RFC_Cmd(RFC_CMD_TX);
//	BSP_Delay1US(50);
//	timeout -= 50;
//	while(RFC_CheckTXOrRX())
//	{
//		if((timeout--) <= 0)
//		{
//			RFC_Cmd(RFC_CMD_PLL);
//			return 0;
//		}
//		BSP_Delay1US(1);
//	}
//
//	RFC_Cmd(RFC_CMD_PLL);
//	return len;
}

UINT8 recv_data(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout) //timeout unit: 1us
{
//	RFC_SetChannel(ch);
//	RFC_WriteID(id);
//	RFC_SetFifoLen(len);
//
//	RFC_Cmd(RFC_CMD_RX);
//	BSP_Delay1US(100);
//	timeout-=100;
//	while(RFC_CheckTXOrRX())
//	{
//		if((timeout--) <= 0)
//		{
//			RFC_Cmd(RFC_CMD_PLL);
//			return 0;
//		}
//		BSP_Delay1US(1);
//	}
//
//	RFC_Cmd(RFC_CMD_PLL);
//
//	if(RFC_CheckCRC() == 0)
//	{
//		return 0;
//	}
//
//	RFC_ReadFifo(data, len);
//	return len;
}

#if 0

void rf_preset_for_hb_recv()
{
	RFC_SetRssiMeasurementMode(1);
}

void rf_exit_from_hb_recv()
{
	RFC_SetRssiMeasurementMode(0);
}

UINT8 recv_data_for_hb(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout) //timeout unit: 1us
{
	RFC_SetChannel(ch);
	RFC_WriteID(id);
	RFC_SetFifoLen(len);
	
	RFC_Cmd(RFC_CMD_RX);
	BSP_Delay1US(100);
	timeout-=100;
	while(RFC_CheckTXOrRX())
	{
		if((timeout--) <= 0)
		{
			RFC_Cmd(RFC_CMD_PLL);
			return 0;
		}
		BSP_Delay1US(1);
	}
	
	RFC_Cmd(RFC_CMD_PLL);
	RFC_ReadFifo(data, len);
	
	if(RFC_CheckCRC() == 0)
	{
		return 255;
	}
	else
	{
		return len;
	}
}

#else

static UINT8 _hb_rssi = 0;

UINT8 get_hb_rssi()
{
	return _hb_rssi;
}

void rf_preset_for_hb_recv()
{
//	RFC_SetGio1Mode(1);
//	RFC_SetRssiMeasurementMode(1);
}

void rf_exit_from_hb_recv()
{
//	RFC_SetGio1Mode(0);
//	RFC_SetRssiMeasurementMode(0);
}

UINT8 recv_data_for_hb(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout) //timeout unit: 1us
{
//	RFC_SetChannel(ch);
//	RFC_WriteID(id);
//	RFC_SetFifoLen(len);
//
//	RFC_Cmd(RFC_CMD_RX);
//	BSP_Delay1US(100);
//	timeout-=100;
//	while(RFC_CheckTXOrRX() == 0)
//	{
//		if((timeout--) <= 0)
//		{
//			RFC_Cmd(RFC_CMD_PLL);
//			return 0;
//		}
//		BSP_Delay1US(1);
//	}
//
//	BSP_Delay100US(1);
//	_hb_rssi = get_rssi();
//	timeout = 5000; //5ms
//	while(RFC_CheckTXOrRX() == 1)
//	{
//		if((timeout--) <= 0)
//		{
//			RFC_Cmd(RFC_CMD_PLL);
//
//			return 0;
//		}
//		BSP_Delay1US(1);
//	}
//
//	RFC_Cmd(RFC_CMD_PLL);
//	RFC_ReadFifo(data, len);
//
//	if(RFC_CheckCRC() == 0)
//	{
//		return 255;
//	}
//	else
//	{
//		return len;
//	}
}
#endif

UINT8 send_without_wait(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout)
{
//	while(RFC_CheckTXOrRX())
//	{
//		if((timeout--) <= 0)
//		{
//			break;
//		}
//		BSP_Delay1US(1);
//	}
//
//	RFC_Cmd(RFC_CMD_PLL);
//
//	RFC_SetChannel(ch);
//	RFC_WriteID(id);
//	RFC_SetFifoLen(len);
//	RFC_WriteFifo(data, len);
//
//	RFC_Cmd(RFC_CMD_TX);
	
	return len;
}

INT16 send_and_recv(UINT8 *id, UINT8 *src, UINT8 src_len, UINT8 ch, UINT16 txbps, UINT16 timeout, \
					UINT8 *recvid, UINT8 *dst, UINT8 dst_len, UINT16 rxbps)
{
	INT16 ret = 0;

	return ret;
}

void set_datarate(UINT16 datarate)
{
//	RFC_SetDataRate(datarate);
}

void set_power(UINT8 power)
{
//	RFC_SetPower(power);
}

void set_cmd_stby(void)
{
//	RFC_Cmd(RFC_CMD_STBY);
}

void set_cmd_pll(void)
{
//	RFC_Cmd(RFC_CMD_PLL);
}

UINT8 get_rssi(void)
{
//	return RFC_GetRssi();
}

void wait(INT32 nus)
{
//	while(RFC_CheckTXOrRX())
//	{
//		nus--;
//		if(nus < 0)
//		{
//			break;
//		}
//		BSP_Delay1US(1);
//	}
//
//	RFC_Cmd(RFC_CMD_PLL);
}

void set_rx_para(UINT8 *id, UINT16 datarate, UINT8 ch, UINT8 fifosize)
{
//	RFC_SetChannel(ch);
//	RFC_SetDataRate(datarate);
//	RFC_WriteID(id);
//	RFC_SetFifoLen(fifosize);
}

void set_rx_start(void)
{
//	RFC_Cmd(RFC_CMD_RX);
}

INT32 get_rx_data(UINT8 *dst, UINT8 dstsize)
{
//	if(RFC_CheckCRC() == 0)
//	{
//		return 0;
//	}
//	else
//	{
//		RFC_ReadFifo(dst, dstsize);
//		return (INT32)dstsize;
//	}
}

INT32 check_rx_status(UINT16 timeout) //unit ms
{
//	INT32 to = timeout * 1000;
//
//	while(RFC_CheckTXOrRX())
//	{
//		if((to--) <= 0)
//		{
//			return 1; //rxing
//		}
//		BSP_Delay1US(1);
//	}
//
//	return 0; // rx end
}



