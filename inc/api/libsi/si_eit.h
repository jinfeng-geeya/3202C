#ifndef __SI_EIT_H__
#define  __SI_EIT_H__

#include <api/libsi/si_module.h>
#include <api/libtsi/sie.h>

struct eit_section_info {
	INT32 service_nr;
	struct extension_info ext[PSI_MODULE_MAX_PROGRAM_NUMBER];
};

enum {
	SIE_EIT_RINGBUF_LEN = 0x10000,
	SIE_EIT_ASCH_TABLE_CNT = 0x10, /*actural schedule table 0x50~0x5f*/
	
	SIE_EIT_APRESET_FLAG = 0x01,	/*actural preset section hit flag*/
	SIE_EIT_AFOLLOW_FLAG = 0x02,	/*actural following section hit flag*/

	SIE_EIT_SINGLE_SERVICE = 0x01,
	SIE_EIT_WHOLE_TP = 0x02,

};

struct sie_eit_config
{
	UINT8 eit_mode;
	UINT16 cur_service_id;
	UINT8* buffer;
	UINT32 buf_len;
};


INT32 start_eit(struct sie_eit_config *config, si_dmx_event_t event, si_handler_t handle);
INT32 stop_eit();


#endif

