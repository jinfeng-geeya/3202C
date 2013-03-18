/*
 * Copyright (C) ALi Shanghai Corp. 2006
 *
 * Description
 *	Search according to PSI.
 *
 * History
 *	20060117	Zhengdao Li	0.01	Port to Dolphin.
 *	20060123	Zhengdao Li	0.02	-S/-C/-T unified.
 *	20060209	Zhengdao Li	0.03	Bug fix for STO full handling.
 *	20060315	Zhengdao Li	0.04	bug fix for NIT search dumphead.
 *	20060421	Zhengdao Li	0.05	remove CI_NODE as it is dynamic now.
 *	20060612	Zhengdao Li	0.06	full scan of PMT program number.
 *	20060707	Zhengdao Li	0.07	full scan of multiple sections.
 *	20110622	Steven				clean code for DVB-C 3202/C only.
 */
 
#include <types.h>
#include <sys_config.h>
#include <retcode.h>

#include <osal/osal.h>

#include <hld/hld_dev.h>
#include <hld/dmx/dmx_dev.h>
#include <hld/dmx/dmx.h>

#include <hld/nim/nim_dev.h>
#include <hld/nim/nim.h>

#include <api/libc/list.h>

#include <api/libc/string.h>
#include <api/libchar/lib_char.h>

#include <api/libtsi/db_3l.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/sie.h>
#include <api/libtsi/psi_search.h>
#include <api/libtsi/psi_db.h>

#include <api/libsi/si_module.h>

#include <api/libsi/si_service_type.h>
#include <api/libsi/si_desc_service_list.h>

#include <api/libsi/psi_pat.h>
#include <api/libsi/psi_pmt.h>
#include <api/libsi/si_sdt.h>
#include <api/libsi/si_nit.h>
#include <api/libsi/si_utility.h>


#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#include <api/libsi/lib_nvod.h>
#include <api/libsi/si_desc_cab_delivery.h>
#ifndef PSI_NVOD_SUPPORT
#define PSI_NOVD_SUPPORT
#endif
#include <api/libsi/si_bat.h>
#include <api/libsi/si_eit.h>
#include <api/libpub/lib_as.h>

#endif
#if(MW_MODULE == MW_AVIT)
#include <udi/avit/j_lib.h>
#endif

#define PSI_DEBUG_LEVEL				0
#if (PSI_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
//#include <api/libtsi/dvb_si_code.h>
#define PSI_PRINTF				libc_printf
#else
#define PSI_PRINTF(...)				do{}while(0)
#endif
#if (PSI_DEBUG_LEVEL>1)
#define PSI_INFO				libc_printf
#else
#define PSI_INFO(...)				do{}while(0)
#endif


#define PSI_INVALID_EXT_ID	0xFFFF


//max table count when search program(NIT/PAT/SDT/BAT), except EIT etc.
#ifdef PSI_SEACH_BAT_SUPPORT
#define PSI_TABLE_COUNT	4
#define PSI_SEARCH_BAT_EXT_NUMBER	PSI_MODULE_MAX_BAT_EXT_NUMBER
#else
#define PSI_TABLE_COUNT	3
#define PSI_SEARCH_BAT_EXT_NUMBER	0
#endif


struct psi_module_info {
	struct dmx_device *dmx;
//	struct nim_device *nim;

	UINT16 search_scheme;
//	UINT16 storage_scheme;
	
	UINT16 valid_counter;	//pmt
	UINT16 pmt_counter;
	UINT16 nit_counter;
	UINT16 flag256_nr;

	T_NODE xponder[2];	//0 from DB, 1 from PSI/SI
	P_NODE program[3];	//see psi_adjust_node(), when save program, need compare with DB P_NODE

	struct si_filter_param fparam;

	struct pat_section_info pat;
	struct sdt_section_info sdt;
#ifdef PSI_SEACH_BAT_SUPPORT
	struct bat_section_info bat;
#endif
	struct nit_section_info nit;
	
	struct section_parameter param[PSI_TABLE_COUNT];
	struct program_map maps[PSI_MODULE_MAX_PROGRAM_NUMBER];
	struct list_head roots[PSI_MODULE_MAX_PROGRAM_NUMBER];
	UINT8 flag256[PSI_TABLE_COUNT+PSI_SEARCH_SDT_OTH_COUNT+PSI_SEARCH_BAT_EXT_NUMBER][PSI_MODULE_MSECTION_NUMBER>>3];

//	struct pmt_es_info es_buff[PSI_MODULE_COMPONENT_NUMBER];
	UINT8 buffer[PSI_SHORT_SECTION_LENGTH*PSI_MODULE_CONCURRENT_FILTER];

};

typedef void(* psi_pre_fn_t)(struct psi_module_info *info,
	struct section_parameter *sparam);

typedef INT32(* psi_post_fn_t)(struct psi_module_info *info, 
	UINT32 search_scheme, psi_event_t on_event, BOOL *need_tune);

static struct psi_module_info *psi_info;
static OSAL_ID psi_flag = OSAL_INVALID_ID;

static const struct section_info pmt_info = {
	.pid		= PSI_STUFF_PID,
	.table_id	= PSI_PMT_TABLE_ID,
	.table_mask 	= 0xFF,
	.flag		= PSI_FLAG_STAT_SECTION,
	.timeout	= PSI_MODULE_PMT_TIMEOUT,
	.buflen		= PSI_SHORT_SECTION_LENGTH,
	.parser		= NULL,
};

RET_CODE psi_service_type_exist(UINT8 service_type)
{
	UINT32 i;
	UINT8 defined_service_type[] =
	{
		SERVICE_TYPE_DTV,
		SERVICE_TYPE_DRADIO,
		SERVICE_TYPE_TELTEXT,
		SERVICE_TYPE_NVOD_REF,
		SERVICE_TYPE_NVOD_TS,
		SERVICE_TYPE_MOSAIC,
		SERVICE_TYPE_FMRADIO,
       SERVICE_TYPE_DATABROAD,
       SERVICE_TYPE_SD_MPEG4,
       SERVICE_TYPE_HD_MPEG4,
       SERVICE_TYPE_DATA_SERVICE,
	};

	for(i=0;i<sizeof(defined_service_type);i++)
	{
		if(service_type==defined_service_type[i])
			break;
	}
	if(i<sizeof(defined_service_type))
	{
		return RET_SUCCESS;
	}
	else
	{
		PSI_PRINTF("%s: service type %d not supported!\n", __FUNCTION__,service_type);
		return !RET_SUCCESS;
	}
}

INT32 psi_module_init(struct dmx_device *dmx, struct nim_device *nim, 
	void *work_place, INT32 length)
{
	PSI_PRINTF("%s()\n", __FUNCTION__);

	if (psi_flag ==OSAL_INVALID_ID) {
		psi_flag = osal_flag_create(PSI_FLAG_STAT_MUTEX);
		if (psi_flag == OSAL_INVALID_ID) {
			PSI_PRINTF("%s: psi_flag create failure!\n");
			return ERR_FAILUE;
		}
	} else {
		osal_flag_clear(psi_flag, 0xFFFFFFFF);
		osal_flag_set(psi_flag, PSI_FLAG_STAT_MUTEX);
	}
	
	if ((work_place==NULL)
		||(length<(INT32)sizeof(struct psi_module_info))) {
		PSI_PRINTF("%s: work_place parameter invalid!\n", __FUNCTION__);
		return ERR_FAILUE;
	}
	
	psi_info = (struct psi_module_info *)work_place;
	MEMSET(psi_info, 0, sizeof(struct psi_module_info));
	PSI_PRINTF("%s: dmx - 0x%X, nim - 0x%X\n", __FUNCTION__,dmx,nim);
	PSI_PRINTF("%s: work_place addr - 0x%X, length - %d\n", __FUNCTION__,work_place,length);
	PSI_PRINTF("%s: psi module size = %d!\n", __FUNCTION__,sizeof(struct psi_module_info));

	psi_info->dmx = dmx;
//	psi_info->nim = nim;

	return SUCCESS;
}

INT32 psi_info_init(struct dmx_device *dmx, struct nim_device *nim, 
	void *work_place, INT32 length)
{	
	PSI_PRINTF("%s()\n", __FUNCTION__);

	if ((work_place==NULL)
		||(length<(INT32)sizeof(struct psi_module_info))) {
		PSI_PRINTF("%s: work_place parameter invalid!\n", __FUNCTION__);
		return ERR_FAILUE;
	}
	psi_info = (struct psi_module_info *)work_place;
	MEMSET(psi_info, 0, sizeof(struct psi_module_info));

	psi_info->dmx = dmx;
//	psi_info->nim = nim;
	
	return SUCCESS;
}

static INT32 psi_wait_stat(UINT32 good, UINT32 clear)
{
	INT32 ret = SI_SUCCESS;
#define ABNORMAL_STAT	(PSI_FLAG_STAT_ABORT|PSI_FLAG_STAT_STOFULL|PSI_FLAG_STAT_RELEASE)
	UINT32 flag;
	osal_flag_wait(&flag, psi_flag, good|ABNORMAL_STAT, OSAL_TWF_ORW, 
		OSAL_WAIT_FOREVER_TIME);

	if (flag&(PSI_FLAG_STAT_ABORT|PSI_FLAG_STAT_RELEASE)) {
		ret = SI_UBREAK;
	} else if (flag&PSI_FLAG_STAT_STOFULL) {
		ret = SI_STOFULL;
	} else if (flag&clear) {
		osal_flag_clear(psi_flag, flag&clear);
	}
	return ret;
}

static __inline__ INT32 psi_poll_stat(UINT32 stat)
{
	UINT32 flag;
	return (osal_flag_wait(&flag, psi_flag, stat, OSAL_TWF_ORW, 0)==OSAL_E_OK)?
		SI_SUCCESS : SI_SBREAK;
}

