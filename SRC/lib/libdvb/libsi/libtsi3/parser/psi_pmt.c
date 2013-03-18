#include <types.h>
#include <sys_config.h>

#include <api/libc/string.h>
#include <api/libc/printf.h>

#include <api/libtsi/p_search.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/si_descriptor.h>
#include <api/libtsi/si_section.h>

#include <api/libsi/si_module.h>
#include <api/libsi/psi_pmt.h>
#include <api/libsi/desc_loop_parser.h>
#include <api/libsi/si_service_type.h>

#define PP_DEBUG_LEVEL			0
#if (PP_DEBUG_LEVEL>0)
#define PP_PRINTF			libc_printf
#else
#define PP_PRINTF(...)			do{}while(0)
#endif


static INT32 iso639_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
	UINT8 i = 0;
	UINT8 k = 0;
	struct pmt_es_info *es = (struct pmt_es_info *)priv;

	//es->lang size is only 3 byte, but some abnomal component audio has more than 1 language
	for( i = 0; i < len; i += 4)
	{
        if((k+3) < ES_INFO_MAX_LANG_SIZE)
        {
		    MEMCPY(&es->lang[k], (desc + i), 3);
		    k += 3;
        }
        else
        {
            break;
        }
	}
	return SI_SUCCESS;
}

static INT32 loop1_ca_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
	PROG_INFO *info = (PROG_INFO *)priv;

	if (info->CA_count < P_MAX_CA_NUM)
	{
		info->CA_info[info->CA_count].CA_system_id = (desc[0]<<8) |desc[1];
		info->CA_info[info->CA_count].CA_pid = ((desc[2]&0x1F)<<8) |desc[3];
		PP_PRINTF("PMT loop1 ca_desc_handler, ca_sysid=0x%x,pid=%d,CA_count=%d\n",
			info->CA_info[info->CA_count].CA_system_id,info->CA_info[info->CA_count].CA_pid,info->CA_count+1);

		info->CA_count++;
	}
	else
	{
		PP_PRINTF("%s: ca count %d full!\n",__FUNCTION__,info->CA_count);
	}
	return SI_SUCCESS;
}


static INT32 loop2_ca_desc_handle(UINT8 tag, UINT8 len, UINT8 *desc, void *priv)
{
	struct pmt_es_info *info = (struct pmt_es_info *)priv;

	if (info->cas_count < P_MAX_CA_NUM)
	{
		info->cas_sysid[info->cas_count]= (desc[0]<<8) |desc[1];
		info->cas_pid[info->cas_count] = ((desc[2]&0x1F)<<8) |desc[3];
		PP_PRINTF("PMT loop2 ca_desc_handler, ca_sysid=0x%x,pid=%d,CA_count=%d\n",
			info->cas_sysid[info->cas_count],info->cas_pid[info->cas_count],info->cas_count+1);
		info->cas_count++;
	}
	else
	{
		PP_PRINTF("%s: ca count %d full!\n",__FUNCTION__,info->cas_count);
	}
	return SI_SUCCESS;
}

static struct desc_table pmt_loop1[] = {
	{
		CA_DESCRIPTOR,
		ES_CA_EXIST_BIT,
		loop1_ca_desc_handle,
	}
};

/*
 * please pay attention that this array should be sorted by tag in ascend order.
 * other wise the desc_loop_parser won't work out!!!
 */
static struct desc_table pmt_loop2[] = {
	{	
		DTS_REGST_DESC, 
		ES_DTS_EXIST_BIT,
		NULL,
	},
	{
		CA_DESCRIPTOR,
		ES_CA_EXIST_BIT,
		loop2_ca_desc_handle,
	},
	{
		ISO_639_LANGUAGE_DESCRIPTOR, 
		ES_ISO639_EXIST_BIT,
		iso639_desc_handle,
	},
	{
		TELTEXT_DESCRIPTOR,
		ES_TTX_EXIST_BIT,
		NULL,
	},
	{
		SUBTITLE_DESCRIPTOR,
		ES_SUB_EXIST_BIT,
		NULL,
	},
	{
		AC3_DESCRIPTOR,
		ES_AC3_EXIST_BIT,
		NULL,
	},
	{
		ENHANCED_AC3_DESCRIPTOR,
		ES_EAC3_EXIST_BIT,
		NULL,
	},		
	{
		AAC_DESCRIPTOR,
		ES_AAC_EXIST_BIT,
		NULL,
	},
};

