
#include <string.h>
#include <stdio.h>

#include <ti/sysbios/BIOS.h>

#include "debug.h"
#include "xmodem.h"
#include "crc16.h"
#include "flash.h"
#include "device.h"
#include "event.h"

#include "bsp.h"

#if 0
#define X_DEBUG(x) printf x
#else
#define X_DEBUG(x) ((void)0)
#endif



#define XMODEM_CMD_DFT			0X00
#define XMODEM_CMD_SOH			0X01
#define XMODEM_CMD_EOT			0X04
#define XMODEM_CMD_ACK			0X06
#define XMODEM_CMD_NAK			0X15
#define XMODEM_CMD_CAN			0X18


#define XMODEM_OFFSET_BAS		0
#define XMODEM_OFFSET_CMD		XMODEM_OFFSET_BAS
#define XMODEM_OFFSET_SN		(XMODEM_OFFSET_BAS+XMODEM_LEN_CMD)
#define XMODEM_OFFSET_DAT		(XMODEM_OFFSET_BAS+XMODEM_LEN_CMD+XMODEM_LEN_SN)
#define XMODEM_OFFSET_CRC		(XMODEM_OFFSET_BAS+XMODEM_LEN_CMD+XMODEM_LEN_SN+XMODEM_LEN_DAT)	

//#define TIMEOUT_RECV				10
#define RETRYTIME_TX				10
#define RETRYTIME_NAK				10

uint8_t recCmdAckFlg    =   0;
/*
** xmodem function
*/
static UINT8 Xmodem_CheckCrc(UINT8 *xbuf)
{
	UINT16 crc1, crc2;

	memcpy((void *)&crc1, xbuf+XMODEM_OFFSET_CRC, sizeof(crc1));
	crc2 = Crc16_Cal(xbuf, XMODEM_LEN_ALL-XMODEM_LEN_CRC);

	if(crc1 != crc2)
	{
		return 1;
	}
	else
	{
		return 1;
	}
}

UINT8 Xmodem_RecvCmd(INT32 dev, INT32 timeout)
{
	UINT8 cmd = 0;
	INT32 recv_len = 0;

	if(Device_Select(dev, timeout) > 0)
	{
		recv_len = Device_Recv(dev, &cmd, sizeof(cmd), timeout);
		if(recv_len != sizeof(cmd))
		{
			cmd = 0;			
		}
	}
	
	X_DEBUG((">>>Xmodem_RecvCmd:\r\nrecv_len = %d, cmd = 0x%02X.\r\n", recv_len, cmd));
	
	return cmd;
}

INT32 Xmodem_SendCmd(INT32 dev, UINT8 cmd, UINT8 recv_ack_flag, INT32 timeout)
{
	INT32 retry_time = RETRYTIME_TX;
	INT32 send_len = 0;
	UINT8 recv_ack = 0;
	INT32 read_len = 0;
	INT32 send_ret = 0;
	GGGDEBUG((">>>Xmodem_SendCmd:\r\ncmd = 0x%02X, flag = %d, timeout = %d, ", cmd, recv_ack_flag, timeout));
	X_DEBUG((">>>Xmodem_SendCmd:\r\ncmd = 0x%02X, flag = %d, timeout = %d, ", cmd, recv_ack_flag, timeout));
	while(retry_time > 0)
	{	
		send_ret = Device_Send(dev, &cmd, sizeof(cmd), timeout);
		X_DEBUG(("send return %d, ", send_ret));
		GGGDEBUG(("send return %d, ", send_ret));
		if(send_ret != sizeof(cmd))
		{
			retry_time--;
			continue;
		}

		if(recv_ack_flag == 0)
		{
			send_len += 1;
			break;
		}
	
		if(Device_Select(dev, timeout) <= 0)
		{
			retry_time--;
			continue;
		}

		recCmdAckFlg = true;
		Device_Recv_pend(EVENT_WAIT_FOREVER);
		read_len = xcb_recv_len_once;
		recv_ack = recv_once_buf[0];
		GGGDEBUG(("recv len = %d, ack = 0x%02X.", read_len, recv_ack));
		if(read_len != sizeof(recv_ack))
		{
			retry_time--;
			continue;
		}
		
		if(recv_ack == XMODEM_CMD_ACK)
		{
			send_len += 1;
			break;
		}
		else if(recv_ack == XMODEM_CMD_CAN)
		{
			break;
		}
		else //XMODEM_CMD_CAN or other, cancl this send loop
		{
			retry_time--;
			continue;
		}
	}

	X_DEBUG(("\r\n"));
	GGGDEBUG(("\r\n"));
	return send_len;
}

