/***************************************************************************************************
	This header file is generated by the Vega from a xform file.
	Please don't modify it!!
---------------------------------------------------------------------------------------------------*/
		
#ifndef __WIN_DISPLAYSET_VEGA_H__C830FA95_20DE_45D3_88A0_9EE5D11E9EC3
#define __WIN_DISPLAYSET_VEGA_H__C830FA95_20DE_45D3_88A0_9EE5D11E9EC3


/*******************************************************************************
*	Objects definition
*******************************************************************************/
extern CONTAINER displayset_con;
extern CONTAINER displayset_action;
extern TEXT_FIELD displayset_txt1;
extern TEXT_FIELD displayset_txt2;
extern CONTAINER g_win_displayset;
extern TEXT_FIELD displayset_back;
extern TEXT_FIELD bg_alpha;
extern TEXT_FIELD displayset_modify;
extern TEXT_FIELD displayset_comfire;
extern TEXT_FIELD displayset_title;
extern TEXT_FIELD display_scale;
extern TEXT_FIELD displayset_exit;

//g_win_displayset ------------------------------------------------------------------------
static VACTION displayset_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT displayset_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER g_win_displayset = {
	.head = {
		.bType = OT_CONTAINER,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 1, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {0, 0, 720, 576},
		.style = {
			.bShowIdx = WSTL_RCI_BACKGROUND, .bHLIdx = WSTL_RCI_BACKGROUND, .bSelIdx = WSTL_RCI_BACKGROUND, .bGrayIdx = WSTL_RCI_BACKGROUND
		},
		.pfnKeyMap = displayset_keymap, .pfnCallback = displayset_callback,
		.pNext = (POBJECT_HEAD)NULL, .pRoot = (POBJECT_HEAD)NULL
	},
	.pNextInCntn = (POBJECT_HEAD)&displayset_con,
	.FocusObjectID = 1,
	.bHiliteAsWhole = 0,
};

//displayset_con ------------------------------------------------------------------------
static VACTION displayset_con_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT displayset_con_callback(POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2);

CONTAINER displayset_con = {
	.head = {
		.bType = OT_CONTAINER,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 1, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {286, 110, 384, 300},
		.style = {
			.bShowIdx = WSTL_RCI_COM_MENU_BG, .bHLIdx = WSTL_RCI_COM_MENU_BG, .bSelIdx = WSTL_RCI_COM_MENU_BG, .bGrayIdx = WSTL_RCI_COM_MENU_BG
		},
		.pfnKeyMap = displayset_con_keymap, .pfnCallback = displayset_con_callback,
		.pNext = (POBJECT_HEAD)NULL, .pRoot = (POBJECT_HEAD)&g_win_displayset
	},
	.pNextInCntn = (POBJECT_HEAD)&displayset_title,
	.FocusObjectID = 1,
	.bHiliteAsWhole = 0,
};

//displayset_title ------------------------------------------------------------------------

TEXT_FIELD displayset_title = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {327, 116, 119, 24},
		.style = {
			.bShowIdx = WSTL_RCI_IGNORE, .bHLIdx = WSTL_RCI_IGNORE, .bSelIdx = WSTL_RCI_IGNORE, .bGrayIdx = WSTL_RCI_IGNORE
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&displayset_action, .pRoot = (POBJECT_HEAD)&displayset_con
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = RS_DISPLAY_SETTINGS,
	.pString = NULL
};

//displayset_action ------------------------------------------------------------------------

