#include <types.h>
#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libsi/si_config.h>
#include <api/libtsi/db_3l.h>

#include <api/libtsi/p_search.h>
#include <api/libtsi/psi_db.h>
#include <api/libsi/si_service_type.h>

#define PD_DEBUG_LEVEL			0
#if (PD_DEBUG_LEVEL>1)
#include <api/libc/printf.h>
#define PD_PRINTF			libc_printf
#elif (PD_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define PD_PRINTF			PRINTF
#else
#define PD_PRINTF(...)			do{}while(0)
#endif

static prog_node_return prog_callback =NULL;

/*******************************************************************************
*   Function name: get_usr_lang_idx() & set_usr_lang_idx()&translate_idx_to_service_lang()
*   description:
*           1. provide to UI to tell MDL which language user select.
*******************************************************************************/
static INT8 usr_lang_idx=0;
static UINT8 service_lang[3][3] ={{'e','n','g'}, {'g','l','a'}, {'w','e','l'}};
INT8 get_usr_lang_idx()
{
    return (INT8)usr_lang_idx;
}
INT8 set_usr_lang_idx(INT8 idx)
{
    usr_lang_idx = idx;
    return (INT8)SI_SUCCESS;
}
UINT8* translate_idx_to_service_lang(INT8 usr_lang_idx)
{
    if(usr_lang_idx<0 || usr_lang_idx>2)
    {
        usr_lang_idx = 0;
    }
    return service_lang[usr_lang_idx];
}

/*******************************************************************************
*   Function name: find_usr_sel_audio_com()
*   Description:
*       1. find the apropriate audio compent according usr_sel_service_language.
*       2. return audio index.
********************************************************************************/ 
UINT32 find_usr_sel_audio_com(PROG_INFO *p_prog, INT8 usr_lang_sel)
{
    INT32 i=0,j=0;
    UINT32 cur_audio=0;
    if(usr_lang_sel<0 || usr_lang_sel>2)
    {
        usr_lang_sel = 0;
    }
    if(p_prog->audio_count > 1)
    {
        INT32 k=0; 
        UINT32 record[P_MAX_AUDIO_NUM];
        MEMSET(record, 0, P_MAX_AUDIO_NUM*4);
        UINT32 default_sel = 0xFFFFFFFF;
        //UINT8 lang[3][3] ={{'e','n','g'}, {'g','l','a'}, {'w','e','l'}};
        for(j=0; j<p_prog->audio_count; j++)
        {
                record[k++] = (UINT32)j;
        }
        if(k >0)
        {
            for(j=0;j<k;j++)
            {
                if(MEMCMP(p_prog->audio_lang[record[j]], translate_idx_to_service_lang(usr_lang_sel), 3)==0)
                {
                    break;
                }
                else if(default_sel == 0xFFFFFFFF)
                {
                    if( (MEMCMP(p_prog->audio_lang[record[j]], "und", 3)!=0)
                        && (MEMCMP(p_prog->audio_lang[record[j]], "UND", 3)!=0))
                    {
                        default_sel = record[j];
                    }
                }
            }
            if(j==k)
            {
                if(default_sel!=0xFFFFFFFF)
                {
                    cur_audio = default_sel;
                }
                else
                {
                    cur_audio = record[0];
                }
            }
            else
            {
                cur_audio = record[j];
            }
        }
        else
        {
            cur_audio = 0;
        }
    }
    else
    {
        cur_audio = 0;
    }
    return cur_audio;
}

INT32 prog_callback_register(prog_node_return callback)
{
	if(callback != NULL)
	{
		prog_callback = callback;
		return SI_SUCCESS;
	}
	else
		return -1;
}

INT32 prog_callback_unregister( )
{
	prog_callback = NULL;
	return SI_SUCCESS;
}

/*
 * name		: pd_pg2db
 * description	: convert PSI program information into database node.
 * parameter	: 2
 * @pg		: the database node.
 * @info	: the PSI program information.
 * return value	: INT32
 * @SI_SUCCESS
 */
