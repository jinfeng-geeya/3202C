/*
 * Copyright (C) ALi Shanghai Corp. 2004
 *
 * File: lib_as.c
 *
 * Content: auto-scan service implementation for M3327 chip based IRD.
 *
 * History:
 *    Date	Version		Author		Comment
 *    ========	======= 	========	========
 */
#include <types.h>
#include <sys_config.h>
#include <retcode.h>
#include <mediatypes.h>
#include <bus/tsi/tsi.h>
#include <osal/osal.h>
#include <api/libc/alloc.h>
#include <api/libc/string.h>
#include <hld/hld_dev.h>
#include <hld/nim/nim.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>
#include <api/libnim/lib_nim.h>


#include "../as_dvbc.h"
#include <api/libtsi/p_search.h>
#include <api/libtsi/tmp_info.h>
#include <api/libtsi/psi_search.h>

#include <api/libpub/lib_pub.h>
#include <api/libpub/as_module.h>



#define AS_DEBUG_LEVEL 			0
#if (AS_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define AS_PRINTF			libc_printf
#define AS_SDBBP			SDBBP
#else
#define AS_PRINTF(...)		do{}while(0)
#define AS_SDBBP(...)			do{}while(0)
#endif

#ifdef __MM_VBV_START_ADDR
#ifdef DUAL_ENABLE
#define __MM_AS_START_ADDR		(__MM_EPG_BUFFER_START&0x8FFFFFFF)
#define __MM_PSI_START_ADDR		((__MM_EPG_BUFFER_START&0x8FFFFFFF)+sizeof(struct as_module_info))
#else
#define __MM_AS_START_ADDR		(__MM_VBV_START_ADDR&0x8FFFFFFF)
#define __MM_PSI_START_ADDR		((__MM_VBV_START_ADDR&0x8FFFFFFF)+sizeof(struct as_module_info))
#endif
#else
#if (DATA_BROADCAST==DATA_BROADCAST_IPANEL)
#define __MM_AS_START_ADDR		((UINT32)IPANEL_MW_BUF_START&0x8FFFFFFF)
#define __MM_PSI_START_ADDR		(((UINT32)IPANEL_MW_BUF_START&0x8FFFFFFF)+sizeof(struct as_module_info))
#else
void *as_buf = NULL;
#define AS_BUF_LEN 0x48000
#define __MM_AS_START_ADDR		((UINT32)as_buf&0x8FFFFFFF)
#define __MM_PSI_START_ADDR		(((UINT32)as_buf&0x8FFFFFFF)+sizeof(struct as_module_info))
#endif
#endif

static struct as_module_info *as_info = NULL;

#if (AS_DEBUG_LEVEL > 0)
static void AS_DUMP_SAT(S_NODE *sat)
{
    AS_PRINTF("***************************************\n");
    AS_PRINTF("* sat id = %d\n", sat->sat_id);
    AS_PRINTF("* sat lnb_type = %d\n", sat->lnb_type);
    AS_PRINTF("* sat lnb_low = %d\n", sat->lnb_low);
    AS_PRINTF("* sat lnb_high = %d\n", sat->lnb_high);
    AS_PRINTF("* sat polar = %d\n", sat->pol);
    AS_PRINTF("***************************************\n");
}

static void AS_DUMP_TP(T_NODE *tp)
{
    AS_PRINTF("***************************************\n");
    AS_PRINTF("* tp id = %d\n", tp->tp_id);
    AS_PRINTF("* tp freq = %d\n", tp->frq);
    AS_PRINTF("* tp sym = %d\n", tp->sym);
    AS_PRINTF("* tp pol = %d\n", tp->pol);
    AS_PRINTF("* tp FEC inner = %d\n", tp->FEC_inner);
    AS_PRINTF("***************************************\n");
}
#else
#define AS_DUMP_SAT(...)			do{}while(0)
#define AS_DUMP_TP(...)			do{}while(0)
#endif

#ifdef NIT_AUTO_SEARCH_SUPPORT
static BOOL b_found_nit_tp = FALSE; //TRUE: found nit in a tp; FALSE: no
#endif