static __inline__ void psi_convert_string(DB_ETYPE *dest, UINT8 *src)
{
    UINT32 refcode=0;
	
//#ifdef DB_USE_UNICODE_STRING
#ifdef DB_USE_UNICODE_STRING
#ifdef GB2312_SUPPORT
	refcode = 2312;//GB2312 coding
#endif

#ifdef CHINESE_SUPPORT
	refcode = 0x15; //UTF-8
#endif
	dvb_to_unicode(src+1, *src, dest, MAX_SERVICE_NAME_LENGTH, refcode);
#else
	dvb_string_to_ascii(dest, src, MAX_SERVICE_NAME_LENGTH);
#endif
}

static INT32 psi_store_program(PROG_INFO *program, 
	UINT8 *service_name, UINT8 *provider_name, UINT32 progress, 
	UINT16 search_scheme, UINT16 storage_scheme, psi_event_t on_event)
{
	INT32 i, ret;
	P_NODE *node = psi_info->program;
	UINT32 program_number = 0;

	MEMSET(node, 0, sizeof(P_NODE));

	program_number = program->prog_number;

	ret=psi_get_service(&psi_info->sdt, program_number, service_name, 
		provider_name);

#ifdef USE_SDT_SERVICE_NAME
    if(ret==SI_SKIP)
    {
        PSI_PRINTF("%s This service has no name in sdt,programe num:%d",__FUNCTION__,program_number);
        return SI_SUCCESS;
    }
#endif

	PSI_PRINTF("%s(): prog_number[%d], name=%.*s, provider=%.*s\n",__FUNCTION__,program_number,
		*service_name, service_name+1, *provider_name,provider_name+1);
	
	psi_convert_string((DB_ETYPE *)program->service_name, service_name);
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	psi_convert_string((DB_ETYPE *)program->service_provider_name, 
		provider_name);
#endif

#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
	struct mtl_service_data *  pData =   psi_get_multi_service_name(&psi_info->sdt, program_number);
	
	for(i=0; pData && i<pData->mult_name_count; i++)
	{
		dvb_to_unicode(pData->mtl_name[i].service_name, pData->mtl_name[i].service_name_length, (UINT16*)&program->mtl_service_name[i][0], MAX_SERVICE_NAME_LENGTH, 0);
		MEMCPY(&program->mtl_iso639_code[i][0], pData->mtl_name[i].ISO_639_language_code, 3);			
	}
	if(pData)
		program->mtl_name_count=pData->mult_name_count;
#endif

#ifdef SERVICE_TYPE_FROM_SDT
	psi_get_service_type(&psi_info->sdt, program_number, &program->service_type);
#endif

	ret = psi_install_prog(psi_info->sdt.service_name, program, node, 
		search_scheme, storage_scheme);

	if (SI_STOFULL == ret) {
		return SI_STOFULL;
	} else if (SI_SUCCESS != ret) {
		node = NULL;
	}
	
	if (on_event) {
		ret = on_event(progress, node);
	}

	return ret;
}

static struct program_map *psi_tree_lookup(struct si_filter_t *sfilter, 
	UINT16 program_number)
{
	struct program_map *map;
	struct list_head *ptr, *ptn;
	ptr = (struct list_head *)sfilter->priv[0];
	list_for_each(ptn, ptr) {
		map = list_entry(ptn, struct program_map, pm_list);
		if (map->pm_number == program_number) {
			return map;
		}
	}
	return NULL;
}

static __inline__ BOOL psi_remove_map(struct list_head *ptr)
{
	BOOL ret = (ptr->next != ptr->prev);
	if (ret ==TRUE)
		list_del(ptr);
	return ret;
}

static UINT16 psi_inspect_pmt(struct list_head *ptr, UINT16 *cnt)
{
	struct list_head *ptn;
	struct program_map *map=NULL;
	UINT16 counter = 0;

	list_for_each(ptn, ptr) {
		map = list_entry(ptn, struct program_map, pm_list);
		++counter;
	}

	*cnt = counter;

	if (map != NULL) {
		return map->pm_pid;
	} else {
		PSI_PRINTF("the root is empty!!!\n");
		return 0;
	}
}

static sie_status_t psi_on_pmt_filter_close(UINT16 pid, 
	struct si_filter_t *filter, UINT8 reason, UINT8 *buffer, INT32 length)
{
	struct list_head *ptr, *ptn;
	struct program_map *map;
	ptr = (struct list_head *)(filter->priv[0]);
	list_for_each(ptn, ptr) {
		map = list_entry(ptn, struct program_map, pm_list);
		if (!map->pm_stat) {
			osal_task_dispatch_off();
			++psi_info->pmt_counter;
			osal_task_dispatch_on();
			PSI_PRINTF("+");
			map->pm_stat = 1;
		}
	}
	osal_flag_set(psi_flag, PSI_FLAG_STAT_SECTION);
	if (psi_info->pmt_counter==psi_info->pat.map_counter) {
		PSI_PRINTF("*");
		osal_flag_set(psi_flag, PSI_FLAG_STAT_PMTALL);
	} else {
		PSI_PRINTF("%s(): wait for %d PMT tables, tick=%d.\n", __FUNCTION__,
			psi_info->pat.map_counter - psi_info->pmt_counter, osal_get_tick());
	}

	return sie_freed;
}

static sie_status_t psi_analyze_pmt(UINT16 pid, UINT16 program_number, 
	struct program_map *map, UINT8 *buffer, INT32 length)
{
	if (psi_poll_stat(PSI_FLAG_STAT_ABORT|PSI_FLAG_STAT_STOFULL)==SI_SUCCESS) {
		PSI_PRINTF("psi_analyze_pmt: abort!\n");
		return sie_freed;
	} else {
		PROG_INFO *program;
		PSI_INFO("\n%s(): PMT[0x%x], prognumber[%d],",__FUNCTION__,pid, program_number);
#if 0
		DUMP_SECTION(buffer, length, 1);
#endif	
		program = &map->pm_info;
		program->pmt_pid = pid; 
		program->prog_number = program_number;
		
		program->teletext_pid = program->subtitle_pid = PSI_STUFF_PID;
		
		if (psi_pmt_parser(buffer, program,
			PSI_MODULE_COMPONENT_NUMBER) == SUCCESS) {
			PSI_PRINTF("PCR[%d]\n", program->pcr_pid);
			++psi_info->valid_counter;
			program->pmt_status = 1;
		} else {
			PSI_PRINTF("	prog parsed INcorrectly!\n");
			program->pmt_status = 2;
		}

		return psi_remove_map(&map->pm_list)==TRUE? 
			sie_started: sie_freed;
	}
}

static BOOL psi_on_pmt_section_hit(UINT16 pid, struct si_filter_t *filter,
	UINT8 reason, UINT8 *buffer, INT32 length)
{
	struct program_map *map;
	sie_status_t stat;
	UINT16 program_number; 
	
	program_number = (buffer[3]<<8)|buffer[4];
	map = psi_tree_lookup(filter, program_number);

	if (map == NULL) {
		PSI_PRINTF("prog_number %d not found!\n", program_number);
		return FALSE;
	}

	PSI_PRINTF("\n%s(): PMT pid[0x%x], prognumber[%d] hit!\n", __FUNCTION__,pid,program_number);
	stat = psi_analyze_pmt(pid, program_number, map, buffer, length);
	map->pm_stat = 2;

	osal_task_dispatch_off();
	++psi_info->pmt_counter;
	osal_task_dispatch_on();
	
	return (stat == sie_started)? FALSE: TRUE;
}

static sie_status_t si_msection_filter_close(UINT16 pid, 
	struct si_filter_t *filter, UINT8 reason, UINT8 *buffer, INT32 length)
{
	struct section_parameter *param;
	INT32 i;
	struct si_filter_param fparam;

	sie_copy_config(filter, &fparam);

	for(i=0; i<fparam.mask_value.value_num; i++) {
		param = (struct section_parameter *)filter->priv[i];
		//PSI_PRINTF("\n%s(): pid=0x%x, time=%d\n", __FUNCTION__,pid, osal_get_tick());
		osal_flag_set(psi_flag, param->info->flag);
	}
	
	return sie_freed;
}

