/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: win_game_othello.c
 *
 *  Description: Othello game for Copper UI.
 *
 *  History:
 *  Date            Author  Version  Comment
 *  ====            ======  =======  =======
 *  1.  2006.5.31   Liu Lan 0.1.000  Initial
 *
 ****************************************************************************/

#include <sys_config.h>
#include <types.h>

#include <osal/osal.h>
#include <hld/pan/pan_dev.h>

//#include <api/libosd/osd_lib.h>

#include "string.id"
#include "images.id"
#include "osd_config.h"

#include "copper_common/dev_handle.h"
#include "copper_common/com_api.h"
#include "copper_common/system_data.h"

#include "win_com_popup.h"
#include "win_com_list.h"
#include "win_com.h"

#include "win_game_com.h"

#include "game_othello.h"


#define GAME_MSG_LEFT       (TV_OFFSET_L +(OSD_MAX_WIDTH-250)/2)
#define GAME_MSG_TOP        (TV_OFFSET_T+(OSD_MAX_HEIGHT-130)/2)
#define GAME_MSG_WIDTH      250
#define GAME_MSG_HEIGHT     130


#define BOARD_COLS 8
#define BOARD_ROWS 8

#define GRID_WIDTH  40
#define CHESS_SIZE  28

#define CENTER_OFFSET   ((GRID_WIDTH - CHESS_SIZE) / 2)

#define BK_WIDTH  (BOARD_COLS) * GRID_WIDTH
#define BK_HEIGHT (BOARD_ROWS) * GRID_WIDTH

#define BK_X_OFFSET (TV_OFFSET_L+94)
#define BK_Y_OFFSET (TV_OFFSET_L+119)


#ifdef USE_16BIT_OSD
#define WSTL_GAME_LINE_IDX      0xd35f
#define WSTL_GAME_CURSOR_IDX    0xd35f
#define WSTL_GAME_BACKGROUND_IDX 0xba39
#else
#define WSTL_GAME_LINE_IDX      0xF3
#define WSTL_GAME_CURSOR_IDX    0xF3
#define WSTL_GAME_BACKGROUND_IDX 0x79
#endif

#define WSTL_GAME_BG_02 WSTL_TXT_12

static UINT8 Class = 1;
static INT16 BlackPoint = 2, WhitePoint = 2, cur_col, cur_row;
static board_type chess_board;


//*******************************************************************************

static void draw_chess(UINT8 row, UINT8 col, UINT32 color)
{
	UINT16 rsc_id;
	VSCR*	lpVscr ;
#ifdef USE_LIB_GE
	lpVscr= NULL;//OSD_GetTaskVscr(osal_task_get_current_id());
#else
	lpVscr=NULL;
#endif
	rsc_id = color == CHESS_BLACK ? IM_BLACK : IM_WHITE;
	OSD_DrawPicture(BK_X_OFFSET + col * GRID_WIDTH + CENTER_OFFSET,  \
		BK_Y_OFFSET + row * GRID_WIDTH + CENTER_OFFSET,  \
		rsc_id, LIB_ICON, C_MIXER, lpVscr);
}




static void draw_cursor(UINT8 row, UINT8 col, UINT32 color)
{
	OSD_DrawVerLine(BK_X_OFFSET + col * GRID_WIDTH + 4, BK_Y_OFFSET + row * GRID_WIDTH + 4, GRID_WIDTH / 4, color, NULL);

	OSD_DrawVerLine(BK_X_OFFSET + (col + 1) *GRID_WIDTH - 4, BK_Y_OFFSET + row * GRID_WIDTH + 4, GRID_WIDTH / 4, color, NULL);

	OSD_DrawVerLine(BK_X_OFFSET + col * GRID_WIDTH + 4, BK_Y_OFFSET + (row + 1) *GRID_WIDTH - GRID_WIDTH / 4-4, GRID_WIDTH / 4, color, NULL);

	OSD_DrawVerLine(BK_X_OFFSET + (col + 1) *GRID_WIDTH - 4, BK_Y_OFFSET + (row + 1) *GRID_WIDTH - GRID_WIDTH / 4-4, GRID_WIDTH / 4, color, NULL);


	OSD_DrawHorLine(BK_X_OFFSET + col * GRID_WIDTH + 4, BK_Y_OFFSET + row * GRID_WIDTH + 4, GRID_WIDTH / 4, color, NULL);

	OSD_DrawHorLine(BK_X_OFFSET + col * GRID_WIDTH + 4, BK_Y_OFFSET + (row + 1) *GRID_WIDTH - 4, GRID_WIDTH / 4, color, NULL);

	OSD_DrawHorLine(BK_X_OFFSET + (col + 1) *GRID_WIDTH - 4-GRID_WIDTH / 4, BK_Y_OFFSET + row * GRID_WIDTH + 4, GRID_WIDTH / 4, color, NULL);

	OSD_DrawHorLine(BK_X_OFFSET + (col + 1) *GRID_WIDTH - 4-GRID_WIDTH / 4, BK_Y_OFFSET + (row + 1) *GRID_WIDTH - 4, GRID_WIDTH / 4, color, NULL);
}