/*NIt search callback for update tp*/
static UINT8 as_update_tp(TP_INFO *t_info)
{
    INT32 i;
    T_NODE* tp = &(as_info->xp);

	if(as_info->param.as_frontend_type == FRONTEND_TYPE_C)
	{
//for jilin prj,filter the HD program.      
#ifdef JILIN_FEATURE
            if((t_info->c_info.frequency==72200)
                ||(t_info->c_info.frequency==73000)
                ||(t_info->c_info.frequency==73800))
            {
                libc_printf("\ncur freq:%d\n",t_info->c_info.frequency);
                return as_info->stop_scan_signal;
            }
#endif       
		//Copy Input TP Info to Current TP
    		dvbc_tp_info2db(t_info, &as_info->cur_tp);
	    //Check whether Current TP Param is Valid or not
	    	if(dvbc_tp_sanity_check(FALSE, &as_info->cur_tp, NULL) == FALSE)
	    	{
	        		return as_info->stop_scan_signal;
	    	}
	}
	

	//Search TP Info in DB. If can't find, then Add this TP Info into DB
    	as_info->cur_tp.sat_id = as_info->cur_sat.sat_id;
    	if(db_search_lookup_node(TYPE_SEARCH_TP_NODE, &as_info->cur_tp) != SUCCESS)
    	{
        	if(add_node(TYPE_SEARCH_TP_NODE, as_info->cur_tp.sat_id, &as_info->cur_tp) == STORE_SPACE_FULL)
        	{
            		return 1;
        	}
    	}

    	AS_DUMP_TP(&as_info->cur_tp);
	//Search Current TP in Waiting Scan TP List, if find, then Return, otherwise Add Current TP 
	//to Waiting Scan TP List

    	for(i = as_info->tp_nr - 1; i >= 0; i--)
    	{
        	if (as_info->tp_id[i] == as_info->cur_tp.tp_id && as_info->tp_si[i])
        	{
            		return as_info->stop_scan_signal; 
        	}
	}		


    	as_info->tp_id[as_info->tp_nr++] = as_info->cur_tp.tp_id;
#ifdef NIT_AUTO_SEARCH_SUPPORT// add for auto scan with nit search 
	if(as_info->atp.nit_enable == 1 && as_info->tp_nr > 0)
		b_found_nit_tp = TRUE;
	else 
		b_found_nit_tp = FALSE;
#endif

	if(as_info->param.as_handler != NULL)
    	{
        	as_info->param.as_handler(ASM_TYPE_INC_TRANSPONDER, 0);
    	}

    	return as_info->stop_scan_signal; 
}



/*description	: transpoder program search update callback.*/
static UINT8 psi_callback(UINT8 progress, void *param)
{
    UINT16 pg_pos;

    //nit scan update tp
    if (progress>=AS_PROGRESS_NIT)
    {
    	if(progress==AS_PROGRESS_NIT_TP_CNT)
    	{
    		UINT16 tp_cnt = *((UINT16 *)param);
    		
    		if(as_info->param.as_handler!=NULL)
    			as_info->param.as_handler(ASM_TYPE_NIT_TP_CNT, tp_cnt);
    		return as_info->stop_scan_signal;
    	}

    	return as_update_tp((TP_INFO *)param);
    }
    else if (progress!=AS_PROGRESS_NIT_VER_CHANGE)
    {
        if (param)
        {
            P_NODE *p_node = (P_NODE *)param;
            
            pg_pos = get_prog_pos(p_node->prog_id);
            if(as_info->param.as_handler!=NULL)
    		{
    			as_info->param.as_handler(ASM_TYPE_ADD_PROGRAM, pg_pos);
    		}
        }

		if ((as_info->param.as_method == AS_METHOD_TP) && (as_info->param.as_handler != NULL))
		{
    		as_info->param.as_handler(ASM_TYPE_PROGRESS, progress);
		}
	}
	else if(progress==AS_PROGRESS_NIT_VER_CHANGE)
	{
		if(as_info->param.as_handler!=NULL)
			as_info->param.as_handler(ASM_TYPE_NIT_VER_CHANGE, *((UINT8 *)param));
		return as_info->stop_scan_signal;
	}

	return as_info->stop_scan_signal;
}

/* According to input as info and tp info, set anntenna and nim of frontend */
static UINT32 as_frontend_tuning(struct as_module_info *info, T_NODE *tp)
{
    UINT32 time_out = 0;
	union ft_xpond *xponder = &info->xponder;
	struct ft_antenna *antenna = &info->antenna;

	//Set TP Info for DVB-S/C/T
	MEMSET(xponder, 0, sizeof(union ft_xpond));

	if(tp->ft_type == FRONTEND_TYPE_C)
	{
    	time_out = AS_TUNE_TIMEOUT_C;
    	xponder->c_info.type = tp->ft_type;
    	xponder->c_info.tp_id = tp->tp_id;
    	xponder->c_info.frq = tp->frq;
    	xponder->c_info.sym = tp->sym;
	//FEC_inner store modulation in this case
    	xponder->c_info.modulation = tp->FEC_inner;
	}

	//Set NIM
	frontend_set_nim(as_info->nim, antenna, xponder, 1);

	return time_out;
}

