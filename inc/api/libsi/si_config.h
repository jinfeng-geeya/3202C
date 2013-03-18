#ifndef __SI_CONFIG_H__
#define __SI_CONFIG_H__

#ifdef SI_MODULE_CONFIG
/*SI module function config*/

/* NIT */
#define _SERVICE_ATTRIBUTE_ENABLE_		//parse service attribute descriptor
#define INVALID_PRIVATE_DATA_SPEC  0xFFFFFFFF
#ifndef PRIVATE_DATA_SPEC 
#define PRIVATE_DATA_SPEC 0x0000233a    //-0x0000233a for UK-DTG , 0x00000028 for France HD-TNT
#endif
//#define _LCN_ENABLE_		//parse logical channel number descriptor
#ifndef _LCN_ENABLE_
#define _LCN_ENABLE_		//parse logical channel number descriptor
#endif

/********************************************
           UK DTT spec
********************************************/
#define DTG_CONTENT		//dtg content descriptor is deferent with common spec
#define DTG_AUDIO_SELECT	//dtg iso639 descriptor may have more than one sound track. eg: left,right
#define DTG_LCN_ASSIGN		//locate undefined LCN according to DTG
#define DTG_SUB_SELECT		//DTG select subtitle according to subtitle type
#define DTG_CHAR			//proccess nil, SHY
//#define DTG_SAME_TS_PROCCESS	//present only one if origin_network_id/TS_id is same
//#define DTG_SAME_SERVICE_PROCCESS	//present only one if origin_network_id/service_id is same
//#define DTG_REGIONAL_VARIANT	//migrate one to next highest LCN if origin_network_id/LCN is same

#endif


//20110623, add si search config
//support SDT other ts table when search program
//#define PSI_SEARCH_SDT_OTH_SUPPORT

//support BAT in si seach.
//#define PSI_SEACH_BAT_SUPPORT


#endif /* __SI_MODULE_H__ */

