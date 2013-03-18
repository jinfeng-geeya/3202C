#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <hld/dis/vpo.h>
#include <hld/decv/vdec_driver.h>
#include <hld/hld_dev.h>
#include <api/libimagedec/imagedec.h>
#include "imagedec_main.h"
#include "imagedec_exif.h"

extern volatile bool g_imagedec_stop_flag[IMAGEDEC_MAX_INSTANCE];

IMAGEDEC_STATIC 
void skip_buffer_bytes(UINT32 num)
{
	jpeg_printf("skip num <%d>\n",num);
	while (num)
  	{
    		get_bits_1(8);num--;
  	}
}
IMAGEDEC_STATIC 
bool entry_jpeginterchangeformat(pImagedec_hdl phdl)
{
	pImagedec_app1_exif pexif = (pImagedec_app1_exif)(phdl->app.item[IMAGEDEC_EXIF_APP].par);
	UINT32 image_offset = 0;UINT16 type = 0;

	type = get_bits_1(16);
	if(pexif->byte_order)
			BIG_TO_LIT_16(type);
	if(EXIF_ENTRY_TYPE_LONG != type)
	{
		jpeg_printf("jpeg intformat type error <%d>\n",type);
		skip_buffer_bytes(IMAGEDEC_TIFF_IFD_ENTRY_SIZE - 4);
		return FALSE;
	}
	/*skip the count field*/
	get_bits_1(16);get_bits_1(16);
	image_offset = get_bits_1(16)<<16;image_offset |= get_bits_1(16);
	if(pexif->byte_order)
			BIG_TO_LIT_32(image_offset);
	pexif->image_offset = image_offset;pexif->image_find = 1;
	jpeg_printf("jpeg int format suc offset <%x>\n",pexif->image_offset);
	return TRUE;
}
IMAGEDEC_STATIC 
bool entry_jpeginterchangeformatlength(pImagedec_hdl phdl)
{
	pImagedec_app1_exif pexif = (pImagedec_app1_exif)(phdl->app.item[IMAGEDEC_EXIF_APP].par);
	UINT32 image_length = 0;UINT32 type = 0;

	type = get_bits_1(16);
	if(pexif->byte_order)
			BIG_TO_LIT_16(type);
	if(EXIF_ENTRY_TYPE_LONG != type)
	{
		jpeg_printf("jpeg intformat type error <%d>\n",type);
		skip_buffer_bytes(IMAGEDEC_TIFF_IFD_ENTRY_SIZE - 4);
		return FALSE;
	}
	/*skip the count field*/
	get_bits_1(16);get_bits_1(16);
	image_length = get_bits_1(16)<<16;image_length |= get_bits_1(16);
	if(pexif->byte_order)
		BIG_TO_LIT_32(image_length);
	pexif->image_length = image_length;
	jpeg_printf("jpeg int format length suc <%x>\n",pexif->image_length);
	return TRUE;
}
void store_bit_info(pImagedec_app1_exif pexif,pImage_decoder pdecoder,pImagedec_stream_mgr pinstream,UINT8 index)
{
	pexif->tmp_bit_info.tmp_bits_left[index] = pdecoder->bits_left;
	pexif->tmp_bit_info.tmp_bit_buf[index] = pdecoder->bit.bit_buf;

	pexif->tmp_bit_info.tmp_buf_ofs[index] = pinstream->Pin_buf_ofs;
	pexif->tmp_bit_info.tmp_in_buf_left[index] = pinstream->in_buf_left;
}
void recover_bit_info(pImagedec_app1_exif pexif,pImage_decoder pdecoder,pImagedec_stream_mgr pinstream,UINT8 index)
{
	pdecoder->bits_left = pexif->tmp_bit_info.tmp_bits_left[index];
	pdecoder->bit.bit_buf = pexif->tmp_bit_info.tmp_bit_buf[index];

	pinstream->Pin_buf_ofs = pexif->tmp_bit_info.tmp_buf_ofs[index];
	pinstream->in_buf_left = pexif->tmp_bit_info.tmp_in_buf_left[index];
}
IMAGEDEC_STATIC 
bool tiff_header(pImagedec_hdl phdl)
{
	pImagedec_app1_exif pexif = (pImagedec_app1_exif)(phdl->app.item[IMAGEDEC_EXIF_APP].par);
	UINT32 tmp1 = 0,tmp2 = 0,count = 0;

	JPEG_ENTRY;
	/*bytes0-1. byte order*/
	switch(get_bits_1(16))
	{
		case 0x4949:
			pexif->byte_order = 1;
			break;
		case 0x4D4D:
			pexif->byte_order = 0;
			break;
		default:
			return FALSE;
	}
	count += 2;jpeg_printf("exif byte order <%d>\n",pexif->byte_order);
	tmp1 = get_bits_1(16);
	tmp2 = get_bits_1(16)<<16;tmp2 |= get_bits_1(16);count += 6;
	if(pexif->byte_order)
	{
		BIG_TO_LIT_16(tmp1);BIG_TO_LIT_32(tmp2);
	}
	pexif->tiff.imagic_num = tmp1;pexif->tiff.next_ifd_offset = tmp2;
	jpeg_printf("fst ifd offset <%x>\n",pexif->tiff.next_ifd_offset);
	pexif->tiff_offset += count;
	JPEG_EXIT;
	return TRUE;	
}
/*return value :  num of entrys done*/
IMAGEDEC_STATIC 
int tiff_ifd_parsing(pImagedec_hdl phdl)
{
	pImagedec_app1_exif pexif = (pImagedec_app1_exif)(phdl->app.item[IMAGEDEC_EXIF_APP].par);
	pImagedec_tiff ptiff = &(pexif->tiff);
	UINT32 num_entrys = 0,count = 0, tag = 0;
	int i,ret = 0;

	JPEG_ENTRY;
	while(pexif->tiff_offset < ptiff->next_ifd_offset)
	{
		get_bits_1(8);pexif->tiff_offset++;
	}
	num_entrys = get_bits_1(16);count += 2;
	if(pexif->byte_order)
		BIG_TO_LIT_16(num_entrys);
	ptiff->ifd[0].entry_num = num_entrys;
	jpeg_printf("entry num in the ifd <%d>\n",num_entrys);
	for(i=0;i<(int)num_entrys;i++)
	{
		tag = get_bits_1(16);
		if(pexif->byte_order)
			BIG_TO_LIT_16(tag);
		switch(tag)
		{
			case EXIF_JPEGInterchangeFormat:
				ret += entry_jpeginterchangeformat(phdl);
				break;
			case EXIF_JPEGInterchangeFormatLength:
				ret += entry_jpeginterchangeformatlength(phdl);
				break;
			default:
				jpeg_printf("tag <%x>\n",tag);
				skip_buffer_bytes(IMAGEDEC_TIFF_IFD_ENTRY_SIZE - 2);
		}
		count += IMAGEDEC_TIFF_IFD_ENTRY_SIZE;	
	}
	pexif->tiff_offset += count;
	jpeg_printf("valid entry num <%d>\n",ret);
	JPEG_EXIT;
	return ret;
}

