#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>

#include <api/libtsi/p_search.h>
#include <api/libtsi/sie.h>
#include <api/libsi/si_module.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/si_bat.h>


#ifdef PSI_SEACH_BAT_SUPPORT

#define INVALID_LCN		0xFFFF

//#define BAT_DEBUG
#ifdef BAT_DEBUG
#define BAT_PRINTF	libc_printf
#define BAT_EVENT_PRINTF libc_printf
#else
#define BAT_PRINTF(...)	        do{}while(0)
#define BAT_EVENT_PRINTF(...)	do{}while(0)
#endif


static INT32 bat_on_bouqname_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv)
{
	INT32 i;
	struct bat_section_info *b_info = (struct bat_section_info *)priv;

	BAT_PRINTF("%s:  bouquet name descriptor\n",__FUNCTION__);

	//check if this banquet already eixst
	for(i=0; i<b_info->bouq_cnt; i++)
	{
		if(b_info->b_id==b_info->bouq_id[i])
		{
			MEMCPY(b_info->bouq_name[i], data, len);
			BAT_PRINTF("bouq name[%d]: %s\n",i,b_info->bouq_name[i]);
			return SUCCESS;
		}
	}

	if (i >= b_info->bouq_cnt && b_info->bouq_cnt < PSI_MODULE_MAX_BAT_EXT_NUMBER)	//not found
	{
		b_info->bouq_id[i] = b_info->b_id;
		MEMCPY(b_info->bouq_name[i], data, len);
		b_info->bouq_cnt ++;
		BAT_PRINTF("bouq name[%d]: %s\n",i,b_info->bouq_name[i]);
	}
	
	return SUCCESS;
}

static INT32 bat_on_servicelist_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv)
{
	INT32 i, j;
	UINT16 sid;
	struct bat_section_info *b_info = (struct bat_section_info *)priv;
	
	BAT_PRINTF("%s: service list descriptor\n",__FUNCTION__);
	for(i=0; i<len; i += 3)
	{
		sid = (data[i]<<8)|data[i+1];
		for(j=0; j<b_info->service_cnt; j++)
		{
			if ((b_info->s_info[j].sid ==sid)&&(b_info->s_info[j].ts_id==b_info->ts_id)
				&&(b_info->s_info[j].bouq_id==b_info->b_id))
			{
				b_info->s_info[j].service_type = data[i+2];
				BAT_PRINTF("service id[0x%X], type[0x%X]\n",sid, data[i+2]);
				break;
			}
		}
		if((j==b_info->service_cnt)&&(j<PSI_MODULE_MAX_BAT_SINFO_NUMBER))
		{
			BAT_PRINTF("service id[0x%X], type[0x%X]\n",sid, data[i+2]);
			b_info->s_info[b_info->service_cnt].bouq_id = b_info->b_id;
			b_info->s_info[b_info->service_cnt].ts_id = b_info->ts_id;
			b_info->s_info[b_info->service_cnt].orig_netid = b_info->orig_netid;
			b_info->s_info[b_info->service_cnt].sid = sid;
			b_info->s_info[b_info->service_cnt].service_type = data[i+2];
			b_info->service_cnt++;
			BAT_PRINTF("service count: %d\n",b_info->service_cnt);
		}
	}
	
	return SUCCESS;
}

static INT32 bat_on_lcn_desc(UINT8 tag, UINT8 len, UINT8 *data, void *priv)
{
	INT32 i, j;
	UINT16 sid, lcn;
	struct bat_section_info *b_info = (struct bat_section_info *)priv;

	BAT_PRINTF("%s: lcn descriptor\n",__FUNCTION__);
	
	for(i=0; i<len; i += 4)
	{
		sid = (data[i]<<8)|data[i+1];
		lcn = (data[i+2]<<8)|data[i+3];
		BAT_PRINTF("service id[0x%X], lcn[0x%X]\n",sid, lcn);
		for(j=0; j<b_info->service_cnt; j++)
		{
			if ((b_info->s_info[j].sid ==sid)&&(b_info->s_info[j].ts_id==b_info->ts_id)
				&&(b_info->s_info[j].bouq_id==b_info->b_id))
			{
				b_info->s_info[j].lcn = lcn;
				break;
			}
		}
		if((j==b_info->service_cnt)&&(j<PSI_MODULE_MAX_BAT_SINFO_NUMBER))
		{
			b_info->s_info[b_info->service_cnt].bouq_id = b_info->b_id;
			b_info->s_info[b_info->service_cnt].ts_id = b_info->ts_id;
			b_info->s_info[b_info->service_cnt].orig_netid = b_info->orig_netid;
			b_info->s_info[b_info->service_cnt].sid = sid;
			b_info->s_info[b_info->service_cnt].lcn = lcn;
			b_info->service_cnt++;
			BAT_PRINTF("service count: %d\n",b_info->service_cnt);
		}
	}
	
	return SUCCESS;
}

static struct desc_table bat_loop1[] = {
	{
		BOUQUET_NAME_DESCRIPTOR,
		0,
		bat_on_bouqname_desc,
	},
};

