#ifndef __EPG_DB_H__
#define  __EPG_DB_H__

#ifdef WIN32
#include "common.h"
typedef UINT32 			OSAL_ID;
#else
#include <types.h>
#include <osal/osal.h>
#endif

#ifdef __cplusplus
extern "C"
{
#endif // __cplusplus

typedef BOOL (*check_func_t)(struct DB_NODE *, void *);


#define EPG_EVENT_BLOCK -2
#define EPG_EVENT_BLOCK_EXTEND -1
#define EPG_EVENT_RECIEVE  0

//general node check function struct
struct DB_CHECK
{
	check_func_t check_func; //check node function
	void *priv;					//data for check function
};

INT32 init_epg_db(OSAL_ID flag, UINT8 *buffer, UINT32 buf_len);
INT32 reset_epg_db();
INT32 release_epg_db();
INT32 get_epg_db_status();

//api for epg
INT32 add_event(struct DB_NODE *node);
INT32 del_events(UINT32 tp_id, UINT16 service_id, UINT8 tab_num, UINT8 event_mask);

INT32 create_view(UINT32 *view_buf, UINT32 buf_size,struct DB_CHECK *check);
INT32 reset_epg_view();
INT32 get_event_by_pos(UINT32 pos, struct DB_NODE* dst, UINT8* buf, UINT16 size);

#ifdef __cplusplus
}
#endif // __cplusplus

#endif //__EPG_DB_H__