static BOOL append_audio_pid(struct pmt_es_info *es, PROG_INFO *info, UINT16 attr)
{
	BOOL ret;
	UINT32 audio_cnt;
	UINT8 j = 0;
	UINT8 *lang = es->lang;

	audio_cnt = info->audio_count;
	if(audio_cnt < P_MAX_AUDIO_NUM) {
#ifdef CONAX_AUDIO_LAN_USE
		if (lang[0]&&lang[1]&&lang[2]) {
			MEMCPY(info->audio_lang[audio_cnt], lang, 3);
			info->audio_pid[audio_cnt] = es->pid|attr;
			audio_cnt ++;
			info->audio_count = audio_cnt;
		} else {
			MEMSET(info->audio_lang[audio_cnt], 0, 3);
		}
		if (lang[3]&&lang[4]&&lang[5]) {
			MEMCPY(info->audio_lang[audio_cnt], (lang + 3), 3);
			info->audio_pid[audio_cnt] = es->pid|attr;
			audio_cnt ++;
			info->audio_count = audio_cnt;
		} else {
			MEMSET(info->audio_lang[audio_cnt], 0, 3);
		}
		if ((lang[0]&&lang[1]&&lang[2]) == 0)
		{
			info->audio_pid[audio_cnt] = es->pid|attr;
			info->audio_count = audio_cnt+1;
		}
#else
		info->audio_pid[audio_cnt] = es->pid|attr;
		if (lang[0]&&lang[1]&&lang[2]) {
			MEMCPY(info->audio_lang[audio_cnt], lang, 3);
		} else {
			MEMSET(info->audio_lang[audio_cnt], 0, 3);
		}
		

	#ifdef NL_BZ
		if (lang[3]&&lang[4]&&lang[5]) {
			MEMCPY(info->audio_lang[audio_cnt + 3], (lang + 3), 3);
		}
		else {
			MEMSET(info->audio_lang[audio_cnt + 3], 0, 3);
		}
	#endif				

		info->audio_count = audio_cnt+1;
	
	#ifdef AUDIO_CHANNEL_LANG_SUPPORT
		if ((info->audio_count < P_MAX_AUDIO_NUM) &&
			(lang[3] && lang[4] && lang[5]))
		{	// add one more audio pid for the channel language
			MEMCPY(info->audio_lang[info->audio_count], (lang + 3), 3);
			info->audio_pid[info->audio_count] = es->pid | attr;
			info->audio_count++;
		}
	#endif
#endif
		PP_PRINTF("+aud_pid=%d\n", es->pid);
		
		ret = TRUE;
	} else {
		PP_PRINTF("-aud_pid=%d\n", es->pid);
		ret = FALSE;
	}
	PP_PRINTF("lang code: %c%c%c\n", lang[0], lang[1], lang[2]);
	
	return ret;
}

/*
  * Name	: psi_parse_loop2
  * Description	: parse pmt second loop.
  * Parameter   : 
  *	INT8 *buff       :seciton buffer
  *     INT16 buff_len   :buffer length
  * Return        :
  */
