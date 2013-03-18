#ifndef __ADV_SHOW_JY_H__
#define __ADV_SHOW_JY_H__
#include "sys_types.h"




#ifdef  __cplusplus
extern "C" {
#endif

struct ap_show_gyad_param{
	UINT16				ad_type;
	UINT16				ad_index;
	OSAL_TIMER_ID		timer;
	struct control*		ctrl_bmp;
	UINT16				bmp_bg_ridx;
	UINT16				left;
	UINT16				top;
	UINT16				width;
	UINT16				height;
	UINT8				timeout;
};

enum
{
	SHOW_ADV_LOGO,
	SHOW_ADV_MAIN_MENU=1,
	SHOW_ADV_CHANNEL_CHANGE=2,
	SHOW_ADV_RADIO,
	SHOW_ADV_ROLL_TXT,
	SHOW_ADV_TYPE_MAX,
};


RET_CODE ap_gyadlogo_show(void);
void Gy_get_time (UINT32 *Curtime);
int adv_read_pf(UINT16 sid,struct bitmap *bmp);





#ifdef  __cplusplus
}
#endif

#endif


