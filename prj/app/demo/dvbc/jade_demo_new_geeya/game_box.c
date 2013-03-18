#include <types.h>
#include <hld/pan/pan_dev.h>
//#include <api/libosd/osd_lib.h>
#include "images.id"
#include "string.id"
#include "osd_config.h"
#include "osdobjs_def.h"
#include "win_com.h"
#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"

#include "win_com_popup.h"
#include "win_com_list.h"

#include "Menus_root.h"
#include "win_game_com.h"
#include "copper_common/menu_api.h"


/*******************************************************************************
 *	Objects definition
 *******************************************************************************/
extern CONTAINER g_win_games;
extern CONTAINER win_game_box;
extern CONTAINER box_info;

extern TEXT_FIELD box_level_name;
extern TEXT_FIELD box_level_num;
extern TEXT_FIELD box_help;

extern TEXT_FIELD box_info_txt1;
extern TEXT_FIELD box_info_txt2;
extern TEXT_FIELD box_info_txt3;
extern TEXT_FIELD box_info_txt4;
extern TEXT_FIELD box_info_txt5;
extern TEXT_FIELD box_info_txt6;
extern TEXT_FIELD box_info_txt7;
extern TEXT_FIELD box_info_txt8;
extern TEXT_FIELD box_info_txt9;

extern BITMAP box_info_bmp1;
extern BITMAP box_info_bmp2;

extern EDIT_FIELD	box_set_num;
extern TEXT_FIELD box_playarea;

static VACTION game_level_keymap(POBJECT_HEAD pObj, UINT32 key);
static PRESULT game_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);
static VACTION game_box_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT game_box_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static VACTION box_info_keymap ( POBJECT_HEAD pObj, UINT32 key );
static PRESULT box_info_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
        
#define BM_MOVED_TO_BAD_POINT 0
#define BM_ERROR_POINT_MAN 1
#define BM_ERROR_CANNOT_MOVE_THERE 2
#define BM_ERROR_POINT_WALL 3
#define BM_ERROR_PIONT_OUT 4
#define BM_MOVED_OK 5
#define BM_BOX_MOVED 6
#define BM_MAN_MOVED 7

#define BM_MESSAGE_THINK_STOPED 100
#define BM_MESSAGE_THINKING 101
#define BM_MESSAGE_NO_ANSWER 102
#define BM_MESSAGE_BUFFER_FULL 103
#define BM_MESSAGE_THINK_OUT 104

#define BM_COMMAND_RUN_THREAD 200
#define BM_COMMAND_STOP_THREAD 201

#define MAXMAPSIZEX 20
#define MAXMAPSIZEY  16

#define NULLB 0
#define NULLWALLB  1
#define WALLB  2
#define MANB  3
#define POINTB 5
#define MANATPOINTB 4
#define BOXATPOINTB 6
#define BOXB 7

#define INFO_CON_L 160
#define INFO_CON_T 148
#define INFO_CON_W 400
#define INFO_CON_H  280

#define INFO_TXT_L   180
#define INFO_TXT_W   360
#define INFO_TXT_H   28
#define INFO_TXT_T   (INFO_CON_T+14)

//#define GAME_BOX_CON WSTL_GAME_INFO_CON
//#define GAME_BOX_TXT WSTL_GAME_INFO_TXT
#define WIN_GAME_BOX_BG WSTL_SEARCH_BG_CON//0//WSTL_MAINMENU_MAIN_CON

#define pausetimer 50
#define SpaceWidth 0//10

#define MAX_LEVEL	36//9

UINT16 box_str[2][3];

char game_box_style[] = "s5";

DEF_CONTAINER ( win_game_box, NULL, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                1, 0, 0, 0, 0, 0, 110, 420, 360, WIN_GAME_BOX_BG, WIN_GAME_BOX_BG, WIN_GAME_BOX_BG, WIN_GAME_BOX_BG,/*MAIN_CON_IDX, MAIN_CON_IDX, MAIN_CON_IDX, MAIN_CON_IDX, */  \
                game_box_keymap, game_box_callback,  \
                 ( POBJECT_HEAD ) &box_playarea, 1, 0 );

