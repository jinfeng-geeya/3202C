/*****************************************************************************************************
  * Description
  *  This file implement TongFang CAS adapter layer
  * Remarks
  *   None.
  * Bugs
  *   None.
  * TODO
  *    * None.
  * History
  *   <table>
  *   \Author         Date        Change Description
  *   ----------    ----------  -------------------
  *     2009.7.30  Initialize 
  *   </table>
  *
*****************************************************************************************************/
#if 0
#include <sys_define.h>
#include <sys_types.h>
#include <osal/osal.h>
#include <mdl/libc/stdio.h>
#include <mdl/libc/stdlib.h>
#include <mdl/libc/string.h>
#include <mdl/algo/fast_crc.h>
#include <hld/smc/smc.h>
#include <hld/dmx/dmx.h>
#endif

#if 0
#include <sys_define.h>
#include <basic_types.h>
#include <osal/osal.h>
#include <api/libfs2/stdio.h>
#include <stdlib.h>
#include <api/libc/string.h>
#include <hld/smc/smc.h>
#include <hld/sto/sto.h>
#include <hld/dmx/dmx.h>


#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#endif
/*
#include <sys_config.h>
#include <types.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libosd/osd_lib.h>
*/

#include <sys_config.h>
#include <types.h>
#include <retcode.h>
#include <hld/hld_dev.h>
#include <osal/osal.h>
#include <api/libc/printf.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/sto/sto.h>
#include <hld/sto/sto_dev.h>
#include <api/libchunk/chunk.h>
#include <api/libc/time.h>

#include <api/libcas/gy/gyca_def.h>
#include <api/libcas/gy/cas_gy.h>

#if 1
#define GYCAS_PRINTF	libc_printf
#define GYCAS_DUMP(data,len) { const int l=(len); int i;\
                         for(i=0 ; i<l ; i++) GYCAS_PRINTF(" %02x",*((data)+i)); \
                         GYCAS_PRINTF("\n"); }
#else
#define GYCAS_PRINTF(...)	do{}while(0)
#define GYCAS_DUMP(data,len);
#endif


#define CAS_MAIL_CHUNK_ID 0x08F70100

#define SECTOR_SIZE        64*1024
//#define CHUNK_HEADER_SIZE     128
#define GYCA_MAILWRITE_CYCLE  2*1000
//#define GYCA_MAIL_HEAD       "GYCAS"

static UINT8 gyca_mail_head[] = "GYCAS";
static UINT32 gyca_email_start_addr = 0xffffffff;
static OSAL_ID gyca_email_mutex = OSAL_INVALID_ID;

#define GYCA_MAIL_HEAD_SIZE  sizeof(gyca_mail_head)


static T_CAMailInfo premail;
static gycamail flashbuf[GYCA_EMAIL_MAX_CNT];
static UINT32 gyca_write_enable_time = 0;
static BOOL gyca_write_enable = FALSE;

static mail_status gyca_mail_status = MAIL_OLD_FULL;


static UINT8 gyca_delmail_all_from_flash(UINT32 addr);
static UINT8 gyca_writemaill_all_to_flash(UINT32 addr, UINT32 mail_head_size);
static UINT8 gyca_readmail_all_from_flash(UINT32 addr, UINT32 mail_head_size);
static UINT8 gyca_getmail_num_from_ram(void);
static UINT8 gyca_write_flash_mail_head(UINT32 addr);
static UINT8 gyca_check_flash_mail_head(UINT32 addr);
static void gyca_mutex_unlock(void);
static void gyca_mutex_lock(void);


static void gyca_mutex_lock(void)
{
	if(gyca_email_mutex != OSAL_INVALID_ID)
		osal_mutex_lock(gyca_email_mutex, OSAL_WAIT_FOREVER_TIME);
	else 
		return ;
}
static void gyca_mutex_unlock(void)
{
	if(gyca_email_mutex != OSAL_INVALID_ID)
		osal_mutex_unlock(gyca_email_mutex);
	else 
		return ;
}

static UINT8 gyca_check_flash_mail_head(UINT32 addr)
{
	struct sto_device *flash_dev = NULL;
	UINT32 mailaddr = addr;
	UINT8 mail_head[GYCA_MAIL_HEAD_SIZE];
	
	//flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(NULL == flash_dev)
	{
		GYCAS_PRINTF("%s: dev_get_by_name failed!\n", __FUNCTION__);
		return 1;
	}
	//if (SUCCESS  != flash_dev->get_data(flash_dev, mail_head, mailaddr, GYCA_MAIL_HEAD_SIZE))
	if (GYCA_MAIL_HEAD_SIZE  != sto_get_data(flash_dev, mail_head, mailaddr, GYCA_MAIL_HEAD_SIZE))
	{
		GYCAS_PRINTF("%s: sto_get_data failed!\n", __FUNCTION__);
		return 2;
	}
	gyca_mutex_lock();
	if(MEMCMP(mail_head,gyca_mail_head, GYCA_MAIL_HEAD_SIZE))
	{
		gyca_mutex_unlock();
		return 3;
	}
	gyca_mutex_unlock();
	
	return 0;
}

