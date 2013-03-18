#ifndef _WIN_GAME_COM_H_
#define _WIN_GAME_COM_H_

extern CONTAINER game_con;
extern TEXT_FIELD txt_start;

extern PRESULT(*game_key_proc)(UINT32, UINT8, UINT8);
extern PRESULT(*game_setup)(POBJECT_HEAD, VEVENT, UINT32, UINT32);
extern void(*game_start)(void);
extern void(*game_pre_open)(void);
extern void(*game_1st_draw)(void);
extern void(*game_event)(UINT32, UINT32);

#endif