INT32 psi_pg2db(P_NODE *pg, PROG_INFO *info)
{
	INT32 i;
	MEMSET(pg,0,sizeof(P_NODE));

	pg->audio_volume = AUDIO_DEFAULT_VOLUME;

#ifdef DEFAULT_AUDIO_CHANNEL
	// Set the default channel, by Jie Wu, 2005.12.1
	pg->audio_channel = DEFAULT_AUDIO_CHANNEL;
#endif
	pg->audio_count = info->audio_count;

	MEMCPY(pg->audio_pid, info->audio_pid, info->audio_count*sizeof(info->audio_pid[0]));
	
//#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
//	MEMCPY(pg->audio_type, info->audio_type, info->audio_count*sizeof(info->audio_type[0]));
//#endif
//#if (defined(_ISDBT_ENABLE_))
//	MEMCPY(pg->audio_com_tag, info->audio_com_tag, info->audio_count*sizeof(info->audio_com_tag[0]));
//#endif

//#ifdef _SERVICE_ATTRIBUTE_ENABLE_
//	pg->visible_flag = info->visible_service_flag;
//	pg->num_sel_flag = info->numeric_selection_flag;
//#endif

	for(i=0;i<(INT32)info->audio_count;i++) 
	{
		get_audio_lang2b(info->audio_lang[i], (UINT8 *)&pg->audio_lang[i]);
	}	
	pg->av_flag = info->av_flag;

	/* check CA descriptor */
	if(info->CA_count >0) {
		pg->ca_mode = 1;
	}

#ifdef DB_CAS_SUPPORT
	//cas count
	pg->cas_count = (info->CA_count < MAX_CAS_CNT)?info->CA_count : MAX_CAS_CNT;
	for(i = 0; i < pg->cas_count; i++)
		pg->cas_sysid[i] = info->CA_info[i].CA_system_id;

#endif	

 	pg->pmt_pid = info->pmt_pid;

//#if (SYS_PROJECT_FE == PROJECT_FE_DVBT && defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
//	pg->preset_flag = 1;
//	pg->pmt_version = info->pmt_version;
//#endif

	pg->pcr_pid = info->pcr_pid;
	pg->prog_number = info->prog_number;
	
	//if no service descriptor in sdt
	if(info->service_type != 0)
	{
		//error detect
		if((info->av_flag==1) && (info->service_type == SERVICE_TYPE_DRADIO))
			pg->service_type = SERVICE_TYPE_DTV;
		else if((info->av_flag==0) && (info->service_type == SERVICE_TYPE_DTV))
			pg->service_type = SERVICE_TYPE_DRADIO;
		else
			pg->service_type = info->service_type;
	}
	else
		pg->service_type = (info->av_flag==1) ? SERVICE_TYPE_DTV : SERVICE_TYPE_DRADIO;
			
		
	pg->teletext_pid = info->teletext_pid;
	pg->subtitle_pid = info->subtitle_pid;
	pg->sat_id = info->sat_id;
	pg->tp_id = info->tp_id;
//#if (SYS_PROJECT_FE == PROJECT_FE_DVBS)
//    pg->h264_flag = ((info->video_pid & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG) || (info->avc_video_exist == 1);
//    pg->mpeg4_flag = info->mpeg_4_video_exist;
//#else
    pg->h264_flag = ((info->video_pid & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG);
//#endif
	pg->video_pid = info->video_pid&0x1fff;
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#ifndef DEFAULT_AUDIO_CHANNEL

	pg->audio_channel = info->track;
#endif
	if(info->volume != 0)
		pg->audio_volume = info->volume;
#ifdef MULTI_BOUQUET_ID_SUPPORT
    pg->bouquet_count = 1;
	pg->bouquet_id[0] = info->bouquet_id;
#else
    pg->bouquet_id = info->bouquet_id;
#endif    
	pg->logical_channel_num = info->logical_channel_num;

	pg->nvod_sid = info->ref_sid;
#endif

/*
#ifdef RAPS_SUPPORT
	if( prog_callback == NULL)//raps tp won't use this function
	{
      	pg->Internal_number = raps_get_max_internalnumber();
        raps_set_max_internalnumber(pg->Internal_number+1);
	}
#endif
*/

	DB_STRCPY((DB_ETYPE *)pg->service_name, 
		(DB_ETYPE *)info->service_name);
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
      pg->mtl_name_count = info->mtl_name_count;
      for(i=0;i<info->mtl_name_count;i++)
      	{
		get_audio_lang2b(info->mtl_iso639_code[i], (UINT8 *)&pg->mtl_iso639_code[i]);
      	}

	for(i=0;i<info->mtl_name_count;i++)
      	{
		DB_STRCPY((DB_ETYPE *)&pg->mtl_service_name[i], 
			(DB_ETYPE *)&info->mtl_service_name[i]);
      	}
#endif	
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	DB_STRCPY((DB_ETYPE *)pg->service_provider_name, 
		(DB_ETYPE *)info->service_provider_name);
#endif

#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))

	if(info->lcn_true == FALSE)
	{
		pg->LCN = find_max_LCN_next()-1;//find_max_LCN()+1;
		pg->orig_LCN = INVALID_LCN_NUM;
	}
	else
	{
		pg->orig_LCN = pg->LCN = info->lcn;
	}
/*
#ifdef SUPPORT_FRANCE_HD
	if(info->hd_lcn_true == TRUE)
	{
		pg->hd_lcn = info->hd_lcn;
		pg->hd_lcn_ture=info->hd_lcn_true ;
	}
#endif
*/

	pg->mheg5_exist = info->mheg5_exist;
	pg->LCN_true = info->lcn_true;
//#if (SYS_PROJECT_FE == PROJECT_FE_DVBT) || (SYS_PROJECT_FE == PROJECT_FE_ISDBT)
//	pg->default_index = find_max_default_index()+1;
//#endif
#endif


//#ifdef LEGACY_HOST_SUPPORT
//	pg->reserve_2= info->hdplus_flag;
//#endif

	return SI_SUCCESS;
}


/*
 * name		: psi_monitor_pg2db
 * description	: convert PSI program information into database node for si_monitor.
 * parameter	: 2
 * @pg		: the database node.
 * @info	: the PSI program information.
 * return value	: INT32
 * @SI_SUCCESS
 */
INT32 psi_monitor_pg2db(P_NODE *pg, PROG_INFO *info)
{
	INT32 i;
	MEMSET(pg,0,sizeof(P_NODE));

	pg->audio_volume = AUDIO_DEFAULT_VOLUME;

#ifdef DEFAULT_AUDIO_CHANNEL
	// Set the default channel, by Jie Wu, 2005.12.1
	pg->audio_channel = DEFAULT_AUDIO_CHANNEL;
#endif
	pg->audio_count = info->audio_count;

	MEMCPY(pg->audio_pid, info->audio_pid, info->audio_count*sizeof(info->audio_pid[0]));
	
//#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
//	MEMCPY(pg->audio_type, info->audio_type, info->audio_count*sizeof(info->audio_type[0]));
//#endif
//#if (defined(_ISDBT_ENABLE_))
//	MEMCPY(pg->audio_com_tag, info->audio_com_tag, info->audio_count*sizeof(info->audio_com_tag[0]));
//#endif

/*
#ifdef _SERVICE_ATTRIBUTE_ENABLE_
        pg->visible_flag = info->visible_service_flag;
        pg->num_sel_flag = info->numeric_selection_flag;
#endif
*/

	for(i=0;i<(INT32)info->audio_count;i++) {
		get_audio_lang2b(info->audio_lang[i], (UINT8 *)&pg->audio_lang[i]);
	}	
	pg->av_flag = info->av_flag;

	/* check CA descriptor */
	if(info->CA_count >0) {
		pg->ca_mode = 1;
	}

#ifdef DB_CAS_SUPPORT
		//cas count
		pg->cas_count = (info->CA_count < MAX_CAS_CNT)?info->CA_count : MAX_CAS_CNT;
		for(i = 0; i < pg->cas_count; i++)
			pg->cas_sysid[i] = info->CA_info[i].CA_system_id;
		
#endif	

 	pg->pmt_pid = info->pmt_pid;
	pg->pcr_pid = info->pcr_pid;
	pg->prog_number = info->prog_number;
	pg->service_type = info->service_type;
		
	pg->teletext_pid = info->teletext_pid;
	pg->subtitle_pid = info->subtitle_pid;
	pg->sat_id = info->sat_id;
	pg->tp_id = info->tp_id;
    pg->h264_flag = ((info->video_pid & H264_VIDEO_PID_FLAG) == H264_VIDEO_PID_FLAG);
	pg->video_pid = info->video_pid&0x1fff;
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)

	pg->audio_channel = info->track;
	if(info->volume != 0)
		pg->audio_volume = info->volume;
#ifdef MULTI_BOUQUET_ID_SUPPORT
    pg->bouquet_count = 1;
    pg->bouquet_id[0] = info->bouquet_id;
#else
	pg->bouquet_id = info->bouquet_id;
#endif    
	pg->logical_channel_num = info->logical_channel_num;
#ifdef _LCN_ENABLE_
	pg->LCN=info->lcn;
#endif
	pg->nvod_sid = info->ref_sid;
#endif

	DB_STRCPY((DB_ETYPE *)pg->service_name, 
		(DB_ETYPE *)info->service_name);
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	DB_STRCPY((DB_ETYPE *)pg->service_provider_name, 
		(DB_ETYPE *)info->service_provider_name);
#endif

#if 0
//#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_) ||defined(_MHEG5_V20_ENABLE_))

	if(info->lcn_true == FALSE)
	{
		pg->LCN = find_max_LCN_next()-1;//find_max_LCN()+1;
		pg->orig_LCN = INVALID_LCN_NUM;
	}
	else
	{
		pg->orig_LCN = pg->LCN = info->lcn;
	}

	pg->mheg5_exist = info->mheg5_exist;
	pg->LCN_true = info->lcn_true;
#if (SYS_PROJECT_FE == PROJECT_FE_DVBT)
	pg->default_index = find_max_default_index()+1;
#endif
#endif
	return SI_SUCCESS;
}

