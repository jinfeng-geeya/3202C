#include <types.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include "win_cas_com.h"

/*******************************************************************************
* Objects definition
*******************************************************************************/
#define BMAIL_ICON_L 172
#define BMAIL_ICON_T 60
#define BMAIL_ICON_W 30
#define BMAIL_ICON_H 28

#define DETITLE_ICON_L 220
#define DETITLE_ICON_T 60
#define DETITLE_ICON_W 30
#define DETITLE_ICON_H 28

extern BITMAP win_bmail_bmp;
extern BITMAP win_detitle_bmp;
DEF_BITMAP ( win_bmail_bmp, NULL, NULL, C_ATTR_ACTIVE, 0, \
             1, 0, 0, 0, 0, BMAIL_ICON_L, BMAIL_ICON_T, BMAIL_ICON_W, BMAIL_ICON_H, WSTL_TRANS_IX, WSTL_TRANS_IX, 0, 0,   \
             NULL, NULL,  \
             C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, IM_EMAIL)

DEF_BITMAP ( win_detitle_bmp, NULL, NULL, C_ATTR_ACTIVE, 0, \
             1, 0, 0, 0, 0, DETITLE_ICON_L, DETITLE_ICON_T, DETITLE_ICON_W, DETITLE_ICON_H, WSTL_TRANS_IX, WSTL_TRANS_IX, 0, 0,   \
             NULL, NULL,  \
             C_ALIGN_CENTER | C_ALIGN_VCENTER, 0, 0, IM_DETITLE)
             
/*******************************************************************************
* Function definition
*******************************************************************************/

void ShowBMailOSDOnOff ( UINT8 flag )
{
	OBJECT_HEAD* obj;
	obj = ( OBJECT_HEAD* ) & win_bmail_bmp;

	if ( 1== flag)
		OSD_DrawObject ( obj, C_UPDATE_ALL );
	else if (0 == flag)
		OSD_ClearObject ( obj, 0 );				
}

void ShowDetitleOSDOnOff ( UINT8 flag )
{
	OBJECT_HEAD* obj;
	obj = ( OBJECT_HEAD* ) & win_detitle_bmp;

	if ( 1 == flag)
		OSD_DrawObject ( obj, C_UPDATE_ALL );
	else if (0 == flag)
		OSD_ClearObject ( obj, 0 );				
}