DEF_TEXTFIELD ( box_playarea, &win_game_box, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
               1, 0, 0, 0, 0, 160, 148, 0/*400*/, 0/*280*/,0, 0, 0, 0, \
                NULL, game_callback, \
               C_ALIGN_CENTER, 0, 0, \
                0, NULL );
 /*
DEF_TEXTFIELD ( box_level_name, &win_game_box, &box_level_num, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                1, 0, 0, 0, 0, 480, 50, 100, 28,  WSTL_TXT_12, WSTL_TXT_12, WSTL_TXT_12, WSTL_TXT_12, \
                NULL, NULL, \
                C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, RS_BOX_CUR_LEVEL, NULL );

DEF_TEXTFIELD ( box_level_num, &win_game_box, &box_help, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                1, 0, 0, 0, 0, 600, 50, 22, 26,  WSTL_TXT_12, WSTL_TXT_12, WSTL_TXT_12, WSTL_TXT_12, \
                NULL, NULL, \
                C_ALIGN_LEFT | C_ALIGN_VCENTER, 0, 0, 0, box_str[1]);

DEF_TEXTFIELD ( box_help, &win_game_box, NULL, C_ATTR_ACTIVE, C_FONT_DEFAULT, \
                1, 0, 0, 0, 0, 50, 500, 620, 28,  WSTL_TXT_12, WSTL_TXT_12, WSTL_TXT_12, WSTL_TXT_12, \
                NULL, NULL, \
                C_ALIGN_CENTER| C_ALIGN_VCENTER, 0, 0, RS_BOX_HELP, NULL );

DEF_CONTAINER ( box_info,  NULL, NULL, C_ATTR_ACTIVE, 0, \
                1, 0, 0, 0, 0, INFO_CON_L, INFO_CON_T, INFO_CON_W, INFO_CON_H, GAME_BOX_CON, GAME_BOX_CON, GAME_BOX_CON, GAME_BOX_CON,  \
                 box_info_keymap, box_info_callback,  \
                 ( POBJECT_HEAD ) &box_info_txt1, 1, 0 );

#define LDEF_TXT(root,varTxt,nxtObj,ID,IDl,IDr,IDu,IDd,l,t,w,h,resID)		\
    DEF_TEXTFIELD(varTxt,&root,nxtObj,C_ATTR_ACTIVE,0, \
    ID,IDl,IDr,IDu,IDd, l,t,w,h, GAME_BOX_TXT,GAME_BOX_TXT,GAME_BOX_TXT,GAME_BOX_TXT,   \
    NULL,NULL,  \
    C_ALIGN_LEFT| C_ALIGN_VCENTER, 0,0,resID,NULL)
*/    
#define LDEF_BMP(root,varbmp,nxtObj,l,t,w,h,bmpID)		\
    DEF_BITMAP(varbmp,&root,nxtObj,C_ATTR_ACTIVE,0, \
    1,1,1,1,1, l,t,w,h, 0, 0, 0, 0,   \
    NULL,NULL,  \
    C_ALIGN_CENTER | C_ALIGN_VCENTER, 0,0,bmpID)
/*
LDEF_TXT( box_info, box_info_txt1, &box_info_txt2, 1, 1, 1, 1, 1,  INFO_TXT_L+50, INFO_TXT_T+0*INFO_TXT_H, 220, INFO_TXT_H, RS_GAME_BOX_TXT1)
LDEF_TXT( box_info, box_info_txt2, &box_info_txt3, 1, 1, 1, 1, 1,  INFO_TXT_L, INFO_TXT_T+1*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT2)
LDEF_TXT( box_info, box_info_txt3, &box_info_txt4, 1, 1, 1, 1, 1,  INFO_TXT_L+10, INFO_TXT_T+2*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT3)
LDEF_TXT( box_info, box_info_txt4, &box_info_txt5, 1, 1, 1, 1, 1,  INFO_TXT_L+10, INFO_TXT_T+3*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT4)
LDEF_TXT( box_info, box_info_txt5, &box_info_txt6, 1, 1, 1, 1, 1,  INFO_TXT_L+10, INFO_TXT_T+4*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT5)
LDEF_TXT( box_info, box_info_txt6, &box_info_txt7, 1, 1, 1, 1, 1,  INFO_TXT_L, INFO_TXT_T+5*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT6)
LDEF_TXT( box_info, box_info_txt7, &box_info_txt8, 1, 1, 1, 1, 1,  INFO_TXT_L+10, INFO_TXT_T+6*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT7)
LDEF_TXT( box_info, box_info_txt8, &box_info_txt9, 1, 1, 1, 1, 1,  INFO_TXT_L+10, INFO_TXT_T+7*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT8)
LDEF_TXT( box_info, box_info_txt9, &box_info_bmp1,1, 1, 1, 1, 1,  INFO_TXT_L+10, INFO_TXT_T+8*INFO_TXT_H, INFO_TXT_W, INFO_TXT_H, RS_GAME_BOX_TXT9)

LDEF_BMP( box_info, box_info_bmp1, &box_info_bmp2, INFO_TXT_L+15, INFO_TXT_T+6*INFO_TXT_H+2, 24, 24, IM_HELP_ICON_LR)
LDEF_BMP( box_info, box_info_bmp2, 				NULL,INFO_TXT_L+41, INFO_TXT_T+6*INFO_TXT_H+2, 24, 24, IM_HELP_ICON_TB)
*/
#define VK_RIGHT V_KEY_RIGHT
#define VK_UP V_KEY_UP
#define VK_LEFT V_KEY_LEFT
#define VK_DOWN V_KEY_DOWN

