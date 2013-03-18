/*
 * Copyright (C) ALi Shanghai Corp. 2006
 *
 * si_descriptor.h
 *
 * defines the descriptor syntax related marcos and inline functions.
 */
#ifndef __SI_DESCRIPTOR_H__
#define __SI_DESCRIPTOR_H__
#include <types.h>
#include <sys_config.h>

enum descriptors_syntax {
	/* ISO 13818-1 defined descriptors */
/*	
	descriptor_tag	TS	PS	Identification
	0	n/a	n/a	Reserved
	1	n/a	n/a	Reserved
	2	X	X	video_stream_descriptor
	3	X	X	audio_stream_descriptor
	4	X	X	hierarchy_descriptor
	5	X	X	registration_descriptor
	6	X	X	data_stream_alignment_descriptor
	7	X	X	target_background_grid_descriptor
	8	X	X	video_window_descriptor
	9	X	X	CA_descriptor
	10	X	X	ISO_639_language_descriptor
	11	X	X	system_clock_descriptor
	12	X	X	multiplex_buffer_utilization_descriptor
	13	X	X	copyright_descriptor
	14	X		maximum bitrate descriptor
	15	X	X	private data indicator descriptor
	16	X	X	smoothing buffer descriptor
	17	X		STD_descriptor
	18	X	X	IBP descriptor
	19-63	n/a	n/a	ITU-T Rec. H.222.0 | ISO/IEC 13818-1 Reserved
	64-255	n/a	n/a	User Private
*/
	VIDEO_STREAM_DESCRIPTOR			= 0x02,
	AUDIO_STREAM_DESCRIPTOR			= 0x03,
	HIERACHY_DESCRIPTOR			= 0x04,
	REGISTRATION_DESCRIPTOR			= 0x05,
	DTS_REGISTRATION_DESCRIPTOR		= 0x05,
	DATA_STREAM_ALIGNMENT_DESCRIPTOR	= 0x06,
	TARGET_BACKGROUND_GRID_DESCRIPTOR	= 0x07,
	VIDEO_WINDOW_DESCRIPTOR			= 0x08,
	CA_DESCRIPTOR				= 0x09,
	ISO_639_LANGUAGE_DESCRIPTOR		= 0x0A,
	SYSTEM_CLOCK_DESCRIPTOR			= 0x0B,
	MULTIPLEX_BUFFER_UTILIZATION_DESCRIPTOR	= 0x0C,
	COPYRIGHT_DESCRIPTOR			= 0x0D,
	MAXIMUM_BITRATE_DESCRIPTOR		= 0x0E,
	PRIVATE_DATA_INDICATOR_DESCRIPTOR	= 0x0F,
	SMOOTHING_BUFFER_DESCRIPTOR		= 0x10,
	STD_DESCRIPTOR				= 0x11,
	IBP_DESCRIPTOR				= 0x12,
	/* ISO 13818-6 defined descriptors */
	
