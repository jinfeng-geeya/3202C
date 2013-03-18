#include <sys_config.h>
#include <sys_parameters.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>

#include <hld/pan/pan_dev.h>
#include <hld/osd/osddrv_dev.h>
#include <hld/osd/osddrv.h>

#include <api/libloader/ld_osd_lib.h>
#include <api/libchunk/chunk.h>

#include "boot_osd_rsc.h"
#include "dev_handle.h"
#include "../copper_common/boot_system_data.h"
#include "../copper_common/stb_data.h"

#define MULTI_MENU_LANG 1
#include "resource/font/fonts_bak_array.h"
#include "resource/string_tab/strings_array.h"

#define RSC_PRINTF(...)

UINT8 *font_table = NULL;

font_array_t font_array[10];
UINT32 font_array_size=0;

#define FONT_MAX_SIZE 1
font_array_t *font_asc[FONT_MAX_SIZE];
font_array_t *font_chn[FONT_MAX_SIZE];
font_array_t *cur_font;

// font array init
static BOOL OSDGetFontarray(UINT8 **table, font_array_t *array)
{
	UINT8 *image = (UINT8 *)*table;
	UINT32 size,i,j;
	
	font_array_size = *(int*)(&image[0]);
	size = *(int*)(&image[4]);

	if(sizeof(font_array_t) != size)
		return FALSE;

	for(i=0; i<font_array_size; i++)
	{
		MEMCPY(&array[i], &image[8+i*size], size);
		if(MEMCMP(array[i].lang,"asc",3)==0)
		{
			for(j=0; j<FONT_MAX_SIZE; j++)
				font_asc[j] = &array[i];
		}
		else 	if(MEMCMP(array[i].lang,"chn",3)==0)
		{
			for(j=0; j<FONT_MAX_SIZE; j++)
				font_chn[j] = &array[i];
		}
	}

	*table = image+8+font_array_size*size;
	
	return TRUE;
}

// Font search
static  font_info_t *OSDSearch(UINT16 uStart, UINT16 uEnd, UINT16 uAim, font_array_t *font, UINT8 *pTab)
{
	font_info_t *font_info;
	UINT16 start, end, len;
	UINT16 midValue;
	UINT32 offset;
	UINT16 size;

	size = font->fontinfo_size;
	if(size != sizeof(font_info_t))
		return NULL;
	
	offset = font->offset;
	
	for (start = uStart, end = uEnd; start <= end;)
	{
		font_info = (font_info_t*)&pTab[offset+((start + end + 1) / 2)*size];
		midValue = font_info->code;

		if ((start == end) && (uAim != midValue))
			return NULL;

		if (uAim > midValue)
		{
			start = (end + start + 1) / 2;
		}
		else if (uAim < midValue)
		{
			end = (end + start) / 2;
		}
		else
		{
			return (font_info_t*)&pTab[offset+((start + end + 1) / 2)*size];
		}

	}
	return FALSE;
}

static font_info_t* OSDSearchFont(UINT16 uIdx, font_array_t *font, UINT8 *pTab)
{
	font_info_t *font_info;
	UINT32 len,offset;

	font_info = (font_info_t*)&pTab[font->offset];
	
	if (uIdx < font_info->code)
		return FALSE;

	return OSDSearch(0, (font->count- 1), uIdx, font, pTab);
}

static const font_info_t *get_font(UINT16 wLibClass, UINT16 *wIndex)
{
	unsigned char fontsize;
	unsigned char *ch, high;
	UINT16 lib_type, font_idx;



	fontsize = wLibClass &0x00FF;
	lib_type = wLibClass &0xFF00;

	//	if(fontsize!=0)		PRINTF("!!!!!!!!!!!!!fontsize=%d\n",fontsize);
	if (fontsize > FONT_MAX_SIZE)
		fontsize = FONT_MAX_SIZE;


	switch (lib_type)
	{
		case LIB_FONT_DEFAULT:
			cur_font = font_asc[fontsize];
			break;
#if (MULTI_MENU_LANG)
		case LIB_FONT_CHN:
			cur_font = font_chn[fontsize];
			break;
#endif
		default:
			return NULL;
	}

	if (cur_font->count== 0)
	{
		return NULL;
	}
	else
	{	
		return OSDSearchFont(*wIndex, cur_font, font_table);
	}

	return NULL;

}

