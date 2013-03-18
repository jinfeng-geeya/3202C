/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef _OBJ_TEXTFIELD_H_
#define _OBJ_TEXTFIELD_H_

typedef struct _TEXT_FIELD
{
	OBJECT_HEAD head;
	UINT8 bAlign;
	UINT8 bX;
	UINT8 bY;
	UINT16 wStringID;  // string id
	UINT16* pString;   // unicode string pointer

	UINT32 slave;
}TEXT_FIELD, *PTEXT_FIELD;

typedef struct SLAVE_TXTFLD
{
	GUI_RECT  frame;
	COLOR_STYLE style;
	UINT8 bX;
	UINT8 bY;
    
	UINT16* pString;   // unicode string pointer    
}SDTXT, *PSDTXT;

void OSD_SetTextFieldStrPoint(PTEXT_FIELD pCtrl, UINT16*	pStr);
void OSD_SetSDTxtContent(PSDTXT pCtrl, UINT32 StringType, UINT32 Value);
void OSD_SetTextFieldContent(PTEXT_FIELD pCtrl, UINT32 StringType, UINT32 Value);
void OSD_DrawTextFieldCell(PTEXT_FIELD pCtrl,UINT8 bStyleIdx,UINT32 nCmdDraw);
VACTION OSD_TextFieldKeyMap	(POBJECT_HEAD pObj,	UINT32 vkey);
PRESULT OSD_TextFieldProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#define OSD_SetSDTxtStrPoint(txt,str)	\
	do{(txt)->pString = (UINT16 *)str;}while(0)
#define OSD_SetTextFieldStrId(txt,id)	\
	do{(txt)->wStringID = (UINT16)id;}while(0)
#define OSD_SetTextFieldStrPoint(txt,str)	\
	do{(txt)->pString = (UINT16 *)str;}while(0)
	
#define OSD_GetTextFieldStrId(txt)	\
	(txt)->wStringID
#define OSD_GetTextFieldStrPoint(txt)	\
	(txt)->pString

#endif//_OBJ_TEXTFIELD_H_
