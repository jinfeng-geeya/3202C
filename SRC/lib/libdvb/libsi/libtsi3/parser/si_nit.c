#include <types.h>
#include <sys_config.h>

#include <api/libc/string.h>

#include <api/libtsi/p_search.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>

#include <api/libsi/si_nit.h>
#include <api/libsi/desc_loop_parser.h>

#define SN_DEBUG_LEVEL			0
#if (SN_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define SN_PRINTF			libc_printf
#else
#define SN_PRINTF(...)			do{}while(0)
#endif

INT32 si_nit_parser(UINT8 *data, INT32 len, struct section_parameter *param)
{
	INT32 i, ret, dloop_len;
	struct network_descriptor *nw_desc;
	struct nit_section_info *n_info = (struct nit_section_info *)param->priv;
	struct transport_stream_info *ts_info;
	if (data == NULL) {
		n_info->xp_nr = 0;
		return SI_SUCCESS;
	}

#if(defined( _MHEG5_ENABLE_) || defined( _MHEG5_V20_ENABLE_) || defined(NETWORK_ID_ENABLE) || defined(STAR_PROG_SEARCH_NIT_SDT_SUPPORT))
	//network_id
	n_info->net_id = (data[3]<<8) | data[4];
    //libc_printf("%s: net id: %X\n",__FUNCTION__,n_info->net_id);
#endif

	/*get nit version*/
	n_info->ext[0].version = (data[5]&0x3E)>>1;
	
	nw_desc = &((struct nit_section *)data)->network;
	dloop_len = SI_MERGE_HL8(nw_desc->network_descriptor_length);
	
	if (n_info->lp1_nr&&(dloop_len>0)) {
		ret = desc_loop_parser(nw_desc->descriptor, dloop_len, 
			n_info->loop1, n_info->lp1_nr, NULL, n_info);
		if (ret != SI_SUCCESS) {
			SN_PRINTF("%s: loop1 parse failure!\n",
				__FUNCTION__);
			return ret;
		}
	}
	
	if (n_info->lp2_nr==0) {
		return SI_SUCCESS;
	}
	
	for(i=sizeof(struct nit_section)+dloop_len-4; i<len-4; 
		i += sizeof(struct transport_stream_info)+dloop_len) {
		ts_info = (struct transport_stream_info *)(data+i);
		n_info->tsid = SI_MERGE_UINT16(ts_info->transport_stream_id);
		n_info->onid = SI_MERGE_UINT16(ts_info->original_network_id);
		dloop_len = SI_MERGE_HL8(ts_info->transport_stream_length);
		if (dloop_len>0) {
			ret = desc_loop_parser(ts_info->descriptor,
				dloop_len, n_info->loop2, n_info->lp2_nr, NULL, n_info);
			if (ret != SI_SUCCESS) {
				SN_PRINTF("%s: loop2 parse failure!\n",
					__FUNCTION__);
				return ret;
			}
		}
	}
		
	return SI_SUCCESS;
}

#if(defined( _MHEG5_ENABLE_) ||defined(_LCN_ENABLE_)||defined(_MHEG5_V20_ENABLE_))
//parse LCN descriptor
INT32 si_on_lcn_desc(UINT8 tag, UINT8 length, UINT8 *data, void *priv)
{
	struct nit_section_info *n_info = (struct nit_section_info *)priv;

	if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
	{
		SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
		return SI_SUCCESS;
	}

	//parse lcn loop	
	while (length >= 4)
	{
		n_info->s_info[n_info->s_nr].onid = n_info->onid;
		n_info->s_info[n_info->s_nr].tsid = n_info->tsid;
		n_info->s_info[n_info->s_nr].sid = (data[0]<<8)|data[1];
		n_info->s_info[n_info->s_nr].lcn = ((data[2]&0x03)<<8)|data[3];
		SN_PRINTF("LCN[%d]: %d - onid: 0x%X, tsid: 0x%X, sid: 0x%X\n",
					n_info->s_nr,
					n_info->s_info[n_info->s_nr].lcn,
					n_info->s_info[n_info->s_nr].onid,
					n_info->s_info[n_info->s_nr].tsid,
					n_info->s_info[n_info->s_nr].sid
					);
		n_info->s_nr ++;
	
		if (n_info->s_nr==PSI_MODULE_MAX_NIT_SINFO_NUMBER)
		{
			SN_PRINTF("%s: maximum lcn collected!\n", __FUNCTION__);
			return SI_SUCCESS;
		}

		data += 4;
		length -= 4;
	}
	
	return SI_SUCCESS;
}


// get LCN from NIT
INT32 si_nit_get_info(struct nit_section_info *nsi,
	UINT16 onid, UINT16 tsid, PROG_INFO *pg)
{
	INT32 i;

	for(i=0; i<nsi->s_nr; i++)
	{
		if ((nsi->s_info[i].sid == pg->prog_number)
			&&(nsi->s_info[i].tsid == tsid)
			&&(nsi->s_info[i].onid == onid))
		{
			pg->lcn = nsi->s_info[i].lcn;
			pg->lcn_true = TRUE;
			SN_PRINTF("found lcn: %d\n",pg->lcn);
			return SI_SUCCESS;
		}
	}	
	SN_PRINTF("not found lcn of prog 0x%X!!\n",pg->prog_number);
	return ERR_FAILUE;
}
#endif