static ID_RSC OSDExt_GetFontLibId(UINT16 EnvID)
{
	lpLANG_TABLE pLangTab = (lpLANG_TABLE)g_LangRscMapTable;
	UINT8 i;

	for (i = 0; i < SIZE_LANG_TABLE; i++)
		if (pLangTab->LangId == EnvID)
			return pLangTab->FontId;
		else
			pLangTab++;

	return LIB_FONT_DEFAULT;
}

static ID_RSC gLangEnv = 0;
static const char *cur_string;

UINT16 OSD_GetLangEnvironment(void)
{
	return gLangEnv;
}

BOOL OSD_SetLangEnvironment(UINT16 langclass)
{
	ID_RSC FontRscId;

	if (langclass >= SIZE_LANG_TABLE)
		langclass = 0;

	gLangEnv = langclass;
	FontRscId = OSDExt_GetMsgLibId(langclass);

	switch (FontRscId)
	{
		case LIB_STRING_CHINESE:
			cur_string = S_Chinese_strs_array;
			break;
		case LIB_STRING_ENGLISH:
		default:
			cur_string = English_strs_array;
			PRINTF("========Change to LIB_STRING_ENGLISH=========\n");
			break;
	}
}

UINT8 font_bitmap[1024*12];

UINT8* 	OSD_GetRscObjData(UINT16 wLibClass,UINT16 uIndex/*UINT8* pbIndex*/,OBJECTINFO* pObjectInfo)
{
	UINT16 wIndex,wClass,stride;
	const font_info_t *font_info;
	UINT8* pbDatabuf=NULL;
	unsigned long i,j,font_width,font_height;
	const unsigned long	*pu32;
	unsigned long size,data_offset,u32 = 0,u32_offset,cur_pix,dest_offset,dest_byte_offset,dest_bit_offset;
	unsigned char *font_byte = NULL;

	const unsigned char *str_array;
	unsigned long str_num;
	unsigned long offset;

	RSC_PRINTF("Call %s:\n", __FUNCTION__);

	pObjectInfo->m_wObjClass = wLibClass;
	
	wIndex=uIndex;
	wClass = wLibClass &0xF000;

	if (wClass == LIB_FONT)
	{
		RSC_PRINTF("Get LIB_FONT:  0x%04x,wIndex=%d\n",wLibClass,wIndex);

		font_info = get_font(wLibClass,&wIndex);
		if(font_info==NULL)	return NULL;

		pObjectInfo->m_objAttr.m_wWidth = font_info->stride*8;//(font->width[wIndex] + 7) / 8 * 8;
		font_height = pObjectInfo->m_objAttr.m_wHeight = font_info->height;
		pObjectInfo->m_objAttr.m_wActualWidth = font_info->width;
		pObjectInfo->m_objAttr.m_bColor = cur_font->bit_count;

		data_offset = cur_font->offset + font_info->data_offset;

		font_width = font_info->width;
		stride = font_info->stride;
		dest_offset = 0;
		MEMSET(font_bitmap, 0, font_height *stride);	
		for (i = 0; i < font_height; i++, dest_offset += stride)
		{
			dest_byte_offset = 0;
			for(j=0; j<stride; j++)
			{
				u32_offset =((data_offset+dest_offset+j)*8) &0x1F;
				if(u32_offset==0)
					u32 = *(UINT32*)&font_table[data_offset+dest_offset+j];
				font_byte = &font_bitmap[dest_offset + dest_byte_offset++];
				*font_byte = (u32 >> (24-u32_offset))&0xff ;
			}
		}

		pbDatabuf = font_bitmap;
		RSC_PRINTF("Get LIB_FONT:  0x%04x,wIndex=%d OK!\n",wLibClass,wIndex);
	}
	else if (wClass == LIB_STRING)
	{
		wIndex -= 1;
		str_array = cur_string;
		RSC_PRINTF("0x%02x,0x%02x,0x%02x \n", str_array[0], str_array[1], str_array[2]);

		str_num = (str_array[1] << 8) + str_array[2];

		RSC_PRINTF("Get LIB_STRING: 0x%04x,wIndex=%d,str_num=%d\n", wLibClass, wIndex, str_num);
		if ( wIndex >= str_num ) return NULL;

		offset = (str_array[3+2 * wIndex] << 8) + str_array[3+2 * wIndex + 1];

		pbDatabuf = (UINT8*) &str_array[offset];
		RSC_PRINTF("Get LIB_STRING: 0x%04x,wIndex=%d,offset=%d,pbDatabuf=0x%x\n", wLibClass, wIndex, offset, pbDatabuf);

	}
	return pbDatabuf;
}

