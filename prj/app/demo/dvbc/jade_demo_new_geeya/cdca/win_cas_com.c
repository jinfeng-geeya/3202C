#include <stdarg.h>
#include <stddef.h>
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
//#include <api/libosd/osd_lib.h>
#include "../win_com.h"
#include "../string.id"
#include "../images.id"
#include "../win_com_popup.h"
#include "win_cas_com.h"

/************************************************************
*				Below are Pin input related functions
*************************************************************/
UINT8		pin_pwd_password_str[MAX_PIN_PWD_ARRAY][PIN_PWD_LEN];
static UINT8	g_pin_Input_Bit_Number;
static UINT8	g_pin_Input_Status;

//set the pin internal status
void pin_pwd_set_status(UINT8 bit, UINT8 status)
{
	g_pin_Input_Bit_Number = bit;
	g_pin_Input_Status = status;
}

void pin_pwd_get_status(UINT8 *bit, UINT8 *status)
{
	*bit = g_pin_Input_Bit_Number;
	*status = g_pin_Input_Status;
}

//index: bitmap for string array
void pin_pwd_init(UINT8 index)
{
	UINT8 i;
	for (i=0; i<MAX_PIN_PWD_ARRAY; i++)
	{
		if (index&(1<<i))
			MEMSET(pin_pwd_password_str[i],0xff,sizeof(pin_pwd_password_str[i]));
	}
}

void pin_pwd_set_str(UINT8 index) 
{
	UINT8 temp[10] = {0,};
	if ( ( g_pin_Input_Status == TRUE ) && ( g_pin_Input_Bit_Number > 0 ) )
	{
		switch ( g_pin_Input_Bit_Number )
		{
			case 1:
				sprintf ( temp, "*-----" );
				break;
			case 2:
				sprintf ( temp, "**----" );
				break;
			case 3:
				sprintf ( temp, "***---" );
				break;
			case 4:
				sprintf ( temp, "****--" );
				break;
			case 5:
				sprintf ( temp, "*****-" );
				break;
			case 6:
				sprintf ( temp, "******" );
				break;
			case 7:
				//sprintf ( temp, "*******-" );
				break;
			case 8:
				//sprintf ( temp, "********" );
				break;
		}
	}
	else
	{
		sprintf ( temp, "------" );
		pin_pwd_init(1<<index);
	}
	ComAscStr2Uni ( temp, display_strs[index+PIN_POSTION] );
}


void pin_pwd_input(UINT8 item, UINT8 number)
{
	if ( g_pin_Input_Status == TRUE )
	{
		switch ( g_pin_Input_Bit_Number )
		{
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
				pin_pwd_password_str[item][g_pin_Input_Bit_Number] = number + '0';
				g_pin_Input_Bit_Number++;
				break;
			case 6:
				pin_pwd_set_str(item);
				pin_pwd_init(1<<item);
				pin_pwd_password_str[item][0] = number + '0';
				g_pin_Input_Bit_Number = 1;
				break;
		#if 0
			case 0:
			case 1:
			case 2:
			case 3:
			case 4:
			case 5:
			case 6:
			case 7:
				pin_pwd_password_str[item][g_pin_Input_Bit_Number] = number;
				g_pin_Input_Bit_Number++;
				break;
			case 8:
				pin_pwd_set_str(item);
				pin_pwd_init(1<<item);
				pin_pwd_password_str[item][0] = number;
				g_pin_Input_Bit_Number = 1;
				break;
		#endif
		}
	}
	else
	{
		g_pin_Input_Status = TRUE;
        	pin_pwd_password_str[item][0] = number + '0';
		g_pin_Input_Bit_Number = 1;
	}
}

void pin_pwd_drawback(UINT8 item)
{
	if (FALSE == g_pin_Input_Status)
	{
		return;
	}
	if (g_pin_Input_Bit_Number>0)
	{
		g_pin_Input_Bit_Number --;
		pin_pwd_password_str[item][g_pin_Input_Bit_Number] = 0xff;
	}
	else
	{
		g_pin_Input_Status = FALSE;
	}
	pin_pwd_set_str(item);
}

UINT8 pin_pwd_is_same(UINT8 index1, UINT8 index2, UINT32 len)
{
	UINT32 i;

	if (len>PIN_PWD_LEN)
		return 0;
	
	for (i=0; i<len; i++)
	{
		if (pin_pwd_password_str[index1][i] != pin_pwd_password_str[index2][i])
			break;
	}

	if (i >= len)
		return TRUE;
	else 
		return FALSE;
}

