#ifndef __DB_BASIC_OPERATION_H__
#define __DB_BASIC_OPERATION_H__

#define BO_MAX_SECTOR_NUM		64
#define BO_SECTOR_INFO_SIZE	16
#ifndef DB_RAM_BACKUP
#define BO_RECLAIM_THRESHOLD	2
#else
#define BO_RECLAIM_THRESHOLD	0
#endif
/*valid flag "10100101", indicate data in flash is valid*/
#define BO_VALID_FLAG	0xA5

#define BO_HEADER_START	0xF00FF00F
#define BO_HEADER_END		0x12345678
//#define BO_HEADER_NODE_SIZE	6

#define BO_SEARCH_FROM_START	1
#define BO_SEARCH_FROM_END	2
#define BO_SEARCH_OPERATION	1
#define BO_SEARCH_DATA			2

/*flash sector status*/
enum  
{
	BO_SS_UNKNOWN = 0xffff,
	BO_SS_SPARE = 0xfffc,
	BO_SS_COPYING = 0xfff0,
	BO_SS_VALID = 0xffc0,
	BO_SS_SELECT = 0xff00,
	BO_SS_ERASING = 0xfc00,
	BO_SS_INVALID = 0x0000,
}BO_Sec_Status;

/*flash sector type*/
enum
{
	BO_TYPE_UNKNOWN = 0xff,
	BO_TYPE_HEAD = 0xfc,
	BO_TYPE_DATA = 0xf0,
	BO_TYPE_TMP = 0xc0,
}BO_Sec_Type;


struct BO_Sec_Info
{
	UINT8 type;
	UINT16 status;
	UINT32 logic_number;
	UINT32 erase_count;
	UINT8 valid_flag;

}__attribute__((packed));




INT32 BO_flash_write(UINT8 *data, UINT32 len, UINT32 addr);

#if defined(DB_MULTI_SECTOR)
//multi sector api
UINT32 BO_get_max_logicnum(UINT8 *sec_index);
INT32 BO_get_free_sector(UINT8 *sec_index);
UINT8 BO_get_reserve_sec();
UINT8 BO_get_sec_num();
UINT8 BO_get_header_sec();
INT32 BO_set_header_sec(UINT8 sec_idx);
UINT8 BO_get_cur_data_sec( );
INT32 BO_set_cur_data_sec(UINT8 sec_idx);

INT32 BO_read_sec_info(UINT8 sec_idx, struct BO_Sec_Info *sec_info);
INT32 BO_write_sec_info(UINT8 sec_idx, struct BO_Sec_Info *sec_info);
void BO_set_sec_list(UINT8 sec_idx, struct BO_Sec_Info sec_info);

#endif

INT32 BO_set_data_freeaddr(UINT32 addr);
INT32 BO_set_head_freeaddr(UINT32 addr);
INT32 BO_set_header_addr(UINT32 addr);

UINT32 BO_get_db_data_space();

INT32 BO_get_header(UINT32 * head_addr, UINT16 *node_num,UINT32 * free_addr);

INT32 BO_read_data(UINT32 addr, UINT32 len, UINT8 *data);
INT32 BO_write_data(UINT8 *data, UINT32 len, UINT32 *addr);
INT32 BO_write_operation(UINT8 *data, UINT32 len, UINT32 *addr);
INT32 BO_erase_sector(UINT8 sec_index);
INT32 BO_init(UINT32 db_base_addr, UINT32 db_length);
INT32 BO_reset( );




#endif

