/*-----------------------------------------------------------------------------
*	Copyright (C) 2010 ALI Corp. All rights reserved.
*	GUI object library with GE HW accerlaration by Shine Zhou.
*-----------------------------------------------------------------------------*/
#ifndef  _OBJ_MATRIXBOX_H_
#define _OBJ_MATRIXBOX_H_

#include <basic_types.h>
#include <api/libge/osd_lib.h>

#define DT_LEFT     0x00    // H left, default
#define DT_VCENTER  0x00    // V center, default
#define DT_CENTER   0x01    // H center
#define DT_RIGHT    0x02    // H right
#define DT_TOP      0x04    // V top
#define DT_BOTTOM   0x08    // V bottom
#define DT_BORDER   0x20    // draw picture and fill rect with background color
#define DT_MIXER    0x40    // if DT_MIXER specified, not draw transparent color in picture;
                            // otherwise, draw transparent color with background color.
                            
#define ITEM_SELECTED           0x80
#define ITEM_UNSELECTED         0x7f
typedef struct _MATRIX_BOX_NODE
{
    UINT32      str; // strid or *str
    UINT16      num; // num or iconid
    UINT8       bAttr;                  // bit 0 -> 1 Active, 0->inactive. Definition is same as 
//  bAction     7       6-1     0
//      0       unselected          inactive
//      1       selected                active
}MATRIX_BOX_NODE, *PMATRIX_BOX_NODE;

#define MATRIX_TYPE_NULL			0x00
#define MATRIX_TYPE_WCHAR		0x01
#define MATRIX_TYPE_STRID		0x02
#define MATRIX_TYPE_NUMBER		0x04
#define MATRIX_TYPE_BITMAP		0x08

// MatrixBox style: bit(0)=padding mode, bit(1-2)=row mode, bit(3)=col mode
#define MATRIX_DEFAULT          0x00    // padding, row loop, col loop
#define MATRIX_NO_PADDING       0x01    // don't draw padding cell
#define MATRIX_ROW_WRAP         0x02    // wrap to next row
#define MATRIX_ROW_STAY         0x04    // don't move when out of row range
#define MATRIX_COL_STAY         0x08    // don't move when out of col range
#define MATRIX_PADDING_MODE(style)  ((UINT8)(style) & 0x01)
#define MATRIX_ROW_MODE(style)      ((UINT8)(style) & 0x06)
#define MATRIX_COL_MODE(style)      ((UINT8)(style) & 0x08)
//#define MATRIX_STYLE_DEFAULT            0x00    // default: padding, line wrap
//#define MATRIX_STYLE_LINEWRAP           0x01    // loop to next line
//#define MATRIX_STYLE_LOOP_HOR_DISABLE   0x02    // hor loop disable
//#define MATRIX_STYLE_DRAW_AFTER_CHANGE  0x04    // draw item after calling change proc

typedef struct _MATRIX_BOX
{
	OBJECT_HEAD head;
	UINT8 bStyle;
	UINT8 bCellType;
	UINT16 nCount;
	PMATRIX_BOX_NODE pCellTable;

	UINT16 nRow;
	UINT16 nCol;

	UINT8 bAssignType;
	INT16 bIntervalX;
	INT16 bIntervalY;
	INT8 bIntervalTt;	
	INT8 bIntervalTl;	
	INT8 bIntervalIt;	
	INT8 bIntervalIl;		
	UINT16 nPos;            // the item index of current position	
	
	UINT32 slave;
}MATRIX_BOX, *PMATRIX_BOX;

typedef struct SLAVE_MTXBOX
{
	GUI_RECT  frame;
	COLOR_STYLE style;
	
	INT16 bIntervalX;
	INT16 bIntervalY;
}SD_MTXBOX, *PSDMTXBOX;

#define	OSD_SetMatrixBoxTShift(mb,x, y)	  \
	do{ \
		((PMATRIX_BOX)(mb))->bIntervalTl = (INT8)(x); \
		((PMATRIX_BOX)(mb))->bIntervalTt = (INT8)(y); \
	}while(0)

#define	OSD_SetMatrixBoxIShift(mb,x, y)	  \
	do{ \
		((PMATRIX_BOX)(mb))->bIntervalIl = (INT8)(x); \
		((PMATRIX_BOX)(mb))->bIntervalIt = (INT8)(y); \
	}while(0)

VACTION OSD_MatrixboxKeyMap(POBJECT_HEAD pObj, UINT32 vkey);
PRESULT OSD_MatrixboxProc(POBJECT_HEAD pObjHead, UINT32 msg_type, UINT32 msg,UINT32 param1);
void OSD_DrawMatrixBoxCell(PMATRIX_BOX mb, UINT8 bStyleIdx, UINT32 nCmdDraw);
void OSD_GetMtrxBoxCellLocation(PMATRIX_BOX m, UINT16 nPos, UINT16 *pCol, UINT16 *pRow);
UINT16 OSD_GetMtrxboxCurPos(PMATRIX_BOX m);
void OSD_SetMtrxboxCurPos(PMATRIX_BOX m, UINT16 nPos);
void OSD_SetMtrxboxCount(PMATRIX_BOX m, UINT16 count, UINT16 row, UINT16 col);
PRESULT OSD_ShiftMatrixBoxSelHor(PMATRIX_BOX m, short sShiftX);
PRESULT OSD_ShiftMatrixBoxSelVer(PMATRIX_BOX m, short sShiftX);

#endif