static INT32 pmt_parse_loop2(UINT8 *pmt, UINT16 pmt_len, PROG_INFO *p_info,
	INT32 max_es_nr)
{
	INT32 ret, es_cnt;
	UINT32 i, loop_len;
	struct pmt_stream_info *info;
	struct pmt_es_info es;
	UINT32 loop1_len = ((pmt[10]<<8)|pmt[11])&0x0FFF;
	UINT16 invalid_flag = 1;
	UINT32 j=0;
	UINT8 k=0;

	PP_PRINTF("\n %s(): es stream :\n",__FUNCTION__);
	for(es_cnt=0, i = sizeof(struct pmt_section)-4+loop1_len;i < (UINT32)pmt_len-4;
		i += sizeof(struct pmt_stream_info)+loop_len)
	{
		info = (struct pmt_stream_info *)(pmt+i);
		MEMSET(&es, 0, sizeof(struct pmt_es_info));
		es.stream_type = info->stream_type;
		es.pid = SI_MERGE_HL8(info->elementary_pid);
		loop_len = SI_MERGE_HL8(info->es_info_length);	

		ret = desc_loop_parser(info->descriptor, loop_len, 
			pmt_loop2, ARRAY_SIZE(pmt_loop2), &es.stat, (void *)&es);

		if (ret != SI_SUCCESS)
			PP_PRINTF("%s: desc_loop_parser error, error= %d\n", ret);

		if (es.stat&ES_CA_EXIST)
		{
			//check if current es ca system is same with those recorded ones
			for(j=0;j<es.cas_count;j++)
			{
//fix BUG17636, to get all ecm pids
/*				for(k=0; k<p_info->CA_count; k++)
				{
					if(p_info->CA_info[k].CA_system_id==es.cas_sysid[j])
					{
						PP_PRINTF("es pid=%d, ca count=%d, cas_sysid[%d]=0x%x has same ca sysid with p_info->CA_info[%d]!\n"
						, es.pid, es.cas_count, j,es.cas_sysid[j], k);
						break;
					}
				}
				if( k==p_info->CA_count)*/
				{
					if (p_info->CA_count < P_MAX_CA_NUM)
					{
						p_info->CA_info[p_info->CA_count].CA_system_id = es.cas_sysid[j];
						p_info->CA_info[p_info->CA_count].CA_pid = es.cas_pid[j];
						p_info->CA_count++;
					}
					else
					{
						PP_PRINTF("pmt_parse_loop2: ca system id count %d full!\n",p_info->CA_count);
						break;
					}
				}
			}
		}

        PP_PRINTF("es stream type=%d\n",es.stream_type);
		switch(es.stream_type)
		{
			case MPEG1_AUDIO_STREAM:
			case MPEG2_AUDIO_STREAM:
				if (TRUE == append_audio_pid(&es, p_info, 0))
					invalid_flag = 0;
				break;
			case PRIVATE_DATA_STREAM:
			//case MPEG_AAC_STREAM:
			//case AC3_AUDIO_STREAM:
				if(es.stat&ES_TTX_EXIST)
					p_info->teletext_pid = es.pid;
				else if(es.stat&ES_SUB_EXIST)
					p_info->subtitle_pid = es.pid;
#ifdef AC3DEC
				else if((es.stat&ES_AC3_EXIST)&&(TRUE == append_audio_pid(&es, p_info,AC3_DES_EXIST)))
				{				
					//if(es.stream_type == AC3_AUDIO_STREAM)
					//p_info->prog_number +=1;
					invalid_flag = 0;
				}
                #if 1
				else if((es.stat&ES_EAC3_EXIST)&&(TRUE == append_audio_pid(&es, p_info,EAC3_DES_EXIST)))
				{				
					//if(es.stream_type == AC3_AUDIO_STREAM)
					//p_info->prog_number +=1;				
					invalid_flag = 0;
				} 
                #endif
#endif
				break;
#ifdef AC3DEC
            case AC3_AUDIO_STREAM:
                if(TRUE == append_audio_pid(&es, p_info,AC3_DES_EXIST))
                {				
                    invalid_flag = 0;
                }
                break;
            case MPEG_AAC_STREAM:
				if(TRUE == append_audio_pid(&es, p_info,AAC_DES_EXIST))
                {						
                    invalid_flag = 0;
                }
                break;
            case MPEG_ADTS_AAC_STREAM:
                if(TRUE==append_audio_pid(&es, p_info,ADTS_AAC_DES_EXIST))
                {
                    invalid_flag = 0;
                }
                break;
#endif
			case MPEG1_VIDEO_STREAM:
			case MPEG2_VIDEO_STREAM:
			case MPEG4_VIDEO_STREAM:
	        	if (p_info->video_pid == PSI_STUFF_PID)
				{
					p_info->av_flag = 1;
					p_info->video_pid = es.pid;
					invalid_flag = 0;
				}
				break;
       		case H264_VIDEO_STREAM:
	        	if (p_info->video_pid == PSI_STUFF_PID) 
                {
				    p_info->av_flag = 1;
				    p_info->video_pid = es.pid|H264_VIDEO_PID_FLAG;
				    invalid_flag = 0;
			    }
                break;


#if (SYS_PROJECT_FE == PROJECT_FE_DVBC)	

			//case ISO13818_6_TYPE_A:
			case 0x05:
			case ISO13818_6_TYPE_B:
			//case ISO13818_6_TYPE_C:
			//case ISO13818_6_TYPE_D:
					invalid_flag = 0;
				break;

#endif

			default:
				PP_PRINTF("es_type = 0x%x unknown!\n",__FUNCTION__, es.stream_type);
				break;
		}
		if (++es_cnt>=max_es_nr)
		{
			PP_PRINTF("es_cnt = %d exceed max_es_nr!\n",__FUNCTION__, es_cnt);
			break;
		}

	}

	PP_PRINTF("%s: es_cnt = %d!\n\n",__FUNCTION__, es_cnt);
	return invalid_flag? ERR_NO_PROG_INFO : SI_SUCCESS;;
}