UINT8 m_mx;
UINT8 m_my;
UINT8 m_map[MAXMAPSIZEX][MAXMAPSIZEY];
UINT8 m_bnm[MAXMAPSIZEX][MAXMAPSIZEY];
UINT8 m_NumberOfBox;
UINT8 m_size;
UINT8 m_mapsizex;
UINT8 m_mapsizey;
UINT8 m_x;
UINT8 m_y;
UINT8 m_boxmovestepnumber;
UINT8 m_noanswerrecoded;
UINT8 m_CanUndo;
INT8 m_TheMapsMoveWaySavePoint;
INT8 m_computerthinkout;
INT8 m_TheShowStep;
INT8 m_CanMoveToBadPoint;
INT8 m_wayfilechanged;
UINT8 dx[4] = { 1, 0, -1, 0 };
UINT8 dy[4] = { 0, -1, 0, 1 };
//UINT16 ImgIndex=IM_GAME_BOX_BG;
UINT8 curGameIndex=1;
UINT16 Game_con_left=45;
UINT16 Game_con_top=110;
typedef struct {
	INT8 manx;
	INT8 many;
	INT8 boxx;
	INT8 boxy;
}PathStruct;
UINT16 m_pathIndex;
PathStruct m_path[1024];

static void InitGame(int GameIndex);
static UINT8 MoveTowards(INT8 x, INT8 y);
static UINT8 DrawIt(INT8 x, INT8 y);
static void AddMapToPath(INT8 manx, INT8 many, INT8 boxx, INT8 boxy);
static void BoxMoveOk(PathStruct *record);
static void DrawAllMap();
void SetMyFrame();
extern UINT8* M1Layouts[];
extern struct osd_device *g_osd_dev;
static  VACTION my_con_keymap ( POBJECT_HEAD pObj, UINT32 key );
static  PRESULT my_con_callback ( POBJECT_HEAD pObj, VEVENT event, UINT32 param1, UINT32 param2 );
static void Box_start(void);
static void Box_pre_open(void);
static void Box_event(UINT32 ,UINT32 );
static void Box_1st_draw(void);
static void Box_changeFrame(void);
static PRESULT Box_key_proc(UINT32 vkey, UINT8 key_repeat_cnt, UINT8 key_status);
static PRESULT Box_setup(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2);


static ID_RSC myImages[]=
{
    IM_GAME_FLOOR,
    IM_GAME_FLOOR,
    IM_GAME_WALL,
    IM_GAME_MAN,
    IM_GAME_MAN,
    IM_GAME_TARGET,
    IM_GAME_BOX2,
    IM_GAME_BOX1,
} ;

/*static VACTION box_info_keymap ( POBJECT_HEAD pObj, UINT32 key )
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			act = VACT_CLOSE;
			break;
	}
	return act;
}
static PRESULT box_info_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	switch( event)
	{
		case EVN_POST_CLOSE:
			OSD_DrawFill(0, 0, 720, 576, 0x5F/*255* /, NULL);
			break;
		default:
			break;
	}
	return ret;
}
*/
static PRESULT game_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	UINT32 vkey;
	UINT8 key_repeat_cnt;
	UINT8 key_status;
	PRESULT ret = PROC_PASS;

	switch (event)
	{
		case EVN_UNKNOWNKEY_GOT:
			ret = PROC_LOOP;
			key_repeat_cnt = (UINT8)(param1 >> 8);
			key_status = (UINT8)(param1 >> 16);
			ap_hk_to_vk(0, param1, &vkey);

			ret = game_key_proc(vkey, key_repeat_cnt, key_status);			
			
			break;
	}

	return ret;
}