UINT8 recv_once_buf[XMODEM_LEN_ALL] = {0};

INT32 Xmodem_RecvOnce(xmodem_t *x, INT32 dev, UINT8 **dst, INT32 timeout)
{
	INT32 ret = 0;
	UINT8 tx_cmd = XMODEM_CMD_NAK;
//	INT32 recv_len = 0;
//	INT32 copy_len = 0;

	X_DEBUG((">>>Xmodem_RecvOnce:\r\n"));
	
	BSP_GPIO_ToggleDebugPin();
//	memset(recv_once_buf, 0, XMODEM_LEN_ALL);
//	recv_len = Device_Recv(dev, recv_once_buf, sizeof(recv_once_buf), timeout);
	BSP_GPIO_ToggleDebugPin();

	X_DEBUG(("recv data len=%d, cmd=0x%02X,sn=%d,lastsn=%d, ", xcb_recv_len, recv_once_buf[0], recv_once_buf[1], x->last_recv_sn));
//	pdebughex(recv_once_buf, sizeof(recv_once_buf));
	if (recv_once_buf[2] == 0x35 && recv_once_buf[2] == 0x30){
	    GGGDEBUG(("up"));
	}
	
	if(xcb_recv_len_once==XMODEM_LEN_ALL)
	{
		/* check crc */
		BSP_GPIO_ToggleDebugPin();
		if(Xmodem_CheckCrc(recv_once_buf) == 0)
		{
			X_DEBUG(("crc check error!"));
			goto recv_tx_ack;
		}
		BSP_GPIO_ToggleDebugPin();
		
		/* check cmd */
		if(recv_once_buf[0] != XMODEM_CMD_SOH)
		{
			X_DEBUG(("cmd check error!"));
			goto recv_tx_ack;
		}	
		
		/* check sn */
		if(recv_once_buf[1] == (UINT8)(x->last_recv_sn+1))
		{
			x->last_recv_cmd = recv_once_buf[0];
			x->last_recv_sn += 1;
			tx_cmd = XMODEM_CMD_ACK;
			//recv_len = recv_len > len ? len : recv_len;
//			copy_len = XMODEM_LEN_DAT > len ? len : XMODEM_LEN_DAT;
			*dst = recv_once_buf + XMODEM_OFFSET_DAT;
//			memcpy(dst, recv_buf+XMODEM_OFFSET_DAT, copy_len);
			ret = XMODEM_LEN_DAT;
		}
		else if(recv_once_buf[1] == x->last_recv_sn)
		{
		    GGGDEBUG(("ASAMEPKG%d,CMD:%d", xcb_recv_len_once, recv_once_buf[0]));
			X_DEBUG(("a same pkg!"));
			tx_cmd = XMODEM_CMD_ACK;
			goto recv_tx_ack;
		}
		else //total wrong sn
		{
			X_DEBUG(("sn check error!"));
			goto recv_tx_ack;
		}
	}
	else if(xcb_recv_len_once==XMODEM_LEN_CMD)
	{
		X_DEBUG(("recv cmd: 0x%02X, ", recv_once_buf[0]));
		if(recv_once_buf[0] == XMODEM_CMD_EOT)
		{
			tx_cmd = XMODEM_CMD_ACK;
			x->last_recv_cmd = XMODEM_CMD_EOT;
//			BSP_Delay1MS(30);
			goto recv_tx_ack;
		}
	}

recv_tx_ack:
	if(tx_cmd == XMODEM_CMD_NAK)
	{
		if((++x->nak_times) >= RETRYTIME_NAK)
		{
			tx_cmd = XMODEM_CMD_CAN;
			x->nak_times = 0;
			ret = -1;
			GGGDEBUG(("RET-1"));
		}
	}
	else //ack
	{	
		x->nak_times = 0;
	}
	X_DEBUG(("\r\n"));
	
	Xmodem_SendCmd(dev, tx_cmd, 0, timeout);
	
	return ret;
}
#if 0
INT32 Xmodem_Recv(xmodem_t *x, INT32 dev, UINT8 *dst, INT32 dst_len, INT32 timeout)
{
	INT32 recv_len_total = 0;
	INT32 recv_len_once = 0;
	INT32 copy_len = 0;
//	UINT8 recv_buf[XMODEM_LEN_DAT] = {0};
	UINT8 *pRecv = NULL;

	while(dst_len > 0)
	{
		if(Device_Select(dev, timeout) <= 0)
		{
			continue;
		}

//		memset(recv_buf, 0, sizeof(recv_buf));
		recv_len_once = Xmodem_RecvOnce(x, dev, &pRecv, timeout);
		if(recv_len_once < 0)
		{
			break;
		}
		
		copy_len = recv_len_once > dst_len ? dst_len: recv_len_once;
		if(copy_len == 0)
		{
			if(x->last_recv_cmd == XMODEM_CMD_EOT)
			{
				break;
			}
			else
			{
				continue;
			}
		}

		memcpy(dst, pRecv, copy_len);

		dst += copy_len;
		dst_len -= copy_len;
		recv_len_total += copy_len;		
	}
	
	return recv_len_total;
}
#endif
/*
 ** tx functions
 */
