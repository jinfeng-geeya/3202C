#include "sys_config.h"
#include "api/libc/printf.h"
#include "api/libc/string.h"
#include "api/libc/alloc.h"
#include "osal/osal.h"

//#include <api/libge/osd_lib.h>
//#include <api/libge/osd_common_draw.h>
#include <api/libchunk/chunk.h>

#include "osd_config.h"
#include "osd_rsc.h"

#include "images.id"
#include "win_com.h"
#define MULTI_MENU_LANG 1

//#include "rsc/bmp/bitmaps_array.h"
#include "rsc/str/strings_array.h"
//#include "rsc/font/fonts_array.h"
#include "rsc/pal/Palette_tbl.h"
#include "rsc/pal/palettes_array.h"
#include "rsc/wstyle/winstyles_array.h"


#define RSC_PRINTF(...)

static ID_RSC gLangEnv = 0;
static UINT8 cur_wstyle = 0;
static const char *cur_string;

UINT8 *bmp_table=NULL;

UINT8 *font_table = NULL;

font_array_t font_array[10];
UINT32 font_array_size=0;

#define FONT_MAX_SIZE 4
font_array_t *font_asc[FONT_MAX_SIZE];
font_array_t *font_chn[FONT_MAX_SIZE];
font_array_t *cur_font;
/*******************************************************************/
/*		Local function defintion
/*******************************************************************/
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


// BMP decoder
static unsigned long bmp_rle_decode(const unsigned char *rle_bmp_data, unsigned long data_size, unsigned char *bmp_data)
{
	unsigned long i, out_size;
	unsigned char k, size;

	out_size = 0;
	i = 0;
	do
	{
		size = *(rle_bmp_data + i);
		if (size < 128)
		{
			for (k = 0; k < size; k++)
				*(bmp_data + out_size + k) = *(rle_bmp_data + i + 1);
			i += 2;
		}
		else
		{
			size -= 128;
			for (k = 0; k < size; k++)
				*(bmp_data + out_size + k) = *(rle_bmp_data + i + 1+k);
			i += size + 1;
		}

		out_size += size;
	}
	while (i < data_size);

	return out_size;
}

static unsigned long bmp_rle16_decode(const unsigned char *rle16_bmp_data, unsigned long data_size, unsigned char *bmp_data)
{
	unsigned long i,out_size;
	unsigned char k,size;

	out_size = 0;
	i = 0;
	do
	{
		size = *(rle16_bmp_data + i);
		if(size<128)
		{
			for(k=0;k<size*2;k=k+2)
			{
				*(bmp_data + out_size + k) = *(rle16_bmp_data + i + 1);
				*(bmp_data + out_size + k + 1) = *(rle16_bmp_data + i + 2);
			}
			i+= 3;
		}
		else
		{
			size -= 128;
			for(k=0;k<size*2;k=k+2/*k++*/)
			{
				*(bmp_data + out_size + k) = *(rle16_bmp_data + i + 1 + k);
				*(bmp_data + out_size + k + 1) = *(rle16_bmp_data + i + 2 + k);
			}
			i+= size*2 + 1;
		}

		out_size += size*2;
	}while(i < data_size);

	return out_size;
}

static unsigned long clut8_rle_argb1555(const UINT16 *pal_tbl, const unsigned char *rle_bmp_data, unsigned long data_size, UINT16 *bmp_data)
{
	unsigned long i,out_size;
	unsigned char k,size;

	i = 0;
	out_size = 0;
	do
	{
		size = *(rle_bmp_data + i);
		if(size<128)
		{
			for(k=0;k<size;k++)
				*(bmp_data + out_size + k) = pal_tbl[*(rle_bmp_data + i + 1)];
			i+= 2;
		}
		else
		{
			size -= 128;
			for(k=0;k<size;k++)
				*(bmp_data + out_size + k) = pal_tbl[*(rle_bmp_data + i + 1 + k)];
			i+= size + 1;
		}
		out_size += size;
	}while(i < data_size);

	return out_size;
}
//
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

