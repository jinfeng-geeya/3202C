#ifndef __STB_HWINFO_H__
#define __STB_HWINFO_H__

#define STB_HWINFO_BASE_ADDR	(0x1E000)
#define STB_HWINFO_SERIAL_OFF	(0)
#define STB_HWINFO_SN_LEN 				26
#define STB_HWINFO_MAC_LEN 			6
#define STB_HWINFO_MAC_OFF	(STB_HWINFO_SERIAL_OFF+STB_HWINFO_SN_LEN)

#define STB_HWINFO_OUI_OFF		(STB_HWINFO_MAC_OFF+STB_HWINFO_MAC_LEN)
#define STB_HWINFO_HWMODEL_OFF	(STB_HWINFO_OUI_OFF+4)
#define STB_HWINFO_HWVERSION_OFF	(STB_HWINFO_HWMODEL_OFF+4)

#define STB_HWINFO_LEN	(STB_HWINFO_HWVERSION_OFF+4)

#define COM_MONITOR_CHECK_PAUSE	0X1
#define COM_MONITOR_CHECK_STBID	0X2

typedef struct{    
	unsigned char local_code[4];    
	unsigned char factory_code[2];    
	unsigned char batch_number[3];    
	unsigned char stb_type[2];    
	unsigned char hw_version[2];    
	unsigned char desc_type;    
	unsigned char desc[12];
}STB_SERIAL;

typedef struct{    
	unsigned char stb_serial[STB_HWINFO_SN_LEN];
	unsigned char stb_mac[STB_HWINFO_MAC_LEN];
	unsigned int stb_oui;
	unsigned int  stb_hw_model;
	unsigned int  stb_hw_version;
}STB_HWINFO;

INT8 get_stb_hwinfo(UINT32 *oui, UINT32 *hw_model, UINT32 *hw_version);
INT8 get_stb_serial(UINT8* serial);
INT8 get_stb_mac(UINT8* mac);

#endif