static BOOL si_msection_event(UINT16 pid, struct si_filter_t *filter,
	UINT8 reason, UINT8 *buffer, INT32 length)
{
	INT32 i, idx;
	INT32 ret = SI_SUCCESS;
	BOOL need_reset;
	BOOL ret_val;
	struct section_parameter *param;
	struct section_header *sh = (struct section_header *)buffer;
	struct si_filter_param fparam;
	struct extension_info *ext;
	UINT8 section_number = sh->section_number;
	UINT8 last_section_number = sh->last_section_number;
	UINT16 ext_id = SI_MERGE_UINT16(sh->specific_id);

	PSI_PRINTF("\n%s(): PID[0x%x] table_id[0x%x] extension_id[0x%x] sectin[%d] HIT, last_sec_num=%d!\n", 
		__FUNCTION__, pid, buffer[0],ext_id, buffer[6], buffer[7]);

	reason = reason&0x03;
	PSI_PRINTF("hit reason: %d\n", reason);
	param = (struct section_parameter *)filter->priv[reason];
	
	idx = 0;
	if (param->ext_nr) {
		PSI_PRINTF("ext nr: %d\n", param->ext_nr);
		for(i=0, ext=NULL; i<param->ext_nr; i++) {
			if (param->ext[i].id == ext_id || param->ext[i].id == PSI_INVALID_EXT_ID) {
			//if (param->ext[i].id == ext_id) {
				PSI_PRINTF("	extension id 0x%x found/alloc -> ext[%d]!\n", ext_id,i);
				param->ext[i].id = ext_id;
				ext = param->ext+i;
				idx = i;
				break;
			}
		}
		if (i==param->ext_nr) {
			PSI_PRINTF("	extension id 0x%x not found!\n", ext_id);
			return FALSE;
		}
	} else {
		idx = 0;
		ext = param->ext;
	}
	//this section first hit, need set table sections hit flag bitmap
	if (0 == ext->hit) 
	{
		ext->version = sh->version_number;
		ext->last_section_number = last_section_number;
		need_reset = TRUE;
	} 
	//this section not first hit, but table version changed,need reget the table sections 
	else if (sh->version_number != ext->version) 
	{
		PSI_INFO("	version change!\n");
		param->parser(NULL, idx, param);
		ext->hit = 0;
		if (--param->retry_cnt==0) {
			return TRUE;
		}
		need_reset = TRUE;
	} 
	//this section not first hit, not need reset
	else
	{
		need_reset = FALSE;
	}
	//set table sections hit flag bitmap
	if (TRUE == need_reset)
	{
		MEMSET(ext->flag, 0xFF, (last_section_number>>3)+1);
	}
	//this section not the one wanted, return false
	else if (!(ext->flag[section_number>>3]&(1<<(section_number&7))))
	{
		PSI_INFO("section exist!\n");

		/*if (param->ext_nr)
		{
			ext->hit_times++;
			if (buffer[0] == 0x46 && ext->hit_times >= 3)	//第3次重复收到同样的section，则返回TRUE
			{
				PSI_INFO("section exist for 3 times!\n");
				return TRUE;
			}
		}*/
		return FALSE;
	}

	ext->flag[section_number>>3] &= ~(1<<(section_number&7));
	ret = param->parser(buffer, length, param);

	if (ret != SI_SUCCESS) {
		PSI_INFO("parser return value %d != SUCCESS!\n",ret);
		if (ret == 2) {
			PSI_INFO("parser return value %d = 2!\n",ret);
			osal_flag_set(psi_flag, PSI_FLAG_STAT_STOFULL);
		}
		return TRUE;
	}
	//check if get all sections of the table
	if  (ext->hit++==last_section_number)
	{
		ret_val = TRUE;
		//check if each extion get all sections
		for(i=0; i<param->ext_nr; i++)
		{
			if (param->ext[i].hit>param->ext[i].last_section_number)
				continue;

			ret_val = FALSE;
			break;
		}
	} 
	//else need continu to get the left sections
	else 
	{
		ret_val = FALSE;
	}
	//the sections for value[reasion] is completed
	if (ret_val == TRUE)
	{
		sie_copy_config(filter, &fparam);
		if (fparam.mask_value.value_num == 1)
		{
			ret_val = TRUE;
		}
		//this filter has multi values
		else
		{
			PSI_INFO("mask value num = %d\n",fparam.mask_value.value_num);
			--fparam.mask_value.value_num;
			i = fparam.mask_value.value_num-reason;
			if (i>0)
			{
				MEMMOVE(fparam.mask_value.value[reason],fparam.mask_value.value[reason+1],
					MAX_SEC_MASK_LEN*i);
				MEMMOVE(&filter->priv[reason], &filter->priv[reason+1],sizeof(void *)*i);
			}
			sie_config_filter(filter, &fparam);
			osal_flag_set(psi_flag, param->info->flag);
			ret_val =  FALSE;
		}
	}
	return ret_val;
}

static void psi_init_sr(struct si_filter_param *fparam, 
	const struct section_info *info, struct section_parameter *param, 
	si_parser_t parser, INT32 idx)
{
	fparam->attr[idx] = SI_ATTR_HAVE_CRC32|SI_ATTR_HAVE_HEADER;

	PSI_INFO("%s: idx = %d\n",__FUNCTION__,idx);
	if (idx == 0) {
		fparam->timeout = info->timeout;
		fparam->mask_value.mask[0] = info->table_mask;
		#ifndef DVBC_FP
		fparam->mask_value.mask[1] = 0x80;
		#endif
		fparam->mask_value.mask[5] = 0x01;
		fparam->mask_value.mask_len = 6;
	} else if (info->timeout>fparam->timeout) {
		fparam->timeout = info->timeout;
	}

	fparam->mask_value.value[idx][0] = info->table_id;
	#ifndef DVBC_FP
	fparam->mask_value.value[idx][1] = 0x80;
	#endif
	fparam->mask_value.value[idx][5] = 0x01;
	
	if (param != NULL) {
		param->parser = parser;
		param->retry_cnt = PSI_MODULE_RETRY_CNT;
		param->info = (struct section_info *)info;
		if (idx == 0) {
			fparam->section_event = si_msection_event;
			fparam->section_parser = si_msection_filter_close;
		}
	}
}

static __inline__ INT16 psi_make_tree(struct program_map *map, INT16 map_nr, 
	struct list_head *roots)
{
	INT16 i, k, n;

	PSI_INFO("%s: map nr = %d\n",__FUNCTION__,map_nr);
	for(i=map_nr-1, n=0; i>=0; i--) {
		for(k=i-1; k>=0; k--) {
			if (map[k].pm_pid == map[i].pm_pid) {
				break;
			}
		}
		if (k<0) {
			list_add(&roots[n++], &map[i].pm_list);
		} else {
			list_add(&map[k].pm_list, &map[i].pm_list);
		}
	}

	return n;
}

static void psi_prepare_pat(struct psi_module_info *info,
	struct section_parameter *sparam)
{
	struct pat_section_info *psi = &info->pat;
	sparam->priv = psi;
	sparam->ext = psi->ext;
	
	psi->map = info->maps;
	psi->max_map_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
	psi->ts_id = info->xponder[0].t_s_id;	
	psi->ext[0].flag = info->flag256[info->flag256_nr++];
}

static void psi_prepare_sdt(struct psi_module_info *info,
	struct section_parameter *sparam)
{
	int i;
	struct sdt_section_info *ssi = &info->sdt;
	sparam->priv = ssi;
	sparam->ext = ssi->ext;

	ssi->max_nd_nr = ssi->max_sd_nr = PSI_MODULE_MAX_PROGRAM_NUMBER;
	ssi->max_name_len = PSI_MODULE_NAME_SIZE;
	ssi->onid = info->xponder[0].network_id;	

	//if need support other ts SDT:
#ifdef PSI_SEARCH_SDT_OTH_SUPPORT
	if ((psi_info->search_scheme&P_SEARCH_SDT_OTH))
	{
		sparam->ext_nr = MAX_HITMAP_FLAGS_COUNT - PSI_TABLE_COUNT + 1;		//SDT actual(ts0) + SDT other(ts1,ts2...)
		for (i=0;i<sparam->ext_nr;i++)
		{
			ssi->ext[i].flag = info->flag256[info->flag256_nr++];
			ssi->ext[i].id = PSI_INVALID_EXT_ID;	//以ext_id(ts_id)区分
			//ssi->ext[i].hit_times = 0;
		}
	}
	else
#endif
	{
		ssi->ext[0].flag = info->flag256[info->flag256_nr++];
	}
}

#ifdef PSI_SEACH_BAT_SUPPORT
static void psi_prepare_bat(struct psi_module_info *info,
	struct section_parameter *sparam)
{
	int i;
	UINT16 b_id[PSI_MODULE_MAX_BAT_EXT_NUMBER];
	struct bat_section_info *bsi = &info->bat;
	sparam->priv = bsi;
	sparam->ext = bsi->ext;

	sparam->ext_nr = si_porting_get_bouquet_id(b_id, PSI_MODULE_MAX_BAT_EXT_NUMBER);
//	sparam->ext_nr = PSI_MODULE_MAX_BAT_EXT_NUMBER;
	for (i=0;i<sparam->ext_nr;i++)
	{
		bsi->ext[i].flag = info->flag256[info->flag256_nr++];
		//bsi->ext[i].id = PSI_INVALID_EXT_ID;
		bsi->ext[i].id = b_id[i];
		PSI_INFO("	CSTM bouquet id[%d]: 0x%X\n",i,b_id[i]);
	}
//	bsi->ext[0].flag = info->flag256[info->flag256_nr++];
}
#endif

static struct desc_table nit_loop2[] = {
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
	{
		.tag = SERVICE_LIST_DESCRIPTOR,
		.bit = 0,
		.parser = nit_on_service_list_desc,
	},
	{
		.tag = CABLE_DELIVERY_SYSTEM_DESCRIPTOR,
		.bit = 0,
		.parser = si_on_cab_delivery_desc,
	},
#ifdef BASE_TP_HALF_SCAN_SUPPORT
#ifdef DVBC_FP
	{
		.tag = 0x83,
		.bit = 0,
		.parser = nit_on_logical_channel_desc,
	},
#else
	{
		.tag = 0x82,
		.bit = 0,
		.parser = nit_on_logical_channel_desc,
	},
#endif
#endif
#endif
};

static void psi_prepare_nit(struct psi_module_info *info,
	struct section_parameter *sparam)
{
	struct nit_section_info *nsi= &info->nit;

	sparam->priv = nsi;
	sparam->ext = nsi->ext;
	
	nsi->ext[0].flag = psi_info->flag256[psi_info->flag256_nr++];
	
	nsi->lp1_nr = 0;

	nsi->lp2_nr = ARRAY_SIZE(nit_loop2);
	nsi->loop2 = nit_loop2;
}

/* 
 * Format the PMT relation tree, all section with same PID shall
 * be lined up into a linked-list.
 */
static INT32 psi_prepare_pmt(struct psi_module_info *info,
	struct section_parameter *sparam)
{
	INT16 i, n;
	UINT16 pid, cnt;
	INT32 ret_val, ret;
	
