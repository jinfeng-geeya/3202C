#ifndef _OBJ_MULTITEXT_H_
#define _OBJ_MULTITEXT_H_


typedef struct _TEXT_CONTENT
{
	UINT8 bTextType; // specifies string type of pText
	union _text
	{
		UINT16 wStringID;  //STRING_ID
		UINT16* pString;   //STRING_UNICODE
	}text;
}TEXT_CONTENT, *PTEXT_CONTENT;

typedef struct _MULTI_TEXT
{
	OBJECT_HEAD head;
	UINT8 bAlign;
	UINT8 bCount;  // ARRAY_SIZE(pTextTable)
	UINT16 nLine;  // current first line idx for display
	OSD_RECT rcText;
	SCROLL_BAR 		*scrollBar;
	PTEXT_CONTENT pTextTable;
}MULTI_TEXT, *PMULTI_TEXT;

#define MAX_LINE_NUM    200

typedef struct
{
    UINT8   total_lines;
    UINT8   page_lines;
    UINT8   topline_idx;
    UINT8   topline_tblidx;
    UINT8*  topline_pstr;
    UINT8   line_height[MAX_LINE_NUM];
    UINT16  total_height;
    UINT16  page_height;
}mtxtinfo_t;

/////////////////////////////////////////////////
// attribute set functions
#define OSD_SetMultiTextLine(mt,line) \
	(mt)->nLine = (UINT16)(line)
	
void OSD_DrawMultiTextCell(PMULTI_TEXT pCtrl, UINT8 bStyleIdx,UINT32 nCmdDraw);
VACTION OSD_MultiTextKeyMap	(POBJECT_HEAD pObj,	UINT32 vkey);
PRESULT OSD_MultiTextProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif//_OBJ_MULTITEXT_H_

