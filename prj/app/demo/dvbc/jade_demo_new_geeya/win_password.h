#ifndef _WIN_PASSWORD_H_
#define _WIN_PASSWORD_H_

typedef enum
{
	WIN_PWD_CHAN = 0,
	WIN_PWD_MENU,		
} WIN_PWD_TYPE;

typedef enum
{
	WIN_PWD_INPUT = 0,
	WIN_PWD_EDIT,		
} WIN_PWD_INPUT_TYPE;

BOOL win_pwd_open(UINT32 *vkey_exist_array, UINT32 vkey_exist_cnt, OSD_RECT *rc);
BOOL win_pwd_get_input(char *input_pwd);

//return 1:pwd correct,or 0 
typedef UINT8 (*cb_pwd_check)(UINT32 param1,UINT32 param2);
void SetPwdCheckCB(cb_pwd_check cb);

#endif //_WIN_PASSWORD_H_
