#ifndef _OBJ_BITMAP_H_
#define _OBJ_BITMAP_H_

typedef struct _BITMAP
{
    OBJECT_HEAD head;
    UINT8 bAlign;
    UINT8 bX;
    UINT8 bY;
    UINT16 wIconID;
}BITMAP, *PBITMAP;

#define OSD_SetBitmapContent(pCtrl,iconID) \
    (pCtrl)->wIconID = iconID
#define OSD_GetBitmapContent(pCtrl) \
    (pCtrl)->wIconID

void OSD_DrawBitmapCell(PBITMAP pCtrl,UINT8 bStyleIdx,UINT32 nCmdDraw);

VACTION OSD_BitmapKeyMap(POBJECT_HEAD pObj,	UINT32 vkey);
PRESULT OSD_BitmapProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);





#endif//_OBJ_BITMAP_H_