BOOL OSD_GetObjInfo(UINT16 wLibClass,UINT16 uIndex/*UINT8* pbIndex*/,OBJECTINFO* pObjectInfo)
{
	UINT16	wClass,wIndex;
	const font_info_t *font_info;
	
	RSC_PRINTF("Call %s:\n",__FUNCTION__);

	if(pObjectInfo==NULL)
		return 0;

	pObjectInfo->m_wObjClass = wLibClass;
	
	// the content dwIndex points to is UNICODE. 
	wIndex= uIndex;
	wClass = wLibClass &0xF000;

	if (wClass == LIB_FONT)
	{
		font_info = get_font(wLibClass,&wIndex);
		if(font_info==NULL)	return FALSE;

		pObjectInfo->m_objAttr.m_wWidth = font_info->stride*8;
		pObjectInfo->m_objAttr.m_wHeight = font_info->height;
		pObjectInfo->m_objAttr.m_wActualWidth = font_info->width;
		pObjectInfo->m_objAttr.m_bColor = cur_font->bit_count;
	}
	else
	{
		RSC_PRINTF("Get Unkown infor: 0x%04x,wIndex=%d\n", wLibClass, wIndex);
		return FALSE;
	}
	RSC_PRINTF("Get INFOR: 0x%04x,wIndex=%d\n",wLibClass,wIndex);

	return TRUE;
}

ID_RSC OSD_GetDefaultFontLib(UINT16 uChar)
{
	UINT8 hbyte, lbyte;

	hbyte = (UINT8)(uChar >> 8);
	lbyte = (UINT8)uChar;

	if (uChar < 0x80)
		return LIB_FONT_DEFAULT;
	else
		return LIB_FONT_CHN;

}

ID_RSC OSDExt_GetMsgLibId(UINT16 EnvID)
{
	lpLANG_TABLE pLangTab = (lpLANG_TABLE)g_LangRscMapTable;
	UINT8 i;

	for (i = 0; i < SIZE_LANG_TABLE; i++)
		if (pLangTab->LangId == EnvID)
			return pLangTab->StrId;
		else
			pLangTab++;
	return LIB_STRING_ENGLISH;
}

/* Calcuate InternalBufferSize for 7-zip */
#define LZMA_BASE_SIZE  1846
#define LZMA_LIT_SIZE   768
#define BUFFER_SIZE     ((LZMA_BASE_SIZE + (LZMA_LIT_SIZE << (0 + 2))) * sizeof(UINT16))

extern int unlzo(UINT8 *, UINT8 *, UINT8 *);
extern int un7zip(UINT8 *, UINT8 *, UINT8 *);

typedef int (*unzip_t)(UINT8 *, UINT8 *, UINT8 *);