static VACTION game_box_keymap(POBJECT_HEAD obj, UINT32 key)
{
	VACTION act = VACT_PASS;

	switch (key)
	{
		case V_KEY_MENU:
		case V_KEY_EXIT:
			act = VACT_CLOSE;
			break;
		case V_KEY_ENTER:
			act = VACT_ENTER;
			break;
		default:
			act = OSD_ContainerKeyMap(obj, key);
			break;
	}

	return act;
}


static PRESULT game_box_callback(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 unact;

	switch (event)
	{
		case EVN_PRE_OPEN:
			break;
        case EVN_POST_OPEN:
              game_pre_open();
            break;
		case EVN_PRE_CLOSE:
			*((UINT32*)param2) &= ~C_CLOSE_CLRBACK_FLG;
			break;

		case EVN_POST_DRAW:
			game_1st_draw();
			break;

		case EVN_MSG_GOT:
			game_event(param1, param2);
			break;
		case EVN_UNKNOWN_ACTION:
			unact = (VACTION)(param1 >> 16);
			if (unact == VACT_ENTER)
			game_start();
			
			break;
	}
	return ret;
}

void NewGame(UINT8 index)
{
    InitGame(index);
    DrawAllMap();
}
void Gamemain()
{
    NewGame(curGameIndex-1);
}
void reset_M_pathIndex()
{
	m_pathIndex=0;
}
static void InitGame(int GameIndex)
{
    int i,j;
    struct YCbCrColor cr;
    UINT8* Data=M1Layouts[GameIndex];
    UINT8* tp = Data+2;
	if(NULL==Data)
		return;
    if ( m_x > MAXMAPSIZEX || m_y > MAXMAPSIZEY ) return ;
#if 0
   cr.uY = 0x1;
	cr.uCb = 0x0;
	cr.uCr = 0x0;
	//umPC_setBackground_color(cr.uY,cr.uCb,cr.uCr);
 #endif
 #ifdef USE_16BIT_OSD
    OSD_DrawObject((POBJECT_HEAD)&game_con, C_UPDATE_ALL);
 #else
    OSD_DrawFill(0, 110, 415, 360, 0x79/*255*/, NULL);
 #endif
 	reset_M_pathIndex();
	m_x=*Data++;
	m_y=*Data++;
	for(j=0;j<m_y;j++)
		{
		for(i=0;i<m_x;i++)
			{
				m_map[i][j] =(UINT8)*Data++;
				switch( m_map[i][j] )
				{
					case '@':
						m_map[i][j] = '.';
						m_bnm[i][j] = '#';
						m_NumberOfBox++;
						break;
					case '$':
						m_map[i][j] = '.';
						m_bnm[i][j] = '^';
						break;
					case '^':
						m_map[i][j] = ' ';
						m_bnm[i][j] = '^';
						break;
					case '#':
						m_map[i][j] = ' ';
						m_bnm[i][j] = '#';
						m_NumberOfBox++;
						break;
					default:
						m_bnm[i][j] = 0;
				}
			}
		}
		m_mapsizex = m_x;
		m_mapsizey = m_y;

		//OSD_SetTextFieldContent( &box_level_num, STRING_ANSI, (UINT32)"01");
		//OSD_DrawObject(( POBJECT_HEAD )&box_level_name,C_UPDATE_ALL);
  		//OSD_DrawObject(( POBJECT_HEAD )&box_level_num,C_UPDATE_ALL);
		//OSD_DrawObject(( POBJECT_HEAD )&box_help,C_UPDATE_ALL);
}

