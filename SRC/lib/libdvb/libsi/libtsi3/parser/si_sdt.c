/*
 * SI_SDT.C
 *
 * SI SDT parser implementation file
 *
 * 1. 2006.07.10	Zhengdao Li	0.01		Initialize.
 */
#include <types.h>
#include <sys_config.h>

#include <api/libc/string.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>
#include <api/libtsi/p_search.h>
#include <api/libtsi/psi_db.h>

#include <api/libsi/si_module.h>
#include <api/libsi/si_sdt.h>
#include <api/libsi/desc_loop_parser.h>

#define SS_DEBUG_LEVEL			0
#if (SS_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SS_PRINTF			libc_printf
#else
#define SS_PRINTF(...)			do{}while(0)
#endif

#define SDT_DESCRIPTOR_MAX_CNT		8

struct nvod_reference_descriptor_ripped {
	struct {
		UINT8 transport_stream_id[2];
		UINT8 original_network_id[2];
		UINT8 service_id[2];
	}__attribute__((packed)) ref_list[0];
}__attribute__((packed));



static INT32 sdt_on_service_descriptor(UINT8 tag, UINT8 length, UINT8 *data,
	void *priv)
{
	struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
	struct service_data *desc = s_info->sd;
	struct service_descriptor_ripped *sdr;
	INT32 name_pos = s_info->name_pos;
	INT32 pos = s_info->sd_nr;
		
	sdr = (struct service_descriptor_ripped *)data;
#ifdef CELRUN_SOUTH_AMERICA /* modify to support DVB-C channel in South America */
	switch( sdr->service_type )
	{
		  /* 0x80~0xA0 : Service provider defined */
		  case 0x82: // 130
		  case 0x80: // 128
		  case 0x86: // 134
		  case 0x99: // 153
		  case 0x9b: // 155
				 desc[pos].service_type = 0x01;//SERVICE_TYPE_DTV;
				 s_info->service_type = 0x01;//SERVICE_TYPE_DTV;
				 break;
		  default:
				  desc[pos].service_type = sdr->service_type;
				  s_info->service_type = sdr->service_type;
				 break;
	}
#else
	desc[pos].service_type = sdr->service_type;
	s_info->service_type = sdr->service_type;
#endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	if ((name_pos+sdr->service_provider_name_length)<s_info->max_name_len) {
		desc[pos].service_provider_length = sdr->service_provider_name_length;
		desc[pos].service_provider = (UINT8 *)MEMCPY(s_info->names+name_pos, 
			sdr->service_provider_name, 
			sdr->service_provider_name_length);
		name_pos += sdr->service_provider_name_length;
	} else {
		return SI_SBREAK;
	}
#endif

	sdr = (struct service_descriptor_ripped *)(data+sdr->service_provider_name_length);
	if (name_pos+sdr->service_name_length<s_info->max_name_len) {
		desc[pos].service_name_length = sdr->service_name_length;
		desc[pos].service_name = (UINT8 *)MEMCPY(s_info->names+name_pos, 
			sdr->service_name, 
			sdr->service_name_length);
		name_pos += sdr->service_name_length;	
	} else {
		s_info->name_pos = name_pos;
		return SI_SBREAK;
	}
	s_info->name_pos = name_pos;
	SS_PRINTF("%s(): service id [%d],service type[%d],name len [%d], provider name len [%d]\n", __FUNCTION__,s_info->sid,desc[pos].service_type,desc[pos].service_name_length,desc[pos].service_provider_length);
	
	
	return SI_SUCCESS;
}

#if(MW_MODULE == MW_AVIT)
static INT32 sdt_on_db_descriptor(UINT8 tag, UINT8 length, UINT8 *data,
	void *priv)
{
	struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
	if(length==0 || data==NULL)
	{
		return SI_SUCCESS;
	}
		
	s_info->db_id = SI_MERGE_UINT16(data);
	
	return SI_SUCCESS;
}
#endif