UINT8 *RSC_expand(UINT8* addr, UINT32 offset, UINT8* zip_mode)
{
	unzip_t unzip=NULL;
	UINT8 *entry=0;
	UINT8 *buffer;
	UINT32 code_len;
	UINT8 *codestart=0;
	UINT8 *compressed;

	if(MEMCMP(zip_mode, "uzip", 4)==0)
		unzip = NULL;
	else if(MEMCMP(zip_mode, "7zip", 4)==0)
		unzip = un7zip;
	else if(MEMCMP(zip_mode, "lzo", 3)==0)
		unzip = unlzo;
		
	/* Copy compressed code into DRAM */
	sto_get_data(g_sto_dev, (UINT8*)&code_len, offset + CHUNK_HEADER_SIZE+4, 4);

	compressed = MALLOC(code_len);
	if(compressed == NULL)
		return NULL;
	
	sto_get_data(g_sto_dev, compressed, offset + CHUNK_HEADER_SIZE+0x10, code_len);
	
	//codestart = (UINT8 *)(rsc_font_start);
	codestart = (UINT8 *)(addr);
	entry = codestart;

	buffer = MALLOC(BUFFER_SIZE);
	if (buffer == NULL)
	{
		FREE(compressed);
		entry = 0;
	}
	else
	{
		if (unzip(compressed, codestart, buffer) != 0)
		{
			PRINTF("Boot loader: Decompress error!\n");
			entry = 0;
		}
		FREE(compressed);
		FREE(buffer);
	}

    return entry;
}

BOOL RSC_font_init()
{
	UINT32 chid,offset;
	UINT8 zip_mode[4];
	UINT32 uncompress_len;

	chid = RSC_FONT_ID;
	offset = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
	if(offset & 0x80000000) 
		return FALSE;
	
	sto_get_data(g_sto_dev, zip_mode, offset+0x88, 4);
	sto_get_data(g_sto_dev, (UINT8*)&uncompress_len, offset+0x80, 4);
	font_table = MALLOC(uncompress_len);
	RSC_expand(font_table,offset,zip_mode);
	OSDGetFontarray(&font_table, font_array);

	return TRUE;
}

void RSC_font_bak_init()
{
	font_info_t font_info;
	UINT8 *p_data;
	UINT32 data_offset,data_len,stride;

	font_table = font_bak+0x10;
	OSDGetFontarray(&font_table, font_array);
}

UINT8 *RSC_expand_fromnand(UINT8* addr, UINT32 offset, UINT8* zip_mode)
{
	unzip_t unzip=NULL;
	UINT8 *entry=0;
	UINT8 *buffer;
	UINT32 code_len, uncompressed_len;
	UINT8 *codestart=0;
	UINT8 *compressed;

	if(MEMCMP(zip_mode, "uzip", 4)==0)
		unzip = NULL;
	else if(MEMCMP(zip_mode, "7zip", 4)==0)
		unzip = un7zip;
	else if(MEMCMP(zip_mode, "lzo", 3)==0)
		unzip = unlzo;
		
	/* Copy compressed code into DRAM */
	code_len = *(UINT32 *)(offset + CHUNK_HEADER_SIZE+4);
	uncompressed_len = *(UINT32 *)(offset + CHUNK_HEADER_SIZE);

	compressed = (UINT8 *)offset + CHUNK_HEADER_SIZE+0x10;
	
	codestart = (UINT8 *)(addr);
	entry = codestart;

	if(unzip == NULL)
	{
		MEMCPY(codestart, compressed, uncompressed_len);
	}
	else
	{
		buffer = MALLOC(BUFFER_SIZE);
		if (buffer == NULL)
		{
			FREE(compressed);
			entry = 0;
		}
		if (unzip(compressed, codestart, buffer) != 0)
		{
			PRINTF("Boot loader: Decompress error!\n");
			entry = 0;
		}
		FREE(buffer);
	}

    return entry;
}

void RSC_font_init_fromnand()
{
	UINT32 chid;
	UINT8 *addr;
	UINT8 zip_mode[5];
	UINT32 uncompress_len;

	chid = RSC_FONT_ID;
	addr = chunk_goto(&chid, 0xFFFFFF00, 1);
	MEMSET(zip_mode, 0, sizeof(zip_mode));
	MEMCPY(zip_mode, addr+0x88, 4);
	uncompress_len = *(UINT32 *)(addr+0x80);
	UPG_PRINTF("font offset = 0x%x, uncompressed_len = 0x%x, zip_mode = %s\n", addr, uncompress_len, zip_mode);
	font_table = MALLOC(uncompress_len);
	RSC_expand_fromnand(font_table,(UINT32)addr,zip_mode);
	OSDGetFontarray(&font_table, font_array);
}

