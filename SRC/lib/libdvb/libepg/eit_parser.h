#ifndef __EIT_PARSER_H__
#define  __EIT_PARSER_H__

#include <types.h>
#include <api/libsi/lib_epg.h>

#ifdef __cplusplus
extern "C"{
#endif

INT32 eit_sec_parser(UINT32 tp_id, UINT8* buf, UINT32 len, EIT_CALL_BACK call_back);

#ifdef __cplusplus
}
#endif

#endif //__EIT_PARSER_H__