#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
/*parse multilingual service name descriptor*/
static INT32 sdt_on_mtilingual_service_desc(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
	UINT8 name_len,prov_len;
	struct sdt_section_info *s_info = (struct sdt_section_info *)priv;	
	UINT8 i = 0;
	UINT8 k=0;	

	k = s_info->sd_nr;	
	i = s_info->mtl_s_data[k].mult_name_count;	
	
	while(len>0)
	{
		prov_len = desc[3];

		//s_info->mtl_s_data[0].mtl_name[i].ISO_639_language_code = (desc[k]<<16)|(desc[k+1]<<8)|desc[k+2];		
		s_info->mtl_s_data[k].mtl_name[i].ISO_639_language_code[0] = desc[0];
		s_info->mtl_s_data[k].mtl_name[i].ISO_639_language_code[1] = desc[1];	
		s_info->mtl_s_data[k].mtl_name[i].ISO_639_language_code[2] = desc[2];	

		s_info->mtl_s_data[k].mtl_name[i].service_provider_name_length = prov_len;
		//s_info->mtl_s_data[0].mtl_name[i].service_provider_name =(UINT8*)MEMCPY(s_info->mtl_s_data[0].names+s_info->mtl_s_data[0].name_pos, desc+k+4,prov_len);	
		s_info->mtl_s_data[k].mtl_name[i].service_provider_name =(UINT8*)MEMCPY(s_info->mtl_s_data[k].names+s_info->mtl_s_data[k].name_pos, desc+4,prov_len);	
		desc += (prov_len+4);	
		name_len = desc[0];		
		//	k=(prov_len+4+k);
		//name_len = desc[k];				
		s_info->mtl_s_data[k].name_pos += prov_len;
		
		s_info->mtl_s_data[k].mtl_name[i].service_name_length = name_len;
		//s_info->mtl_s_data[0].mtl_name[i].service_name =(UINT8*)MEMCPY(s_info->mtl_s_data[0].names+s_info->mtl_s_data[0].name_pos, desc+k+1, name_len);
		s_info->mtl_s_data[k].mtl_name[i].service_name =(UINT8*)MEMCPY(s_info->mtl_s_data[k].names+s_info->mtl_s_data[k].name_pos, desc+1, name_len);
		s_info->mtl_s_data[k].name_pos+=name_len;		

		len -= 5+prov_len+name_len;
		desc += (name_len+1);
		//k=(name_len+1+k);
		i++;
	}

	s_info->mtl_s_data[k].program_number = s_info->sd[k].program_number;
	s_info->mtl_s_data[k].mult_name_count = i;
	return SI_SUCCESS;
}

#endif


#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)
#include <api/libsi/si_service_type.h>

static INT32 sdt_on_nvod_ref_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
	INT32 i = 0, j = 0;
	INT32 cnt = length/6;
	struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
	struct nvod_reference_descriptor_ripped *nrd;
	nrd = (struct nvod_reference_descriptor_ripped *)data;
	UINT8 max_ref_service_cnt = sizeof(s_info->nvod.ref_id)/sizeof(s_info->nvod.ref_id[0]);
	UINT8 max_service_cnt = sizeof(s_info->nvod.sid)/sizeof(s_info->nvod.sid[0]);
	
	UINT16 t_s_id,origin_netid, service_id;
	
	//if(s_info->service_type==SERVICE_TYPE_NVOD_REF)
	{
		for(j=0; j < s_info->nvod.ref_cnt; j++)
			if(s_info->sid==s_info->nvod.ref_id[j])
				break;
		if(j==s_info->nvod.ref_cnt)	
		{
			if(s_info->nvod.ref_cnt < max_ref_service_cnt)
				s_info->nvod.ref_id[s_info->nvod.ref_cnt++] = s_info->sid;
			else
				SS_PRINTF("nvod_reference descriptor: ref service cnt to max cnt\n");
		}
	}

	for(i = 0; i<cnt; i++)
	{
		t_s_id = SI_MERGE_UINT16(nrd->ref_list[i].transport_stream_id);
		origin_netid = SI_MERGE_UINT16(nrd->ref_list[i].original_network_id);
		service_id = SI_MERGE_UINT16(nrd->ref_list[i].service_id);
		SS_PRINTF("nvod_reference descriptor: ts_id[%d],origin_netid[%d], service_id[%d]\n",t_s_id,origin_netid,service_id);
		if((t_s_id==s_info->t_s_id) /*&& (origin_netid==s_info->onid)*/)
		{
			for(j=0; j< s_info->nvod.scnt; j++)
				if(service_id==s_info->nvod.sid[j].sid)
					break;
			if(j==s_info->nvod.scnt)	
			{
				if(s_info->nvod.scnt < max_service_cnt)
				{
					s_info->nvod.sid[s_info->nvod.scnt].sid = service_id;
					s_info->nvod.sid[s_info->nvod.scnt].ref_id = s_info->sid;
					s_info->nvod.sid[s_info->nvod.scnt].t_s_id = t_s_id;
					s_info->nvod.sid[s_info->nvod.scnt].origin_netid = origin_netid;
					s_info->nvod.scnt++;
				}
				else
				{
					SS_PRINTF("nvod_reference descriptor: service cnt to max cnt\n");
				}
			}
		}
	}

	return SI_SUCCESS;
}