/* Set as info(including tp id, timeout, and so on) for one specific tp */
static void as_dvbs_preset_tuning(void *param)
{
    struct as_tuning_param *atp;
    T_NODE *tp;
    atp = (struct as_tuning_param *)param;

    if (atp->finish == TRUE)
	{
    	return;
	}
    while ((INT32)atp->pos <= as_info->tp_nr)
    {
        tp = &as_info->xp;
        if (get_tp_by_id(as_info->tp_id[atp->pos], tp)!=SUCCESS)
    	{
        		break;
    	}
        atp->nit_enable = as_info->tp_nit[atp->pos];
        atp->si_enable = as_info->tp_si[atp->pos];
        atp->tp_id = as_info->tp_id[atp->pos++];
        atp->timeout = as_frontend_tuning(as_info, tp) + osal_get_tick();
        return;
    }
    atp->finish = TRUE;
}

/*DVBC blind scan in fact is tp scan by bandwith step*/
#ifdef DVBC_COUNTRY_BAND_SUPPORT
static UINT32 c_srh_tp_step = 0;
static UINT32 dvbc_bs_freq_end = 0;
static void as_dvbc_blind_tuning(void *param)
{
	struct as_tuning_param *atp = (struct as_tuning_param *)param;
	union ft_xpond xponder;

	UINT32 cnt_finish=0, cnt_band=0, cnt_ttl=0,index=0;

	//If Blind Scan Already Finished, then Return
	if(dvbc_bs_freq_end)
		atp->finish = TRUE;

	if (atp->finish == TRUE)
	{
		return;
	}

	MEMSET(&xponder, 0, sizeof(xponder));
	xponder.common.type = atp->ft_type = as_info->param.as_frontend_type;
	if(atp->ft_type==FRONTEND_TYPE_C)
	{
        atp->ft.c_param.sym = as_info->param.ft[atp->ft_count].c_param.sym;
        atp->ft.c_param.constellation = as_info->param.ft[atp->ft_count].c_param.constellation;

        xponder.c_info.frq = atp->pos;
        xponder.c_info.sym = atp->ft.c_param.sym;                   // as_info->param.ft.c_param.sym;
        xponder.c_info.modulation = atp->ft.c_param.constellation;	// as_info->param.ft.c_param.constellation;
	}

	//libc_printf("as_dvbc_blind_tuning frq:%d sym:%d modulation:%d timeout:%d\n",
	//	xponder.c_info.frq, xponder.c_info.sym,  xponder.c_info.modulation, atp->timeout);

	frontend_set_nim(as_info->nim, NULL, &xponder, 1);

	if(as_info->param.as_handler)
	{
		//as_info->param.as_handler(ASM_TYPE_SCANNING_INFO,1+c_srh_tp_step+((atp->cur_band&0x7f)<<8));

		for(index = 0; index < atp->band_count; index++)
		{
			cnt_band = 1+ atp->band_group_param[index].end_ch_no - atp->band_group_param[index].start_ch_no;

             //if(as_info->param.ft_count > 1)
             cnt_band *= as_info->param.ft_count;
			cnt_ttl += cnt_band;

			if(atp->cur_band>index)
				cnt_finish += cnt_band;
		}

         if(c_srh_tp_step != 0)
         {
            if(atp->ft_count == as_info->param.ft_count-1)
                 cnt_finish += c_srh_tp_step;
            else
                cnt_finish += (c_srh_tp_step - (as_info->param.ft_count-1 - atp->ft_count));
         }

		as_info->param.as_handler(ASM_TYPE_PROGRESS, 100*cnt_finish/cnt_ttl);
	}
	c_srh_tp_step++;

     if(as_info->param.ft_count > 1)
     {
        if(atp->cur_band < atp->band_count)
        {
            if(atp->ft_count < as_info->param.ft_count-1)
            {
                atp->ft_count++;
                //atp->ft.c_param.sym = as_info->param.ft[atp->ft_count].c_param.sym;
                //atp->ft.c_param.constellation = as_info->param.ft[atp->ft_count].c_param.constellation;
                atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;
                return;
            }
            else
            {
                atp->ft_count = 0;
               // atp->ft.c_param.sym = as_info->param.ft[0].c_param.sym;
               // atp->ft.c_param.constellation = as_info->param.ft[0].c_param.constellation;
            }
        }
    }

	while(atp->cur_band < atp->band_count)
	{
		if(atp->pos+atp->step <= atp->limit)
		{
			atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;
			atp->pos += atp->step;

			if(atp->start == atp->limit && atp->step==0) // only one frequency in this band
				atp->step = 1;
			return;
		}
		atp->cur_band++;
		if(atp->cur_band < atp->band_count)
		{
			atp->start = atp->band_group_param[atp->cur_band].start_freq;
			atp->limit = atp->band_group_param[atp->cur_band].end_freq;
			atp->step = atp->band_group_param[atp->cur_band].freq_step;
			atp->pos = atp->start- atp->step;
			c_srh_tp_step = 0;
		}
	}

	//if ((atp->pos < atp->limit) && (as_info->param.as_handler))
	//{
	//		as_info->param.as_handler(ASM_TYPE_PROGRESS, 99*(atp->pos-atp->start)/(atp->limit-atp->start));
	//}

	atp->timeout += AS_TUNE_TIMEOUT_C; // for the last frequency
	dvbc_bs_freq_end = 1;

}
#else
/*DVBC and DVBT blind scan in fact is tp scan by bandwith step*/
static void as_dvbc_blind_tuning(void *param)
{
    	struct as_tuning_param *atp = (struct as_tuning_param *)param;
    	union ft_xpond xpond;

	//If Blind Scan Already Finished, then Return
    	if (atp->finish == TRUE)
    	{
        	return;
    	}

    MEMSET(&xpond, 0, sizeof(xpond));
    xpond.common.type = atp->ft_type = as_info->param.as_frontend_type;
    if(atp->ft_type==FRONTEND_TYPE_C)
    {
        xpond.c_info.frq = atp->pos;
        xpond.c_info.sym = as_info->param.ft.c_param.sym;
        xpond.c_info.modulation = as_info->param.ft.c_param.constellation;
    }
     
    while(atp->pos <= atp->limit)
    {
        frontend_set_nim(as_info->nim, NULL, &xpond, 1);
	if((atp->ft_type==FRONTEND_TYPE_C) &&(as_info->param.as_handler))
	{
		as_info->param.as_handler(ASM_TYPE_INC_TRANSPONDER,atp->pos);
	}
        atp->timeout = osal_get_tick()+ AS_TUNE_TIMEOUT_C;

#ifdef JILIN_FEATURE
        if(atp->pos==71400)//skip the tp for HD program:722,730,738
        {
            atp->pos=74600;
            return;
        }
#endif

        atp->pos += atp->step;
        if(atp->ft_type==FRONTEND_TYPE_C)
        {
            if ((atp->pos>DVBC_FREQUENCY_VHF_HIGH)&&(atp->pos<DVBC_FREQUENCY_UHF_LOW))
            {
                		if (atp->limit >= DVBC_FREQUENCY_UHF_LOW)
                		{
                    			atp->pos = DVBC_FREQUENCY_UHF_LOW;
                		}
				else
				{
                    			return;
				}
            		}
        	}

        	if ((atp->pos < atp->limit) && (as_info->param.as_handler))
        	{
            		as_info->param.as_handler(ASM_TYPE_PROGRESS, 99*(atp->pos-atp->start)/(atp->limit-atp->start));
        	}
   
        return;
    }
    atp->finish = TRUE;
}
#endif

