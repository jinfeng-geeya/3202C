#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libpub/lib_pub.h>

#include "win_cas_com.h"

/*******************************************************************************
*	Local functions & variables declare
*******************************************************************************/

/************************************************************************
 *Testing TFCA osdmsg display
 ************************************************************************/
 
static void osdmsg_test()
{
	static UINT32 pp = 0;
	UINT8 i, temp[180] = {0,};
	if (0 == (pp%4))
	{
		/*for (i=0; i<180; i++)
			temp[i] = 0x30 + (i%10);*/
		UINT8 *p;
		UINT8 tmp[200]= {0,};
		//p = OSD_GetUnicodeString(RS_SHOWMSG_WATCHLEVEL);
		//ComUniStrToAsc(p, tmp);
		MEMCPY(temp,"func被一对括号包含，且左边有 一个*号，说明func是一个指针，跳出括号，右边也有个括号，这类函数具有int *和int (*)(int*)这样的形参，123456。再来看一看func的形参int (*f)(int*)，类似前面的解释，f也是一个函数指针，指向的函数具有int*类型的形参，返回值为int。",200);//"abcdefg我们是害虫1232334天下足球44");
		//MEMCPY(&temp[STRLEN(temp)],tmp, STRLEN(tmp));
		CDSTBCA_ShowOSDMessage(CDCA_OSD_TOP, temp);
	}
	else if (1 == (pp%4))
	{
		for (i=0; i<180; i++)
			temp[i] = 0x41 + (i%26);
		CDSTBCA_ShowOSDMessage(CDCA_OSD_BOTTOM, temp);
	}
	else if (2 == (pp%4))
	{
		CDSTBCA_HideOSDMessage(CDCA_OSD_TOP);
	}
	else if (3 == (pp%4))
	{
		CDSTBCA_HideOSDMessage(CDCA_OSD_BOTTOM);
	}

	pp ++;
}


/************************************************************************
 *Testing TFCA lock service
 ************************************************************************/
static ID lock_timer = OSAL_INVALID_ID;

static void lockservice_timer_func ( UINT unused )
{
	api_stop_timer (&lock_timer);
	CDSTBCA_UNLockService();
}
static void lockservice_test()
{
	static UINT8 i=0;
	SCDCALockService LockService;

	if (0 == i)
	{
		LockService.m_fec_inner = LockService.m_fec_outer = 0;
		LockService.m_Modulation = 3;
		LockService.m_symbol_rate = 0x00068750;
		LockService.m_dwFrequency = 0x02990000;
		LockService.m_wPcrPid = 848;
		LockService.m_ComponentNum = 2;
		LockService.m_CompArr[0].m_CompType = 1;
		LockService.m_CompArr[0].m_wCompPID = 848;
		LockService.m_CompArr[0].m_wECMPID = 0x503;
		LockService.m_CompArr[1].m_CompType = 3;
		LockService.m_CompArr[1].m_wCompPID = 850;
		LockService.m_CompArr[1].m_wECMPID = 0x503;
		libc_printf("$$time1: \n");
		CDSTBCA_LockService(&LockService);
		api_start_timer ("LockService", 5*1000, lockservice_timer_func);	
	}
	else if (1 == i)
	{
		LockService.m_fec_inner = LockService.m_fec_outer = 0;
		LockService.m_Modulation = 3;
		LockService.m_symbol_rate = 0x00068750;
		LockService.m_dwFrequency = 0x02990000;
		LockService.m_wPcrPid = 880;
		LockService.m_ComponentNum = 2;
		LockService.m_CompArr[0].m_CompType = 1;
		LockService.m_CompArr[0].m_wCompPID = 880;
		LockService.m_CompArr[0].m_wECMPID = 0x505;
		LockService.m_CompArr[1].m_CompType = 3;
		LockService.m_CompArr[1].m_wCompPID = 882;
		LockService.m_CompArr[1].m_wECMPID = 0x505;
		libc_printf("$$time2: !\n");
		CDSTBCA_LockService(&LockService);
		api_start_timer ("LockService", 5*1000, lockservice_timer_func);	
	}
	else if (2 == i)
	{ 
		LockService.m_fec_inner = LockService.m_fec_outer = 0;
		LockService.m_Modulation = 3;
		LockService.m_symbol_rate = 0x00068750;
		LockService.m_dwFrequency = 0x02990000;
		LockService.m_wPcrPid = 816;
		LockService.m_ComponentNum = 2;
		LockService.m_CompArr[0].m_CompType = 1;
		LockService.m_CompArr[0].m_wCompPID = 816;
		LockService.m_CompArr[0].m_wECMPID = 0;
		LockService.m_CompArr[1].m_CompType = 3;
		LockService.m_CompArr[1].m_wCompPID = 818;
		LockService.m_CompArr[1].m_wECMPID = 0;
		libc_printf("$$time3: \n");
		CDSTBCA_LockService(&LockService);
		api_start_timer ("LockService", 5*1000, lockservice_timer_func);	
	}
	else if (3 == i)
	{
		LockService.m_fec_inner = LockService.m_fec_outer = 0;
		LockService.m_Modulation = 3;
		LockService.m_symbol_rate = 0x00068750;
		LockService.m_dwFrequency = 0x05560000;
		LockService.m_wPcrPid = 848;
		LockService.m_ComponentNum = 2;
		LockService.m_CompArr[0].m_CompType = 1;
		LockService.m_CompArr[0].m_wCompPID = 848;
		LockService.m_CompArr[0].m_wECMPID = 0x503;
		LockService.m_CompArr[1].m_CompType = 3;
		LockService.m_CompArr[1].m_wCompPID = 850;
		LockService.m_CompArr[1].m_wECMPID = 0x503;
		libc_printf("$$time1: \n");
		CDSTBCA_LockService(&LockService);
		api_start_timer ("LockService", 5*1000, lockservice_timer_func);	
	}

	i++;
	i %= 4;

}