static INT32 Xmodem_MakePkg(UINT8 cmd, UINT8 sn, UINT8  *src, UINT16 src_len, \
								UINT8 *dst, UINT16 dst_len)
{
	INT32 ret = 0;
	UINT16 crc = 0;

	if(src_len > XMODEM_LEN_DAT)
	{
		ret = -1;
		goto done;
	}
	
	memset(dst, 0, dst_len);
	dst[0] = cmd;
	dst[1] = sn;
	memcpy(dst+XMODEM_OFFSET_DAT, src, src_len);
	crc = Crc16_Cal(dst, XMODEM_LEN_ALL-XMODEM_LEN_CRC);
	memcpy(dst+XMODEM_OFFSET_CRC, &crc, XMODEM_LEN_CRC);

	ret = XMODEM_LEN_ALL;
done:
	return ret;
}

INT32 Xmodem_SendOnce(xmodem_t *x, INT32 dev, UINT8 *src, INT32 len, INT32 timeout)
{
	INT32 ret = 0;
	INT32 send_len = 0;
	UINT8 send_buf[XMODEM_LEN_ALL] = {0};
	INT32 retry_time = RETRYTIME_TX;
	UINT8 recv_ack = 0;

	GGGDEBUG((">>>Xmodem_SendOnce:\r\n"));
	X_DEBUG((">>>Xmodem_SendOnce:\r\n"));
	send_len = len > XMODEM_LEN_DAT ? XMODEM_LEN_DAT : len;
	Xmodem_MakePkg(XMODEM_CMD_SOH, x->send_sn, src, send_len, send_buf, sizeof(send_buf));
	X_DEBUG(("pkg sn=%d,send_len=%d,tx_cmd=0x%02X, ", x->send_sn, send_len, XMODEM_CMD_SOH));
	GGGDEBUG(("pkg sn=%d,send_len=%d,tx_cmd=0x%02X, ", x->send_sn, send_len, XMODEM_CMD_SOH));
	/* tx soh,sig,stx */
	while((retry_time--) > 0)
	{	
		if(Device_Send(dev, send_buf, sizeof(send_buf), timeout) != sizeof(send_buf))
		{
			continue;
		}

		//recv_ack = Xmodem_RecvCmd(dev, timeout);
        recCmdAckFlg = true;
        Device_Recv_pend(EVENT_WAIT_FOREVER);
        recv_ack = recv_once_buf[0];
		X_DEBUG(("recv ack: 0x%02X.", recv_ack));
		GGGDEBUG(("recv ack: 0x%02X.", recv_ack));
		if(recv_ack == XMODEM_CMD_ACK)
		{
			x->send_sn += 1;
			ret = send_len;
			break;
		}
		else if(recv_ack == XMODEM_CMD_CAN)
		{
			ret = -1;
			break;
		}
		else // "NAK" or other, retry
		{
			ret = 0;
			continue;
		}
	}

	if(retry_time <= 0)
	{
		ret = -1;
	}
	
	X_DEBUG(("\r\n"));
	return ret;
}