	struct list_head *ptr;
	struct si_filter_t *sfilter;
	struct si_filter_param *fp = &info->fparam;
	#ifdef SI_SUPPORT_MUTI_DMX	
        struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
        #endif

	
	psi_init_sr(fp, &pmt_info, NULL, NULL, 0);
	
	fp->section_event = psi_on_pmt_section_hit;
	fp->section_parser = psi_on_pmt_filter_close;
	
	for(i=0; i<PSI_MODULE_MAX_PROGRAM_NUMBER; i++) {
		INIT_LIST_HEAD(&info->maps[i].pm_list);
	}
	//link programs have same pmt pid, return the number of such links
	n = psi_make_tree(info->maps, info->pat.map_counter, info->roots);

	PSI_PRINTF("\n%s(): PMT total pg cnt=%d, pid cnt=%d\n", __FUNCTION__,info->pat.map_counter, n);
	for(i=n-1; i>=0; i--) {
		PSI_PRINTF("	pmt[%d]", i);
		ptr = &info->roots[i];
		pid = psi_inspect_pmt(ptr, &cnt);
		if(pid == 0 || pid >= 8191)// Invalid pmt pid
		{
			PSI_PRINTF("\n	invalid pmt pid!\n");
			n--;
			continue;
		}
		do {
			PSI_PRINTF("alloc.., ");
			#ifdef SI_SUPPORT_MUTI_DMX
			sfilter = sie_alloc_filter_ext(dmx,pid, NULL, 
				PSI_SHORT_SECTION_LENGTH,
				PSI_SHORT_SECTION_LENGTH,0);
			#else
			sfilter = sie_alloc_filter(pid, NULL, 
				PSI_SHORT_SECTION_LENGTH,
				PSI_SHORT_SECTION_LENGTH);
			#endif
			//PSI_PRINTF("flt addr[0x%x], ", sfilter);
			if (sfilter==NULL) {
                if (pid < 0x20) // Invalid pmt pid
                {
					PSI_PRINTF("\n	invalid pmt pid 0x%X!\n",pid);
                    n--;
                    break;
                }
				osal_flag_clear(psi_flag, 
					PSI_FLAG_STAT_SECTION);
				ret = psi_wait_stat(PSI_FLAG_STAT_SECTION, 
					PSI_FLAG_STAT_SECTION);
				if (ret != SI_SUCCESS) {
					PSI_PRINTF("	break\n");
					return ERR_FAILUE;
				}
				PSI_PRINTF("##but alloc failed!\n");
			} else {
				sfilter->priv[0] = (void *)ptr;
				PSI_PRINTF("pid [0x%x]\n", pid);
				fp->timeout = (cnt==1)? 
					PSI_MODULE_PMT_TIMEOUT : 
					PSI_MODULE_PMT_TIMEOUT_MAX;
				fp->mask_value.value_num = 1;
				sie_config_filter(sfilter, fp);
				ret_val = sie_enable_filter(sfilter);
				if (ret_val !=SI_SUCCESS) {
					PSI_PRINTF("	!!!enable flt failed!\n");
					sfilter = NULL;
					osal_task_sleep(20);
				}
			}
		}while(sfilter == NULL);
	}

    if (n <= 0) // None valid pmt pid
    {
		PSI_PRINTF("\n	none invalid pmt pid!\n");
		osal_flag_set(psi_flag, 
			PSI_FLAG_STAT_PMTALL);
        return ERR_FAILUE;
    }

	PSI_PRINTF("%s(): all pmt requested!tick=%d\n", __FUNCTION__,osal_get_tick());
	return SUCCESS;
}

static INT32 psi_on_pat(struct psi_module_info *info, UINT32 search_scheme,
	psi_event_t on_event, BOOL *need_tune)
{
	UINT8 FEC;
	INT32 ret;
	ret = psi_wait_stat(PSI_FLAG_STAT_PAT, 0);

	if ((ret!=SI_SUCCESS)||(psi_info->pat.map_counter==0)) {
		if (ret==SI_SUCCESS) {
			*need_tune = TRUE;
			ret = SI_SKIP;
		}
		return ret;
	} 

        if (search_scheme == P_SEARCH_NIT)
        {
        }
        else
	    psi_prepare_pmt(info, NULL);

	psi_info->xponder[1].t_s_id = psi_info->pat.ts_id;
	return SI_SUCCESS;
}

static INT32 psi_on_pmt(struct psi_module_info *info, UINT32 search_scheme,
	psi_event_t on_event, BOOL *need_tune)
{
	INT32 ret;
	ret = psi_wait_stat(PSI_FLAG_STAT_PMTALL, PSI_FLAG_STAT_PMTALL);
	if ((ret != SI_SUCCESS)||(info->valid_counter==0)) {
		if (ret==SI_SUCCESS) {
			*need_tune = TRUE;
			ret = SI_SKIP;
		}
	} else {
		PSI_PRINTF("\n%s(): all pmt retrieved!tick=%d.\n", __FUNCTION__,osal_get_tick());
	}
	return ret;
}

static INT32 psi_on_nit(struct psi_module_info *info, UINT32 search_scheme,
	psi_event_t on_event, BOOL *need_tune)
{
	INT32 i, ret;

	struct nit_section_info *nsi;
	if ((search_scheme&(P_SEARCH_NIT))==0) {
		return SI_SUCCESS;
	}

	ret = psi_wait_stat(PSI_FLAG_STAT_NIT, PSI_FLAG_STAT_NIT);
	if (ret != SI_SUCCESS) {
		return ret;
	}

	PSI_PRINTF("\n%s(): \n", __FUNCTION__);
	
	nsi = &info->nit;

	/*send nit version to lib as*/
	on_event(AS_PROGRESS_NIT_VER_CHANGE, &nsi->ext[0].version);
	
	//progress=0xf0 means nit tp count 
	ret = on_event(0xF0, &nsi->xp_nr);
	
	for(i=0; i<nsi->xp_nr; i++)
	{
		if (ret != SI_SUCCESS)
			break;
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
		INT32 j;
		if (search_scheme&P_SEARCH_NVOD)
		{
			for(j=0; j<nsi->nvod_nr; j++)
			{
				if (nsi->xp[i].common.tsid == nsi->nvod_tsid[j]) 
				{
					PSI_PRINTF("adding TP: %d, %d\n",
						nsi->xp[i].c_info.frequency,
						nsi->xp[i].c_info.symbol_rate);
					ret = on_event(0xFF, &nsi->xp[i]);
					break;
				}
			}
			
		} 
		else 
#endif
		{
			ret = on_event(0xFF, &nsi->xp[i]);
			PSI_PRINTF("%s: add xp[%d] %d-%d-%d\n", __FUNCTION__,i,
				nsi->xp[i].c_info.frequency,nsi->xp[i].c_info.symbol_rate,nsi->xp[i].c_info.modulation);
		}
	} 

	return ret;
}

static INT32 psi_on_sdt(struct psi_module_info *info, UINT32 search_scheme,
	psi_event_t event, BOOL *need_tune)
{
	INT32 i, j, k;
	INT32 cnt;
	UINT8 flag;
	INT32 ret;
	ret = psi_wait_stat(PSI_FLAG_STAT_SDT, PSI_FLAG_STAT_SDT);

	if (ret != SI_SUCCESS)
	{
		PSI_PRINTF("\n%s(): flag wait(%d) != SUCCESS\n", __FUNCTION__,ret);
	}

	//on pmt before on sdt, so can merge PMT/SDT here
	for(i=0; i<info->pmt_counter; i++)
	{
		if (info->maps[i].pm_stat)
		{
			for(j=0;j<info->sdt.sd_nr; j++)
			{
				if(info->maps[i].pm_number==info->sdt.sd[j].program_number)
				{
                #ifdef MANUAL_DEFINE_SERVICE_TYPE
                    info->maps[i].pm_info.service_type = service_type_mapper(info->sdt.sd[j].service_type);
                #else
					if(RET_SUCCESS==psi_service_type_exist(info->sdt.sd[j].service_type))
					{
	   					info->maps[i].pm_info.service_type = info->sdt.sd[j].service_type;
					}
                #endif
					
				#ifdef NVOD_FEATURE	
					//service private info in sdt
					info->maps[i].pm_info.volume = info->sdt.private[j].volumn_set;
					info->maps[i].pm_info.track = info->sdt.private[j].audio_track;
					if(info->sdt.private[j].logic_chan_number !=0)
						info->maps[i].pm_info.logical_channel_num = info->sdt.private[j].logic_chan_number;
                                /*some service has time_shift flag, but not in nvod_reference
						descriptor, need to check such service*/
					if(info->sdt.sd[j].timeshift_flag==1)
					{
						info->maps[i].pm_info.service_type = SERVICE_TYPE_NVOD_TS;
						info->maps[i].pm_info.ref_sid = info->sdt.sd[j].ref_service_id;
					}
				#endif
					break;
				}
			}
		#ifdef NVOD_FEATURE			
			for(k=0; k<info->sdt.nvod.scnt; k++)
			{
				if (info->sdt.nvod.sid[k].sid==info->maps[i].pm_number)
				{
					info->maps[i].pm_info.service_type = SERVICE_TYPE_NVOD_TS;
					info->maps[i].pm_info.ref_sid = info->sdt.nvod.sid[k].ref_id;
					//save nvod service in normal search
					//if(search_scheme&P_SEARCH_NVOD)
					{
					        
						info->maps[i].pm_info.sat_id = info->xponder[1].sat_id;
						info->maps[i].pm_info.tp_id = info->xponder[1].tp_id;
						nvod_save_program(&info->maps[i].pm_info, &info->xponder[1]);
					
					}
					
					break;
				}
			}
		#endif
		}
	}
#ifdef NVOD_FEATURE				
	//if this tp has nvod info, set nvod_flag
	if((info->sdt.nvod.ref_cnt > 0) && (info->sdt.nvod.scnt > 0))
	{
		psi_info->xponder[1].nvod_flg = 1;
        //if use new libnvod, open following function
		//nvod_add_sdt_info(&info->xponder[1],&info->sdt.nvod);
	}
	if(search_scheme&P_SEARCH_NVOD)
	{
		if((info->sdt.nvod.ref_cnt > 0) && (info->sdt.nvod.scnt > 0))
		{
            nvod_scan_start(&info->xponder[1],&info->sdt.nvod);
		}
	}
#endif

	if (ret == SI_SUCCESS)
	{
		psi_info->xponder[1].network_id = psi_info->sdt.onid;
		psi_info->xponder[1].t_s_id = psi_info->sdt.t_s_id;
		PSI_PRINTF("\n%s(): org net id 0x%X, ts id 0x%X\n", __FUNCTION__,psi_info->sdt.onid,psi_info->sdt.t_s_id);
#ifdef DYNAMIC_SERVICE_SUPPORT
		psi_info->xponder[1].sdt_version = psi_info->sdt.sdt_version;
#endif
		if (MEMCMP(psi_info->xponder, psi_info->xponder+1, sizeof(T_NODE)) != 0)
		{
			PSI_PRINTF("\n%s(): xponder changed!\n", __FUNCTION__);
			modify_tp(psi_info->xponder[1].tp_id, psi_info->xponder+1);
			MEMCPY(psi_info->xponder, psi_info->xponder+1, sizeof(T_NODE));
		}
	}
	
	return ret;
}