static UINT32 t_srh_tp_step = 0;
static UINT32 dvbt_bs_freq_end = 0;

//for tuner to select the nim device
static INT32 as_select_nim(S_NODE *snode)
{
    INT32 res = SUCCESS;
	UINT8 nim_id = 0;         //0 or 1

    if(snode->tuner1_valid == 1)
    {
		nim_id = 0;
        tsi_select(TSI_DMX_0, ts_route_get_nim_tsiid(nim_id));
        as_info->nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    }
    else if(snode->tuner2_valid == 1)
    {
		nim_id = 1;
    	tsi_select(TSI_DMX_0, ts_route_get_nim_tsiid(nim_id));
    	as_info->nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
    }
    else
    {
        nim_id = 0;
        as_info->nim = (struct nim_device *)dev_get_by_id(HLD_DEV_TYPE_NIM, nim_id);
        return res;
    }
    return res;

}

static INT32 as_enum_scan(UINT16 sat_id, UINT16 type, psi_tuning_t tuning_fn)
{
    BOOL db_ret = TRUE;
    INT32 ret = 0;
    INT32 func_ret = 0;
    UINT32 search_scheme;
    UINT8 lock_stat;
    struct as_tuning_param *atp = &as_info->atp;
    as_update_event_t update_event = as_info->param.as_handler;

    AS_PRINTF("start at %d\n", osal_get_tick());
    	
    	if(update_event != NULL)
    	{
        	update_event(ASM_TYPE_PROGRESS, AS_PROGRESS_SCANSTART+1);
    	}

	//Set First TP Info
    	if(tuning_fn != NULL)
    	{
        	tuning_fn(atp);
    	}

	//TP Scan Loop
    	while((atp->finish == FALSE) && (ret == 0))
    	{
    		//Set Search Scheme, that is whether Search NIT or not
        	search_scheme = as_info->param.as_prog_attr;
        	if (atp->nit_enable)
        	{            		
                     if (atp->si_enable)
                     {
                        search_scheme |= P_SEARCH_NIT;
                     }
                     else
                     {
                        search_scheme = P_SEARCH_NIT;
                     }
        	}

        	AS_PRINTF("id=%d, t_nr=%d\n",atp->tp_id, as_info->tp_nr);
        	lock_stat = 0;

		//Show New TP for DVB-S by Upper Layer
       	if ((tuning_fn == as_dvbs_preset_tuning) && (update_event))
        	{
            		update_event(ASM_TYPE_ADD_TRANSPONDER, atp->tp_id);
        	}

		//Try to Lock Signal
       	do
       	{
            		nim_get_lock(as_info->nim, &lock_stat);
            		if((lock_stat) || (as_info->stop_scan_signal))
            		{
                		break;
            		}
            		osal_task_sleep(1);
        	} while((INT32)osal_get_tick() < (INT32)atp->timeout);

        	AS_PRINTF("lock stat = %d\n", lock_stat);
        	if (!as_info->stop_scan_signal)
        	{
        		//If Lock Signal, then Search Prog in this TP
            		if (lock_stat)
            		{
                		//Scan from Start Frq to End Frq for DVB-C and DVB-T
                		if (type == ATP_TYPE_STEP)
                		{
                			//For DVB-C, Set TP Info according to as_info
                    			if(as_info->param.as_frontend_type == FRONTEND_TYPE_C)
                    			{
                        			db_ret = dvbc_alloc_transponder(as_info, atp, &as_info->xp, sat_id);
                    			}
								if (db_ret != TRUE)
                    			{
                        			ret = 1;
                        			break;
                    			}
					//Show New TP for DVB-C and DVB-T by Upper Layer
                    			if(update_event)
                    			{
                        			update_event(ASM_TYPE_ADD_TRANSPONDER, atp->tp_id);//notify ap show tp
                    			}
                		}

				//Prog Search in this TP
#if (defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
                		psi_module_init(as_info->dmx, as_info->nim, (void *)__MM_PSI_START_ADDR, 400*1024);
#else
                		psi_module_init(as_info->dmx, as_info->nim, (void *)__MM_PSI_START_ADDR, 180*1024);
#endif
				func_ret = psi_search_start(sat_id, atp->tp_id, search_scheme, as_info->param.as_p_add_cfg, psi_callback, tuning_fn, atp);
                		if (SI_STOFULL == func_ret)
                		{
                    			ret= 1;
                		}
            		}
			//Not Lock Signal, then Switch to next TP
            		else
            		{
						#ifdef NIT_AUTO_SEARCH_SUPPORT
						if(tuning_fn== as_dvbc_blind_tuning && b_found_nit_tp)
						{// because doing auto scan,so atp->pos must > AS_MIN_CAB_FREQUENCY and we need to change to nit search.
							if(atp->pos > AS_MIN_CAB_FREQUENCY)
								atp->pos = 0;
							tuning_fn = as_dvbs_preset_tuning;
						}
						#endif
                		tuning_fn(atp);
            		}
        	}
			
        	if (as_info->stop_scan_signal || ret)
        	{
            		break;
        	}

		//Show Scan Progress by Upper Layer
        	if ((!as_info->tp_nit[0]) && (type == ATP_TYPE_DBASE) && (update_event))
        	{
            		update_event(ASM_TYPE_PROGRESS, atp->pos*99/(as_info->tp_nr+2));
        	}
    }
#ifdef NIT_AUTO_SEARCH_SUPPORT		
	b_found_nit_tp = FALSE; // nit search done
#endif	
    AS_PRINTF("end at %d\n", osal_get_tick());
    return ret;
}