static void othello_draw_background(void)
{
    UINT8 i, j;

    drawSquare(BK_X_OFFSET, BK_Y_OFFSET, 320, 320, WSTL_GAME_BG_02);

    for (i = 0; i <= BOARD_COLS; i++)
    {
        OSD_DrawVerLine(BK_X_OFFSET + i * GRID_WIDTH, \
              BK_Y_OFFSET, BOARD_ROWS * GRID_WIDTH, \
              WSTL_GAME_LINE_IDX, NULL);
    }
    for (i = 0; i <= BOARD_ROWS; i++)
    {
        OSD_DrawHorLine(BK_X_OFFSET, BK_Y_OFFSET + i * GRID_WIDTH, \
              BOARD_COLS * GRID_WIDTH, WSTL_GAME_LINE_IDX, NULL);
        OSD_DrawHorLine(BK_X_OFFSET, \
              BK_Y_OFFSET + i * GRID_WIDTH + 1, \
              BOARD_COLS * GRID_WIDTH, WSTL_GAME_LINE_IDX, NULL);
	}

	for (i = 0; i < BOARD_ROWS; i++)
	{
		for (j = 0; j < BOARD_COLS; j++)
		{
			if (chess_board.board[i + 1][j + 1] == CHESS_BLACK)
				draw_chess(i, j, CHESS_BLACK);
			if (chess_board.board[i + 1][j + 1] == CHESS_WHITE)
				draw_chess(i, j, CHESS_WHITE);
		}
	}

	draw_cursor(cur_row, cur_col, WSTL_GAME_CURSOR_IDX);
}


static void othello_init(void)
{
	g_iGameLevel = Class - 1;

	cur_col = 3;
	cur_row = 2;
	BlackPoint = 2;
	WhitePoint = 2;
	init_board(&chess_board);
}


static void callback(UINT8 type, INT32 wparam, UINT32 lparam)
{
	UINT8 row, col, back_saved;
	UINT8 winner;
	win_popup_choice_t popResult;

	switch (type)
	{
		case TRANCHESS:
			row = wparam / 10-1;
			col = wparam % 10-1;
			draw_chess(row, col, (UINT8)lparam);
			break;

		case USER_NO_STEP:
			break;

		case COMPUTER_NO_STEP:
			break;

		case GAME_OVER:
			winner = (UINT8)wparam;
			update_status(Class, BlackPoint, WhitePoint);

			win_compopup_init(WIN_POPUP_TYPE_OK);
			win_compopup_set_frame(GAME_MSG_LEFT, GAME_MSG_TOP, GAME_MSG_WIDTH, GAME_MSG_HEIGHT);

			if (winner == COMPUTER_WIN)
			{
				//"Sorry! You lost!"
				win_compopup_set_msg_ext(NULL, NULL, RS_GAME_SORRY_YOU_LOST);
			}
			else if (winner == USER_WIN)
			{
				//"You Win!"
				win_compopup_set_msg_ext(NULL, NULL, RS_GAME_YOU_WIN);
			}
			else
			{
				//"DOGFALL!"
				win_compopup_set_msg_ext(NULL, NULL, RS_GAME_DOGFALL);
			}
			win_compopup_open_ext(&back_saved);

			othello_init();
            OSD_ObjOpen((POBJECT_HEAD)&game_con, 0xFFFFFFFF) ;    
			/*
			othello_draw_background();
			update_status(Class, BlackPoint, WhitePoint,TRUE);

			OSD_SetAttr((POBJECT_HEAD) &txt_start, C_ATTR_ACTIVE);
			OSD_ChangeFocus((POBJECT_HEAD) &game_con, 1,  \
				C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);
            */
			break;
	}
}