/************************************************************************
 *Testing IPPV display
 ************************************************************************/
static void IPPV_window_test()
{
	static UINT8 i;
	SCDCAIppvBuyInfo buyinfo;
	if (0 == i)
	{
		UINT8 base = 2;
		buyinfo.m_wTvsID = base;
		buyinfo.m_bySlotID = base*2;
		buyinfo.m_dwProductID = 0x5a5a5a;
		buyinfo.m_byPriceNum = 2;
		buyinfo.m_Price[0].m_byPriceCode = CDCA_IPPVPRICETYPE_TPPVVIEW;
		buyinfo.m_Price[0].m_wPrice = base*3;
		
		buyinfo.m_Price[1].m_byPriceCode = CDCA_IPPVPRICETYPE_TPPVVIEWTAPING;
		buyinfo.m_Price[1].m_wPrice = base*4;
		
		CDSTBCA_StartIppvBuyDlg(CDCA_IPPV_FREEVIEWED_SEGMENT, 0x5a, &buyinfo);
	}
	else if (1 == i)
	{
		CDSTBCA_HideIPPVDlg(0x5a);
	}
	if (2 == i)
	{
		UINT8 base = 3;
		buyinfo.m_wTvsID = base;
		buyinfo.m_bySlotID = base*2;
		buyinfo.m_dwProductID = 0xffffffff;
		buyinfo.m_byPriceNum = 2;
		buyinfo.m_Price[0].m_byPriceCode = CDCA_IPPVPRICETYPE_TPPVVIEW;
		buyinfo.m_Price[0].m_wPrice = base*3;
		
		buyinfo.m_Price[1].m_byPriceCode = CDCA_IPPVPRICETYPE_TPPVVIEWTAPING;
		buyinfo.m_Price[1].m_wPrice = base*4;
		
		CDSTBCA_StartIppvBuyDlg(CDCA_IPPV_FREEVIEWED_SEGMENT, 0x5a, &buyinfo);
	}
	else if (3 == i)
	{
		CDSTBCA_HideIPPVDlg(0x5a);
	}
	if (4 == i)
	{
		UINT8 base = 4;
		buyinfo.m_wTvsID = base;
		buyinfo.m_bySlotID = base*2;
		buyinfo.m_dwProductID = 0xffffffff;
		buyinfo.m_byPriceNum = 2;
		buyinfo.m_Price[0].m_byPriceCode = CDCA_IPPVPRICETYPE_TPPVVIEW;
		buyinfo.m_Price[0].m_wPrice = base*3;
		
		buyinfo.m_Price[1].m_byPriceCode = CDCA_IPPVPRICETYPE_TPPVVIEWTAPING;
		buyinfo.m_Price[1].m_wPrice = base*4;
		
		CDSTBCA_StartIppvBuyDlg(CDCA_IPPT_PAYVIEWED_SEGMENT, 0x5a, &buyinfo);
	}
	else if (5 == i)
	{
		CDSTBCA_HideIPPVDlg(0x5a);
	}
	i++;
	i%=6;
}
/************************************************************************
 *Testing Email, Detitle Icon display
 ************************************************************************/
static void mail_detitle_icon_test()
{
	static UINT8 i;
	if (0 == i)
	{
		ap_cas_callback(CAS_MSG_EMAIL_NOTIFY, CDCA_Email_New);
	}
	else if (1 == i)
	{
		ap_cas_callback(CAS_MSG_EMAIL_NOTIFY, CDCA_Email_SpaceExhaust);
	}
	else if (2 == i)
	{
		ap_cas_callback(CAS_MSG_EMAIL_NOTIFY, CDCA_Email_IconHide);
	}
	else if (3 == i)
	{
		ap_cas_callback(CAS_MSG_DETITLE_RECEIVED, CDCA_Detitle_Received);
	}
	else if (4 == i)
	{
		ap_cas_callback(CAS_MSG_DETITLE_RECEIVED, CDCA_Detitle_Space_Small);
	}
	else if (5 == i)
	{
		ap_cas_callback(CAS_MSG_DETITLE_RECEIVED, CDCA_Detitle_All_Readed);
	}
	i++;
	i%=6;
}

/************************************************************************
 *Testing Finger print display
 ************************************************************************/
static void finger_print_test()
{
	static UINT8 i;
	if (0 == i)
	{
		CDSTBCA_ShowFingerMessage(0x5a, 123456789);
	}
	else if (1 == i)
	{
		CDSTBCA_ShowFingerMessage(0x5a, 0);
	}

	i++;
	i%=2;
}

static void card_request_feeding()
{
	CDSTBCA_RequestFeeding(3);
}

static void show_buymsg()
{
	static UINT8 i = 0x01;
	CDSTBCA_ShowBuyMessage(0x100, i);
	i++;
	i%=0x21;
	
}

static void request_feeding()
{
	static UINT8 i=0;
	CDSTBCA_RequestFeeding(i);
	i++;
	i%=2;
}

/*******************************************************************************
*	Testing function entry
*******************************************************************************/
void win_func_testing_proc()
{
	//osdmsg_test();  //tfca display subtitle
	
	//lockservice_test(); //issue: when editing, receive lock msg, no response later :(

	//card_request_feeding();

	//IPPV_window_test();//test ippv popup

	//mail_detitle_icon_test();

	//finger_print_test(); //display finger print

	//show_buymsg();
	
	//request_feeding();

}