UINT8 pin_pwd_len(UINT8 *pin)
{
	UINT8 i=0;

	if (NULL == pin)
		return 0;
	
	for (i=0; i<PIN_PWD_LEN; i++)
	{
		if (pin[i] == 0xff)
			break;
	}

	return i;
}
/************************************************************
*				Below are common functions for AP
*************************************************************/
static UINT8 IsLeapYear(UINT16 iYear)
{
    return !(iYear%4)&&(iYear%100) || !(iYear%400);
}

void YMD_calculate(UINT32 iDiffDays, UINT8 *date)
{
	UINT32 year,month,day, offset;
	UINT16 total, i;
	UINT8 month_day[]={29,31,28,31,30,31,30,31,31,30,31,30,31}; 
	UINT8 temp[11];
	year = 2000;//start from 2000/01/01. 0 means 2000/01/01, 1-2000/01/02....
	month = 1;
	day = 1;
	
	/*if (0 == iDiffDays)
	{
		MEMSET(temp, 0, sizeof(temp));
		//wincom_AscToMBUni(temp, date);
		MEMCPY(date, temp, sizeof(temp));
		return;
	}*/
	offset = iDiffDays;

	total = (IsLeapYear(year))?366:365;
      while(offset>=total)     //Add years 
      {  
          year++;  
          offset-=total;  
          total = (IsLeapYear(year))?366:365; 
      }  

   	i= (IsLeapYear(year)&&(month==2))?0:month;
      while(offset>=month_day[i])         //add months  
      {  
          month++;  
          if(month>12)  
          {  
              year++;  
              month=1;  
          }  
          offset-=month_day[i];  
         i= (IsLeapYear(year)&&(month==2))?0:month; 
      }  
	  
      day+=offset;  
      while(day>month_day[i])             //add days
      {  
          day-=month_day[i];  
          month++;  
          if(month>12)  
          {  
              year++;  
              month=1;  
          }  
      }  
	//printf("year is:%d  nmonth is:%d  day is:%d\n",year,month,day);
	MEMSET(temp, 0, sizeof(temp));
	sprintf(temp, "%4d/%02d/%02d", year, month, day);
	//wincom_AscToMBUni(temp, date);
	MEMCPY(date, temp, sizeof(temp));
}

BOOL CaculateSysDateToUTC(UINT32 sysDay, date_time * date)
{
	INT32 nYear,nMon,nDay,nTemp,nDate = sysDay;
	nYear = 1970;
	do
	{	/*year.*/
		nTemp = (IsLeapYear(nYear))?366:365;
		if(nDate - nTemp < 0)
			break;
		nDate -= nTemp;
		nYear ++;
	}while(TRUE);
	
	nMon = 1;
	do
	{	/*month.*/
		if (2 == nMon) 
		{
			nTemp = (IsLeapYear(nYear))? 29: 28;
		}
		else if(4 == nMon || 6 == nMon || 9 == nMon || 11 == nMon)
			nTemp = 30;
		else
			nTemp = 31;
		if(nDate - nTemp < 0)
			break;
		nDate -= nTemp;
		nMon ++;
	}while(TRUE);
	if(1 > nMon || 12 < nMon)
	{
		return FALSE;
	}
	
	nDay = 1;
	nDay += nDate; /*day.*/
	if(1 > nDay || 31 < nDay)
	{
		return FALSE;
	}

	date->year = nYear;
	date->month = nMon;
	date->day = nDay;
	return TRUE;
}

static void CaculateSysDateToHMS(UINT32 second, date_time *clock)
{
	clock->hour = (UINT8)(second / 3600);
	clock->min = (UINT8)((second % 3600) / 60);
	clock->sec = (UINT8)((second % 3600) % 60);
	
}

void CaculateSysDateToGMT(UINT32 c_time, date_time *email_time)
{
       c_time=c_time+8*3600;
	CaculateSysDateToUTC((UINT32)(c_time / 86400), email_time);//24*3600
	CaculateSysDateToHMS((c_time % 86400), email_time);
}

//convert the sysdate c_time to unicode and ouput in *date: y/m/d h:m:s
void convert_sysdate_to_uni(UINT32 c_time, UINT16 *date)
{
	date_time time;
	UINT8 asc_time[20];

	CaculateSysDateToGMT(c_time, &time);
	sprintf(asc_time, "%4d/%02d/%02d %02d:%02d:%02d",time.year,time.month,time.day,time.hour,time.min,time.sec);
	wincom_AscToMBUni(asc_time,date);
}


/************************************************************
*				Below are help bar related variables
*************************************************************/

void cas_popup_msg(char *str, char *unistr, UINT16 strID)
{
	UINT8 save;
	win_compopup_init(WIN_POPUP_TYPE_OK);
	win_compopup_set_msg(str, unistr, strID);
	win_compopup_open_ext(&save);
}

