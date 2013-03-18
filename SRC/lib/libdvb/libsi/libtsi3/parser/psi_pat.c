#include <types.h>
#include <sys_config.h>

#include <api/libtsi/si_types.h>
#include <api/libtsi/si_section.h>

#include <api/libsi/si_module.h>
#include <api/libsi/psi_pat.h>

#define PP_DEBUG_LEVEL			0
#if (PP_DEBUG_LEVEL>0)
#include <api/libc/printf.h>
#define PP_PRINTF			libc_printf
#else
#define PP_PRINTF(...)			do{}while(0)
#endif

INT32 psi_pat_parser(UINT8 *section, INT32 length, 
	struct section_parameter *param)
{
	INT32 i, k;
	UINT16 program_number;
	struct pat_stream_info *info;
	struct program_map *maps;
	struct pat_section_info *s_info = (struct pat_section_info *)param->priv;

	if (section==NULL) {
		s_info->map_counter = 0;
		return SI_SUCCESS;
	} 

	if (0 == section[6]) {
		s_info->ts_id = (section[3]<<8)|section[4];
	}
	
	maps = s_info->map;
	PP_PRINTF("\nPAT parse:\n");
    PP_PRINTF("ts_id=%d\n",s_info->ts_id);
	for(i=sizeof(struct pat_section)-PSI_SECTION_CRC_LENGTH;
	    i<length-PSI_SECTION_CRC_LENGTH;
	    i+= sizeof(struct pat_stream_info)) {
	    	info = (struct pat_stream_info *)(section+i);
		program_number = SI_MERGE_UINT16(info->program_number);
		
		if (program_number) { /* program PMT PID */
			for(k=0; k<s_info->map_counter; k++) {
				if (maps[k].pm_number == program_number) {
					PP_PRINTF("%s: pgnumber[%d] duplicate at pos %d!\n",
						__FUNCTION__, program_number, k);
					break;
				}
			}
			
			if (k==s_info->map_counter) {
				maps[k].pm_number = program_number;
				maps[k].pm_pid = SI_MERGE_HL8(info->pid);

				PP_PRINTF("pmt pid[%d], program number[%d]\n",
					 maps[k].pm_pid, maps[k].pm_number);

				if (++s_info->map_counter>=s_info->max_map_nr) {
					PP_PRINTF("%s: total prog cnt[%d]>=max cnt[%d]!\n",
						__FUNCTION__, s_info->map_counter,
						s_info->max_map_nr);
					return SI_SBREAK;
				}
			}
		} else { /* NIT PID value */
			s_info->nit_pid = SI_MERGE_HL8(info->pid);
		}
	}
	PP_PRINTF("%s: parse end!\n", __FUNCTION__);
	return SI_SUCCESS;
}