static INT32 sdt_on_timeshift_service_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
	struct sdt_section_info *s_info = (struct sdt_section_info *)priv;
	UINT16 nvod_ref_sid = 0;
	struct service_data *pservice = s_info->sd;
	INT32 pos = s_info->sd_nr;
	INT32 i = 0;
	UINT8 max_ref_service_cnt = sizeof(s_info->nvod.ref_id)/sizeof(s_info->nvod.ref_id[0]);
	UINT8 max_service_cnt = sizeof(s_info->nvod.sid)/sizeof(s_info->nvod.sid[0]);

	if(length==0 || data==NULL)
		return SI_SUCCESS;
	nvod_ref_sid = (data[0]<<8)|data[1];
	SS_PRINTF("time shift service descriptor, reference_service_id[%d]\n",nvod_ref_sid);
	pservice[pos].timeshift_flag = 1;
	pservice[pos].ref_service_id = nvod_ref_sid;

	/*in some places nvod has some tp, sdt only has timeshift_service_desctiptor, not has 
	nvod_reference_descriptor, we need add these services to nvod info*/
	//check if this reference_service_id already saved
	for(i=0; i< s_info->nvod.ref_cnt; i++)
			if(nvod_ref_sid==s_info->nvod.ref_id[i])
				break;
	if(i==s_info->nvod.ref_cnt)	
	{
		if(s_info->nvod.ref_cnt < max_ref_service_cnt)
			s_info->nvod.ref_id[s_info->nvod.ref_cnt++] = nvod_ref_sid;
		else
			SS_PRINTF("time shift service descriptor: ref service cnt to max cnt\n");
	}
	//check if this service_id already saved
	for(i=0; i< s_info->nvod.scnt; i++)
			if(pservice[pos].program_number==s_info->nvod.sid[i].sid)
				break;
	if(i==s_info->nvod.scnt)	
	{
		if(s_info->nvod.scnt < max_service_cnt)
		{
			s_info->nvod.sid[s_info->nvod.scnt].sid = pservice[pos].program_number;
			s_info->nvod.sid[s_info->nvod.scnt].ref_id = nvod_ref_sid;
			s_info->nvod.sid[s_info->nvod.scnt].t_s_id = s_info->t_s_id;
			s_info->nvod.sid[s_info->nvod.scnt].origin_netid = s_info->onid;
			s_info->nvod.scnt++;
		}
		else
			SS_PRINTF("time shift service descriptor: service cnt to max cnt\n");
	}
	

	return SI_SUCCESS;
}
#if(MW_MODULE == MW_AVIT)
static UINT32 sdt_parse_descriptor_cnt = 4;
#else
static UINT32 sdt_parse_descriptor_cnt = 3;
#endif
#endif

static struct desc_table sdt_desc_table[SDT_DESCRIPTOR_MAX_CNT] = {
	{
		.tag = SERVICE_DESCRIPTOR,
		.bit = 0,
		.parser = sdt_on_service_descriptor,
	},
#if(MW_MODULE == MW_AVIT)
    {
		.tag = DATA_BROADCAST_DESCRIPTOR,
		.bit = 0,
		.parser = sdt_on_db_descriptor,
	},
#endif
#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
	{
		.tag = MULTILINGUAL_SERVICE_NAME_DESCRIPTOR,
		.bit = 0,
		.parser = sdt_on_mtilingual_service_desc,
	},
#endif	
#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)	
	{
		.tag = NVOD_REFERENCE_DESCRIPTOR,
		.bit = 0,
		.parser = sdt_on_nvod_ref_desc,
	},
	{
		.tag = TIME_SHIFTED_SERVICE_DESCRIPTOR,
		.bit = 0,
		.parser = sdt_on_timeshift_service_desc,
	},
	
#endif
};