#ifdef PSI_SEACH_BAT_SUPPORT
static INT32 psi_on_bat(struct psi_module_info *info, UINT32 search_scheme,
	psi_event_t event, BOOL *need_tune)
{
	INT32 ret;
	ret = psi_wait_stat(PSI_FLAG_STAT_BAT, PSI_FLAG_STAT_BAT);

	return SI_SUCCESS;
}
#endif

#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
static INT32 psi_on_eit(struct psi_module_info *info, UINT32 search_scheme,
	psi_event_t on_event, BOOL *need_tune)
{
	if ((search_scheme&P_SEARCH_NVOD)&&(info->sdt.nvod.ref_cnt)) {
		return psi_wait_stat(PSI_FLAG_STAT_EIT_ASCH, PSI_FLAG_STAT_EIT_ASCH);
	}
	return SI_SUCCESS;
}

void psi_nvod_eit_complete()
{
	osal_flag_set(psi_flag, PSI_FLAG_STAT_EIT_ASCH);
}
#endif

static const psi_pre_fn_t pre_fn_array[] = {
	psi_prepare_nit,
	psi_prepare_pat,
	psi_prepare_sdt,
#ifdef PSI_SEACH_BAT_SUPPORT
	psi_prepare_bat,
#endif
};

static const psi_post_fn_t post_fn_array[] = {
	psi_on_pat,
	psi_on_nit,
	psi_on_pmt, //on pmt shall before on sdt!
	psi_on_sdt,
#ifdef PSI_SEACH_BAT_SUPPORT
	psi_on_bat,
#endif
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)	
	psi_on_eit,
#endif
};

const struct section_info psi_sections[PSI_TABLE_COUNT] = {
	{
		.pid		= PSI_NIT_PID,
		.table_id	= PSI_NIT_TABLE_ID,
		.table_mask	= 0xFF,
		.flag		= PSI_FLAG_STAT_NIT,
		.buflen		= PSI_SHORT_SECTION_LENGTH,
		.timeout	= PSI_MODULE_NIT_TIMEOUT,
		.parser		= si_nit_parser,
	},
	{
		.pid 		= PSI_PAT_PID,
		.table_id	= PSI_PAT_TABLE_ID,
		.table_mask	= 0xFF,
		.flag		= PSI_FLAG_STAT_PAT,
		.buflen		= PSI_SHORT_SECTION_LENGTH,
		.timeout	= PSI_MODULE_PAT_TIMEOUT,
		.parser		= psi_pat_parser,
	},
	{
		.pid		= PSI_SDT_PID,
		.table_id	= PSI_SDT_TABLE_ID,
		.table_mask	= 0xFF,	//0xFF, actual 0x42 only
							//0xFB,	to support other 0x46
		.flag		= PSI_FLAG_STAT_SDT,
	#if(MW_MODULE == MW_AVIT)
		.buflen		= PSI_LONG_SECTION_LENGTH,
       #else
		.buflen		= PSI_SHORT_SECTION_LENGTH,
	#endif	
		.timeout	= PSI_MODULE_SDT_TIMEOUT,
		.parser		= si_sdt_parser,
	},
#ifdef PSI_SEACH_BAT_SUPPORT
	{
		.pid		= PSI_BAT_PID,
		.table_id	= PSI_BAT_TABLE_ID,
		.table_mask	= 0xFF,
		.flag		= PSI_FLAG_STAT_BAT,
		.buflen		= PSI_SHORT_SECTION_LENGTH,
		.timeout	= PSI_MODULE_BAT_TIMEOUT,
		.parser		= si_bat_parser,
	},
#endif
};

