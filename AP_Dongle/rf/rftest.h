#ifndef _RF_TEST_H_
#define _RF_TEST_H_

#include "datatype.h"

INT32 rft_check_ber_data(UINT8 *src, INT32 len);
INT32 rft_ber(UINT8 *ack_buf, INT32 size);
void rft_tx_null(void);
INT32 rft_scan_bg(UINT8 *src, INT32 srclen, UINT8 *dst, INT32 dstsize);
INT32 rf_txrx(UINT8 *cmd_buf, INT32 cmd_len, UINT8 *ack_buf, INT32 ack_buf_size);

#endif