INT32 si_sdt_add_priv_descriptor(struct desc_table *pdescriptor, UINT8 descriptor_cnt)
{
	INT32 i,j ;
	INT8 add_cnt = 0;

	if(descriptor_cnt >=SDT_DESCRIPTOR_MAX_CNT)
	{
		SS_PRINTF("%s(), invalid param\n",__FUNCTION__);
		return -1;
	}
	for(i = 0; i < SDT_DESCRIPTOR_MAX_CNT; i++)
	{
		if((sdt_desc_table[i].tag==0)&&(sdt_desc_table[i].parser==NULL))
		{
			sdt_parse_descriptor_cnt = i;
			break;
		}
	}
	if((i+descriptor_cnt) >SDT_DESCRIPTOR_MAX_CNT)
	{
		SS_PRINTF("%s(), add descriptor cnt[%d] exceed max cnt!\n",__FUNCTION__,descriptor_cnt);
		add_cnt = SDT_DESCRIPTOR_MAX_CNT - i ;
	}
	else
		add_cnt = descriptor_cnt;
	if(add_cnt > 0)
	{
		MEMCPY(&sdt_desc_table[i], pdescriptor, add_cnt*sizeof(struct desc_table));
		sdt_parse_descriptor_cnt = i+add_cnt;
	}

	return SI_SUCCESS;
}

#ifdef NVOD_ZHANGZHOU
static struct nvod_zhangzhou_data  nvod_sd[PSI_MODULE_MAX_PROGRAM_NUMBER];
static UINT8 nvod_sd_count;
UINT8 get_nvod_sd_count()
{
    return nvod_sd_count;
}
void set_nvod_sd_count(UINT8 count)
{
    nvod_sd_count=count;
}
void* get_nvod_sd( UINT8 * nvod_count)
{
    
    *nvod_count=nvod_sd_count;
    return (void *)&nvod_sd;
}
#endif


INT32 si_sdt_parser(UINT8 *section, INT32 length, struct section_parameter *param)
{
	INT32 i, descriptors_length, ret;
	struct sdt_stream_info *info;
	struct sdt_section_info *s_info = (struct sdt_section_info *)param->priv;
	if (section==NULL) {
		s_info->sd_nr = s_info->name_pos = s_info->nd_nr = 0;
		return SI_SUCCESS;
	} 

	//if (0 == section[6]) {
		/* 
		 * Here we assume that original network is always the same
		 * however, EN300468 didn't say so, so it's not follow SPEC.
		 */
	//	s_info->onid = (section[8]<<8)|section[9];
	//}Wrong
	
	/*why above? if sdt is multi-section, first hitted section_number!=0, 
	we can't get original_network_id*/
	s_info->onid = (section[8]<<8)|section[9];
	s_info->sdt_version = (section[5]&0x3E)>>1;
	s_info->t_s_id = (section[3]<<8)|section[4];

#ifdef NVOD_ZHANGZHOU
    MEMSET(nvod_sd,0,sizeof(nvod_sd));
    nvod_sd_count=0;
#endif 

	SS_PRINTF("\n%s(): ts_id[%d], origin_netid[%d]\n", __FUNCTION__,s_info->t_s_id, s_info->onid);
	for(i=sizeof(struct sdt_section)-PSI_SECTION_CRC_LENGTH;
	    i<length-PSI_SECTION_CRC_LENGTH;
	    i+=sizeof(struct sdt_stream_info)+descriptors_length) {
	    	info = (struct sdt_stream_info *)(section+i);
	    	descriptors_length = SI_MERGE_HL8(info->descriptor_loop_length);
		s_info->sid = SI_MERGE_UINT16(info->service_id);
		s_info->sd[s_info->sd_nr].program_number = s_info->sid;
		SS_PRINTF("service id [%d]\n", s_info->sid);

		//SS_PRINTF("eit schedule flag[%d], p/f flag[%d], run_status[%d]\n", info->EIT_schedule_flag,
		//	info->EIT_present_following_flag,info->running_status);
		ret = desc_loop_parser(info->descriptor, descriptors_length, 
			sdt_desc_table, sdt_parse_descriptor_cnt, NULL, s_info);
        
#ifdef NVOD_ZHANGZHOU
        if(s_info->sd[s_info->sd_nr].service_type==SERVICE_TYPE_NVOD_REF)//0x04
        {
            nvod_sd[nvod_sd_count].program_number=s_info->sd[s_info->sd_nr].program_number;
            nvod_sd[nvod_sd_count].ref_service_id=s_info->sd[s_info->sd_nr].ref_service_id;
            nvod_sd[nvod_sd_count].service_name_length=s_info->sd[s_info->sd_nr].service_name_length;
            nvod_sd[nvod_sd_count].service_type=SERVICE_TYPE_NVOD_REF;
            if(s_info->sd[s_info->sd_nr].service_name_length<=NVOD_REF_MAX_LEN)
                MEMCPY(nvod_sd[nvod_sd_count].service_name,s_info->sd[s_info->sd_nr].service_name,s_info->sd[s_info->sd_nr].service_name_length);            
               
           // MEMCPY(&nvod_sd[nvod_sd_count],&s_info->sd[s_info->sd_nr],sizeof(nvod_sd[s_info->sd_nr]));
            nvod_sd_count++;
        }
#endif 		
		s_info->sd_nr++;
		SS_PRINTF("%s: sd_nr %d\n",__FUNCTION__,s_info->sd_nr);
		if (ret != SI_SUCCESS) {
			SS_PRINTF("%s: name space not big enough!\n",
				__FUNCTION__);
			return SI_SUCCESS;
		}
	}
	return SI_SUCCESS;
}