INT32 Xmodem_Send(xmodem_t *x, INT32 dev, UINT8 *src, INT32 len, INT32 timeout)
{
	UINT8 *ptr = src;
	INT32 left_len = len;
	INT32 send_len_once = 0;
	INT32 send_len_total = 0;

	GGGDEBUG((">>>Xmodem_Send:\r\n"));
	X_DEBUG((">>>Xmodem_Send:\r\n"));
	/* init send */
	x->send_sn = 1;
	/* send data */
	while(left_len > 0)
	{	
		send_len_once = Xmodem_SendOnce(x, dev, ptr, left_len, timeout);
		if(send_len_once > 0) //ack
		{
			ptr += send_len_once;
			left_len -= send_len_once;
			send_len_total += send_len_once;
			send_len_once = 0;
		}
		else if(send_len_once == 0) // nak or other
		{
			continue;
		}
		else // "NAK" or other, retry
		{
			send_len_total = 0;
			break;
		}
	}
	
	/* all data has been sent, send eot */
	if(left_len <= 0)
	{
		if(Xmodem_SendCmd(dev, XMODEM_CMD_EOT, 1, timeout) != 1)
		{
			send_len_total = 0;
		}
	}

	return send_len_total;
}

void Xmodem_Reset(xmodem_t *x)
{
	memset(x, 0, sizeof(xmodem_t));
}



xmodem_t xcb;
UINT8 xcb_recv_buf[XCB_RECV_BUF_SIZE] = {0};
INT32 xcb_recv_len = 0;
INT32 xcb_recv_len_once = 0;

void Xmodem_InitCallback(void)
{
	memset(&xcb, 0 , sizeof(xmodem_t));
	memset(xcb_recv_buf, 0, sizeof(xcb_recv_buf));
	xcb_recv_len = 0;
}

INT32 Xmodem_RecvCallBack(void)
{
	INT32 copy_len = 0;
	INT32 dst_len = 0;
	INT32 ret = 0;
	UINT8 *pRecv = NULL;

	X_DEBUG((">>>Xmodem_RecvCallBack:\r\n"));
	xcb_recv_len_once = Xmodem_RecvOnce(&xcb, 1, &pRecv, 100);
	if(xcb_recv_len_once < 0)
	{
		ret = -1;
		goto done;
	}
	else if(xcb_recv_len_once == 0)
	{
		if(xcb.last_recv_cmd == XMODEM_CMD_EOT)
		{
			ret = xcb_recv_len;
			X_DEBUG((">>>Xmodem_RecvCallBack recv EOT, recv len = %d.\r\n", xcb_recv_len));
			goto done;
		}
		else
		{
			goto done;
		}
	}
	
	/* xcb_recv_len_once > 0, copy data */
	dst_len = XCB_RECV_BUF_SIZE - xcb_recv_len_once%XCB_RECV_BUF_SIZE;
	copy_len = xcb_recv_len_once > dst_len ? dst_len: xcb_recv_len_once;
	
	if(copy_len > 0)
	{
		memcpy(xcb_recv_buf+xcb_recv_len%XCB_RECV_BUF_SIZE, pRecv, copy_len);
		xcb_recv_len += copy_len;
	}

done:
	return ret;
}

