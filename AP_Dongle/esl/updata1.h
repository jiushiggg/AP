#include "datatype.h"
#include "updata.h"

#define MAX_FAILED_PKG_NUM	10

#pragma pack(1)
typedef struct {
	UINT8  esl_id[4];
	UINT8  ack;
	UINT8  sleep_flag;
	UINT32 first_pkg_addr;
	UINT16 total_pkg_num;
	UINT16 failed_pkg_offset;
	UINT16  failed_pkg_num;
	UINT8  failed_pkg[MAX_FAILED_PKG_NUM*2];
} mode1_esl_t;
#pragma pack(1)

UINT16 get_missed_sn_r(UINT8 *data, UINT8 offset);
UINT8 check_failed_pkg_r(UINT16 sn, UINT8 *buf, UINT8 failed_num);

UINT16 m1_init_data(UINT32 addr, UINT32 len, updata_table_t *table);
UINT8 m1_updata_loop(updata_table_t *table);
INT32 m1_send_sleep(updata_table_t *table, UINT8 timer);
void m1_make_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset);
void m1_make_new_ack(updata_table_t *table, UINT32 *ack_addr, UINT32 *ack_len, UINT8 sn_offset);
INT32 updata_m1(UINT32 addr, UINT32 len, UINT32 *ack_addr, UINT32* ack_len, updata_table_t *table);
INT32 m1_sleep_all(updata_table_t *table);
