#ifndef __LIB_TMP_INFO_H__
#define __LIB_TMP_INFO_H__

#include <types.h>
#include <retcode.h>
#include <sys_config.h>

#define SAVE_TMP_INFO_FAILED	0x00000001
#define SAVE_TMP_INFO_OUTRANGE	0x00000002
#define LOAD_TMP_INFO_FAILED	0x00000003

#define TMP_DATA_START_ID		0x12345678
#define TMP_DATA_END_ID			0x87654321

INT32 save_tmp_data(UINT8 *buff,UINT16 buff_len);
INT32 load_tmp_data(UINT8 *buff,UINT16 buff_len);

INT32 _init_tmp_info();
INT32 _init_tmp_info_ext(UINT32 base_addr, UINT32 len);


#endif /* __LIB_TMP_INFO_H__ */
