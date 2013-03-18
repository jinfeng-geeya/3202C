/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2002 Copyright (C)
 *
 *  File: sec_pmt.h
 *
 *  Description: the  head file of data types and functions for pmt section.
 *
 *  History:
 *      Date        Author         Version   Comment
 *      ====        ======         =======   =======
 *  1.  2002.12.20  David Wang     0.1.000   Initial
 *  2.  2003.02.11  David Wang	   0.1.001   Modification for new sw tree
 *  3.  2006.06.27  Zhengdao Li	   0.1.002   add some marco for data broadcast.
 ****************************************************************************/

#ifndef _SEC_PMT_H_
#define _SEC_PMT_H_

#include <types.h>
//#include <api/libtsi/dmx_types.h>
//#include "db_info.h"
#define ERR_NO_PROG_INFO				0xFFFF0002

enum {
	MPEG1_VIDEO_STREAM				= 0x01,
	MPEG2_VIDEO_STREAM				= 0x02,
	MPEG1_AUDIO_STREAM				= 0x03,
	MPEG2_AUDIO_STREAM				= 0x04,
	PRIVATE_DATA_STREAM				= 0x06,

	ISO13818_6_TYPE_A				= 0x0a,
	ISO13818_6_TYPE_B				= 0x0b,
	ISO13818_6_TYPE_C				= 0x0c,
	ISO13818_6_TYPE_D				= 0x0d,

    MPEG_ADTS_AAC_STREAM            = 0x0F,//ADTS_AAC
	MPEG4_VIDEO_STREAM				= 0x10, //(ISO/IEC 14496-2 Visual)
	MPEG_AAC_STREAM					= 0x11, //(ISO/IEC 14496-3 Audio withe LATM transport syntax)
	H264_VIDEO_STREAM				= 0x1b, //(AVC video as define in ITU Rec. H.264 |ISO/IEC 14496-10 Video)
	DC_II_VIDEO_STREAM				= 0x80,
	AC3_AUDIO_STREAM				= 0X81,
};

//#if (SYS_PROJECT_FE == PROJECT_FE_DVBT)||(SYS_PROJECT_FE == PROJECT_FE_ATSC)
//#if (MPEG4_DEFINITION == SYS_FUNC_ON)
#define   MPEG_4_VIDEO_DES_EXIST            		0x0001
#define   MPEG_4_AUDIO_DES_EXIST            		0x0002
//#define   AAC_DES_EXIST                               0x0004
#define   AVC_VIDEO_EXIST                            	0x0008
//#endif
//#endif
#define H264_VIDEO_PID_FLAG 			0x2000
#define TELETEXT_DES_EXIST				0x8000
#define SUBTITLE_DES_EXIST				0x4000
#define DTS_DES_EXIST					0x1000

#define AC3_DES_EXIST					(1<<13)//0x2000
#define AAC_DES_EXIST					(2<<13)//0x4000//LATM_AAC
#define EAC3_DES_EXIST					(3<<13)//0x0001//EAC3
#define ADTS_AAC_DES_EXIST				(4<<13)//0x8000//ADTS_AAC

#define AUDIO_TYPE_TEST(pid, type)		(((pid)&0xE000)==(type))

#define STREAM_ID_DES_EXIST				0x0400

#define AC3_REGST_DESC				0x05		//ATSC-jmchen
#define DTS_REGST_DESC					0x05
#define CA_DES_TAG					0x09
#define ISO_639_LANG_DESC_TAG				0x0A

#define STREAM_ID_DESC_TAG				0x52
#define TELETEXT_DES_TAG				0x56
#define SUBTITLE_DES_TAG				0x59
#define AC3_DESC					0x6A
#define DTS_AUDIO_DESC					0x73
#define AAC_DESC					0x79

#define MPEG_4_VIDEO_TAG                         	0x1B
#define MPEG_4_AUDIO_TAG				0x1C
#define AVC_VIDEO_TAG                                	0x28


//fix BUG06114:
//for stream "10873v27500[1].500mb.mpg", its cad loop1 is very long!
#define MAX_CAD_LEN					512
/* to hold the data which get from pmt second loop*/
typedef struct com_info {
	UINT16 com_pid;
	UINT16 com_flag;
	UINT8 com_type;
	UINT8 audio_lang[3];
	UINT16 CAD_len;
	UINT8 CAD_cnt;
//#if  ((SYS_PROJECT_SM & PROJECT_SM_CI) != 0)
	UINT8 *CAD_buff;
//#endif
	UINT8 com_tag;
} COM_INFO;

typedef struct CAD_loop1_info {
	UINT16 CAD_loop1_len;
	UINT8 CAD_loop1_buff[MAX_CAD_LEN];
} CAD_LOOP1_INFO;

typedef struct CA_info {
	UINT16 CA_system_id;
	UINT16 CA_pid;
} CA_INFO;

#ifdef __cplusplus
extern "C"
{
#endif

UINT16 pmt_get_prog_number(UINT8 *buff,INT16 buff_len);

UINT16 pmt_get_pcr_pid(UINT8 *buff,INT16 buff_len);

UINT16 pmt_get_com_list(UINT8 *buff,INT16 buff_len,COM_INFO *com_buff,UINT32 com_buff_len);
#if 0
INT32 pmt_get_CA_descriptor(UINT8 *buff,INT16 buff_len,CA_INFO *CA_info,UINT8 max_CA_num);
#endif

#ifdef __cplusplus
}
#endif/*__cplusplus*/

#endif /* _SEC_PMT_H_ */