static UINT8 gyca_write_flash_mail_head(UINT32 addr)
{
	struct sto_device *flash_dev = NULL;
	UINT32 mailaddr = addr;
	
	//flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(NULL == flash_dev)
	{
		GYCAS_PRINTF("%s: dev_get_by_name failed!\n", __FUNCTION__);
		return 1;
	}
	//gyca_mutex_lock();
	//if (SUCCESS  != flash_dev->put_data(flash_dev, mailaddr, gyca_mail_head, GYCA_MAIL_HEAD_SIZE)) 
	if (GYCA_MAIL_HEAD_SIZE  != sto_put_data(flash_dev, mailaddr, gyca_mail_head, GYCA_MAIL_HEAD_SIZE))
	{
		//gyca_mutex_unlock();
		GYCAS_PRINTF("%s: sto_put_data failed!\n", __FUNCTION__);
		return 2;
	}
	//gyca_mutex_unlock();
	return 0;
}

UINT8 gyca_mail_init(void)
{
	UINT32 chunk_id = CAS_MAIL_CHUNK_ID;
	CHUNK_HEADER chuck_hdr;

	/* get table info base addr by using chunk */
	if(sto_get_chunk_header(chunk_id, &chuck_hdr) == 0)
	{
		GYCAS_PRINTF("Err: %s get chunk header fail!\n", __FUNCTION__);
		return 1;
	}
	gyca_email_mutex = osal_mutex_create();
	if(gyca_email_mutex == OSAL_INVALID_ID)
	{
		GYCAS_PRINTF("Err: %s create mutex failed!\n", __FUNCTION__); 
	}
	gyca_email_start_addr = sto_chunk_goto(&chunk_id, 0xFFFFFFFF, 1) + CHUNK_HEADER_SIZE + SECTOR_SIZE;
	if(!gyca_check_flash_mail_head(gyca_email_start_addr))
		gyca_readmail_all_from_flash(gyca_email_start_addr, GYCA_MAIL_HEAD_SIZE);
	else
	{
		MEMSET(flashbuf, 0, sizeof(flashbuf));
		//gyca_mail_test();
		//gyca_delmail_all_flash(gyca_email_start_addr);
		//gyca_writemaill_all_to_flash(gyca_email_start_addr, GYCA_MAIL_HEAD_SIZE);
	}

}

static UINT8 gyca_getmail_num_from_ram(void)
{	
	UINT8 i;
	for(i=0; i<GYCA_EMAIL_MAX_CNT; i++)
	{
		if(flashbuf[i].flg != 1)
		{
			return i;
		}
	}
	return GYCA_EMAIL_MAX_CNT; //-1
}

UINT8 gyca_getmail_condition_from_ram(UINT8 *m_total, UINT8 *m_new)
{
	UINT8 t = 0, n = 0;
	UINT8 i;

	gyca_mutex_lock();
	for(i=0; i<GYCA_EMAIL_MAX_CNT; i++)
	{
		if(flashbuf[i].flg == 1)
		{
			if(flashbuf[i].status == 1)
				n++;
			t++;
		}
		else 
			break;
	}	
	if(t == 0)
	{
		gyca_mutex_unlock();
		return 0xff;
	}
	
	*m_total = t;
	*m_new = n;
	gyca_mutex_unlock();
	return 0;
}

UINT8 gyca_addmail_one_to_ram(T_CAMailInfo *data)
{
	UINT8 i;
	UINT8 ret = 0xff;
	if(!MEMCMP(&premail, data, sizeof(T_CAMailInfo)))
		return 1;
	
	gyca_mutex_lock();
	ret = gyca_getmail_num_from_ram();
	if(GYCA_EMAIL_MAX_CNT == ret)
	{
		gyca_mutex_unlock();
		return 2;
	}
	i = ret ;
	//gyca_mutex_lock();
	flashbuf[i].flg = 1;
	flashbuf[i].status = 1;
	MEMCPY(&flashbuf[i].info, data, sizeof(T_CAMailInfo));
	gyca_write_enable_time = osal_get_tick();
	gyca_write_enable = TRUE;
	gyca_mutex_unlock();
	return 0;
}