static void get_hstr(UINT8 *hstr, UINT8 *src, INT32 len)
{
	*hstr = len;
	MEMCPY(hstr+1, src, len);
}

INT32 psi_get_service(struct sdt_section_info *info,
	UINT16 program_number, 	UINT8 *service_name, UINT8 *provider_name)
{
	INT32 i;
	struct service_data *sdesc = info->sd;
	for(i=0; i<info->sd_nr; i++, sdesc++)
	{
		if (sdesc->program_number == program_number)
		{
			if (sdesc->service_name_length > 0)
				get_hstr(service_name,sdesc->service_name,sdesc->service_name_length);
			
			else
			#ifndef INDIA_LCN
				psi_fake_name(service_name, program_number);
			#else
			psi_fake_name(service_name, program_number);
				//STRCPY(service_name,"");
			#endif
						
#if (SERVICE_PROVIDER_NAME_OPTION>0)
			if(sdesc->service_provider_length > 0)
				get_hstr(provider_name,sdesc->service_provider,sdesc->service_provider_length);
			else
				psi_fake_name(provider_name, 0);
#endif
			return SI_SUCCESS;
		}
	} 
#ifdef USE_SDT_SERVICE_NAME
    return SI_SKIP;
#endif
	SS_PRINTF("program_number: %d, no service name!\n", program_number);
	#ifndef INDIA_LCN
	psi_fake_name(service_name, program_number);
	#else
	psi_fake_name(service_name, program_number);
	//STRCPY(service_name,"");
	#endif
#if (SERVICE_PROVIDER_NAME_OPTION>0)
	psi_fake_name(provider_name, 0);
#endif
	return SI_SUCCESS;
}

INT32 psi_get_service_type(struct sdt_section_info *info,
	UINT16 program_number, UINT8 *service_type)
{
	INT32 i;
	struct service_data *sdesc = info->sd;
	for(i=0; i<info->sd_nr; i++, sdesc++)
	{
		if (sdesc->program_number == program_number)
		{
			*service_type = sdesc->service_type;
			return SI_SUCCESS;
		}
	} 
	
	return SI_SKIP;
}


#if(defined(SUPPORT_MTILINGUAL_SERVICE_NAME))
struct mtl_service_data *psi_get_multi_service_name(struct sdt_section_info *info,
	UINT16 program_number)
{
	struct mtl_service_data * ret = NULL;
	INT32 i;
	struct mtl_service_data * data = info->mtl_s_data;
	
	for(i=0; i < i<info->sd_nr && i<PSI_MODULE_MAX_PROGRAM_NUMBER; i++, data++)
	{
		if (data->program_number == program_number)
		{
			ret = data;
			break;
		}
	} 
	
	return ret;
}
#endif