static INT32 as_scan(UINT16 sat_id, S_NODE *sat, INT32 method, BOOL cached)
{
    INT32 ret=0;
    UINT8 prog_mode;
   
	//Set Program Mode
	if((as_info->param.as_method == AS_METHOD_NVOD)
        	||(as_info->param.as_method == AS_METHOD_NVOD_MULTI_TP))
	{
    	prog_mode = PROG_NVOD_MODE;
	}
	else if(as_info->param.as_method == AS_METHOD_DATA)
	{
    	prog_mode = PROG_DATA_MODE;
	}
	else
	{
    	prog_mode = PROG_ALL_MODE;
	}

	//Create Program View
    db_search_create_pg_view(TYPE_SAT_NODE, sat_id, prog_mode);

	//Blind Scan
    if (AS_METHOD_FFT == method)
    {
		if(as_info->param.as_frontend_type == FRONTEND_TYPE_C)
        	{
#ifdef DVBC_COUNTRY_BAND_SUPPORT
			c_srh_tp_step = 0;
			dvbc_bs_freq_end = 0;
#endif
			if (dvbc_init_search_param(&as_info->atp, &as_info->param) == FALSE)
           // if (dvbc_init_search_param(&as_info->atp, as_info->param.as_from, as_info->param.as_to) == FALSE)
            		{
                		return -1;
            		}
            		ret = as_enum_scan(sat_id, ATP_TYPE_STEP, as_dvbc_blind_tuning);
        	}        
    }
    else
    {
    	MEMSET(&(as_info->atp), 0, sizeof(struct as_tuning_param));
    	ret = as_enum_scan(sat_id, ATP_TYPE_DBASE, as_dvbs_preset_tuning);
	}

	//Process by Upper Layer
	if ((1 == ret) && (as_info->param.as_handler))
	{
    	as_info->param.as_handler(ASM_TYPE_PROGRESS, AS_PROGRESS_STOFULL);
	}

    return ret;
}