/*decoder image file directory used in the TIFF. only have two ifds in the exif spec*/
IMAGEDEC_STATIC 
bool tiff_ifd(pImagedec_hdl phdl)
{
	pImagedec_app1_exif pexif = (pImagedec_app1_exif)(phdl->app.item[IMAGEDEC_EXIF_APP].par);
	pImagedec_tiff ptiff = &(pexif->tiff);
	UINT32 num_entrys = 0,next_offset = 0,count = 0;
	UINT16 tag = 0;
	int i,ret = 0;

	JPEG_ENTRY;
	/*parsing the ifd till get the jpeg thumbnail related tag. normally at the sec ifd
	now for test, only parsing two ifd, if the tag is not found, just return False*/
	for(i=0;i<IMAGEDEC_TIFF_IFD_NUM;i++)
	{
		if(pexif->length <= ptiff->next_ifd_offset)
		{
			jpeg_printf("offset > length of exif section\n");
			ret = 0;
			break;
		}
		ret +=  tiff_ifd_parsing(phdl);
		/*ready for next parsing*/
		next_offset = get_bits_1(16)<<16;next_offset |= get_bits_1(16);count += 4;
		if(pexif->byte_order)
		{
			BIG_TO_LIT_32(next_offset);
		}
		jpeg_printf("exif next offset <%x>\n",next_offset);
		ptiff->next_ifd_offset = next_offset;
		pexif->tiff_offset += count;count = 0;
		if(!next_offset)
		{
			jpeg_printf("null next IFD\n");
			break;
		}
	}	
	/*skip to the jpeg thumbnail soi offset*/
	if((ret >= 2)&&(1 == pexif->image_find))
	{
	  	UINT8 lastchar = 0, thischar = 0;

		while(pexif->tiff_offset < pexif->image_offset)
		{
			/*check for the stop status*/
			if(g_imagedec_stop_flag[phdl->id])
				return FALSE; 
			get_bits_1(8);pexif->tiff_offset++;
			//jpeg_printf("tiff offset to thumb<%x>\n",pexif->tiff_offset);
		}
		store_bit_info(pexif,&(phdl->decoder),&(phdl->in_stream),1);
		lastchar = get_bits_1(8);thischar = get_bits_1(8);
		if ((lastchar == 0xFF) && (thischar == M_SOI))
		{
			JPEG_EXIT;
			recover_bit_info(pexif,&(phdl->decoder),&(phdl->in_stream),1);
			return TRUE;
		}
	}
	return FALSE;
}

