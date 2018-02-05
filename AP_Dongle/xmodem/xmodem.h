
#include "datatype.h"

typedef struct {
	UINT8 last_recv_cmd;
	UINT8 last_recv_sn;
	UINT8 send_sn;
	INT32 send_retry_times;
	INT32 nak_times;
}xmodem_t;

void Xmodem_Reset(xmodem_t *x);
INT32 Xmodem_Send(xmodem_t *x, INT32 dev, UINT8 *src, INT32 len, INT32 timeout);
INT32 Xmodem_Recv(xmodem_t *x, INT32 dev, UINT8 *dst, INT32 len, INT32 timeout);
INT32 Xmodem_RecvOnce(xmodem_t *x, INT32 dev, UINT8 **dst, INT32 timeout);
void Xmodem_InitCallback(void);
INT32 Xmodem_RecvCallBack(void);
UINT8 *Xmodem_GetCallbackData(UINT32 *len);
INT32 Xmodem_RecvToFlash(xmodem_t *x, INT32 dev, UINT32 addr, INT32 dst_len, INT32 timeout);
INT32 Xmodem_SendFromFlash(xmodem_t *x, INT32 dev, UINT32 addr, INT32 len, INT32 timeout);