/*******************************************************************/
/*		Local function defintion
/*******************************************************************/

UINT16 OSD_GetLangEnvironment(void)
{
	//	return g_FontCache[LIB_CACHE_COUNT-1].m_wFontClass;
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

#if (MULTI_MENU_LANG	)
		case LIB_STRING_CHINESE:
			cur_string = S_Chinese_strs_array;
			break;
#endif
		case LIB_STRING_ENGLISH:
		default:
			cur_string = English_strs_array;
			PRINTF("========Change to LIB_STRING_ENGLISH=========\n");
			break;
	}
}

#define FONT_BITMAP_BUG_SIZE (420 * 1024) // the size refer to font bmp max size
static UINT8 font_bitmap[FONT_BITMAP_BUG_SIZE];
/*return rsc data pointer directly */
UINT8 *OSD_GetRscObjData(UINT16 wLibClass, UINT16 uIndex,OBJECTINFO *pObjectInfo)
{
	UINT16 wIndex, wClass, stride;
	UINT16 wSite;

	const unsigned char *str_array;

	const font_info_t *font_info;
	bitmap_infor_t *bmpinfo;

	BOOL fStatus;
	UINT8 *pbDatabuf = NULL;

	unsigned long i, j, font_width, font_height;
	const unsigned long *pu32;
	unsigned long offset, size, data_offset, u32 = 0, u32_offset, dest_offset, dest_byte_offset;
	unsigned char *font_byte = NULL;
	unsigned long str_num,bmp_num,bmpinfo_size;

	RSC_PRINTF("Call %s:\n", __FUNCTION__);
	
	pObjectInfo->m_wObjClass = wLibClass;
	wIndex = uIndex; //ComMB16ToWord(pbIndex);
	wClass = wLibClass &0xF000;
	
	if (wClass == LIB_ICON)
	{
		wIndex -= 0x8001;

		RSC_PRINTF("Get LIB_ICON: 0x%04x,wIndex=%d\n", wLibClass, wIndex);

		bmpinfo_size =  *((UINT32 *)&bmp_table[0]);
		bmp_num = *((UINT32 *)&bmp_table[4]);
		
		if (wIndex >= bmp_num)
			return NULL;

		bmpinfo = (bitmap_infor_t*)&bmp_table[8+wIndex*bmpinfo_size];

		pObjectInfo->m_objAttr.m_wHeight = bmpinfo->h;
		pObjectInfo->m_objAttr.m_wActualWidth = bmpinfo->w;
		pObjectInfo->m_objAttr.m_bColor = bmpinfo->bit_count;
		if(pObjectInfo->m_objAttr.m_bColor == 32)
			pObjectInfo->m_objAttr.m_wWidth = (bmpinfo->bmp_size / bmpinfo->h)>>2;
		else if(pObjectInfo->m_objAttr.m_bColor == 16)
			pObjectInfo->m_objAttr.m_wWidth = (bmpinfo->bmp_size / bmpinfo->h);//>>1;
		else
			pObjectInfo->m_objAttr.m_wWidth = (bmpinfo->bmp_size / bmpinfo->h);			

		if (bmpinfo->bmp_size > sizeof(font_bitmap))
			return NULL;
		else
			pbDatabuf = font_bitmap;

		if (pbDatabuf != NULL)
		{
#ifdef USE_16BIT_OSD
			if (bmp_rle16_decode(&bmp_table[bmpinfo->data], bmpinfo->data_size, pbDatabuf) != bmpinfo->bmp_size)
#else
            if (bmp_rle_decode(&bmp_table[bmpinfo->data], bmpinfo->data_size, pbDatabuf) != bmpinfo->bmp_size)
#endif
			{
				pbDatabuf = NULL;
			}
		}

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
	else if (wClass == LIB_FONT)
	{
		RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d\n", wLibClass, wIndex);

		font_info = get_font(wLibClass, &wIndex);
		if ( font_info == NULL )	return NULL;

		pObjectInfo->m_objAttr.m_wWidth = font_info->stride*8;//(font->width[wIndex] + 7) / 8 * 8;
		font_height = pObjectInfo->m_objAttr.m_wHeight = font_info->height;
		pObjectInfo->m_objAttr.m_wActualWidth = font_info->width;
		pObjectInfo->m_objAttr.m_bColor = cur_font->bit_count;
		
		data_offset = cur_font->offset + font_info->data_offset;

		font_width = font_info->width;
		stride = font_info->stride;
		RSC_PRINTF("%c,font_width=%d,font_height=%d,stride=%d,data_offset=%d\n", wIndex, font_width, font_info->height, stride, data_offset);
		dest_offset = 0;

		if(font_height *stride > FONT_BITMAP_BUG_SIZE)
			return NULL;
		
		pbDatabuf = font_bitmap;
		MEMSET(pbDatabuf, 0, font_height *stride);
		
		for (i = 0; i < font_height; i++, dest_offset += stride)
		{
			dest_byte_offset = 0;
			for(j=0; j<stride; j++)
			{
				u32_offset =((data_offset+dest_offset+j)*8) &0x1F;
				if(u32_offset==0)
					u32 = *(UINT32*)&font_table[data_offset+dest_offset+j];
				font_byte = &pbDatabuf[dest_offset + dest_byte_offset++];
				*font_byte = (u32 >> (24-u32_offset))&0xff ;
			}
		}
		
		RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d OK!\n", wLibClass, wIndex);

	}
	else if (wClass == LIB_PALLETE)
	{
		RSC_PRINTF("Get LIB_PALLETE: 0x%04x,wIndex=%d\n", wLibClass, wIndex);
		wIndex &= 0x0F;
		if (wIndex >= sizeof(palette_array) / sizeof(palette_array[0]))
			wIndex = 0;
		pbDatabuf = palette_array[wIndex];
	}
	else
	{
		RSC_PRINTF("Get Unkown data: 0x%04x,wIndex=%d\n", wLibClass, wIndex);
		return NULL;
	}

	return pbDatabuf;

}

/* copy rsc data to buffer: font_bitmap_ext , and return font_bitmap_ext*/
UINT8 *OSD_GetRscObjData_ext(UINT16 wLibClass, UINT16 uIndex /*UINT8* pbIndex*/, OBJECTINFO *pObjectInfo, UINT8 *font_bitmap_ext, UINT32 font_bitmap_size)
{
	UINT16 wIndex, wClass, stride;
	UINT16 wSite;

	const unsigned char *str_array;

	const font_info_t *font_info;
	bitmap_infor_t *bmpinfo;

	BOOL fStatus;
	UINT8 *pbDatabuf = NULL;

	unsigned long i, j, font_width, font_height;
	const unsigned long *pu32;
	unsigned long offset, size, data_offset, u32 = 0, u32_offset, dest_offset, dest_byte_offset;
	unsigned char *font_byte = NULL;
	unsigned long str_num, bmp_num,bmpinfo_size;

	UINT32 rsc_size = 0;

	RSC_PRINTF("Call %s:\n", __FUNCTION__);

	if(font_bitmap_ext == NULL)
		return NULL;

	pObjectInfo->m_wObjClass = wLibClass;

	wIndex = uIndex; //ComMB16ToWord(pbIndex);
	wClass = wLibClass &0xF000;

	if (wClass == LIB_ICON)
	{
		wIndex -= 0x8001;

		RSC_PRINTF("Get LIB_ICON: 0x%04x,wIndex=%d\n", wLibClass, wIndex);

		bmpinfo_size =  *((UINT32 *)&bmp_table[0]);
		bmp_num = *((UINT32 *)&bmp_table[4]);
		
		if (wIndex >= bmp_num)
			return NULL;

		bmpinfo = (bitmap_infor_t*)&bmp_table[8+wIndex*bmpinfo_size];

		pObjectInfo->m_objAttr.m_wHeight = bmpinfo->h;
		pObjectInfo->m_objAttr.m_wActualWidth = bmpinfo->w;
		pObjectInfo->m_objAttr.m_bColor = bmpinfo->bit_count;
		if(pObjectInfo->m_objAttr.m_bColor == 32)
			pObjectInfo->m_objAttr.m_wWidth = (bmpinfo->bmp_size / bmpinfo->h)>>2;
		else if(pObjectInfo->m_objAttr.m_bColor == 16)
			pObjectInfo->m_objAttr.m_wWidth = (bmpinfo->bmp_size / bmpinfo->h)>>1;
		else
			pObjectInfo->m_objAttr.m_wWidth = (bmpinfo->bmp_size / bmpinfo->h);			

		if (bmpinfo->bmp_size > sizeof(font_bitmap_ext))
		{
			return NULL;
		}
		else
			pbDatabuf = font_bitmap_ext;

		if (pbDatabuf != NULL)
		{
			if (bmp_rle_decode(&bmp_table[bmpinfo->data], bmpinfo->data_size, pbDatabuf) != bmpinfo->bmp_size)
			{
				pbDatabuf = NULL;
			}
		}
		
		return pbDatabuf;
    
	}
	else if (wClass == LIB_STRING)
	{
		wIndex -= 1;
		str_array = cur_string;
		RSC_PRINTF("0x%02x,0x%02x,0x%02x \n", str_array[0], str_array[1], str_array[2]);

		str_num = (str_array[1] << 8) + str_array[2];

		RSC_PRINTF("Get LIB_STRING: 0x%04x,wIndex=%d,str_num=%d\n", wLibClass, wIndex, str_num);
		if (wIndex >= str_num)
		{
			return NULL;
		}

		offset = (str_array[3+2 * wIndex] << 8) + str_array[3+2 * wIndex + 1];

		pbDatabuf = (UINT8*) &str_array[offset];
		rsc_size = ComUniStrLenExt(pbDatabuf);
		RSC_PRINTF("Get LIB_STRING: 0x%04x,wIndex=%d,offset=%d,pbDatabuf=0x%x\n", wLibClass, wIndex, offset, pbDatabuf);

	}
	else if (wClass == LIB_FONT)
	{
		RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d\n", wLibClass, wIndex);

		font_info = get_font(wLibClass, &wIndex);
		if ( font_info == NULL )	return NULL;

		pObjectInfo->m_objAttr.m_wWidth = font_info->stride*8;//(font->width[wIndex] + 7) / 8 * 8;
		font_height = pObjectInfo->m_objAttr.m_wHeight = font_info->height;
		pObjectInfo->m_objAttr.m_wActualWidth = font_info->width;
		pObjectInfo->m_objAttr.m_bColor = cur_font->bit_count;
		
		data_offset = cur_font->offset + font_info->data_offset;

		font_width = font_info->width;
		stride = font_info->stride;
		RSC_PRINTF("%c,font_width=%d,font_height=%d,stride=%d,data_offset=%d\n", wIndex, font_width, font_info->height, stride, data_offset);
		dest_offset = 0;

		if(font_height *stride > font_bitmap_size)
			return NULL;
		
		pbDatabuf = font_bitmap_ext;
		MEMSET(pbDatabuf, 0, font_height *stride);
		
		for (i = 0; i < font_height; i++, dest_offset += stride)
		{
			dest_byte_offset = 0;
			for(j=0; j<stride; j++)
			{
				u32_offset =((data_offset+dest_offset+j)*8) &0x1F;
				if(u32_offset==0)
					u32 = *(UINT32*)&font_table[data_offset+dest_offset+j];
				font_byte = &pbDatabuf[dest_offset + dest_byte_offset++];
				*font_byte = (u32 >> (24-u32_offset))&0xff ;
			}
		}
		
		RSC_PRINTF("Get LIB_ICON:  0x%04x,wIndex=%d OK!\n", wLibClass, wIndex);
		return pbDatabuf;

	}
	else if (wClass == LIB_PALLETE)
	{
		RSC_PRINTF("Get LIB_PALLETE: 0x%04x,wIndex=%d\n", wLibClass, wIndex);
		wIndex &= 0x0F;
		if (wIndex >= sizeof(palette_array) / sizeof(palette_array[0]))
			wIndex = 0;
		pbDatabuf = palette_array[wIndex];
		rsc_size = sizeof(palette_array[0]);
	}
	else
	{
		RSC_PRINTF("Get Unkown data: 0x%04x,wIndex=%d\n", wLibClass, wIndex);
		return NULL;
	}

	if (pbDatabuf != NULL && rsc_size != 0)
	{
		if(rsc_size > font_bitmap_size)
		{
			RSC_PRINTF("%s: rsc buffer not enough!\n", __FUNCTION__);
			return NULL;
		}
		
		MEMSET(font_bitmap_ext, 0, rsc_size);
		MEMCPY(font_bitmap_ext, pbDatabuf, rsc_size);

		pbDatabuf = font_bitmap_ext;
	}
	return pbDatabuf;

}


static UINT16 COMMB16ToUINT16(UINT16 pVal)
{
	UINT16 wRes = 0;
	UINT16 high, low;

	if (pVal == 0)
		return 0;
	high = ((pVal &0xff00) >> 8);
	low = ((pVal &0x00ff) << 8);

	wRes = (low | high);
	return wRes;
}

static BOOL scrollStopFlag = FALSE;
static UINT8 fontBuf[1024];

void SetScrollStopFlag(BOOL flag)
{
	scrollStopFlag = flag;
}

BOOL GetScrollStopFlag()
{
	return scrollStopFlag;
}

void ScrollString(UINT16 *str, OSD_RECT rc, UINT8 fontColor, UINT8 backColor, VSCR *vScr, UINT8 regionID, UINT8 speed)
{
	UINT16 ch;
	UINT16 i = 0, j, k, chr_w = 10;
	UINT16 font_h, font_w;
	UINT8 times = 0;

	if (str == NULL || vScr == NULL)
		return ;

	if (speed == 0)
		speed = 30;
	//default speed is 30
	MEMSET(vScr->lpbScr, backColor, rc.uHeight *rc.uWidth);
	while (*str != 0)
	{
		if (scrollStopFlag)
		{
			libc_printf(" scrollstop flag : 1\n");
			break;
		}
		ch = COMMB16ToUINT16(*str);
		UINT8 *pDtaBitmap;
		OBJECTINFO RscLibInfo;
		UINT8 fontArray[1024];
		UINT16 RscLibId;
		UINT8 c, width;
		RscLibId = OSD_GetDefaultFontLib(ch);
		pDtaBitmap = OSD_GetRscObjData_ext(RscLibId, ch, &RscLibInfo, fontBuf, 1024);
		if (pDtaBitmap == NULL)
		{
			str++;
			continue;
		}
		MEMCPY(fontArray, pDtaBitmap, 72);
		width = RscLibInfo.m_objAttr.m_wActualWidth;
		for (i = 0; i < width; i++)
		{
			for (k = 0; k < rc.uHeight; k++)
			{
				UINT8 cmask = 0;
				UINT16 pos = k * rc.uWidth;
				for (j = 0; j < rc.uWidth - 1; j++)
				{
					vScr->lpbScr[pos + j] = vScr->lpbScr[pos + j + 1];
				}

				if (k < (rc.uHeight - RscLibInfo.m_objAttr.m_wHeight))
				{
					vScr->lpbScr[pos + rc.uWidth - 1] = backColor;
					continue;
				}
				c = fontArray[(k - (rc.uHeight - RscLibInfo.m_objAttr.m_wHeight))*(RscLibInfo.m_objAttr.m_wWidth / 8) + i / 8];
				cmask = 0x80 >> (i % 8);
				vScr->lpbScr[pos + rc.uWidth - 1] = (c &cmask) ? fontColor : backColor;
			}
			OSD_SetRect2(&vScr->vR, &rc);
			vScr->updatePending = 1;
#ifndef USE_LIB_GE
			OSD_UpdateVscr_ext(vScr, regionID);
#endif
			while (times < speed)
			{
				osal_task_sleep(1);
				times++;
				if (times >= speed)
				{
					times = 0;
					break;
				}
			}
		}
		str++;
	}

	MEMSET(vScr->lpbScr, OSD_TRANSPARENT_COLOR_BYTE, rc.uHeight *rc.uWidth);
	OSD_SetRect2(&vScr->vR, &rc);
	vScr->updatePending = 1;
#ifndef USE_LIB_GE
	OSD_UpdateVscr_ext(vScr, regionID);
#endif
}


BOOL OSD_GetObjInfo(UINT16 wLibClass, UINT16 uIndex /*UINT8* pbIndex*/, OBJECTINFO *pObjectInfo)
{
	UINT16 wClass, wIndex, i;
	UINT32 dwOffset;
	const font_info_t *font_info;
	bitmap_infor_t *bmpinfo;
	unsigned long bmp_num,bmpinfo_size;


	RSC_PRINTF("Call %s:\n", __FUNCTION__);

	pObjectInfo->m_wObjClass = wLibClass;


	// the content dwIndex points to is UNICODE.
	wIndex = uIndex; //ComLB16ToWord(pbIndex);

	// ADD operation help to Get the high-level resource-type
	// add 0x80 to in case of so many string type
	wClass = wLibClass &0xF000;

	// check for security
	// if the lib is one of the following two,pObjectInfo maynot be a empty value
	if (((wClass == LIB_FONT) || (wClass == LIB_ICON)) && (pObjectInfo == NULL))
		return 0;
	if (wClass == LIB_FONT)
	{

		font_info = get_font(wLibClass, &wIndex);
		if ( font_info == NULL )	return FALSE;

		pObjectInfo->m_objAttr.m_wWidth = font_info->stride*8;
		pObjectInfo->m_objAttr.m_wHeight = font_info->height;
		pObjectInfo->m_objAttr.m_wActualWidth = font_info->width;
		pObjectInfo->m_objAttr.m_bColor = cur_font->bit_count;
	}
	else if (wClass == LIB_ICON)
	{

		//PRINTF("Get LIB_ICON INFOR: 0x%04x,wIndex=%d\n",wLibClass,wIndex);
		wIndex -= 0x8001;

		bmpinfo_size =  *((UINT32 *)&bmp_table[0]);
		bmp_num = *((UINT32 *)&bmp_table[4]);
		
		if (wIndex >= bmp_num)
			return FALSE;

		bmpinfo = (bitmap_infor_t*)&bmp_table[8+wIndex*bmpinfo_size];

		pObjectInfo->m_objAttr.m_wWidth = bmpinfo->bmp_size / bmpinfo->h;
		pObjectInfo->m_objAttr.m_wHeight = bmpinfo->h;
		pObjectInfo->m_objAttr.m_wActualWidth = bmpinfo->w;
		pObjectInfo->m_objAttr.m_bColor = bmpinfo->bit_count;
	}
	else
	{

		RSC_PRINTF("Get Unkown infor: 0x%04x,wIndex=%d\n", wLibClass, wIndex);
		return FALSE;
	}


	RSC_PRINTF("Get INFOR: 0x%04x,wIndex=%d\n", wLibClass, wIndex);


	return TRUE;
}

BOOL OSD_ReleaseObjData(UINT8 *lpDataBuf, lpOBJECTINFO pObjectInfo)
{
	if ((pObjectInfo->m_wObjClass &0xF000) == LIB_ICON && lpDataBuf != NULL)
	{
		//	PRINTF("Free Bitmap!\n");
		//printf("FREE 0X%08X\n",lpDataBuf);
		//FREE(lpDataBuf);
	}
	//	else		PRINTF("Rleasee ObjData !!!!(0x%04x,0x%x)\n",pObjectInfo->m_wObjClass,lpDataBuf);

	return TRUE;
}

/**************************************************************************************
 * Function Name:	OSD_GetDefaultFontLib
 *
 * Description:		Get a character's FONT Library ID.
 *
 * Arguments:
 *		pString: 	IN;		The character pointer.
 *
 * Return value:
 *		ID_RSC:		The font Library ID of the caracter
 *
 * NOTES:	Its detecting order is: ASCII char, MASS char, the language we currently used.
 *
 **************************************************************************************/
ID_RSC OSD_GetDefaultFontLib(UINT16 uChar)
{
	UINT8 hbyte, lbyte;

	hbyte = (UINT8)(uChar >> 8);
	lbyte = (UINT8)uChar;

	if (uChar < 0x80)
		return LIB_FONT_DEFAULT;
	else if (uChar <= 0xFF)
		return LIB_FONT_LATIN1;
	else if (uChar < 0x1FF)
		return LIB_FONT_LATINE;
	else if (hbyte >= 0xE0 && hbyte <= 0xF8)
		return LIB_FONT_MASSCHAR1;
	else
		return LIB_FONT_CHN;
	//return OSDExt_GetFontLibId ( OSD_GetLangEnvironment() );

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

PWINSTYLE OSDExt_GetWinStyle(UINT8 bIdx)
{
	if (bIdx < SIZE_WINSTYLE)
		return winstyle_array[cur_wstyle] + bIdx;
	else
		return winstyle_array[cur_wstyle] + 0;
	// Error Handle
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
	unzip_t unzip = NULL;
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

	codestart = (UINT8 *)(addr);

	if(unzip == NULL)
	{
		sto_get_data(g_sto_dev, codestart, offset + CHUNK_HEADER_SIZE+0x10, code_len);
	}
	else
	{
		compressed = MALLOC(code_len);
		if(compressed == NULL)
			return NULL;

		sto_get_data(g_sto_dev, compressed, offset + CHUNK_HEADER_SIZE+0x10, code_len);
		
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
				libc_printf("Boot loader: Decompress error!\n");
				entry = 0;
			}
			FREE(compressed);
			FREE(buffer);
		}
	}
	
	
	

    return entry;
}

#define	RSC_FONT_ID			0x10EF0100
#define	RSC_BMP_ID				0x11EE0100

void RSC_font_init()
{
	UINT32 chid,offset;
	UINT8 zip_mode[4];
	UINT32 uncompress_len;

	chid = RSC_FONT_ID;
	offset = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
	sto_get_data(g_sto_dev, zip_mode, offset+0x88, 4);
	sto_get_data(g_sto_dev, (UINT8*)&uncompress_len, offset+0x80, 4);
	font_table = MALLOC(uncompress_len);
	RSC_expand(font_table,offset,zip_mode);
	OSDGetFontarray(&font_table, font_array);
}

void RSC_bmp_init()
{
	UINT32 chid,offset;
	UINT8 zip_mode[4];
	UINT32 uncompress_len;

	chid = RSC_BMP_ID;
	offset = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
	sto_get_data(g_sto_dev, zip_mode, offset+0x88, 4);
	sto_get_data(g_sto_dev, (UINT8*)&uncompress_len, offset+0x80, 4);
	bmp_table = MALLOC(uncompress_len);
	RSC_expand(bmp_table,offset,zip_mode);
}
	
