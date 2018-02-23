#ifndef _G3_BASIC_H_
#define _G3_BASIC_H_

#include "datatype.h"

void enter_txrx(void);
void exit_txrx(void);
//UINT16 cal_crc16(UINT8 ctrl, const UINT8 *eslid, const UINT8 *pdata, UINT8 len);
UINT8 send_data(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout);
UINT8 recv_data(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout);
void rf_preset_for_hb_recv(void);
void rf_exit_from_hb_recv(void);
UINT8 get_hb_rssi(void);
UINT8 recv_data_for_hb(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout); //timeout unit: 1us
UINT8 send_without_wait(UINT8 *id, UINT8 *data, UINT8 len, UINT8 ch, UINT16 timeout);
INT16 send_and_recv(UINT8 *id, UINT8 *src, UINT8 src_len, UINT8 ch, UINT16 txbps, UINT16 timeout, \
					UINT8 *recvid, UINT8 *dst, UINT8 dst_len, UINT16 rxbps);
void set_datarate(UINT16 datarate);
void set_power(UINT8 power);
void set_cmd_stby(void);
void set_cmd_pll(void);
UINT8 get_rssi(void);
void wait(INT32 nus);
void set_rx_para(UINT8 *id, UINT16 datarate, UINT8 ch, UINT8 fifosize);
void set_rx_start(void);
INT32 get_rx_data(UINT8 *dst, UINT8 dstsize);
INT32 check_rx_status(UINT16 timeout); //unit ms



#endif