IMAGEDEC_STATIC 
bool exif_identify(pImagedec_hdl phdl)
{
	UINT16 c1 = get_bits_1(16),c2 = get_bits_1(16);
	UINT8 c3 = get_bits_1(8);
	int ret = FALSE;
	
	if((0x4578 == c1)&&(0x6966 == c2)&&(0x00 == c3))
	{
		jpeg_printf("exif identify ok\n");
		/*skip 1 bytes padding data*/
		get_bits_1(8);
		ret = TRUE;
	}
	else
	{
		jpeg_printf("exif identify error\n");
	}
	return ret;
}
IMAGEDEC_STATIC
bool exif_header(pImagedec_hdl phdl)
{
	if(!exif_identify(phdl))
		return FALSE;
	if(!tiff_header(phdl))
		return FALSE;
	return tiff_ifd(phdl);
}
/*may be something different from primary jpeg decode. it just support baseline jpeg decoder*/
IMAGEDEC_STATIC 
bool thumbnail_read_header(pImagedec_hdl phdl)
{

	return TRUE;
}
IMAGEDEC_STATIC 
bool thumbnail_start_dec(pImagedec_hdl phdl)
{
	return TRUE;
}
IMAGEDEC_STATIC 
bool thumbnail_read_scanlines(pImagedec_hdl phdl)
{
	
	return TRUE;
}
int g_imagedec_exif_error_code[IMAGEDEC_MAX_INSTANCE];
/*app fuction dealing with the exif thumbnail image*/
bool app_exif_decoder(void *handle)
{
	pImagedec_hdl phdl = (pImagedec_hdl)handle;
	UINT16 exif_length = 0;
	Imagedec_app1_exif exif;
	bool ret = FALSE;
	
	JPEG_ENTRY;
	MEMSET(&exif,0,sizeof(Imagedec_app1_exif));
	phdl->app.item[IMAGEDEC_EXIF_APP].par = (void *)&exif; 
	/*store tmp info about bit parsing meet the error case*/
	store_bit_info(&exif,&(phdl->decoder),&(phdl->in_stream),0);
	/*set the length of the app section*/
	exif.length = get_bits_1(16);
	jpeg_printf("exif length <%x>\n",exif.length);
	if(!exif_header(phdl))
	{
		g_imagedec_exif_error_code[phdl->id] = 0;
		jpeg_printf("parsing exif header fail\n");
		recover_bit_info(&exif,&(phdl->decoder),&(phdl->in_stream),0);
	}
	else
	{
		g_imagedec_exif_error_code[phdl->id] = 1;
		/*decode the thumbnai jpeg image. using the decoder same as the primary picture*/
		jpeg_printf("parsing exif ok\n");
		ret = TRUE;
	}
	JPEG_EXIT;
	return ret;
}