static UINT8 MoveTowards(INT8 x, INT8 y)
{
	UINT8 result=BM_ERROR_CANNOT_MOVE_THERE;
       INT8 i,j;
	INT8 tx = m_mx + x;
	INT8 ty = m_my + y;
	INT8 ttx = tx + x;
	INT8 tty = ty + y;
       if ( x==0 && y==0) return result;
       if ( m_map[tx][ty] != '+' )
       {
    		switch ( m_bnm[tx][ty] )
              {
        		case 0:
        			result=BM_MAN_MOVED;
        			break;
        		case '#':
        			if ( (m_map[ttx][tty] !='+') && (m_bnm[ttx][tty] != '#') )
                            {
        				if (m_CanMoveToBadPoint)
        					result=BM_BOX_MOVED;
        				else if (m_map[ttx][tty] !='*')
        					result=BM_BOX_MOVED;
        				else 
                                          result=BM_MOVED_TO_BAD_POINT;
        			}
    		}
    	}
	if ( (result==BM_MAN_MOVED) || (result==BM_BOX_MOVED) )
        {
		m_bnm[m_mx][m_my] = 0;
		DrawIt( m_mx , m_my );
		m_bnm[tx][ty] = '^';
		DrawIt( tx , ty );
		m_mx = tx;
		m_my = ty;
		if (result==BM_BOX_MOVED)
              {
			m_bnm[ttx][tty] = '#';
			DrawIt( ttx , tty );
			AddMapToPath( tx-x, ty-y, tx, ty );
		}
	}
	
	for(j=0; j<m_y; j++ )for(i=0; i<m_x; i++ )
		if( (m_map[i][j] == '.') && (m_bnm[i][j] != '#') ) 
                    return result;
	BoxMoveOk(m_path);
	return BM_MOVED_OK;
}

UINT8 show(INT8 x,INT8 y,INT8 Index)
{
	int left=Game_con_left;
	int top=Game_con_top;
	int gapx=380/m_x;
	int gapy=320/m_y;
	SetMyFrame();
	if(gapx>25)
	{
		gapx=25;
		left=Game_con_left+(380-m_x*25)/2;
	}if(gapy>25)
	{
		gapy=25;
		top=Game_con_top+(320-m_y*25)/2;
	}
	VSCR*	lpVscr ;



	OSD_DrawPicture(left+x*gapx+SpaceWidth, top+y*gapy+SpaceWidth, myImages[Index],LIB_ICON, 0, NULL);

}
UINT8 hide(INT8 x,INT8 y,INT8 Index)
{
	;
}
UINT8 DrawIt(INT8 x, INT8 y)
{
    INT8 index=0;
	if ( (x<0) || (x>=m_x) || (y<0) || (y>=m_y) ) return;
	switch( m_map[x][y] ){
	case '+':
		index = WALLB;
		break;
	case '-':
		index = NULLB;
		break;
	case ' ':
		{
			switch( m_bnm[x][y] )
			{
			case 0:
				index = NULLWALLB;
				break;
			case '^':
				index = MANB;
				m_mx = x;
				m_my = y;
				break;
			case '#':
				index = BOXB;
                            break;
			}
		}
		break;
	case '*':
		{
			switch( m_bnm[x][y] )
                    {
			case 0:
				index = NULLWALLB;
				break;
			case '^':
				index = MANB;
				m_mx = x;
				m_my = y;
				break;
			case '#':
				index = BOXB;
			}
		}
		break;
	case '.':
		{
			switch( m_bnm[x][y] )
                    {
			case 0:
				index = POINTB;
				break;
			case '^':
				index = MANATPOINTB;
				m_mx = x;
				m_my = y;
				break;
			case '#':
				index = BOXATPOINTB;
			}
		}
		break;
	}

	x += (m_mapsizex - m_x)/2;
	y += (m_mapsizey - m_y)/2;
       if (index != 0)
       {
	       show(x,y,index);
       }
}

static void AddMapToPath(INT8 manx, INT8 many, INT8 boxx, INT8 boxy)
{
       PathStruct* p;
	if(m_pathIndex==1024)
	{
		return ;
	}
	p=&m_path[m_pathIndex];
	p->manx = manx;
	p->many = many;
	p->boxx = boxx;
	p->boxy = boxy;
	m_pathIndex++;
	m_CanUndo = TRUE;
}
void DrawAllMap()
{
    INT8 y,x,yy;
//	m_mapsizex=15;
//	m_mapsizey=10;
	if (m_mapsizex>m_x || m_mapsizey>m_y)
	{
		for( y = 0; y<m_mapsizey; y++ )
			for( x = 0; x<m_mapsizex; x++ )
			{
				show(x,y,NULLB);
			}
	}
	for( yy = 0; yy<m_y; yy++ )
		for( x = 0; x<m_x; x++ ) 
			DrawIt( x, yy );
}
void BoxMoveOk(PathStruct *record)
{
    win_popup_choice_t sel;
    UINT8 back_saved;
    char str[5];
    win_compopup_init( WIN_POPUP_TYPE_OKNO);
	win_compopup_set_msg(NULL, NULL, RS_GAME_BOX_TXT10);
	win_compopup_set_default_choice(WIN_POP_CHOICE_YES);
	sel = win_compopup_open_ext(&back_saved);
	if (sel == WIN_POP_CHOICE_NO)
	{
			NewGame(curGameIndex-1);
	}
    else
    {
	if(curGameIndex==MAX_LEVEL)
		curGameIndex=0;
	curGameIndex++;
       NewGame(curGameIndex-1);
    }
	sprintf(str, "%02d", curGameIndex);
	update_status_box(curGameIndex);
	//OSD_SetTextFieldContent( &box_level_num, STRING_ANSI, (UINT32)str);
  //	OSD_DrawObject((POBJECT_HEAD)&box_level_num, C_UPDATE_CONTENT);
}
//VSCR *myData;
void SetMyFrame()
{
    UINT16 a,b;
    a=SpaceWidth*2+/*m_x*/m_mapsizex*32;
    b=SpaceWidth*2+ /*m_y*/m_mapsizey*32+90;
   // changeFrame((720-a)/2, (576+90- b)/2);
}