	/* EN 300 468 defined descriptors		  NIT, BAT, SDT, EIT, TOT, PMT, SIT	*/
	NETWORK_NAME_DESCRIPTOR			= 0x40, /* Y					*/
	SERVICE_LIST_DESCRIPTOR			= 0x41,	/* Y	Y				*/
	STUFFING_DESCRIPTOR			= 0x42, /* Y	Y    Y	  Y		 Y	*/
	SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR	= 0x43, /* Y					*/
	CABLE_DELIVERY_SYSTEM_DESCRIPTOR	= 0x44, /* Y					*/
	VBI_DATA_DESCRIPTOR			= 0x45, /* 			    Y		*/
	VBI_TELETEXT_DESCRIPTOR			= 0x46, /* 			    Y		*/
	BOUQUET_NAME_DESCRIPTOR			= 0x47, /*	Y    Y			 Y	*/
	SERVICE_DESCRIPTOR			= 0x48, /*	     Y			 Y	*/
	COUNTRY_AVAILABILITY_DESCRIPTOR		= 0x49, /* 	Y    Y			 Y	*/
	LINKAGE_DESCRIPTOR			= 0x4A,	/* Y	Y    Y	  Y		 Y	*/
	NVOD_REFERENCE_DESCRIPTOR		= 0x4B,	/*	     Y	  		 Y	*/
	TIME_SHIFTED_SERVICE_DESCRIPTOR		= 0x4C, /*	     Y			 Y	*/
	SHORT_EVENT_DESCRIPTOR			= 0x4D,	/*		  Y		 Y	*/
	EXTENDED_EVENT_DESCRIPTOR		= 0x4E,	/*		  Y		 Y	*/
	TIME_SHIFTED_EVENT_DESCRIPTOR		= 0x4F, /*		  Y		 Y	*/
	COMPONENT_DESCRIPTOR			= 0x50,	/*		  Y		 Y	*/
	MOSAIC_DESCRIPTOR			= 0x51,	/*	     Y		    Y	 Y	*/		
	STREAM_IDENTIFIER_DESCRIPTOR		= 0x52,	/*			    Y		*/
	CA_IDENTIFIER_DESCRIPTOR		= 0x53,	/*	Y    Y	  Y		 Y	*/
	CONTENT_DESCRIPTOR			= 0x54, /*		  Y		 Y	*/
	PARENTAL_RATING_DESCRIPTOR		= 0x55, /*		  Y		 Y	*/
	TELTEXT_DESCRIPTOR			= 0x56,	/*			    Y		*/
	TELEPHONE_DESCRIPTOR			= 0x57,	/*	     Y	  Y		 Y	*/
	LOCAL_TIME_OFFSET_DESCRIPTOR		= 0x58,	/* 			Y		*/
	SUBTITLE_DESCRIPTOR			= 0x59,	/* 			    Y		*/
	TERRESTRIAL_DELIEVERY_SYSTEM_DESCRIPTOR	= 0x5A,	/* Y					*/
	MULTILINGUAL_NETWORK_NAME_DESCRIPTOR	= 0x5B,	/* Y					*/
	MULTILINGUAL_BOUQUET_NAME_DESCRIPTOR	= 0x5C,	/*	Y				*/
	MULTILINGUAL_SERVICE_NAME_DESCRIPTOR	= 0x5D, /**/
	MULTILINGUAL_COMPONENT_DESCRIPTOR	= 0x5E,
	PRIVATE_DATA_SPECIFIER_DESCRIPTOR	= 0x5F,
	SERVICE_MOVE_DESCRIPTOR			= 0x60,
	SHORT_SMOOTHING_BUFFER_DESCRIPTOR	= 0x61,
	FREQUENCY_LIST_DESCRIPTOR		= 0x62,
	PARTIAL_TRANSPORT_STREAM_DESCRIPTOR	= 0x63,
	DATA_BROADCAST_DESCRIPTOR		= 0x64,
	SCRAMBLING_DESCRIPTOR			= 0x65,
	DATA_BROADCAST_ID_DESCRIPTOR		= 0x66,
	TRANSPORT_STREAM_DESCRIPTOR		= 0x67,
	DSNG_DESCRIPTOR				= 0x68,
	PDC_DESCRIPTOR				= 0x69,
	AC3_DESCRIPTOR				= 0x6A,
	ANCILLARY_DATA_DESCRIPTOR		= 0x6B,
	CELL_LIST_DESCRIPTOR			= 0x6C,
	CELL_FREQUENCY_LINK_DESCRIPTOR		= 0x6D,
	ANNOUNCEMENT_SUPPORT_DESCRIPTOR		= 0x6E,
	APPLICATION_SIGNALLING_DESCRIPTOR	= 0x6F,
	ADAPTATION_FIELD_DATA_DESCRIPTOR	= 0x70,
	SERVICE_IDENTIFIER_DESCRIPTOR		= 0x71,
	SERVICE_AVAILABILITY_DESCRIPTOR		= 0x72,
	DEFAULT_AUTHORITY_DESCRIPTOR		= 0x73,
	RELATED_CONTENT_DESCRIPTOR		= 0x74,
	TVA_IDE_DESCRIPTOR			= 0x75,
	CONTENT_IDENTIFIER_DESCRIPTOR		= 0x76,
	TIME_SLICE_FEC_IDENTIFIER_DESCRIPTOR	= 0x77,
	ECM_REPETITION_RATE_DESCRIPTOR		= 0x78,
	S2_SATELLITE_DELIVERY_SYSTEM_DESCRIPTOR	= 0x79,
	ENHANCED_AC3_DESCRIPTOR			= 0x7A,
	DTS_DESCRIPTOR				= 0x7B,
	AAC_DESCRIPTOR				= 0x7C,
	
