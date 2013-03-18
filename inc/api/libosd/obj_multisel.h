#ifndef _OBJ_MULTISEL_H_
#define _OBJ_MULTISEL_H_

typedef struct _MULTISEL
{
    OBJECT_HEAD head;

    UINT8 bAlign;
    //UINT8 bStyle;   // default: Horizontal, No arrow
    UINT8 bX;       // X offset
    UINT8 bY;
    UINT8 bSelType; // specifies string type of pSelTable

    void* pSelTable;   // pointer type is determined by bSelType
                        // bSelType = STRING_ID: UINT16[dwTotal]
                        //            STRING_UNICODE: PWCHAR[dwTotal]
                        //            STRING_NUMBER: UINT32[3]
                        //                           pSelTable[0]: the minimum value
                        //                           pSelTable[1]: the maximum value
                        //                           pSelTable[2]: the step
                        //            STRING_NUM_TOTAL: the same as STRING_NUMBER, 
                        //                              but display in format "value/total".
                        //            STRING_PROC: callback get string function
    INT16 nCount;      // ARRAY_SIZE(pSelTable) determined by bSelType

//    UINT16 wLeftIcon;
//    UINT16 wRightIcon;

//    UINT16 wLabelID;
//    PCLABEL_ATTR pLabel;

//    PLIST pList;  // to support drop-down list.

    INT16 nSel;         // determined by bSelType
                        // bSelType = STRING_ID/STRING_UNICODE: current sel to pSelTable
                        //            STRING_NUMBER/STRING_NUM_TOTAL: the current value
                        //            STRING_PROC: current sel.
}MULTISEL, *PMULTISEL;

/////////////////////////////////////////////////
// attribute set functions
#define OSD_SetMultiselStyle(s,style) \
	(s)->bStyle = (UINT8)(style)
	
#define OSD_SetMultiselXoffset(s,xoffset) \
	(s)->bX = (UINT8)(xoffset)
	
#define OSD_SetMultiselSelType(s,selType) \
	(s)->bSelType = (UINT8)(selType)

#define OSD_SetMultiselSelTable(s,selTable) \
	(s)->pSelTable = (selTable)
	
#define OSD_SetMultiselCount(s,count) \
	(s)->nCount = (INT16)(count)

#define OSD_SetMultiselSel(s,sel) \
	(s)->nSel = (INT16)(sel)
/////////////////////////////////////////////////
// attribute get functions
#define OSD_GetMultiselSel(s) \
	(s)->nSel

#define OSD_GetMultiselCount(s) \
	(s)->nCount


void 	OSD_DrawMultiselCell(PMULTISEL pCtrl, UINT8 bStyleIdx, UINT32 nCmdDraw);
VACTION OSD_MultiselKeyMap	(POBJECT_HEAD pObj,	UINT32 vkey);
PRESULT OSD_MultiselProc(POBJECT_HEAD pObj, UINT32 msg_type, UINT32 msg,UINT32 param1);

#endif//_OBJ_MULTISEL_H_