BOOL Undo()
{
    UINT16 n = m_pathIndex;
    INT8 tx;
	INT8 ty;
    PathStruct *p;
	if ( !m_CanUndo ||n==0) return FALSE;

	p = (PathStruct*)&m_path[m_pathIndex-1];
	m_pathIndex--;
	if (n==1) m_CanUndo = FALSE;

	m_bnm[m_mx][m_my] = 0;
	m_bnm[p->boxx][p->boxy] = '#';
	m_bnm[p->manx][p->many] = '^';
	tx = 2 * p->boxx - p->manx;
	ty = 2 * p->boxy - p->many;
	m_bnm[tx][ty] = 0;

	DrawIt( m_mx, m_my );
	DrawIt( p->boxx, p->boxy );
	DrawIt( p->manx, p->many );
	DrawIt( tx, ty );
	
	m_mx = p->manx;
	m_my = p->many;
	return TRUE;
}
void game_Box_init(void)
{
	InitGame(curGameIndex-1);    //InitGame(0);
  	game_key_proc = Box_key_proc;
	game_start = Box_start;
	game_pre_open = Box_pre_open;
	game_1st_draw =Box_1st_draw;
	game_event = Box_event;
	game_setup = Box_setup;
      // game_changeFrame = Box_changeFrame;
}
static void Box_start(void)
{
}
static void Box_pre_open(void)
{
	setup_game_panel(RS_GAME_BOX, RS_BOX_CUR_LEVEL, NULL, NULL);
    update_status_box(curGameIndex);  //	update_status_box(1);
//	DrawAllMap();
 }
static void Box_event(UINT32 i, UINT32 j)
{

}
static void Box_1st_draw(void)
{
	DrawAllMap();
}
static void Box_changeFrame(void)
{
}

