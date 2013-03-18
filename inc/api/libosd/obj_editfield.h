#ifndef _OBJ_EDITFIELD_H_
#define _OBJ_EDITFIELD_H_

#if (SYS_PROJECT_FE == PROJECT_FE_ATSC)
#include <api/libtsi/sec_stt.h>
#else
#include <api/libtsi/sec_tdt2.h>
#endif
//for bStyle
#define NORMAL_EDIT_MODE 0x00
#define SELECT_EDIT_MODE  0x10
#define SELECT_STATUS 0x0
#define EDIT_STATUS 0x1
#define FORCE_TO_VALBAK 0x2

#define	EDITFIELD_SUPPORT_TIME	1

typedef enum
{
    CURSOR_NO = 0,      /* No cursor, can change immediately: eg PID*/
    CURSOR_NORMAL,     /* Cursor under the char, length must be fixed */  
    CURSOR_SPECIAL,    /* Only EDIT mode and not reach to maxlen has cursor '_'*/
}CUSOR_MODE;

typedef struct _EDIT_FIELD
{
    OBJECT_HEAD head;
    UINT8 bAlign;
    UINT8 bX;
    UINT8 bY;
    UINT8 bMaxLen;

    char* pcsPattern; // fraction: "fXY" - X: integer width, Y: fraction width
                      // password: "pIML" - I: the initial char for password
		        	  // M: the masked char for password
                      // L: the password length
                      // range: "rNNNNN~MMMMM" - NNNNN: the minimum value
                      //                         MMMMM: the maximum value
                      // date: "dFS" - F: format index, S: seperator index
                      //       F = 0: yyyy-mm-dd
                      //           1: yy-mm-dd
                      //           2: mm-dd-yyyy
                      //           3: mm-dd-yy
                      //           4: dd-mm-yyyy
                      //       S = 0: '.'
                      //           1: '-'
                      //           2: '/'
                      // time: "t0" - hh:mm:ss
                      //       "t1" - hh:mm
                      //       "t2" - mm:ss
                      // 		"T0" - hh:mm:ss AM/PM
                      //		"T1" - hh:mm AM/PM
                      //		"T2" - mm:ss AM/PM
                      // string: "sLLLLL" - LLLLL: the string maximum length(advanced feature)
 					  // ip address: "iS"  S = 0: '.', 1: '-', 2: '/'
 					  // mask data, "mIMLL"
 					  //	I: the initial char for mask
		        	  // 	M: the masked char for password
                      // 	LL: the mask max length
//    UINT16 wSuffixID; //??
//    UINT16 wLabelID;  //??
//    PCLABEL_ATTR pLabel;  //??

    UINT8 bStyle;  // SELECT_STATUS / EDIT_STATUS | NORMAL_EDIT_MODE / SELECT_EDIT_MODE
    UINT8 bCursorMode;      // cursor mode
    UINT8 bCursor;
    UINT16 *pString;
	UINT16 *pPrefix;   //eg. E 168
	UINT16 *pSuffix;    //eg. xxx MHz
	UINT32  valbak;
}EDIT_FIELD, *PEDIT_FIELD;

/////////////////////////////////////////////////
// attribute set functions
#define OSD_SetEditFieldPattern(e,pattern) \
	(e)->pcsPattern = (pattern)

#define OSD_SetEditFieldPrefixPoint(e,prefix) \
	(e)->pPrefix = (prefix)

#define OSD_SetEditFieldSuffixPoint(e,suffix) \
	(e)->pSuffix = (suffix)

#define OSD_SetEditFieldStyle(e,style) \
	(e)->bStyle = (UINT8)(style)

#define OSD_SetEditFieldCursorMode(e,cursormode) \
	(e)->bCursorMode = (UINT8)(cursormode)

#define OSD_SetEditFieldCursor(e,cursor) \
	(e)->bCursor = (UINT8)(cursor)

#define OSD_SetEditFieldStrPoint(e,str) \
	(e)->pString = (UINT16 *)(str)

#define OSD_SetEditFieldValbak(e,val_bak) \
	(e)->valbak = (UINT32)(val_bak)

#define OSD_GetEditFieldMode(e)	\
	( (e)->bStyle & SELECT_EDIT_MODE)
		
#define OSD_GetEditFieldStatus(e)	\
	( (e)->bStyle & EDIT_STATUS)

#define OSD_GetEditFieldDefaultVal(e)	\
	((e)->bStyle & FORCE_TO_VALBAK)

void 	OSD_DrawEditFieldCell(PEDIT_FIELD pCtrl, UINT8 bStyleIdx, UINT32 nCmdDraw);
VACTION OSD_EditFieldKeyMap	(POBJECT_HEAD pObj,	UINT32 vkey);
PRESULT OSD_EditFieldProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);


BOOL OSD_SetEditFieldContent(PEDIT_FIELD pCtrl, UINT32 StringType, UINT32 dwValue);
BOOL OSD_SetEditFieldPrefix (PEDIT_FIELD pCtrl, UINT32 StringType, UINT32 dwValue);
BOOL OSD_SetEditFieldSuffix (PEDIT_FIELD pCtrl, UINT32 StringType, UINT32 dwValue);

BOOL OSD_GetEditFieldIntValue(PEDIT_FIELD pCtrl,UINT32 *pValue);
BOOL OSD_GetEditFieldTimeDate(PEDIT_FIELD pCtrl,date_time* dt);
UINT32 OSD_GetEditFieldContent(PEDIT_FIELD pCtrl);

#endif//_OBJ_EDITFIELD_H_
