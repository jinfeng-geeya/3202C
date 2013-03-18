#ifndef __PSI_PMT_H__
#define __PSI_PMT_H__
#include <types.h>
#include <sys_config.h>

#include <api/libtsi/p_search.h>

enum {
	ES_DTS_EXIST_BIT		= 0,
	ES_CA_EXIST_BIT			= 1,
	ES_ISO639_EXIST_BIT		= 2,
	ES_MPEG4_VIDEO_EXIST_BIT	= 3,
	ES_MPEG4_AUDIO_EXIST_BIT	= 4,
	ES_AVC_VIDEO_EXIST_BIT		= 5,
	ES_TTX_EXIST_BIT		= 6,
	ES_SUB_EXIST_BIT		= 7,
	ES_AC3_EXIST_BIT		= 8,
	ES_AAC_EXIST_BIT		= 9,

	ES_EAC3_EXIST_BIT         = 13,		
};

enum {
	ES_DTS_EXIST			= 1<<ES_DTS_EXIST_BIT,
	ES_CA_EXIST			= 1<<ES_CA_EXIST_BIT,
	ES_ISO639_EXIST			= 1<<ES_ISO639_EXIST_BIT,
	ES_MPEG4_VIDEO_EXIST		= 1<<ES_MPEG4_VIDEO_EXIST_BIT,
	ES_MPEG4_AUDIO_EXIST		= 1<<ES_MPEG4_AUDIO_EXIST_BIT,
	ES_AVC_VIDEO_EXIST		= 1<<ES_AVC_VIDEO_EXIST_BIT,
	ES_TTX_EXIST			= 1<<ES_TTX_EXIST_BIT,
	ES_SUB_EXIST			= 1<<ES_SUB_EXIST_BIT,
	ES_AC3_EXIST			= 1<<ES_AC3_EXIST_BIT,
	ES_AAC_EXIST			= 1<<ES_AAC_EXIST_BIT,
	ES_EAC3_EXIST			= 1<<ES_EAC3_EXIST_BIT, 

};

//#if ((SYS_PROJECT_SM&PROJECT_SM_CI)!=0)	
//#define MAX_ES_CA_DESC_LEN		0x40
//#endif
#define ES_INFO_ONE_LANG_SIZE		3	// one audio language is 3 characters
#define ES_INFO_MAX_LANG_SIZE       64  // MAX 21 audio languages in one descriptor
struct pmt_es_info {
	UINT16 pid;
	UINT8 stream_type;
	UINT8 lang[ES_INFO_MAX_LANG_SIZE];
	UINT32 stat;

	UINT8 cas_count;
	UINT16 cas_sysid[P_MAX_CA_NUM];
	UINT16 cas_pid[P_MAX_CA_NUM];

};

#ifdef __cplusplus
extern "C"{
#endif

INT32 psi_pmt_parser(UINT8 *pmt, PROG_INFO *p_info, INT32 max_es_nr);

#ifdef __cplusplus
}
#endif

#endif /* __PSI_PMT_H__ */