UINT8 *Xmodem_GetCallbackData(UINT32 *len)
{
	*len = xcb_recv_len;
	return xcb_recv_buf;
}

INT32 Xmodem_RecvToFlash(xmodem_t *x, INT32 dev, UINT32 addr, INT32 dst_len, INT32 timeout)
{
	INT32 recv_len_total = 0;
	INT32 recv_len_once = 0;
	INT32 copy_len = 0;
//	UINT8 recv_buf[XMODEM_LEN_DAT] = {0};
	UINT8 *pRecv = NULL;

	X_DEBUG((">>>Xmodem_RecvToFlash start:\r\n"));
	while((dst_len > 0) || (x->last_recv_cmd != XMODEM_CMD_EOT))
	{
		if(Device_Select(dev, timeout) <= 0)
		{
			X_DEBUG(("Xmodem_RecvToFlash recv timeout!"));
			break;
		}

		recv_len_once = Xmodem_RecvOnce(x, dev, &pRecv, timeout);
		if(recv_len_once < 0)
		{
		    GGGDEBUG(("Xmodem_RecvToFlash recv data len = %d, < 0!", recv_len_once));
			X_DEBUG(("Xmodem_RecvToFlash recv data len = %d, < 0!", recv_len_once));
			break;
		}
		
		copy_len = recv_len_once > dst_len ? dst_len: recv_len_once;
		if(copy_len == 0)
		{
			if(x->last_recv_cmd == XMODEM_CMD_EOT)
			{
			    GGGDEBUG(("Xmodem_RecvToFlash recv EOT!!! total recv len = %d.", recv_len_total));
				X_DEBUG(("Xmodem_RecvToFlash recv EOT!!! total recv len = %d.", recv_len_total));
				break;
			}
			else
			{
				continue;
			}
		}

		BSP_GPIO_ToggleDebugPin();
		if(Flash_Write(addr, pRecv, copy_len) == FALSE)
		{   GGGDEBUG(("Xmodem_RecvToFlash flash write error!"));
			X_DEBUG(("Xmodem_RecvToFlash flash write error!"));
			break;
		}
		BSP_GPIO_ToggleDebugPin();

		addr += copy_len;
		dst_len -= copy_len;
		recv_len_total += copy_len;
		
		copy_len = 0;
	}

	GGGDEBUG(("\r\n"));
	X_DEBUG(("\r\n"));	
	return recv_len_total;
}

INT32 Xmodem_SendFromFlash(xmodem_t *x, INT32 dev, UINT32 addr, INT32 len, INT32 timeout)
{
	UINT32 paddr = addr;
	INT32 left_len = len;
	INT32 send_len_once = 0;
	INT32 send_len_total = 0;
	UINT8 send_buf[XMODEM_LEN_DAT] = {0};
	INT32 copy_len = 0;
	
	X_DEBUG((">>>Xmodem_SendFromFlash:\r\n"));
	/* init send */
	x->send_sn = 1;
	/* send data */
	while(left_len > 0)
	{	
		copy_len = left_len > XMODEM_LEN_DAT ? XMODEM_LEN_DAT : left_len;	
		
		if(Flash_Read(paddr, send_buf, copy_len) == FALSE)
		{
			X_DEBUG(("flash read error!"));
			break;
		}
		
		send_len_once = Xmodem_SendOnce(x, dev, send_buf, copy_len, timeout);
		if(send_len_once > 0) //ack
		{
			paddr += send_len_once;
			left_len -= send_len_once;
			send_len_total += send_len_once;
			send_len_once = 0;
		}
		else if(send_len_once == 0) // nak or other
		{
			continue;
		}
		else // "NAK" or other, retry
		{
			send_len_total = 0;
			break;
		}
	}
	
	/* all data has been sent, send eot */
	if(left_len <= 0)
	{
		if(Xmodem_SendCmd(dev, XMODEM_CMD_EOT, 1, timeout) != 1)
		{
			send_len_total = 0;
		}
	}

	return send_len_total;
}