#define GAME_MSG_LEFT       165
#define GAME_MSG_TOP        132
#define GAME_MSG_WIDTH      250
#define GAME_MSG_HEIGHT     130
static PRESULT Box_key_proc(UINT32 vkey, UINT8 key_repeat_cnt, UINT8 key_status)
{
	VACTION Action = VACT_PASS;
	char str[5];
	
	PRESULT ret = PROC_LOOP;
	UINT8 back_saved;
	BOOL keydown = TRUE, flag=TRUE;
	
	
	if (key_status == PAN_KEY_PRESSED)
	{
		switch(vkey)
		     {
		          case VK_RIGHT:
				  	MoveTowards( 1, 0 );
					break;
		          case VK_UP:
				  	MoveTowards( 0, -1);
					break;
		          case VK_LEFT:
				  	MoveTowards( -1,0);
					break;
		          case VK_DOWN:
				  	MoveTowards( 0,1);
					break;
		              break;
		          case V_KEY_ENTER:
		              break;

		           case V_KEY_MENU:
			            win_compopup_init(WIN_POPUP_TYPE_OKNO);
						win_compopup_set_frame(GAME_MSG_LEFT, GAME_MSG_TOP, GAME_MSG_WIDTH, GAME_MSG_HEIGHT);
						win_compopup_set_msg_ext(NULL, NULL, RS_GAME_MSG_DO_YOU_QUIT);
						if (win_compopup_open_ext(&back_saved) == WIN_POP_CHOICE_YES)
						{					
							/*curGameIndex=1;
							restore_scene();
							//if (OSD_ObjOpen((POBJECT_HEAD) &g_win_games, 0xFFFFFFFF) != PROC_LEAVE)
							if (OSD_ObjOpen((POBJECT_HEAD) &g_win_sys_management, 0xFFFFFFFF) != PROC_LEAVE)
								menu_stack_pop();*/
							game_Box_init();
                      				 OSD_ObjOpen((POBJECT_HEAD)&game_con, 0xFFFFFFFF) ;  

						}
		              break;
/*		           case V_KEY_EXIT:
		              win_compopup_init(WIN_POPUP_TYPE_OKNO);
					win_compopup_set_frame(GAME_MSG_LEFT, GAME_MSG_TOP, GAME_MSG_WIDTH, GAME_MSG_HEIGHT);
					win_compopup_set_msg_ext(NULL, NULL, RS_GAME_MSG_DO_YOU_QUIT);
					if (win_compopup_open_ext(&back_saved) == WIN_POP_CHOICE_YES)
					{					
						curGameIndex=1;
						restore_scene();
						BackToFullScrPlay();
					}

				break;	
*/
		  //         case V_KEY_MENU:
                   case V_KEY_EXIT:
			            win_compopup_init(WIN_POPUP_TYPE_OKNO);
						win_compopup_set_frame(GAME_MSG_LEFT, GAME_MSG_TOP, GAME_MSG_WIDTH, GAME_MSG_HEIGHT);
						win_compopup_set_msg_ext(NULL, NULL, RS_GAME_MSG_DO_YOU_QUIT);
						if (win_compopup_open_ext(&back_saved) == WIN_POP_CHOICE_YES)
						{					
//							curGameIndex=1;
                            game_Box_init();
                            NewGame(curGameIndex-1);
							update_status_box(curGameIndex);
							OSD_SetAttr((POBJECT_HEAD) &txt_start, C_ATTR_ACTIVE);
							OSD_ChangeFocus((POBJECT_HEAD) &game_con, 1,C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
						}
				break;					 
			   case V_KEY_P_UP:
				if(curGameIndex==MAX_LEVEL)
					curGameIndex=0;
				curGameIndex++;	
		        	NewGame(curGameIndex-1);			
				sprintf(str, "%02d", curGameIndex );
				update_status_box(curGameIndex);
				//OSD_SetTextFieldContent( &box_level_num, STRING_ANSI, (UINT32)str);
 				//OSD_DrawObject((POBJECT_HEAD)&box_level_num, C_UPDATE_CONTENT);
			   	break;
				
			   case V_KEY_P_DOWN:
				if( curGameIndex ==1 )
					curGameIndex=MAX_LEVEL+1;
				curGameIndex--;	
				NewGame(curGameIndex-1);
				sprintf(str, "%02d", curGameIndex);
				update_status_box(curGameIndex);
				//OSD_SetTextFieldContent( &box_level_num, STRING_ANSI, (UINT32)str);
  				//OSD_DrawObject((POBJECT_HEAD)&box_level_num, C_UPDATE_CONTENT);
			   	break;	
			   case V_KEY_RECALL:
			        Undo();
				 break;
			  /* case V_KEY_INFOR:
				if (OSD_ObjOpen((POBJECT_HEAD) &box_info, 0xFFFFFFFF) != PROC_LEAVE)
					menu_stack_push((POBJECT_HEAD) &box_info);
			  	 break;*/
		           default:
				 keydown=FALSE;
		               ret = PROC_PASS;
		               break;
		        }
    }
	return ret;
}
static PRESULT Box_setup(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 unact;
	char str[5];
	switch (event)
	{
		case EVN_KEY_GOT:
			unact = (VACTION)(param1 >> 16);
			switch (unact)
			{
			case VACT_DECREASE:
				if( curGameIndex ==1 )
					curGameIndex=MAX_LEVEL+1;
				curGameIndex--;	
				NewGame(curGameIndex-1);
				sprintf(str, "%02d", curGameIndex);
				update_status_box(curGameIndex);
				break;
			case VACT_INCREASE:
				if(curGameIndex==MAX_LEVEL)
					curGameIndex=0;
				curGameIndex++;	
		        	NewGame(curGameIndex-1);			
				sprintf(str, "%02d", curGameIndex );
				update_status_box(curGameIndex);
				break;
			}
	}
	return ret;
}