/*
 * name		: update_pids
 * description	: copy all PID information from source database node to destination.
 * parameter	: 2
 * @dest	: the target database node.
 * @src		: the source database node.
 * return value	: INT32
 * @SI_SUCCESS
 */
INT32 psi_update_pids(P_NODE *dest, P_NODE *src)
{
	INT32 size;
	
	dest->pmt_pid = src->pmt_pid;
	dest->prog_number = src->prog_number;

	dest->video_pid = src->video_pid;
	dest->pcr_pid = src->pcr_pid;

	dest->teletext_pid = src->teletext_pid;
	dest->subtitle_pid = src->subtitle_pid;
	dest->ca_mode = src->ca_mode;
	dest->audio_count = src->audio_count;

//#if (defined(DTG_AUDIO_SELECT) || defined(AUDIO_DESCRIPTION_SUPPORT))
//	MEMCPY(dest->audio_type, src->audio_type, src->audio_count*sizeof(src->audio_type[0]));
//#endif

    dest->av_flag= src->av_flag;
    if(src->service_type != 0)
	    dest->service_type = src->service_type;
	else
		dest->service_type = (src->av_flag==1) ? SERVICE_TYPE_DTV : SERVICE_TYPE_DRADIO;
	
	size = src->audio_count*sizeof(src->audio_pid[0]);
	MEMCPY(dest->audio_pid, src->audio_pid, size);
	MEMCPY(dest->audio_lang, src->audio_lang, size);
//#if (defined(_ISDBT_ENABLE_))
//	size = src->audio_count*sizeof(src->audio_com_tag[0]);
//	MEMCPY(dest->audio_com_tag, src->audio_com_tag, size);
//#endif
	
	return SI_SUCCESS;
}