CONTAINER displayset_action = {
	.head = {
		.bType = OT_CONTAINER,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {329, 340, 309, 48},
		.style = {
			.bShowIdx = 0, .bHLIdx = 0, .bSelIdx = 0, .bGrayIdx = 0
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&display_scale, .pRoot = (POBJECT_HEAD)&displayset_con
	},
	.pNextInCntn = (POBJECT_HEAD)&displayset_comfire,
	.FocusObjectID = 0,
	.bHiliteAsWhole = 0,
};

//displayset_comfire ------------------------------------------------------------------------

TEXT_FIELD displayset_comfire = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {316, 342, 104, 31},
		.style = {
			.bShowIdx = WSTL_RCI_YELLOW_ITEM, .bHLIdx = WSTL_RCI_YELLOW_ITEM, .bSelIdx = WSTL_RCI_YELLOW_ITEM, .bGrayIdx = WSTL_RCI_YELLOW_ITEM
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&displayset_modify, .pRoot = (POBJECT_HEAD)&displayset_action
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = RS_BUTTON_CONFIRM,
	.pString = NULL
};

//displayset_modify ------------------------------------------------------------------------

TEXT_FIELD displayset_modify = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {420, 342, 66, 31},
		.style = {
			.bShowIdx = WSTL_RCI_IGNORE, .bHLIdx = WSTL_RCI_IGNORE, .bSelIdx = WSTL_RCI_IGNORE, .bGrayIdx = WSTL_RCI_IGNORE
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&displayset_exit, .pRoot = (POBJECT_HEAD)&displayset_action
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = RS_HELP_CHANGE,
	.pString = NULL
};

//displayset_exit ------------------------------------------------------------------------

TEXT_FIELD displayset_exit = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {486, 342, 78, 31},
		.style = {
			.bShowIdx = WSTL_RCI_BLUE_ITEM, .bHLIdx = WSTL_RCI_BLUE_ITEM, .bSelIdx = WSTL_RCI_BLUE_ITEM, .bGrayIdx = WSTL_RCI_BLUE_ITEM
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&displayset_back, .pRoot = (POBJECT_HEAD)&displayset_action
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = RS_HELP_EXIT,
	.pString = NULL
};

//displayset_back ------------------------------------------------------------------------

TEXT_FIELD displayset_back = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {564, 342, 73, 31},
		.style = {
			.bShowIdx = WSTL_RCI_IGNORE, .bHLIdx = WSTL_RCI_IGNORE, .bSelIdx = WSTL_RCI_IGNORE, .bGrayIdx = WSTL_RCI_IGNORE
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)NULL, .pRoot = (POBJECT_HEAD)&displayset_action
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = RS_HELP_BACK,
	.pString = NULL
};

//display_scale ------------------------------------------------------------------------

TEXT_FIELD display_scale = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 2, .bLeftID = 0, .bRightID = 0, .bUpID = 1, .bDownID = 1,
		.frame = {471, 256, 158, 29},
		.style = {
			.bShowIdx = WSTL_COM_SEL_BG, .bHLIdx = WSTL_COM_SEL_ARROW, .bSelIdx = WSTL_COM_SEL_BG, .bGrayIdx = WSTL_COM_SEL_BG
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&displayset_txt1, .pRoot = (POBJECT_HEAD)&displayset_con
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = 0,
	.pString = display_strs[25]
};

//displayset_txt1 ------------------------------------------------------------------------

TEXT_FIELD displayset_txt1 = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {314, 196, 157, 29},
		.style = {
			.bShowIdx = WSTL_RCI_COM_SEL_NAME_BG, .bHLIdx = WSTL_RCI_COM_SEL_NAME_BG, .bSelIdx = WSTL_RCI_COM_SEL_NAME_BG, .bGrayIdx = WSTL_RCI_COM_SEL_NAME_BG
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&displayset_txt2, .pRoot = (POBJECT_HEAD)&displayset_con
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = RS_BACKGROUND_ALPHA,
	.pString = NULL
};

//displayset_txt2 ------------------------------------------------------------------------

TEXT_FIELD displayset_txt2 = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 0, .bLeftID = 0, .bRightID = 0, .bUpID = 0, .bDownID = 0,
		.frame = {314, 256, 157, 29},
		.style = {
			.bShowIdx = WSTL_RCI_COM_SEL_NAME_BG, .bHLIdx = WSTL_RCI_COM_SEL_NAME_BG, .bSelIdx = WSTL_RCI_COM_SEL_NAME_BG, .bGrayIdx = WSTL_RCI_COM_SEL_NAME_BG
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)&bg_alpha, .pRoot = (POBJECT_HEAD)&displayset_con
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = RS_DISPLAY_SCALE,
	.pString = NULL
};

//bg_alpha ------------------------------------------------------------------------

TEXT_FIELD bg_alpha = {
	.head = {
		.bType = OT_TEXTFIELD,
		.bAttr = C_ATTR_ACTIVE, .bFont = 0,
		.bID = 1, .bLeftID = 0, .bRightID = 0, .bUpID = 2, .bDownID = 2,
		.frame = {471, 196, 158, 29},
		.style = {
			.bShowIdx = WSTL_COM_SEL_BG, .bHLIdx = WSTL_COM_SEL_ARROW, .bSelIdx = WSTL_COM_SEL_BG, .bGrayIdx = WSTL_COM_SEL_BG
		},
		.pfnKeyMap = NULL, .pfnCallback = NULL,
		.pNext = (POBJECT_HEAD)NULL, .pRoot = (POBJECT_HEAD)&displayset_con
	},
	.bAlign = C_ALIGN_CENTER|C_ALIGN_VCENTER,
	.bX = 0, .bY = 0,
	.wStringID = 0,
	.pString = display_strs[21]
};
#endif//__WIN_DISPLAYSET_VEGA_H__C830FA95_20DE_45D3_88A0_9EE5D11E9EC3