static PRESULT othello_key_proc(UINT32 vkey, UINT8 key_repeat_cnt, UINT8 key_status)
{
	PRESULT ret = PROC_LOOP;
	UINT8 back_saved;
	if (key_status == PAN_KEY_PRESSED)
	{
		switch (vkey)
		{
			case V_KEY_UP:
			case V_KEY_DOWN:
			case V_KEY_LEFT:
			case V_KEY_RIGHT:
				draw_cursor(cur_row, cur_col, WSTL_GAME_BACKGROUND_IDX);
				if (vkey == V_KEY_UP)
				{
					cur_row = cur_row == 0 ? 7 : cur_row - 1;
				}
				else if (vkey == V_KEY_DOWN)
				{
					cur_row = cur_row == 7 ? 0 : cur_row + 1;
				}
				else if (vkey == V_KEY_LEFT)
				{
					cur_col = cur_col == 0 ? 7 : cur_col - 1;
				}
				else if (vkey == V_KEY_RIGHT)
				{
					cur_col = cur_col == 7 ? 0 : cur_col + 1;
				}
				draw_cursor(cur_row, cur_col, WSTL_GAME_CURSOR_IDX);
				break;

			case V_KEY_ENTER:
				if (do_move_chess(&chess_board, (cur_row + 1) *10+(cur_col + 1), ~computer_side &3, callback))
				{
					draw_cursor(cur_row, cur_col, WSTL_GAME_BACKGROUND_IDX);

					get_chess_score(&chess_board, &WhitePoint, &BlackPoint);
					update_status(Class, BlackPoint, WhitePoint);

					/* computer round */
					computer_play(&chess_board, callback);

					get_chess_score(&chess_board, &WhitePoint, &BlackPoint);
					update_status(Class, BlackPoint, WhitePoint);

					draw_cursor(cur_row, cur_col, WSTL_GAME_CURSOR_IDX);
				}
				break;

			case V_KEY_MENU:
			case V_KEY_EXIT:
				win_compopup_init(WIN_POPUP_TYPE_OKNO);
				win_compopup_set_frame(GAME_MSG_LEFT, GAME_MSG_TOP, GAME_MSG_WIDTH, GAME_MSG_HEIGHT);
				win_compopup_set_msg_ext(NULL, NULL, RS_GAME_MSG_DO_YOU_QUIT);
				if (win_compopup_open_ext(&back_saved) == WIN_POP_CHOICE_YES)
				{
					othello_init();
                    OSD_ObjOpen((POBJECT_HEAD)&game_con, 0xFFFFFFFF) ;  
					/*othello_init();
					othello_draw_background();
					update_status(Class, BlackPoint, WhitePoint,TRUE);
					OSD_SetAttr((POBJECT_HEAD) &txt_start, C_ATTR_ACTIVE);
					OSD_ChangeFocus((POBJECT_HEAD) &game_con, 1,  \
						C_UPDATE_FOCUS | C_DRAW_SIGN_EVN_FLG);*/
				}
				break;
			default:
				ret = PROC_PASS;
				break;
		}
	}

	return ret;
}


static void othello_start(void)
{
	UINT8 back_saved;
	//    othello_init();
	othello_draw_background();
	update_status(Class, BlackPoint, WhitePoint);

	win_compopup_init(WIN_POPUP_TYPE_OKNO);
	win_compopup_set_frame(GAME_MSG_LEFT, GAME_MSG_TOP, GAME_MSG_WIDTH, GAME_MSG_HEIGHT);
	win_compopup_set_msg_ext(NULL, NULL, RS_GAME_MSG_DO_YOU_BLACK);
	if (win_compopup_open_ext(&back_saved) == WIN_POP_CHOICE_NO)
	{
		computer_side = CHESS_BLACK;
		/* computer round */
		computer_play(&chess_board, callback);

		get_chess_score(&chess_board, &WhitePoint, &BlackPoint);
		update_status(Class, BlackPoint, WhitePoint);
	}
}


static PRESULT othello_setup(POBJECT_HEAD obj, VEVENT event, UINT32 param1, UINT32 param2)
{
	PRESULT ret = PROC_PASS;
	UINT8 unact;

	switch (event)
	{
		case EVN_KEY_GOT:
			unact = (VACTION)(param1 >> 16);
			switch (unact)
			{
			case VACT_DECREASE:
				Class = Class == 1 ? 3 : Class - 1;
				break;
			case VACT_INCREASE:
				Class = Class == 3 ? 1 : Class + 1;
				break;
			}
			update_status(Class, BlackPoint, WhitePoint);
			break;
	}

	return ret;
}


static void othello_pre_open(void)
{
	setup_game_panel(RS_GAME_OTHELLO, RS_GAME_CLASS, RS_GAME_BLACK, RS_GAME_WHITE);

    //Class = 1;
	BlackPoint = 2;
	WhitePoint = 2;
	othello_init();

	update_status(Class, BlackPoint, WhitePoint);
}


static void othello_1st_draw(void)
{
	othello_draw_background();
}


static void othello_event ( UINT32 param1, UINT32 param2 )
{}


void game_othello_init(void)
{
	game_key_proc = othello_key_proc;
	game_setup = othello_setup;
	game_start = othello_start;
	game_pre_open = othello_pre_open;
	game_1st_draw = othello_1st_draw;
	game_event = othello_event;
}