	EXTENSION_DESCRIPTOR			= 0x7F,

	/* EN 101 154 defined descriptor */
	
	DTS_AUDIO_DESCRIPTOR			= 0x73,
	/* user defined descriptors 0x80-0xFE */
	/***************************
	 *** in UK DTT ***
	 ***************************/
	PREFERRED_NAME_LIST_DESCRIPTOR		= 0x84,
	SERVICE_ATTRIBUTE_DESCRIPTOR            = 0x86,
	SHORT_SERVICE_NAME_DESCRIPTOR		= 0x87,
	/***************************
	 *** in australian DVB-T ***
	 ***************************/
#if 0//(SYS_PROJECT == PROJECT_HDTV)
	LOGICAL_CHANNEL_NUMBER_DESCRIPTOR	= 0x90,
#else
	LOGICAL_CHANNEL_NUMBER_DESCRIPTOR	= 0x83, 
#endif
	/************************
	 *** in chengdu DVB-C ***
	 ************************/
	CHANNEL_ORDER_DESCRIPTOR		= 0x82,	
	CHANNEL_VOLUME_COMPENSATING_DESCRIPTOR	= 0x83, 

	/* LCN descriptor - Jilin */
	LCN_DESCRIPTOR_JILIN = 0x90,
	
};

struct dvb_descriptor {
	UINT8 descriptor_tag;
	UINT8 descriptor_length;
	UINT8 descriptor[0];
}__attribute__((packed));

/*
 * ripped descriptor format
 */
 
struct service_descriptor_ripped {
	UINT8 service_type;
	UINT8 service_provider_name_length;
	UINT8 service_provider_name[0];
	UINT8 service_name_length;
	UINT8 service_name[0];
}__attribute__((packed));

struct satellite_delivery_system_descriptor_ripped {
	UINT8 frequency[4];
	UINT8 orbital_position[2];
#if (SYS_CPU_ENDIAN == ENDIAN_LITTLE)	
	UINT8 modulation			: 5;
	UINT8 polarization			: 2;
	UINT8 west_east_flag			: 1;
#elif (SYS_CPU_ENDIAN == ENDIAN_BIG)	
	UINT8 west_east_flag			: 1;
	UINT8 polarization			: 2;
	UINT8 modulation			: 5;
#endif	
	UINT8 symbol_rate[3];
	UINT8 FEC_inner				: 4;
	UINT8 symbol_rate_low			: 4;
}__attribute__((packed));

typedef INT32 (*on_descriptor_t)(UINT8 tag, INT32 length, UINT8 *content, void *priv);

struct descriptor_info {
	UINT8 descriptor_tag;
	UINT8 mask_len;
	UINT8 *mask;
	UINT8 *value;
	void *priv;
	on_descriptor_t on_descriptor;
};

INT32 si_parse_content(UINT8 *data, INT32 length, struct descriptor_info *info, INT32 nr);

#endif /* __SI_DESCRIPTOR_H__ */