UINT8 gyca_delmail_one_from_ram(UINT8 index)
{
	UINT8 ret = 0xff;
	UINT8 i;
	
	gyca_mutex_lock();
	ret =  gyca_getmail_num_from_ram();
	if((flashbuf[index].flg != 1) || (index>ret-1))
	{
		gyca_mutex_unlock();
		return 1;
	}
	//gyca_mutex_lock();
	if(index == (ret-1))
		MEMSET(&flashbuf[index], 0, sizeof(gycamail));	
	else
	{
		for(i=index; i<ret; i++)
		{
			MEMCPY(&flashbuf[i], &flashbuf[i+1], sizeof(gycamail));
		}
		MEMSET(&flashbuf[i], 0, sizeof(gycamail));
	}
	gyca_write_enable_time = osal_get_tick();
	gyca_write_enable = TRUE;
	gyca_mutex_unlock();
	return 0;
	
}
UINT8 gyca_delmail_all_from_ram(void)
{
	gyca_mutex_unlock();
	MEMSET(flashbuf, 0, sizeof(gycamail)*GYCA_EMAIL_MAX_CNT);
	gyca_write_enable_time = osal_get_tick();
	gyca_write_enable = TRUE;
	gyca_mutex_unlock();
	return 0;
}


UINT8 gyca_setmail_one_oldstatus_ram(UINT8 index)
{
	UINT8 ret = 0xff;

	gyca_mutex_lock();
	ret =  gyca_getmail_num_from_ram();
	if((flashbuf[index].flg != 1) || (index>ret-1))
	{
		gyca_mutex_unlock();
		return 1;
	}
	//gyca_mutex_lock();
	flashbuf[index].status = 0;
	gyca_write_enable_time = osal_get_tick();
	gyca_write_enable = TRUE;
	gyca_mutex_unlock();
	
	return 0;
}


void gyca_check_new_mail(void)
{
	mail_status status1 = MAIL_OLD_FULL;
	mail_status status2 = MAIL_OLD_FULL;
	UINT8 i;
	gyca_mutex_lock();
	for(i=0; i<GYCA_EMAIL_MAX_CNT; i++)
	{
		if(flashbuf[i].flg == 1)
		{
			if(i == GYCA_EMAIL_MAX_CNT -1)
				status2 =  MAIL_SPACE_FULL;
			if(flashbuf[i].status == 1)
				status1 = MAIL_NEW;
				
		}
		else
			break;
	}
	gyca_mutex_unlock();

	if(status2 == MAIL_SPACE_FULL)
		gyca_setmail_status(MAIL_SPACE_FULL);
	else if(status1 == MAIL_NEW)
	{
		#ifdef STANDBY_BY_PANL_KEY 
		//key_pan_display_led(TRUE,EMAIL_LED);
		#endif
		gyca_setmail_status(MAIL_NEW);
	}
	else 
		gyca_setmail_status(MAIL_OLD_FULL);
	
}

void gyca_setmail_status(mail_status status)
{
	gyca_mail_status = status;
}

mail_status gyca_getmail_status(void)
{
	return gyca_mail_status;
}

UINT8 gyca_readmail_one_from_ram(UINT8 index, gycamail *data)
{
	UINT8 ret = 0xff;
	
	gyca_mutex_lock();
	ret =  gyca_getmail_num_from_ram();
	if((flashbuf[index].flg != 1) || (index>ret-1))
	{	
		gyca_mutex_unlock();
		return 1;
	}
	
	//gyca_mutex_lock();
	MEMCPY(data, &flashbuf[index], sizeof(gycamail));
	gyca_mutex_unlock();
	
	return 0;
}

static UINT8 gyca_readmail_all_from_flash(UINT32 addr, UINT32 mail_head_size)
{
	struct sto_device *flash_dev = NULL;
	UINT32 mailaddr;
	
	mailaddr=addr + mail_head_size;
	//flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(NULL == flash_dev)
	{
		GYCAS_PRINTF("gyca_readmail_all_from_flash: dev_get_by_name failed!\n");
		return 1;
	}

	gyca_mutex_lock();
	if ( sizeof(gycamail)*GYCA_EMAIL_MAX_CNT != sto_get_data(flash_dev, flashbuf, mailaddr, sizeof(gycamail)*GYCA_EMAIL_MAX_CNT))
	{
		MEMSET(flashbuf, 0, sizeof(flashbuf));
		gyca_mutex_unlock();
		GYCAS_PRINTF("%s: sto_get_data failed!\n", __FUNCTION__);
		return 2;
	}
	gyca_mutex_unlock();
	
	return 0;
}