/*
 * name		: update_name
 * description	: copy all name information from source database node to destination.
 * parameter	: 2
 * @dest	: the target database node.
 * @src		: the source database node.
 * return value : INT32
 * @SI_SUCCESS.
 */
INT32 psi_update_name(P_NODE *dest, P_NODE *src)
{
	DB_STRCPY((DB_ETYPE *)dest->service_name, (DB_ETYPE *)src->service_name);
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	DB_STRCPY((DB_ETYPE *)dest->service_provider_name, 
		(DB_ETYPE *)src->service_provider_name);
#endif
	return SI_SUCCESS;
}

#if (SERVICE_PROVIDER_NAME_OPTION>0)
static const UINT8 default_name[]="No Name";
#endif

void psi_fake_name(UINT8 *name, UINT16 program_number)
{
	extern int sprintf (char *s, const char *format, ...);
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	if (program_number) {
#endif		
		sprintf(name+1, "Ch-%d", (int)program_number);
		name[0] = STRLEN(name+1);
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	} else {
		MEMCPY(name+1, default_name, sizeof(default_name));
		name[0] = sizeof(default_name)-1;
	}
#endif
}

static void psi_adjust_node(UINT8 *string_buf, UINT32 pos, 
	P_NODE *program, UINT16 storage_scheme)
{
	P_NODE *db_prog = program+1;
	P_NODE *db_bak = program+2;
	
	if (PROG_ADD_DIRECTLY == storage_scheme) {
		/* nothing to be done here, 
		 * actually, this function is not expecting to be enter here!
		 */
		PD_PRINTF("%s: please check up your function flow!\n", 
			__FUNCTION__);
		return;
	} 
	get_prog_at(pos, db_prog);
	MEMCPY(db_bak, db_prog, sizeof(P_NODE));
	psi_update_pids(db_prog, program);
	//use prog's new name replace old name in database
	if (PROG_ADD_REPLACE_OLD == storage_scheme) {
			psi_update_name(db_prog, program);
		
	} 
	//the preset prog's name keep unchanged
	else if (PROG_ADD_PRESET_REPLACE_NEW == storage_scheme) {
		if (!db_prog->preset_flag) {
			psi_update_name(db_prog, program);
		}
	} 
	//?what does this mean
	else if (PROG_ADD_REPLACE_NONPRESET == storage_scheme) {
		psi_fake_name(string_buf, db_prog->prog_number);
		
		if (DB_STRCMP((DB_ETYPE *)string_buf, 
			(DB_ETYPE *)db_prog->service_name)) {
			if (!db_prog->preset_flag) {
				psi_update_name(db_prog, program);
			}
		}
	} else {
		PD_PRINTF("%s: storage_scheme=0x%02x not supported!\n", 
			__FUNCTION__, storage_scheme);
	}

    db_prog->h264_flag = program->h264_flag;
	
//#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
//	db_prog->preset_flag = 1;
//	db_prog->pmt_version = program->pmt_version;
//#endif

	MEMCPY(program, db_prog, sizeof(P_NODE));	
	if (MEMCMP(db_prog, db_bak, sizeof(P_NODE))) {
#if DB_VERSION < 40
		modify_prog(pos, db_prog);
#else
		modify_prog(db_prog->prog_id, db_prog);
#endif
	}
}
//add the flag for dispaly highlight or not. 1: exist,0: new node. use the lowest bit. an ugly but safely 
//method, maybe one day will improve it.
static UINT32 prog_flg;
UINT32 get_prog_flg()
{
	return prog_flg;
}
INT32 psi_install_prog(UINT8 *string_buf, PROG_INFO *p_info, 
	P_NODE *p_node, UINT32 search_scheme,UINT32 storage_scheme )
{
	UINT16 pos = INVALID_POS_NUM;
	INT32 ret;
	BOOL result = FALSE;

	psi_pg2db(p_node, p_info);
#ifdef SID_AS_LCN
	if(INVALID_LCN_NUM==p_node->orig_LCN||0==p_node->LCN)
	{
		p_node->LCN=p_node->prog_number;
	}
#endif
	prog_flg = 0;
#if DB_VERSION < 40
	pos = (PROG_ADD_DIRECTLY==storage_scheme)? 
		INVALID_POS_NUM : get_prog_pos(p_node);
#else
	if(PROG_ADD_DIRECTLY==storage_scheme)
		pos = INVALID_POS_NUM;
	else if(db_search_lookup_node(TYPE_PROG_NODE, p_node)==SUCCESS)
	{
		prog_flg = 1;
		pos = get_prog_pos(p_node->prog_id);
	}

#endif
	if( prog_callback != NULL)
	{
		result = prog_callback(p_node);
		if( !result)
			return SI_PERROR;
	}
	if( prog_callback == NULL ||result)
		{

	if (INVALID_POS_NUM == pos) {

//#ifndef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
//		p_node->pmt_version = 0;
//#endif

#if DB_VERSION < 40
		if (STORE_SPACE_FULL == add_node(TYPE_PROG_NODE, p_node)) 
#else
		//#if(defined(UNVISIBLE_PROG_NOT_STORAGE))
		//if(p_node->visible_flag)
		//#endif
		ret = add_node(TYPE_PROG_NODE, p_node->tp_id,p_node);
		if ((DBERR_FLASH_FULL==ret) || (DBERR_MAX_LIMIT)==ret) 
#endif
		{
			return SI_STOFULL;
		}
	} else {
		if ((p_info->CA_count)
			&&(0 == (search_scheme&P_SEARCH_SCRAMBLED))
			&&(0 == p_node->preset_flag)) {
			/* new CA program, not preset program. */
			return SI_PERROR;
		}
		psi_adjust_node(string_buf, pos, p_node, storage_scheme);
		
//#ifndef STAR_PROG_SEARCH_NIT_SDT_SUPPORT		
//		p_node->pmt_version = 1;
//#endif

	}

		}

	return SI_SUCCESS;
}