INT32 psi_search_start(
	UINT16 sat_id, 
	UINT32 tp_id,
	UINT32 search_scheme, 
	UINT32 storage_scheme, 
	psi_event_t on_event,
	psi_tuning_t on_tuning,
	void *tuning_param)
{
	INT32 i, n, progress, step;
	INT32 ret_val;
	INT32 ret;
	UINT32 p_attr;
	BOOL need_tune;
	struct program_map *current;
	PROG_INFO *pg_info;
	struct si_filter_t *sfilter;
	struct si_filter_param *fparam;
	#ifdef SI_SUPPORT_MUTI_DMX
	struct dmx_device *dmx0 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	struct dmx_device *dmx1 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
	#endif
        BOOL seach_nit = FALSE;        

	PSI_PRINTF("%s()\n", __FUNCTION__);

	//close sie, to prepare for search
	#ifdef SI_SUPPORT_MUTI_DMX
	sie_close_dmx(dmx1);
	sie_close_dmx(dmx0);
	#else
	sie_close();
	#endif
	
#if (PSI_DEBUG_LEVEL>0)
	UINT32 tick;
	if (psi_info == NULL) {
		PSI_PRINTF("%s: psi module not inited!\n", __FUNCTION__);
		return ERR_FAILUE;
	}

	tick = osal_get_tick();
	PSI_PRINTF("psi_search_start(): start time: %d\n", tick);
#endif
	fparam = &psi_info->fparam;

//	search_scheme |= P_SEARCH_SDT_OTH;	//for test SDT other!!!!
	psi_info->search_scheme = search_scheme;

#ifdef STAR_PROG_SEARCH_NIT_SDT_SUPPORT
	if((search_scheme&P_SEARCH_NIT||search_scheme&P_SEARCH_NIT_STAR ||search_scheme&P_SEARCH_NIT_ALL)&&(0 == star_nit_list_got))
#else
	if(search_scheme&P_SEARCH_NIT)
#endif
        {
            seach_nit = TRUE;
        }       

	get_tp_by_id(tp_id, psi_info->xponder);
	MEMCPY(psi_info->xponder+1, psi_info->xponder, sizeof(T_NODE));

	#ifdef SI_SUPPORT_MUTI_DMX
		ret_val = sie_open_dmx(psi_info->dmx, PSI_MODULE_CONCURRENT_FILTER, 
		psi_info->buffer, 
		PSI_MODULE_CONCURRENT_FILTER*PSI_SHORT_SECTION_LENGTH);
		#else
	ret_val = sie_open(psi_info->dmx, PSI_MODULE_CONCURRENT_FILTER, 
		psi_info->buffer, 
		PSI_MODULE_CONCURRENT_FILTER*PSI_SHORT_SECTION_LENGTH);
	#endif

	//PAT/SDT
	for(sfilter=NULL, n=0, i=/*(search_scheme&(P_SEARCH_NIT|P_SEARCH_NVOD))? 
		PSI_NIT_SECTION_INFO: */PSI_PAT_SECTION_INFO; i<((search_scheme==P_SEARCH_NIT)?PSI_PAT_SECTION_INFO+1:ARRAY_SIZE(psi_sections)); 
		i++) {
		pre_fn_array[i](psi_info, psi_info->param+i);
		
		if ((i==0)||(psi_sections[i-1].pid != psi_sections[i].pid)) {
			#ifdef SI_SUPPORT_MUTI_DMX
			sfilter = sie_alloc_filter_ext(psi_info->dmx,psi_sections[i].pid,	NULL,
				psi_sections[i].buflen, psi_sections[i].buflen,0);
			#else
			sfilter = sie_alloc_filter(psi_sections[i].pid,	NULL,
				psi_sections[i].buflen, psi_sections[i].buflen);
			#endif
			n = 0;	//duplicate PID count
			MEMSET(fparam, 0, sizeof(fparam));
		} 
		sfilter->priv[n] = &psi_info->param[i];

		//check if need search SDT_OTH
#ifdef PSI_SEARCH_SDT_OTH_SUPPORT
		if (psi_sections[i].pid == PSI_SDT_PID && psi_sections[i].table_id == PSI_SDT_TABLE_ID)
		{
			if ((search_scheme&P_SEARCH_SDT_OTH))
			{
				PSI_PRINTF("psi_search_start(): search sdt other enable!\n");
				psi_sections[i].table_mask = 0xFB;
				psi_sections[i].timeout = PSI_MODULE_SDT_OTH_TIMEOUT;
			}
			else
			{
				psi_sections[i].table_mask = 0xFF;
				psi_sections[i].timeout = PSI_MODULE_SDT_TIMEOUT;
			}
		}
#endif

		psi_init_sr(fparam, &psi_sections[i], &psi_info->param[i],
			psi_sections[i].parser, n++);

		if ((i==ARRAY_SIZE(psi_sections)-1)
			||(psi_sections[i+1].pid != psi_sections[i].pid)) {
			fparam->mask_value.value_num = n;
			sie_config_filter(sfilter, fparam);
			ret_val = sie_enable_filter(sfilter);
			PSI_INFO("psi_search_start(): enable flt for pid[0x%x], result=%d\n", psi_sections[i].pid, ret_val);
		}
	}

	//NIT
#ifndef NETWORK_ID_ENABLE
#ifndef _MHEG5_ENABLE_
#ifndef INDIA_LCN
        if(seach_nit)
#endif
#endif
#endif
	{
		UINT16 nit_pid = PSI_NIT_PID;	//default value
		ret = psi_wait_stat(PSI_FLAG_STAT_PAT, 0);
	/*	if (ret != SI_SUCCESS) {
			osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
			goto pss_exit;
		}
    */
		if(ret == SI_SUCCESS && psi_info->pat.nit_pid != 0)	//some pat may have not nit pid
		{
			nit_pid = psi_info->pat.nit_pid;
			PSI_INFO("psi_search_start(): nit pid[0x%x]\n", nit_pid);
		}
		pre_fn_array[0](psi_info, psi_info->param+0);
		#ifdef SI_SUPPORT_MUTI_DMX
		sfilter = sie_alloc_filter_ext(psi_info->dmx, nit_pid,	NULL,
			psi_sections[0].buflen, psi_sections[0].buflen,0);
                #else
		sfilter = sie_alloc_filter(nit_pid,	NULL,
			psi_sections[0].buflen, psi_sections[0].buflen);
                #endif
		MEMSET(fparam, 0, sizeof(fparam));
		sfilter->priv[0] = &psi_info->param[0];
		psi_init_sr(fparam, &psi_sections[0], &psi_info->param[0],
			psi_sections[0].parser, 0);

		fparam->mask_value.value_num = 1;
		sie_config_filter(sfilter, fparam);
		ret_val = sie_enable_filter(sfilter);
		PSI_INFO("psi_search_start(): enable flt for pid[0x%x], result=%d\n", nit_pid, ret_val);

              if (search_scheme == P_SEARCH_NIT)
              {
                  ret = psi_on_nit(psi_info, search_scheme, on_event, 
    			&need_tune);
                  need_tune = TRUE;
                  osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
    		    goto pss_exit;
              }
	}

	//on PAT, open PMT filters, on NIT/PMT/SDT
	for(need_tune = FALSE, i=0; i<ARRAY_SIZE(post_fn_array); i++) {
		ret = on_event(PSI_PROGRESS_ALL_READY*i/(ARRAY_SIZE(post_fn_array)+1), NULL);
		if (ret != SI_SUCCESS) {
			osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
			goto pss_exit;
		}
		
		ret = post_fn_array[i](psi_info, search_scheme, on_event, 
			&need_tune);
		
		if (ret != SI_SUCCESS) {
			if (ret == SI_SKIP) {
				ret = SI_SUCCESS;
			}
                     if(i==0 && seach_nit)
                     {
                        ret = psi_on_nit(psi_info, search_scheme, on_event, 
        			&need_tune);
                        need_tune = TRUE;
                     }
			osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
			goto pss_exit;
		}
	}

#if(MW_MODULE == MW_AVIT)
	int j;
	T_QAMParameters qam_param;
	for(i=0;i<psi_info->pat.map_counter; i++)
	{
		current = &psi_info->maps[i];
		if (current->pm_info.pmt_status == 1)
		{
			pg_info = &current->pm_info;
			/*p_attr = pg_info->CA_count? P_SEARCH_SCRAMBLED: P_SEARCH_FTA;
			if(pg_info->service_type==SERVICE_TYPE_DATABROAD)
				p_attr |= P_SEARCH_DATA;
			else
				p_attr |= pg_info->av_flag? P_SEARCH_TV: P_SEARCH_RADIO;			
			if ((p_attr&search_scheme) != p_attr)
			{
				PSI_INFO("psi_search_start(): program attr(0x%X) not match search scheme(0x%X)\n", p_attr, search_scheme);
				continue;
			}*/
#if 0
			if(pg_info->service_type != SERVICE_TYPE_DATABROAD || (!(psi_info->xponder[0].frq == 40300 || psi_info->xponder[0].frq == 29100
               ||psi_info->xponder[0].frq == 41100||psi_info->xponder[0].frq == 41900
               ||psi_info->xponder[0].frq == 42700||psi_info->xponder[0].frq == 82600)))
				continue;
#else
/*
  JiLin
  搜索到所有的0xc服务后:
    在sdt表中 查找到service_descriptor(descriptor_tag = 0x48)描述子,其中service_provider_name = ipanel 确定是茁壮的数据广播。这部分可以不添加。
          剩下的为通州和佳创的数据广播服务。
          在sdt表中查找数据广播描述符 data_broadcast_descriptor(descriptor=0x64)，其中data_broadcast_id = 0x6的即为佳创数据广播。
*/
            if((pg_info->service_type != SERVICE_TYPE_DATABROAD )||(0x0006 != psi_info->sdt.db_id))
            {
                continue;
            }
#endif
			//psi_get_service(&psi_info->sdt, pg_info->prog_number, psi_info->sdt.service_name, psi_info->sdt.service_provider_name);
			for(j=0;j<psi_info->sdt.sd_nr;j++)
			{
				if (psi_info->sdt.sd[j].program_number==pg_info->prog_number)
				{
					MEMSET(psi_info->sdt.service_name, 0, PSI_SERVICE_NAME_SIZE);
					MEMCPY(psi_info->sdt.service_name, psi_info->sdt.sd[j].service_name, psi_info->sdt.sd[j].service_name_length);
					break;
				}
			}
			if(j==psi_info->sdt.sd_nr)
				continue;

			qam_param.FrequencyKHz  = psi_info->xponder[0].frq * 10;
			qam_param.SymbolRateKHz = psi_info->xponder[0].sym;
			qam_param.ModulatorMode  = psi_info->xponder[0].FEC_inner - QAM16;

			UINT32 real_name_offset = 1;

			if (psi_info->sdt.service_name[0] >= 0x20)
				real_name_offset = 0;
			else if (psi_info->sdt.service_name[0] == 0x1F)
				real_name_offset = 2;
			else if (psi_info->sdt.service_name[0] == 0x10)
				real_name_offset = 3;
			else
				real_name_offset = 1;

			INT32 j_ret = J_LIB_AddService(pg_info->prog_number, pg_info->service_type,
				psi_info->sdt.service_name + real_name_offset, &qam_param);

			//libc_printf("J_LIB_AddService: No.%d freq %d, prog number %d, [%s], ret %d\n", dbc_service_count, psi_info->xponder[0].frq * 10, pg_info->prog_number, psi_info->sdt.service_name + real_name_offset, j_ret);

		}
		else
		{
			PSI_PRINTF("pid[0x%x], pgnumber[0x%x] not parse/recv.\n", 
			current->pm_pid, current->pm_number);
		}
	}
#endif
	if (on_tuning != NULL) {
		on_tuning(tuning_param);
	}
	
	on_event(PSI_PROGRESS_ALL_READY, NULL);

	//process program one by one
	step = PSI_PROGRESS_ALL_LEFT/(psi_info->pmt_counter+1);
	for(i=0, progress= PSI_PROGRESS_ALL_READY;  i<psi_info->pat.map_counter; i++) {
		current = &psi_info->maps[i];
		if (current->pm_info.pmt_status == 1) {
			/*
			 * throw away these PID,
			 * they will be parsed by si_monitor
			 * on playing.
			 */
			progress += step;
			pg_info = &current->pm_info;
			/*must store ttx, subt pid for ca use*/
			pg_info->sat_id = sat_id;
			pg_info->tp_id = tp_id;

			//soc_printf("%04x: ca_count= %d\n", pg_info->prog_number, pg_info->CA_count);
			p_attr = pg_info->CA_count? P_SEARCH_SCRAMBLED: P_SEARCH_FTA;
			if(pg_info->service_type==SERVICE_TYPE_DATABROAD)
			    p_attr |= P_SEARCH_DATA;
			else
			    p_attr |= pg_info->av_flag? P_SEARCH_TV: P_SEARCH_RADIO;			
			
			if ((p_attr&search_scheme) != p_attr) {
				/* this is not a program that matchese the search type. */
				PSI_INFO("psi_search_start(): program attr(0x%X) not match search scheme(0x%X)\n", p_attr, search_scheme);
				continue;
			} 

			if (search_scheme&P_SEARCH_NVOD) {
           		if(pg_info->service_type != SERVICE_TYPE_NVOD_REF
           			 &&pg_info->service_type != SERVICE_TYPE_NVOD_TS)
           		{
					continue;
				}
				
			} 

//BAT will be handled by AP as private???
#ifdef PSI_SEACH_BAT_SUPPORT
			si_bat_get_info(&psi_info->bat, psi_info->sdt.onid,
				psi_info->pat.ts_id, pg_info);
#endif

#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_)||defined(_MHEG5_V20_ENABLE_))
			si_nit_get_info(&psi_info->nit, psi_info->sdt.onid,
				psi_info->pat.ts_id, pg_info);
#endif

			ret = psi_store_program(&current->pm_info,
				psi_info->sdt.service_name, 
				psi_info->sdt.service_provider_name, progress,
				search_scheme, storage_scheme, on_event);
			if (SI_SUCCESS != ret) {
				break;
			}
		} else {
			PSI_PRINTF("pid[0x%x], pgnumber[0x%x] not parse/recv.\n", 
				current->pm_pid, current->pm_number);
		}
	}

pss_exit:
	#ifdef SI_SUPPORT_MUTI_DMX
	//sie_close_dmx(dmx1);
	sie_close_dmx(dmx0);
	#else
	sie_close();
	#endif

	//update xponder
	if (psi_wait_stat(PSI_FLAG_STAT_PAT|PSI_FLAG_STAT_SDT, 
		PSI_FLAG_STAT_PAT|PSI_FLAG_STAT_SDT)==SI_SUCCESS) {
#if(defined(DVBC_INDIA)||defined(DVBC_FP))
		psi_info->xponder[1].t_s_id = psi_info->pat.ts_id;
		//or 
		//psi_info->xponder[1].t_s_id = psi_info->sdt.t_s_id;
		if (psi_info->pat.ts_id != psi_info->sdt.t_s_id)
		{
			PSI_PRINTF("\n%s(): warning - pat ts id(0x%X) != sdt ts id(0x%X) !\n", 
				__FUNCTION__,psi_info->pat.ts_id,psi_info->sdt.t_s_id);
		}
#endif
		psi_info->xponder[1].network_id = psi_info->sdt.onid;
#if(defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined( NETWORK_ID_ENABLE))
		psi_info->xponder[1].net_id = psi_info->nit.net_id;
        PSI_PRINTF("%s: nit net id 0x%X\n",__FUNCTION__,psi_info->nit.net_id);
#endif

#ifdef DYNAMIC_SERVICE_SUPPORT
		psi_info->xponder[1].sdt_version = psi_info->sdt.sdt_version;
#endif
		if (MEMCMP(psi_info->xponder, psi_info->xponder+1, 
			sizeof(T_NODE)) != 0) {
			PSI_PRINTF("\n%s(): xponder changed!\n", __FUNCTION__);
			modify_tp(tp_id, psi_info->xponder+1);
		}
	}
	if ((need_tune==TRUE)&&(on_tuning)) {
		on_tuning(tuning_param);
	}
	
	osal_flag_clear(psi_flag, PSI_FLAG_STAT_ABORT);
	osal_flag_set(psi_flag, PSI_FLAG_STAT_RELEASE);
#if (PSI_DEBUG_LEVEL>0)
	PSI_PRINTF("finished, duration=%d\n", osal_get_tick()-tick);
#endif
	return ret==1? SI_STOFULL: ret;
}