static struct desc_table bat_loop2[] = {
	{
		SERVICE_LIST_DESCRIPTOR,
		0,
		bat_on_servicelist_desc,
	},
	{
		LCN_DESCRIPTOR_JILIN,
		0,
		bat_on_lcn_desc,
	},
};

INT32 si_bat_parser(UINT8 *data, INT32 len, struct section_parameter *param)
{
	INT16 sec_len = 0;
	INT16 bouquet_loop_len = 0;
	INT16 ts_loop_len =0;
	INT16 desc_loop_len = 0;
	INT16 i, j;
	INT16 pos = 0;
	struct bat_section_info *pinfo = (struct bat_section_info *)param->priv;

	BAT_PRINTF("%s: len %d\n",__FUNCTION__,len);

	sec_len = ((data[1]&0xf)<<8) |data[2];
	pinfo->b_id = (data[3]<<8)|data[4];
	BAT_PRINTF("bouquet id = 0x%X\n",pinfo->b_id);
	//empty section
	if(sec_len <= 13 )
	{
		return SUCCESS;
	}
	bouquet_loop_len = ((data[8]&0xf)<<8) |data[9];
	pos = 10+bouquet_loop_len;
	//bouquet descriptor loop
	BAT_PRINTF("bouquet descriptor loop len[%d]\n",bouquet_loop_len);
	for(i = 10; (i<pos)&&(i<sec_len-1); i += 2+data[i+1])
	{
		desc_loop_parser(data+i, data[i+1], bat_loop1, 1, NULL, pinfo);
	}

	//ts loop
	ts_loop_len = ((data[pos]&0xf)<<8) |data[pos+1];
	BAT_PRINTF("ts_stream loop len[%d]\n",ts_loop_len);
	pos += 2+ts_loop_len;
	for(i += 2; (i<pos)&&(i<sec_len-1); i += 6+desc_loop_len)
	{
		pinfo->ts_id = (data[i]<<8)|data[i+1];
		pinfo->orig_netid = (data[i+2]<<8)|data[i+3];
		BAT_PRINTF("ts_id=0x%X,org_netid=0x%X\n",pinfo->ts_id,pinfo->orig_netid);
		desc_loop_len = ((data[i+4]&0xf)<<8)|data[i+5];
		for(j = i+6; (j<(i+6+desc_loop_len))&&(j<(sec_len-1)); j += 2+data[j+1])
		{
			desc_loop_parser(data+j, data[j+1], bat_loop2, 2, NULL, pinfo);
		}
	}
    return SUCCESS;
}

INT32 si_bat_get_info(struct bat_section_info *bat_info, UINT16 onid, UINT16 tsid, PROG_INFO *pg)
{
	INT32 i, cnt;
	struct bat_service_info *bat_sinfo = NULL;

	BAT_PRINTF("%s: \n",__FUNCTION__);

	if(bat_info==NULL)
		return -1;
	bat_sinfo = bat_info->s_info;
	cnt = bat_info->service_cnt;
	for(i=0; i<cnt; i++)
	{
		if ((bat_sinfo[i].sid==pg->prog_number)/*&&(bat_sinfo[i].ts_id == tsid)*/)
		{
			pg->bouquet_id = bat_sinfo[i].bouq_id;
			if (bat_sinfo[i].lcn == 0)
				pg->logical_channel_num = INVALID_LCN;
			else
				pg->logical_channel_num = bat_sinfo[i].lcn;
			BAT_PRINTF("%s: service[0x%X],bouquet[0x%X],lcn[0x%X]\n",__FUNCTION__,bat_sinfo[i].sid,pg->bouquet_id,pg->logical_channel_num);
			return SI_SUCCESS;
		}
	}
	return ERR_FAILUE;
}

UINT16 si_bat_get_bouq_cnt(struct bat_section_info *bat_info, UINT16 *bouq_id, UINT8 *bouq_name, UINT16 max_name_len)
{
	INT16 i;
	UINT16 len = 0;
	UINT16 offset = 0;
	
	if(bat_info==NULL)
		return 0;
	MEMCPY(bouq_id, bat_info->bouq_id, bat_info->bouq_cnt*2);
	if(max_name_len<=2*(MAX_BOUQ_NAME_LENGTH + 1))
		len = max_name_len;
	else
		len = 2*(MAX_BOUQ_NAME_LENGTH + 1);
	for(i=0; i< bat_info->bouq_cnt; i++)
	{	
		MEMCPY(bouq_name+offset, bat_info->bouq_name[i],len);
		offset += max_name_len;
	}

	BAT_PRINTF("%s: bouquet count %d\n",__FUNCTION__,bat_info->bouq_cnt);
	return bat_info->bouq_cnt;
}

//this interface need CSTM to provide!!!
INT32 si_porting_get_bouquet_id(UINT16 *b_id, INT32 max_cnt)
{
	//for jilin
	b_id[0] = 0x7011;
//	b_id[1] = 0x3622;

	return 1;
}

#endif

