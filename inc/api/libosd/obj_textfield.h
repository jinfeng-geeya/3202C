#ifndef _OBJ_TEXTFIELD_H_
#define _OBJ_TEXTFIELD_H_

// String Type Definition


typedef struct _TEXT_FIELD
{
    OBJECT_HEAD head;
    UINT8 bAlign;
//    UINT8 bStyle;
    INT32 bX;
    INT32 bY;
    UINT16 wStringID;  // string id
//    UINT16 wIconID;
//    OSD_RECT rcIcon;
    UINT16* pString;   // unicode string pointer
//    UINT16 nSize;   // pString Buffer size in WCHAR 
}TEXT_FIELD, *PTEXT_FIELD;

void OSD_SetTextFieldStrPoint(PTEXT_FIELD pCtrl, UINT16*	pStr);
void OSD_SetTextFieldContent(PTEXT_FIELD pCtrl, UINT32 StringType, UINT32 Value);

void OSD_DrawTextFieldCell(PTEXT_FIELD pCtrl,UINT8 bStyleIdx,UINT32 nCmdDraw);

VACTION OSD_TextFieldKeyMap	(POBJECT_HEAD pObj,	UINT32 vkey);
PRESULT OSD_TextFieldProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);



#endif//_OBJ_TEXTFIELD_H_