/* description	: auto scan main routine for command parsing and switching.*/
static void as_main(struct libpub_cmd *cmd)
{
    INT32 i, j;
    INT32 ret;
    INT32 method;
    INT32 tp_nr;
    INT32 as_method;
    UINT16 sat_id;
    UINT8 dmx_mode_need_resume = 0;
    UINT32 dmx_dma_mode;
    as_update_event_t event_handler;
    struct dmx_device *dmx = NULL;
    T_NODE *tp = &as_info->cur_tp;;
    S_NODE *sat = &as_info->cur_sat;
    struct ft_antenna *antenna = &as_info->antenna;	
    
    AS_PRINTF("start time stamp = %d\n", osal_get_tick());
    as_method = as_info->param.as_method;
	
    event_handler = as_info->param.as_handler;
	if(event_handler!=NULL)
	{
    	event_handler(ASM_TYPE_PROGRESS, AS_PROGRESS_SCANSTART);
	}

	//Select DMX
	dmx = dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	if(dmx == NULL)
	{
        AS_PRINTF("%s(): select dmx failed!\n");
    	return;
	}
	as_info->dmx = dmx;

#if (defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
	psi_info_init(as_info->dmx, as_info->nim, (void *)__MM_PSI_START_ADDR, 400*1024);
#else
	//fix BUG20874: init psi info to avoid dmx changed by video playing
	psi_info_init(as_info->dmx, as_info->nim, (void *)__MM_PSI_START_ADDR, 180*1024);
#endif	
	//Check and Switch Buffer Mode of Dmx (From Divided Buffer To Union Buffer)
	dmx_io_control(as_info->dmx, DMX_CHK_BUF_MODE, (UINT32)&dmx_dma_mode);
	if(dmx_dma_mode == DMX_DIVIDED_BUFF_MODE)
	{
    	dmx_io_control(as_info->dmx, DMX_SWITCH_TO_UNI_BUF, 0);
    	dmx_mode_need_resume = 1;
	}
	
	for(i = 0; i<as_info->sat_nr; i++)
	{
    	//For Each Satellite, Create its TP View, Set Antenna
    	sat_id = as_info->sat_id[i];
    	db_search_create_tp_view(sat_id);
	
    	if(event_handler != NULL)
    	{
        	event_handler(ASM_TYPE_SATELLITE, sat_id);
    	}

	    //Set Antenna Information according to Satellite Information
    	MEMSET(sat, 0, sizeof(S_NODE));
    	get_sat_by_id(sat_id, sat);

    	AS_DUMP_SAT(sat);
    	MEMSET(antenna, 0, sizeof(struct ft_antenna));
    	//sat2antenna(sat, antenna);
	
    	as_info->dual_LNB = (antenna->lnb_high>0) && (antenna->lnb_low>0) && (antenna->lnb_high != antenna->lnb_low);
//        as_info->voltage_ctrl = (antenna->pol==LNB_POL_H)||(antenna->pol==LNB_POL_V);
        //>>> Unicable begin
        if(antenna->lnb_type == LNB_CTRL_UNICABLE)
        {
    	    as_info->nim_para.unicable = 1;
    	    as_info->nim_para.Fub = antenna->unicable_freq;
        } 
        else
    	    as_info->nim_para.unicable = 0; 
        //<<< Unicable end
        //select right nim and dmx    
        as_select_nim(sat);
        if(as_info->nim ==NULL)
        {
            AS_PRINTF("%s(): select nim failed!\n");
            continue;
        }

    	 //for 29E 2-Tuner the dmx path may not same as default!!
    	 tsi_dmx_src_select(TSI_DMX_0, TSI_TS_A);
    
        //blind scan
        if((AS_METHOD_FFT==as_method)||(AS_METHOD_MULTI_FFT==as_method))
        {
            method = AS_METHOD_FFT;
        }
        else
        {
            as_info->tp_nr = 0;
            MEMSET(as_info->tp_nit, 0,AS_MODULE_MAX_T_NUM*sizeof(UINT8));
            //single tp scan
            if((AS_METHOD_TP==as_method)||(AS_METHOD_TP_PID==as_method)
                ||(AS_METHOD_NIT_TP==as_method)||(AS_METHOD_NVOD==as_method)||(AS_METHOD_FREQ_BW==as_method)
                || (AS_METHOD_NIT_TP_EX==as_method))
            {
                as_info->tp_nr = 1;
				
                as_info->tp_id[0] = as_info->param.as_from;
                if ((AS_METHOD_NIT_TP==as_method || AS_METHOD_NIT_TP_EX==as_method)&&(event_handler!=NULL))
        		{
            		event_handler(ASM_TYPE_INC_TRANSPONDER, 0);
        		}
            }
            //multi tp scan
            else if((AS_METHOD_SAT_TP==as_method)||(AS_METHOD_NIT==as_method)
                ||(AS_METHOD_MULTI_TP==as_method)||(AS_METHOD_MULTI_NIT==as_method)
                ||(AS_METHOD_NVOD_MULTI_TP==as_method))
            {
                tp_nr = get_tp_num_sat(sat_id);
                for(j=0; j<tp_nr; j++)
                {
                    get_tp_at(sat_id, j, tp);
				if(as_info->param.as_frontend_type == FRONTEND_TYPE_C)
        		{
               			if(dvbc_tp_sanity_check(FALSE, tp, NULL) == FALSE)
               			{
                    			continue;
               			}
        		}
                    
                    AS_DUMP_TP(tp);
#ifdef NVOD_FEATURE
                    if((AS_METHOD_NVOD_MULTI_TP == as_method) && (tp->nvod_flg == 1))
#endif
					{
                    	as_info->tp_id[as_info->tp_nr++] = tp->tp_id;
					}

        			if (AS_MODULE_MAX_T_NUM == as_info->tp_nr)
        			{
            			AS_PRINTF("%s: tp exceed AS_MODULE_MAX_T_NUM!\n", __FUNCTION__);
        			}
        		}
            }

	        if((AS_METHOD_NIT==as_method)||(AS_METHOD_NIT_TP==as_method)
                ||(AS_METHOD_MULTI_NIT==as_method) || (AS_METHOD_NIT_TP_EX==as_method))
            {
                MEMSET(as_info->tp_nit, 1, as_info->tp_nr);
                if(AS_METHOD_NIT_TP_EX==as_method)
                {
                    MEMSET(as_info->tp_si, 0, as_info->tp_nr);
                }
                else
                {
                    MEMSET(as_info->tp_si, 1, as_info->tp_nr);
                }
            }
            	
            method = AS_METHOD_TP;
        }

        ret = as_scan(sat_id, sat, method, 0);
        
        update_data( );

    	//if (ret || as_info->stop_scan_signal)
    	if(as_info->stop_scan_signal)     //solve the bug:if the first nim is error,won't scan the second nim
    	{
        	break;
    	}
	}

    //Show Scan Over by Upper Layer
	if(event_handler != NULL)
	{
    	event_handler(ASM_TYPE_PROGRESS, AS_PROGRESS_SCANOVER);
	}

	dmx_io_control(as_info->dmx, CLEAR_STOP_GET_SECTION, 0);
    //Switch Buffer Mode of Dmx (From Union Buffer to Divided Buffer)
	if(dmx_mode_need_resume)
	{
    	dmx_io_control(as_info->dmx, DMX_SWITCH_TO_DVI_BUF, 0);
	}
	sie_open(as_info->dmx, MAX_FILTER_NUMBER, NULL, 0);
     
    osal_task_dispatch_off();
    nim_io_control(as_info->nim, NIM_DRIVER_STOP_ATUOSCAN, 0);
    as_info = NULL;
    AS_PRINTF("end time stamp = %d\n", osal_get_tick());
    osal_task_dispatch_on();
}