INT32 psi_pmt_parser(UINT8 *pmt, PROG_INFO *p_info, INT32 max_es_nr)
{
	INT32 ret;
	UINT32 stat = 0;
	UINT16 len = ((pmt[1]<<8)|pmt[2])&0x0FFF;

	p_info->pcr_pid = ((pmt[8]<<8)|pmt[9])&PSI_STUFF_PID;
	p_info->video_pid = PSI_STUFF_PID;
	p_info->service_type = 0;
	p_info->pmt_version = (pmt[5]&0x3e)>>1;
	p_info->audio_count = 0;
	
	desc_loop_parser(pmt+12, ((pmt[10]<<8)|pmt[11])&0xFFF, 
		pmt_loop1, 1, &stat, p_info);
	
	ret = pmt_parse_loop2(pmt, len, p_info, max_es_nr);
	if(p_info->video_pid == PSI_STUFF_PID)
		p_info->video_pid = 0;
	PP_PRINTF("PMT parse ret=%d, prog num=%d, pmt pid=%d,ca count=%d,av flag=%d\n", ret,p_info->prog_number,
		p_info->pmt_pid, p_info->CA_count,p_info->av_flag);
	
	return ret;
}

#ifdef LIB_TSI3_FULL

UINT8 ecm_count = 0;
static UINT32 psi_pmt_crc_value = 0;
UINT16 ecm_pid_array[32];
void psi_pmt_get_ecm_pid(UINT8 *section, INT32 length, UINT16 param)
{
	UINT16 prog_info_len,section_len,es_info_len;
	UINT16 left_len;
	UINT8 *discriptor_pointer = NULL;
	UINT8 *discriptor_pointer2 = NULL;
	UINT8 loop_length;
	UINT16 ecm_pid = 0;
	UINT32 crc_value;
	
	section_len = ((section[1] & 0x03) << 8)|section[2];

	crc_value = (section[section_len-1] |(section[section_len]<<8)|(section[section_len+1]<<16)|(section[section_len+2]<<24));
	//pmt data not change
	if((crc_value == psi_pmt_crc_value) && (ecm_count != 0))
	{
		//DYN_PID_PRINTF("PMT section content not change, return!");
		return;
	}

	ecm_count = 0;
	
	prog_info_len = ((section[10] & 0x03) << 8)|section[11];
	left_len = section_len -9 -prog_info_len -4;

	
	discriptor_pointer = &section[12];
	discriptor_pointer2 = discriptor_pointer + prog_info_len;

	//first loop, prog_info_length
	//DYN_PID_PRINTF("		parsing pmt loop1:\n");
	while (prog_info_len > 0)
	{
		/*the CA descriptor tag*/
		if (*discriptor_pointer == 0x09)
		{
			ecm_pid = ((discriptor_pointer[4]&0x1F)<<8)|discriptor_pointer[5];
			//check card exist and match or not

			psi_pmt_crc_value = crc_value;
			if(ecm_count<32)
			{
				ecm_pid_array[ecm_count] = ecm_pid;
				ecm_count++;
			}
			else
				libc_printf("ecm_count: %d, count > 32 \n",ecm_count);

		}
		prog_info_len -= 2+discriptor_pointer[1];
		discriptor_pointer += 2+discriptor_pointer[1];
	}
	discriptor_pointer = discriptor_pointer2;

	while(left_len > 0)
	{
		es_info_len = ((discriptor_pointer[3]&0x03)<<8)  | discriptor_pointer[4];
		loop_length = es_info_len;
		discriptor_pointer += 5;
		
		while(loop_length > 0)
		{
			/*the CA descriptor tag*/
			if (*discriptor_pointer == 0x09)
			{
				ecm_pid = ((discriptor_pointer[4]&0x1F)<<8)|discriptor_pointer[5];
				psi_pmt_crc_value = crc_value;

				if(ecm_count<32)
				{
					ecm_pid_array[ecm_count] = ecm_pid;
					ecm_count++;
				}
				else
					libc_printf("ecm_count: %d, count > 32 \n",ecm_count);
			}
			loop_length -= 2+discriptor_pointer[1];
			discriptor_pointer += 2+discriptor_pointer[1];
		}
		left_len -= 5+es_info_len;
	}
	
	return;
}


//#endif
#endif