win_popup_choice_t cas_popup_yesno_msg(char *str, char *unistr, UINT16 strID)
{
	UINT8 save;
	win_popup_choice_t ret;
	win_compopup_init(WIN_POPUP_TYPE_OKNO);
	win_compopup_set_msg(str, unistr, strID);
	ret = win_compopup_open_ext(&save);

	return ret;
}

void win_init_pstring(UINT8 num)
{
	UINT8 i;
	for(i = 0; i < num; i++)
	{
		ComAscStr2Uni("", display_strs[i]);
	}
}

UINT8 cas_inter_print(UINT16 *out, const UINT8 *fmt, va_list arg)
{
	UINT8 ret = TRUE;
	UINT8 ch;
	UINT8 *cp;//char * pointer
	const UINT8 *fmark; //mark the previous position
	UINT16 strid; //Unicode string ID
	UINT16 *pu;//UNICODE *pointer;
	INT32 num; //input number variable
	UINT32 len=0,n=0;
	UINT8 buf[256] ={0,};
	
	out[0] = 0x0000; //make sure we start from the beginning of out
	
	for (;;)
	{
		for (fmark = fmt;(ch = *fmt)!='\0' && ch!='%'; fmt++)
			;
		
		len = ComUniStrLen(out);
		if ((n = fmt-fmark) != 0)
		{
			MEMSET(buf, 0, sizeof(buf));
			MEMCPY(buf, fmark, n);
			ComAscStr2Uni(buf,&out[len]);
			len = ComUniStrLen(out);
		}
		if (ch == '\0')
			return TRUE; //reach the end
		
		fmt++; //skip the "%"
		fmark = fmt;//mark the position for case '0'...case '9'
		ch = *fmt++;
		
		switch(ch)
		{
			case 'i':
			case 'I':
				if ((strid = va_arg(arg, INT32)) == 0)
				{
					libc_printf("%s: NULL@strid!\n", __FUNCTION__);
					ret = FALSE;
					break;
				}
				cp = OSD_GetUnicodeString(strid);
				ComUniStrCopyChar((UINT8 *)&out[len],cp);
				break;
			case 'a':
			case 'A':
				if ((cp = va_arg(arg, UINT8 *)) == NULL)
				{
					libc_printf("%s: NULL@ascii!\n", __FUNCTION__);
					ret = FALSE;
					break;
				}
				ComAscStr2Uni(cp,&out[len]);
				break;
			case 'u':
			case 'U':
				if ((pu = va_arg(arg, UINT16 *)) == NULL)
				{
					libc_printf("%s: NULL@unicode!\n", __FUNCTION__);
					ret = FALSE;
					break;
				}
				ComUniStrCat(&out[len], pu);
				break;
			case 'd':
			case 'D':
				num = va_arg(arg, INT32);
				MEMSET(buf, 0, sizeof(buf));
				sprintf(buf, "%d", num);
				ComAscStr2Uni(buf,&out[len]);
				break;
			case 'x':
			case 'X':
				num = va_arg(arg, INT32);
				MEMSET(buf, 0, sizeof(buf));
				sprintf(buf, "%x", num);
				ComAscStr2Uni(buf,&out[len]);
				break;
			case '0':	case '1': 	case '2': case '3': case '4': case '5': case '6': case '7': case '8': case '9': 
			{
				UINT8 ss[20] = {'%',};
				UINT8 pss = 1;
				num = va_arg(arg, INT32);
				MEMSET(buf, 0, sizeof(buf));
				//sprintf(buf, "%x", num);
				//ComAscStr2Uni(buf,&out[len]);
				while(*fmark != '\0' && *fmark != '%')
				{
					ss[pss++]=*fmark++;
				}
				sprintf(buf, ss, num);
				ComAscStr2Uni(buf,&out[len]);
				fmt = fmark;
				break;
			}
			default:
				libc_printf("%s: unknown format!\n", __FUNCTION__);
				break;
		}
		if (FALSE == ret )
			break;
	}

	return ret;
}

/*
*Describe: Print to Unicode string from string_ID, ascii string, number, unicode string
*
*Usage: simliar to sprintf, cas_sprintf(buf, "asciistring%i%a%d%x", var1, var2...)
*	%i %I: string ID, %a %A: ASCII string, %u %U: UNICODE string
*    %d %D: print to decimal, %x %X:print to Hex
*/
UINT8 cas_sprintf(UINT16 *out, const UINT8 *format, ...)
{
	UINT8 ret;
	va_list arg;
	va_start(arg, format);
	ret = cas_inter_print(out, format, arg);
	//va_end;

	return ret;
}


BOOL ap_cas_wait_sc_ok(UINT32 tmo)
{
	UINT32 tick = osal_get_tick();

	while (osal_get_tick() < (tick + tmo))
	{
		if (gy_get_sc_status())
			return TRUE;

		osal_task_sleep(10);
	}

	return FALSE;
}