/*description	: register auto-scan service task, but not run it.*/
INT32 as_service_register()
{
    libpub_cmd_handler_register(LIBPUB_CMD_AS, as_main);
    return SUCCESS;
}


/*description	: start an auto-scan according to the parameter asp.*/
INT32 as_service_start(struct as_service_param *asp)
{
    struct libpub_cmd cmd;

    if (as_info != NULL)
        return 0;

#ifndef __MM_VBV_START_ADDR
#if (DATA_BROADCAST==DATA_BROADCAST_IPANEL)
#else
	if(NULL == as_buf)
	{
		as_buf = MALLOC(AS_BUF_LEN);
	}
#endif    
#endif	
		
    as_info = (struct as_module_info *)__MM_AS_START_ADDR;
    MEMSET(as_info, 0, sizeof(struct as_module_info));
    MEMCPY(&as_info->param, asp, sizeof(struct as_service_param));
    as_info->param.as_prog_attr |= P_SEARCH_SATTP;
    as_info->sat_nr = asp->as_sat_cnt;

	if(as_info->sat_nr > AS_MODULE_MAX_S_NUM)
		as_info->sat_nr = AS_MODULE_MAX_S_NUM;

    MEMCPY(as_info->sat_id, asp->sat_ids,as_info->sat_nr*sizeof(UINT16));
 
    cmd.cmd_type = LIBPUB_CMD_AS_START;	
    return libpub_send_cmd(&cmd, OSAL_WAIT_FOREVER_TIME);
	
}

