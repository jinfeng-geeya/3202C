#ifndef __DESC_LOOP_PARSER_H__
#define __DESC_LOOP_PARSER_H__
#include <types.h>
#include <sys_config.h>

#include <api/libtsi/si_descriptor.h>

typedef INT32(* desc_parser_t)(UINT8 tag, UINT8 len, UINT8 *data, void *priv);

struct desc_table {
	UINT8 tag;
	UINT8 bit;
	desc_parser_t parser;
};

#ifdef PVR_DYNAMIC_PID_MONITOR_SUPPORT

typedef INT32(* rec_desc_parser_t)(UINT32 param,UINT8 tag, UINT8 len, UINT8 *data, void *priv);

struct rec_desc_table {
	UINT8 tag;
	UINT8 bit;
	rec_desc_parser_t parser;
};

#endif

#if (defined(AUTO_OTA)||defined(_ISDBT_ENABLE_))
enum nit_firstloopdesc_existbit{
	NIT_NETWORKNAME_EXIST_BIT	= 0,
	NIT_LINKAGE_EXIST_BIT			= 1,
	NIT_EWS_EXIST_BIT			= 2,
};
#endif

#ifdef __cplusplus
extern "C"{
#endif

/*
 * parameter item should be arranged in accent order so that the binary search
 * could be applied for better performance.
 */
INT32 desc_loop_parser(UINT8 *data, INT32 len, struct desc_table *item,
	INT32 item_nr, UINT32 *stat, void *priv);

INT32 si_descriptor_parser(UINT8 *data, INT32 length, struct descriptor_info *info, INT32 nr);

#ifdef __cplusplus
}
#endif

#endif /* __DESC_LOOP_PARSER_H__ */