INT32 psi_search_stop()
{
	UINT32 flag;
        #ifdef SI_SUPPORT_MUTI_DMX
	struct dmx_device *dmx0 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	struct dmx_device *dmx1 = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 1);
        #endif

	PSI_PRINTF("%s()\n", __FUNCTION__);

	if (psi_flag == OSAL_INVALID_ID) {
		return SI_EINVAL;
	} else if (osal_flag_wait(&flag, psi_flag, PSI_FLAG_STAT_RELEASE, 
		OSAL_TWF_ANDW, 0)!=OSAL_E_OK) {
		dmx_io_control(psi_info->dmx, IO_STOP_GET_SECTION, 0);
		osal_flag_set(psi_flag, PSI_FLAG_STAT_ABORT);
		osal_flag_wait(&flag, psi_flag, PSI_FLAG_STAT_RELEASE, 
			OSAL_TWF_ANDW, OSAL_WAIT_FOREVER_TIME);
		dmx_io_control(psi_info->dmx, CLEAR_STOP_GET_SECTION, 0);
	}
	
	#ifdef SI_SUPPORT_MUTI_DMX	
	sie_open_dmx(dmx0, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
	sie_open_dmx(dmx1, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
	#else
	sie_open(psi_info->dmx, PSI_MODULE_CONCURRENT_FILTER, NULL, 0);
	#endif


	return SI_SUCCESS;
}

#ifdef BASE_TP_HALF_SCAN_SUPPORT

#define HALF_SCAN_GET_NIT		0x00000001
#define HALF_SCAN_GET_SDT		0x00000010
OSAL_ID nit_sdt_search_flag = INVALID_ID;

struct section_parameter* nit_sec;
struct nit_section_info* nit_info;

static UINT32 nit_version;
static UINT8 nit_section_num;
static UINT8 nit_section_get[32];
static sie_status_t search_on_rec_nit(UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	UINT8 cur_sec;
	UINT8 cur_version;
	UINT8 i;
	if((buff==NULL)||(filter==NULL)||(reason==SIE_REASON_FILTER_TIMEOUT))
		return sie_started;

	cur_version = (*(buff+5) & 0x3E)>>1;
	cur_sec = *(buff+6);
	
	if(nit_section_num==0)
	{
		nit_section_num = *(buff+7);
		nit_version = cur_version;
		MEMSET(nit_section_get, 0, 32);		
	}
	else
	{
		if( nit_section_get[cur_sec]==1 || cur_version != nit_version)
			return sie_started;
	}

	nit_section_get[cur_sec] = 1;
	si_nit_parser(buff, length, nit_sec);

	for(i=0; i<=nit_section_num; i++)
		if(nit_section_get[i] !=1)
			break;
	if(i==nit_section_num+1)
	osal_flag_set(nit_sdt_search_flag, HALF_SCAN_GET_NIT);

	return sie_started;
}


static struct nit_service_info *find_ts_in_nit(UINT16 on_id, UINT16 ts_id, UINT16 service_id)
{
	INT32 i;
	
	for(i=0; i<nit_info->s_nr; i++)
	{
		if((nit_info->s_info[i].onid == on_id)
			&&(nit_info->s_info[i].tsid == ts_id)
			&&(nit_info->s_info[i].sid == service_id))
			return &nit_info->s_info[i];
	}
	return NULL;
}

static UINT8* sdt_get_name(UINT8 *data, UINT32 len)
{
	UINT32 i;
	UINT8 desc_len, desc_tag;
	UINT8 provider_len;
	UINT8 *service_name = NULL;
	
	for(i=0; i<len; i+=(desc_len+2))
	{
		desc_tag = data[i];
		desc_len = data[i+1];

		if(desc_tag == SERVICE_DESCRIPTOR)
		{
			provider_len = data[i+3];
			service_name = data+i+4+provider_len;
			break;
		}
	}

	return service_name;
}


static sie_status_t search_on_rec_sdt(UINT16 pid, struct si_filter_t *filter, 
	UINT8 reason, UINT8 *buff, INT32 length)
{
	struct nit_service_info *info;
	UINT8 *name;
	UINT16 on_id, ts_id, service_id;
	UINT16 descriptors_length;
	
	if((buff==NULL)||(filter==NULL)||(reason==SIE_REASON_FILTER_TIMEOUT))
	{
		osal_flag_set(nit_sdt_search_flag, HALF_SCAN_GET_SDT);
		return sie_freed;
	}

	ts_id = (buff[3]<<8)|buff[4];
	on_id = (buff[8]<<8)|buff[9];
	
	buff += 11;
	length -= (11+PSI_SECTION_CRC_LENGTH);

	while(length > 0)
	{
		service_id = (UINT16)((buff[0]<< 8)|buff[1]);
	    	descriptors_length = (UINT16)(((buff[3]&0x0F)<< 8)|buff[4]);
              
		info = find_ts_in_nit(on_id, ts_id, service_id);

		if((info!=NULL) && (info->service_name == NULL))
		{
			name = sdt_get_name(buff+5, descriptors_length);
			if(name)
			{
				info->service_name = (UINT8 *)MEMCPY(nit_info->names+nit_info->name_pos,
					name+1, name[0]);
				info->name_len = name[0];
				nit_info->name_pos += name[0];
			}
			nit_info->name_nr++;
		}

		buff += (5+descriptors_length);
		length -= (5+descriptors_length);
	}
	
	if(nit_info->name_nr == nit_info->s_nr)
	{
		osal_flag_set(nit_sdt_search_flag, HALF_SCAN_GET_SDT);
		return sie_freed;
	}

	return sie_started;
}

static BOOL search_rec_sdt_event(UINT16 pid, struct si_filter_t *filter,
				UINT8 reason, UINT8 *buf, INT32 length)
{
	return TRUE;
}

UINT32 search_prog_count = 0;

INT32 nit_sdt_search_start(
	UINT16 sat_id, 
	UINT32 tp_id,
	UINT8* work_buffer,
	UINT32 buff_len,
	UINT8* work_db_buffer,
	UINT32 db_buff_len,
	psi_event_t on_event,
	UINT32 time_out,
	UINT8 *stop_flag)
{
	INT32 ret_val;
	struct si_filter_t *sfilter;
	struct si_filter_param fparam;
	//struct dmx_device *dmx = (struct dmx_device *)dev_get_by_id(HLD_DEV_TYPE_DMX, 0);
	UINT8 *nit_buf;
	UINT32 offset = 0;
	UINT32 flgptn = 0;
	UINT32 i,j;
	T_NODE t_node;
	T_NODE main_t_node;
	P_NODE p_node;
	OSAL_ER result;
	BOOL db_changed=FALSE;
	static UINT8 search_ok = 0;
	/*if(search_ok == 1)
	{
		if(on_event)
			on_event(1,0);
		return 0;
	}
	search_ok = 1;
*/
	MEMSET(work_buffer, 0, buff_len);

	//allign memory
	nit_info = (struct nit_section_info*)(work_buffer+offset);
	offset += sizeof(struct nit_section_info);
	nit_sec = (struct section_parameter*)(work_buffer+offset);	
	offset += sizeof(struct section_parameter);

	MEMSET(nit_info, 0, sizeof(struct nit_section_info));
	nit_info->lp1_nr = 0;
	nit_info->lp2_nr = ARRAY_SIZE(nit_loop2);
	nit_info->loop2 = nit_loop2;
	
	MEMSET(nit_sec, 0, sizeof(struct section_parameter));
	nit_sec->priv = nit_info;
	
	nit_buf = work_buffer+offset;

	nit_sdt_search_flag = osal_flag_create(0x00000000);

	//ret_val = sie_open(dmx, PSI_MODULE_CONCURRENT_FILTER, 
	//	work_buffer, 
	//	PSI_MODULE_CONCURRENT_FILTER*PSI_SHORT_SECTION_LENGTH);

	PSI_PRINTF("step1 get nit section information");

	nit_section_num = 0;
	sfilter = sie_alloc_filter(PSI_NIT_PID,	nit_buf,
			PSI_SHORT_SECTION_LENGTH, PSI_SHORT_SECTION_LENGTH);

	if(sfilter == NULL)
	{
		if(on_event)
			on_event(1,0);
		osal_flag_delete(nit_sdt_search_flag);
		//FREE(db_buff);
		return 0;
	}

	MEMSET(&fparam, 0, sizeof(struct si_filter_param));
	fparam.timeout = OSAL_WAIT_FOREVER_TIME;
	fparam.attr[0] = SI_ATTR_HAVE_CRC32;
	//fparam.section_event = __si_monitor_on_sec_event;///for Multi-section monitor

	fparam.section_parser = (si_handler_t)search_on_rec_nit;

	fparam.mask_value.mask_len = 6;
	fparam.mask_value.mask[0] = 0xFF;
	fparam.mask_value.mask[1] = 0x80;
	fparam.mask_value.mask[5] = 0x01;
	fparam.mask_value.value_num = 1;
	fparam.mask_value.value[0][0] = PSI_NIT_TABLE_ID;
	fparam.mask_value.value[0][1] = 0x80;
	fparam.mask_value.value[0][5] = 0x01;

	sie_config_filter(sfilter, &fparam);
	
	ret_val = sie_enable_filter(sfilter);

	result = osal_flag_wait(&flgptn,nit_sdt_search_flag, HALF_SCAN_GET_NIT, OSAL_TWF_ORW, 3000);//3

	osal_flag_clear(nit_sdt_search_flag, HALF_SCAN_GET_NIT);

	sie_abort(PSI_NIT_PID,&fparam.mask_value);
	//sie_abort_filter(sfilter);

	if(result == OSAL_E_TIMEOUT)
	{
		//if(on_event)
		//	on_event(1,0);
		osal_flag_delete(nit_sdt_search_flag);
		//FREE(db_buff);
		//return 0;
	}

	//sdt
	sfilter = sie_alloc_filter(PSI_SDT_PID,	nit_buf,
			PSI_SHORT_SECTION_LENGTH*10, PSI_SHORT_SECTION_LENGTH);
	if(sfilter == NULL)
		{		if(on_event)
			on_event(1,0);
		osal_flag_delete(nit_sdt_search_flag);
		//FREE(db_buff);
		return 0;
		}

	fparam.timeout = 5000;
	fparam.section_parser = (si_handler_t)search_on_rec_sdt;
	fparam.section_event = search_rec_sdt_event;
	fparam.mask_value.mask_len = 6;
	fparam.mask_value.mask[0] = 0xFB;
	fparam.mask_value.mask[1] = 0x80;
	fparam.mask_value.mask[5] = 0x01;
	fparam.mask_value.value_num = 1;
	fparam.mask_value.value[0][0] = PSI_SDT_TABLE_ID;
	fparam.mask_value.value[0][1] = 0x80;
	fparam.mask_value.value[0][5] = 0x01;

	sie_config_filter(sfilter, &fparam);
	
	ret_val = sie_enable_filter(sfilter);

	result =osal_flag_wait(&flgptn,nit_sdt_search_flag, HALF_SCAN_GET_SDT, OSAL_TWF_ORW, 5000);

	osal_flag_clear(nit_sdt_search_flag, HALF_SCAN_GET_SDT);

	sie_abort(PSI_SDT_PID,&fparam.mask_value);
	
	if(result == OSAL_E_TIMEOUT)
	{
	//	if(on_event)
		//	on_event(1,0);
		//FREE(db_buff);
		osal_flag_delete(nit_sdt_search_flag);
//		return 0;
	}
	PSI_PRINTF("step3 update database");
	//update tp
	//recreate_tp_view( VIEW_SINGLE_SAT, 1);
	get_tp_by_id(tp_id, &main_t_node);

	recreate_prog_view(VIEW_SINGLE_SAT|PROG_TVRADIO_MODE , 1);
		libc_printf("start nitsdt search:%d tp,%d prog\n",nit_info->xp_nr,nit_info->s_nr);
	//libc_printf("%x\n",db_buff);
	libc_printf("%x\n",nit_info);

	//db_buff=MALLOC(db_len);
	db_search_init((UINT8 *)work_db_buffer, db_buff_len);
	db_search_create_tp_view(1);
	//recreate_tp_view( VIEW_SINGLE_SAT, 1);
	libc_printf("start nitsdt search:%d tp,%d prog\n",nit_info->xp_nr,nit_info->s_nr);
	//libc_printf("%x\n",db_buff);
	//libc_printf("%x\n",nit_info);

	for(i=0; i<nit_info->xp_nr; i++)
	{
		
		MEMSET(&t_node,0,sizeof(T_NODE));
		t_node.sat_id = 1;
		t_node.ft_type = 0x01;
		t_node.network_id = nit_info->xp[i].c_info.onid;
		t_node.t_s_id = nit_info->xp[i].c_info.tsid;
		t_node.frq = nit_info->xp[i].c_info.frequency;
		t_node.sym = nit_info->xp[i].c_info.symbol_rate;
		t_node.FEC_inner = nit_info->xp[i].c_info.modulation;
			//nit_info->xp[i].c_info.FEC_inner;
		if(t_node.frq==main_t_node.frq&&t_node.sym==main_t_node.sym&&t_node.FEC_inner==main_t_node.FEC_inner)
		{	
			main_t_node.network_id = nit_info->xp[i].c_info.onid;
			main_t_node.t_s_id = nit_info->xp[i].c_info.tsid;
			modify_tp(main_t_node.tp_id, &main_t_node);
			t_node.tp_id=tp_id;
			db_changed=TRUE;
		}
		else if( db_search_lookup_tpnode(TYPE_SEARCH_TP_NODE, &t_node) == DB_SUCCES)//modify tp
		//if(lookup_node(TYPE_TP_NODE, &t_node, t_node.sat_id) ==SUCCESS)
		{
			del_child_prog(TYPE_TP_NODE, t_node.tp_id);
			modify_tp(t_node.tp_id, &t_node);
			db_changed=TRUE;
		}
		else//add tp
		{
			add_node(TYPE_SEARCH_TP_NODE, t_node.sat_id, &t_node);
			db_changed=TRUE;
		}
	
		

		//add all program belong to this tp
		//if (t_node.tp_id==tp_id)
		//	db_search_create_pg_view(TYPE_PROG_NODE, t_node.tp_id, PROG_ALL_MODE);		
		for(j=0;j<nit_info->s_nr; j++)
		{
			if(nit_info->s_info[j].onid == t_node.network_id &&
				nit_info->s_info[j].tsid == t_node.t_s_id)
			{
				search_prog_count++;
				MEMSET(&p_node,0,sizeof(P_NODE));
				p_node.video_pid = 8191;
				p_node.pcr_pid = 8191;
				p_node.audio_count = 1;
				p_node.audio_pid[0] = 8191;
				p_node.sat_id = t_node.sat_id;
				p_node.tp_id = t_node.tp_id;

				
				p_node.prog_number = nit_info->s_info[j].sid;
				p_node.av_flag= (nit_info->s_info[j].service_type==1)?1:0;
				p_node.service_type = (nit_info->s_info[j].service_type==1)?1:2;
				p_node.LCN = nit_info->s_info[j].lcn;
				p_node.audio_count=1;
				p_node.name_len = nit_info->s_info[j].name_len;
				dvb_to_unicode(nit_info->s_info[j].service_name, nit_info->s_info[j].name_len, p_node.service_name,2*(MAX_SERVICE_NAME_LENGTH + 1), 11);
//				MEMCPY(p_node.service_name, nit_info->s_info[j].service_name, nit_info->s_info[j].name_len);
				if(t_node.tp_id==tp_id)
					
				
				{

					if(lookup_node(TYPE_PROG_NODE, &p_node,tp_id)==DB_SUCCES)
					{
						get_prog_by_id(p_node.prog_id, &p_node);
						p_node.LCN = nit_info->s_info[j].lcn;
						p_node.name_len = nit_info->s_info[j].name_len;
						p_node.audio_count=1;
						dvb_to_unicode(nit_info->s_info[j].service_name, nit_info->s_info[j].name_len, p_node.service_name,2*(MAX_SERVICE_NAME_LENGTH + 1), 11);
						modify_prog(p_node.prog_id, &p_node);
						db_changed=TRUE;
						
					}
					else
					{
						add_node(TYPE_PROG_NODE, t_node.tp_id, &p_node);
						db_changed=TRUE;
					}
					
				}
				else
				{
					add_node(TYPE_PROG_NODE, t_node.tp_id, &p_node);
					db_changed=TRUE;
				}
			}
		}
		//if (t_node.tp_id==tp_id)
		//	db_search_create_tp_view(1);	
		}

	update_data();
	libc_printf("nitsdt search :prog saved\n");
	//recreate_tp_view( VIEW_SINGLE_SAT, 1);
	//recreate_prog_view(VIEW_ALL , 0);
	osal_flag_delete(nit_sdt_search_flag);
//	FREE(db_buff);
	//db_buff=NULL;
	if(db_changed)
	{
	if(on_event)
		on_event(0,0);
	}
	else
	{
		if(on_event)
			on_event(1,0);
	}
	osal_flag_delete(nit_sdt_search_flag);
	return 0;
}

#endif

