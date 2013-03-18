#ifndef __SI_SECTION_H__
#define __SI_SECTION_H__
#include <types.h>
#include <sys_config.h>

/**
 * name		: section_header
 * description	: DVB spec defined section header structure, 8 byte.
 */
struct section_header {
	UINT8 table_id;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 section_syntax_indicator		: 1;
	UINT8 reserved1				: 1;
	UINT8 reserved_future_use1		: 2;
	UINT8 section_length_hi			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 section_length_hi			: 4;
	UINT8 reserved_future_use1		: 2;
	UINT8 reserved1				: 1;
	UINT8 section_syntax_indicator		: 1;
#else
#error "please check marco SYS_CPU_ENDIAN in <sys_config.h>"
#endif
	UINT8 section_length_lo;
	UINT8 specific_id[2];
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved2				: 2;
	UINT8 version_number			: 5;
	UINT8 current_next_indicator		: 1;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 current_next_indicator		: 1;
	UINT8 version_number			: 5;
	UINT8 reserved2				: 2;
#endif
	UINT8 section_number;
	UINT8 last_section_number;
}__attribute__((packed));

struct pat_stream_info {
	UINT8 program_number[2];
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
	UINT8 reserved				: 3;
	UINT8 pid_hi				: 5;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 pid_hi				: 5;
	UINT8 reserved				: 3;
#endif
	UINT8 pid_lo;
};

struct pmt_stream_info {
	UINT8 stream_type;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
	UINT8 reserved1				: 3;
	UINT8 elementary_pid_hi			: 5;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 elementary_pid_hi			: 5;
	UINT8 reserved1				: 3;
#endif
	UINT8 elementary_pid_lo;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
	UINT8 reserved2				: 4;
	UINT8 es_info_length_hi			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 es_info_length_hi			: 4;
	UINT8 reserved2				: 4;
#endif	
	UINT8 es_info_length_lo;
	UINT8 descriptor[0];
}__attribute__((packed));

struct sdt_stream_info {
	UINT8 service_id[2];
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved_future_use		: 6;
	UINT8 EIT_schedule_flag			: 1;
	UINT8 EIT_present_following_flag	: 1;
	UINT8 running_status			: 3;
	UINT8 free_ca_mode			: 1;
	UINT8 descriptor_loop_length_hi		: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 EIT_present_following_flag	: 1;
	UINT8 EIT_schedule_flag			: 1;
	UINT8 reserved_future_use		: 6;
	UINT8 descriptor_loop_length_hi		: 4;
	UINT8 free_ca_mode			: 1;
	UINT8 running_status			: 3;
#endif
	UINT8 descriptor_loop_length_lo;
	UINT8 descriptor[0];
}__attribute__((packed));


struct sch_time {
	UINT8 sch_time_start[5];
	UINT8 sch_time_duration[3];
}__attribute__((packed));


struct SDTT_DL_CONT_DESC_1
{
	UINT8 desc_tag;
	UINT8 desc_len;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reboot			:1;
	UINT8 add_on			:1;
	UINT8 compab_flag	:1;
	UINT8 mod_inf_flag	:1;
	UINT8 txt_inf_flag		:1;
	UINT8 reserved1		:3;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 reserved1		:3;
	UINT8 txt_inf_flag		:1;
	UINT8 mod_inf_flag	:1;
	UINT8 compab_flag	:1;
	UINT8 add_on			:1;
	UINT8 reboot			:1;
#endif	
	UINT8 comp_size[4];
	UINT8 download_id[4];
	UINT8 time_out_DII[4];
	UINT8 leak_rate_hi;
	UINT8 leak_rate_md;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 leak_rate_lo		:6;
	UINT8 reserved2		:2;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 reserved2		:2;
	UINT8 leak_rate_lo		:6;
#endif	
	UINT8 comp_tag;
}__attribute__((packed));

//if compab_flag==1
struct SDTT_COMPAB_DESC_1
{
	UINT8 desc_len_hi;
	UINT8 desc_len_lo;
	UINT8 desc_cnt_hi;
	UINT8 desc_cnt_lo;	
}__attribute__((packed));