static UINT8 gyca_writemaill_all_to_flash(UINT32 addr, UINT32 mail_head_size)
{
	struct sto_device *flash_dev = NULL;
	UINT32 mailaddr;
	
	mailaddr=addr + mail_head_size;
	//flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(NULL == flash_dev)
	{
		GYCAS_PRINTF("gyca_writemaill_all_to_flash: dev_get_by_name failed!\n");
		return 1;
	}
	if(gyca_delmail_all_from_flash(addr)!= 0)
		return 2;
	gyca_write_flash_mail_head(addr);
	gyca_mutex_lock();
	if(sizeof(gycamail)*GYCA_EMAIL_MAX_CNT != sto_put_data(flash_dev, mailaddr, flashbuf, sizeof(gycamail)*GYCA_EMAIL_MAX_CNT))
	{
		gyca_mutex_unlock();
		GYCAS_PRINTF("%s: sto_put_data failed!\n", __FUNCTION__);
		return 3;
	}
	gyca_mutex_unlock();
	
	return 0;
}

static UINT8 gyca_delmail_all_from_flash(UINT32 addr)
{
	struct sto_device *flash_dev = NULL;
	UINT32 param, flash_cmd, tmp_param[2];
	UINT8 *buff = NULL;
	INT32  ret = -1;
		
	//flash_dev = (struct sto_device *)dev_get_by_type(NULL, HLD_DEV_TYPE_STO);
	flash_dev = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
	if(NULL == flash_dev)
	{
		GYCAS_PRINTF("gyca_readmail_all_from_flash: dev_get_by_name failed!\n");
		return 1;
	}
	if(flash_dev->totol_size <= 0x400000)
	{
		param = addr << 10;
		param |= 64*1024 >> 10;//CAS_CHUNK_SIZE >> 10;
		flash_cmd = STO_DRIVER_SECTOR_ERASE;
	}
	else
	{
		tmp_param[0] = addr;
		tmp_param[1] = SECTOR_SIZE >> 10;
		param = (UINT32)tmp_param;
		flash_cmd = STO_DRIVER_SECTOR_ERASE_EXT;
	}
	buff = (UINT8 *)MALLOC(SECTOR_SIZE);
	sto_io_control(flash_dev, STO_DRIVER_SECTOR_BUFFER, (UINT32)buff);
	ret = sto_io_control(flash_dev, flash_cmd, param);
	if(buff != NULL)
		free(buff);
	if(SUCCESS != ret )
	{
		GYCAS_PRINTF("%s: Erase failed!\n", __FUNCTION__);
		return 2;
	}
	return 0;
}

void gyca_check_writemail_to_flash(void)
{
	UINT32 tmp = 0;
	if(gyca_write_enable == TRUE)
	{	
		tmp = gyca_write_enable_time;
		if((osal_get_tick() - gyca_write_enable_time) < GYCA_MAILWRITE_CYCLE)
			return ;
		gyca_writemaill_all_to_flash(gyca_email_start_addr, GYCA_MAIL_HEAD_SIZE);
		if(gyca_write_enable_time == tmp)
			gyca_write_enable = FALSE;
	}
}

INT32 mjd_to_tm(UINT32 mjd, struct tm *tm)
{
	UINT32 Y, YY, M, D, K;
	Y = (20*mjd - 301564) / 7305;
	YY = (Y*365 + Y/4);	//(UINT32)(Y * 365.25)
	M = 1000*(10*(mjd-YY) - 149561) / 306001;
	D = mjd - 14956 - YY - M * 306001 / 10000;

	K = (M == 14 || M == 15)? 1: 0;
	tm->tm_year = Y + K;
	tm->tm_mon	= M - 1 - K * 12;
	if (tm->tm_mon > 12) {
		tm->tm_mon = 1;
	}
	tm->tm_mday = D>31? 1: D;
	tm->tm_wday = ((mjd + 2) % 7) + 1;
	--tm->tm_mon; /* tm_mon is starting from 0 */
	/* TODO: last_tm.tm_yday is not set! */
	tm->tm_hour = 0;
	tm->tm_min = 0;
	tm->tm_sec = 0;
	return SUCCESS;
}

void gyca_mail_test(void)
{
	UINT8 i;
	T_CAMailInfo data;
	for(i = 0; i<50; i++)
	{
		MEMSET(&data,0,sizeof(T_CAMailInfo));
		sprintf(data.m_szEmail_Title, "金亚 Email test title %d",i);
		sprintf(data.m_szEmail_Content, "金亚 Email test content %d",i);
		//MEMSET(&data.m_szEmail_Title, i, 50);
		//MEMSET(&data.m_szEmail_Content, i, 112);
		gyca_addmail_one_to_ram(&data);
		libc_printf("MAIL_TEST+++++++++++++++++++++++++++++++++ %d\n",i);
	}
}