/* API for Stop Auto Scan */
INT32 as_service_stop()
{
    	struct nim_device *nim = NULL;
    
    	osal_task_dispatch_off();
    	if (as_info != NULL)
    	{
        	as_info->stop_scan_signal = 1;
		if(as_info->nim!=NULL)
		{
			nim = as_info->nim;
       	 	nim_io_control(nim, NIM_DRIVER_STOP_ATUOSCAN, 1);
		}
    	}
    	osal_task_dispatch_on();

    	if (as_info != NULL)
    	{
        	psi_search_stop();//stop getting psi section
    	}

    	while (as_info)
        	osal_task_sleep(AS_MODULE_SLEEP_SLICE);

#ifndef __MM_VBV_START_ADDR
#if (DATA_BROADCAST==DATA_BROADCAST_IPANEL)
#else
		if(NULL != as_buf)
		{
			FREE(as_buf);
			as_buf = NULL;
		}
#endif        
#endif

    	return SUCCESS;
}


/*description	: return ERR_BUSY when in scan mode.*/
INT32 as_service_query_stat()
{
    return (NULL == as_info)? SUCCESS: ERR_BUSY;
}


/*search for special program that not listed in PAT,PMT,SDT,
* input the video,audio,pcr pids, dmx will check if there is probably such a prog in current TS. 
*/
BOOL as_prog_pid_search(struct dmx_device *dmx,UINT8 av_mode,
UINT16 v_pid, UINT16 a_pid,UINT16 pcr_pid)
{
    UINT16 pid_list[3];
    struct io_param io_param;
    BOOL ret = FALSE;
   
    if(av_mode != 1)
    	{
    		return FALSE;
    	}
    
    	if(dmx == NULL)
    	{
    		return FALSE;
    	}
		
    	pid_list[0] = v_pid;
    	pid_list[1] = a_pid;
    	pid_list[2] = pcr_pid;
    	io_param.io_buff_in = (UINT8 *)pid_list;	
		io_param.buff_in_len = sizeof(pid_list);
    	if(dmx_io_control(dmx, IO_CREATE_AV_STREAM, (UINT32)&io_param) == RET_SUCCESS)
    	{
        	if(dmx_io_control(dmx, IO_STREAM_ENABLE, (UINT32)&io_param) == RET_SUCCESS)
        	{
            		UINT32 tmo = 6;
            		while(tmo)
            		{
                		osal_task_sleep(100);
                		tmo--;
                		if(dmx_io_control(dmx, IS_PROGRAM_LEGAL, (UINT32)&io_param) == RET_SUCCESS)
                		{
                    			ret = TRUE;
                    			break;
                		}
            		}
        	}
    	}

    dmx_io_control(dmx, IO_STREAM_DISABLE, (UINT32)&io_param);

    return ret;
}