struct SDTT_COMPAB_DESC_2
{
	UINT8 desc_type;
	UINT8 desc_len;
	UINT8 specify_type;
	UINT8 specify_data[3];
	UINT8 module_hi;
	UINT8 module_lo;
	UINT8 version_hi;
	UINT8 version_lo;
	UINT8 sub_desc_cnt;
}__attribute__((packed));


//if mod_inf_flag==1
struct SDTT_DL_CONT_MOD	//modules
{
	UINT16 mod_id;
	UINT32 mod_size;
	UINT8 mod_inf_len;
	UINT8 mod_inf_dat[0];
}__attribute__((packed));


struct sdtt_content {
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 group				: 4;
	UINT8 target_ver_hi		: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 target_ver_hi		: 4;
	UINT8 group				: 4;
#endif
	UINT8 target_ver_lo		    ;
	UINT8 new_ver_hi		    	    ;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 new_ver_lo		   	: 4; 
	UINT8 download_level		: 2;
	UINT8 ver_indicator		: 2;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 ver_indicator		: 2;
	UINT8 download_level		: 2;
	UINT8 new_ver_lo		   	: 4; 
#endif
	UINT8 content_ds_len_hi	    ;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 content_ds_len_lo    	: 4;
	UINT8 reserved			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 reserved			: 4;
	UINT8 content_ds_len_lo    	: 4;
#endif
	UINT8 schedule_ds_len_hi         ;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 schedule_ds_len_lo    : 4;
	UINT8 schedule_t_shift		: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 schedule_t_shift		: 4;
	UINT8 schedule_ds_len_lo    : 4;
#endif
	//struct sch_time schdu_time[0];
	//UINT8 descriptor[0];
}__attribute__((packed));

struct transport_stream_info {
	UINT8 transport_stream_id[2];
	UINT8 original_network_id[2];
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
	UINT8 reserved_future_use		: 4;
	UINT8 transport_stream_length_hi	: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 transport_stream_length_hi	: 4;
	UINT8 reserved_future_use		: 4;
#endif
	UINT8 transport_stream_length_lo;
	UINT8 descriptor[0];
}__attribute__((packed));

struct network_descriptor {
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
	UINT8 reserved_future_use		: 4;
	UINT8 network_descriptor_length_hi	: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 network_descriptor_length_hi	: 4;
	UINT8 reserved_future_use		: 4;
#endif
	UINT8 network_descriptor_length_lo;
	UINT8 descriptor[0];
}__attribute__((packed));

struct bouquet_descriptor {
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved_future_use		: 4;
	UINT8 bouquet_descriptors_length_hi	: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 bouquet_descriptors_length_hi	: 4;
	UINT8 reserved_future_use		: 4;
#endif
	UINT8 bouquet_descriptors_length_lo;
	UINT8 descriptor[0];
}__attribute__((packed));

struct event_info {
	UINT8 event_id[2];
	UINT8 start_time[5];
	UINT8 duration[3];
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 running_status			: 3;
	UINT8 free_CA_mode			: 1;
	UINT8 event_descriptors_loop_length_hi	: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 event_descriptors_loop_length_hi	: 4;
	UINT8 free_CA_mode			: 1;
	UINT8 running_status			: 3;
#endif
	UINT8 event_descriptors_loop_length_lo;
	UINT8 descriptor[0];
}__attribute__((packed));

struct pat_section {
	struct section_header pat_sh;
	struct pat_stream_info s_info[0];
	UINT8 crc32[4];
};

struct pmt_section {
	struct section_header pmt_sh;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
	UINT8 reserved1				: 3;
	UINT8 pcr_pid_hi			: 5;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 pcr_pid_hi			: 5;
	UINT8 reserved1				: 3;
#endif
	UINT8 pcr_pid_lo;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)
	UINT8 reserved2				: 4;
	UINT8 program_info_length_hi		: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 program_info_length_hi		: 4;
	UINT8 reserved2				: 4;
#endif	
	UINT8 program_info_length_lo;
	UINT8 p_info[0];
	struct pmt_stream_info s_info[0];
	UINT8 crc32[4];
}__attribute__((packed));

