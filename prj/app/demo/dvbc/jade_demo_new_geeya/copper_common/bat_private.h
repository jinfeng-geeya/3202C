#ifndef _BAT_PRIVATE_H_
#define _BAT_PRIVATE_H_
#include <types.h>
#include <sys_config.h>
#include <api/libsi/si_module.h>


#define BOUQUET_NUM_MAX				16
#define BOUQUET_NAME_NUM_MAX		16
#define TS_IN_BOUQUET_NUM_MAX		16
#define SERVICE_IN_TS_NUM_MAX		16

#define BOUQUET_CNT 11
#define BOUQUET_NAME_DESC_TAG		0x47
#define BOUQUET_SERVICE_LIST_DESC_TAG	0x41

struct bat_hitmap
{
    UINT32 all_hit_value;
    UINT32 hit;
    UINT32 all_hit_again;
};

struct bat_sec_info
{
    UINT16 bat_id;
    struct bat_hitmap hitmap[BOUQUET_CNT];
    UINT8 section_buf[8*1024];
};

typedef struct
{
	UINT16 bouquet_id;
	UINT16 name[BOUQUET_NAME_NUM_MAX];
	UINT16 ts_cnt;
	UINT16 ts_id[TS_IN_BOUQUET_NUM_MAX];
	UINT16 service_cnt[TS_IN_BOUQUET_NUM_MAX];
	UINT16 service_id[TS_IN_BOUQUET_NUM_MAX][SERVICE_IN_TS_NUM_MAX];
}bat_service_item;


INT32 bat_priv_info_init(UINT32 frq, UINT32 symbol, UINT8 qam,BOOL change_to_mainfreq);
void update_services_info();
BOOL get_service_bouquet_info(UINT16 *bouquet_num, UINT16 *bouquet_id_list, UINT16 *bouquet_name);

#endif

