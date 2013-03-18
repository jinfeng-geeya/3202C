#ifndef __UPG_COMMON_H__
#define __UPG_COMMON_H__

struct IDNavi
{
	UINT8 ID;
	UINT8 upID;
	UINT8 downID;
	UINT8 leftID;
	UINT8 rightID;	
};

#ifndef UPG_PRINTF
#define UPG_PRINTF PRINTF
#endif

#endif //__UPG_COMMON_H__