struct sdt_section {
	struct section_header sh;
	UINT8 original_network_id[2];
	UINT8 reserved_future_use;
	struct sdt_stream_info info[0];
	UINT8 crc32[4];
}__attribute__((packed));

struct nit_section {
	struct section_header sh;
	struct network_descriptor network;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved_future_use		: 4;
	UINT8 transport_stream_loop_length_hi	: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)	
	UINT8 transport_stream_loop_length_hi	: 4;
	UINT8 reserved_future_use		: 4;
#endif	
	UINT8 transport_stream_loop_length_lo;
	struct transport_stream_info info[0];
	UINT8 crc32[4];
}__attribute__((packed));

struct bat_section {
	struct section_header sh;
	struct bouquet_descriptor bouquet;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved_future_use		: 4;
	UINT8 transport_stream_loop_length_hi	: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)	
	UINT8 transport_stream_loop_length_hi	: 4;
	UINT8 reserved_future_use		: 4;
#endif	
	UINT8 transport_stream_loop_length_lo;
	struct transport_stream_info info[0];
	UINT8 crc32[4];
}__attribute__((packed));

struct eit_section {
	struct section_header sh;
	UINT8 transport_stream_id[2];
	UINT8 original_network_id[2];
	UINT8 segment_last_section_number;
	UINT8 last_table_id;
	struct event_info info[0];
	UINT8 crc32[4];
}__attribute__((packed));

struct tdt_section {
	UINT8 table_id;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 section_syntax_indicator		: 1;
	UINT8 reserved_future_use		: 1;
	UINT8 reserved				: 2;
	UINT8 section_length_hi			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 section_length_hi			: 4;
	UINT8 reserved				: 2;
	UINT8 reserved_future_use		: 1;
	UINT8 section_syntax_indicator		: 1;
#endif
	UINT8 section_length_lo;
	UINT8 UTC_time[5];
};

struct tot_section {
	UINT8 table_id;
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 section_syntax_indicator		: 1;
	UINT8 reserved_future_use		: 1;
	UINT8 reserved				: 2;
	UINT8 section_length_hi			: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 section_length_hi			: 4;
	UINT8 reserved				: 2;
	UINT8 reserved_future_use		: 1;
	UINT8 section_syntax_indicator		: 1;
#endif
	UINT8 section_length_lo;
	UINT8 UTC_time[5];
#if (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 reserved2				: 4;
	UINT8 descriptor_loop_length_hi		: 4;
#elif (SYS_CPU_ENDIAN == ENDIAN_LITTLE)
	UINT8 descriptor_loop_length_hi		: 4;
	UINT8 reserved2				: 4;
#endif
	UINT8 descriptor_loop_lenght_lo;
	UINT8 descriptor[0];
	UINT8 crc32[4];
};


struct sdtt_section {
	struct section_header sdtt_sh;
	UINT8 stream_id[2];
	UINT8 network_id[2];
	UINT8 service_id[2];
	UINT8 numer_of_content;
	//struct sdtt_content sdtt_cont[0];
	//UINT8 crc32[4];
};

/**
 * name		: SI_MERGE_xxxx
 * description	: a set of MARCO for combine multi-UINT8/bitfield value into integers values.
 */
#define SI_MERGE_HL4(exp)		((UINT16)((exp##_hi<<4)|exp##_lo))
#define SI_MERGE_HL8(exp)		((UINT16)((exp##_hi<<8)|exp##_lo))

static __inline__ UINT16 SI_MERGE_UINT16(UINT8 *ptr)
{
	return (UINT16)((ptr[0]<< 8)|ptr[1]);
}

static __inline__ UINT32 SI_MERGE_UINT24(UINT8 *ptr)
{
	return (UINT32)((ptr[0]<<16)|(ptr[1]<< 8)|ptr[2]);
}

static __inline__ UINT32 SI_MERGE_UINT32(UINT8 *ptr)
{
	return (UINT32)((ptr[0]<<24)|(ptr[1]<<16)|(ptr[2]<<8)|ptr[3]);
}

#endif /* __SI_SECTION_H__ */
