#ifndef __OTA_SPECIAL_H__
#define __OTA_SPECIAL_H__

#include <api/libchunk/chunk.h>

#define CHUNK_OUI_START 0
#define CHUNK_OUI_LEN 6

#define CHUNK_HW_MODEL_START 6
#define CHUNK_HW_MODEL_LEN 4

#define CHUNK_HW_VER_START 10
#define CHUNK_HW_VER_LEN 4

#define CHUNK_SW_MODEL_START 0
#define CHUNK_SW_MODEL_LEN 4

#define CHUNK_SW_VER_START 4
#define CHUNK_SW_VER_LEN 4

typedef struct tp_info
{
	UINT32 frequency;
	UINT16 symbol_rate;
	UINT16 modulation;
} OTA_TP_INFO;

struct NIT_BITMAP
{
    UINT32 all_hit_value;
    UINT32 hit;
    UINT32 all_hit_again;
};


typedef UINT32 (*ota_special_cb)(UINT32 param1,UINT32 param2);
INT32 ota_table_request();
void ota_change_nim();
UINT16 ota_get_download_pid();
void RegOTASpecialCB(ota_special_cb cb);

#endif //__OTA_SPECIAL_H__
