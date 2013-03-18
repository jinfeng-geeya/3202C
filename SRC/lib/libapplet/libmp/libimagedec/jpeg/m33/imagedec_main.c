#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <hal/hal_common.h>

#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>

//#include <api/libfs1/fs.h>
#include "imagedec_main.h"
#include "imagedec_bit.h"
#include "imagedec_osd.h"
#include "imagedec_acc.h"

extern volatile int g_imagedec_info_valid[IMAGEDEC_MAX_INSTANCE];
extern volatile OSAL_ID g_imagedec_wait_flag[IMAGEDEC_MAX_INSTANCE]; 
extern volatile bool g_imagedec_stop_flag[IMAGEDEC_MAX_INSTANCE];
extern volatile int g_imagedec_enable_hw_acce[IMAGEDEC_MAX_INSTANCE];
extern Imagedec_Init_Config g_imagedec_config[IMAGEDEC_MAX_INSTANCE];
extern void (*IDCT_Scaling_Routine)(pImagedec_hdl phdl, int Y_only);

extern imagedec_osd_ins g_imagedec_osd_ins;
extern int g_imagedec_m33_enable_hw_vld;
extern int g_imagedec_m33_extend_hw_vld;
extern int g_imagedec_m33_combine_hw_sw_flag;

int error_code;
extern int g_imagedec_exif_error_code[IMAGEDEC_MAX_INSTANCE];
int jpg_buffer_pos;

int cur_decoding_mcu_row=0;

int image_data_end = 0;
bool eof_flag;
int tem_flag;

static volatile int iset_comp=0;
static int iPro2_progressive; 

int *temp_IDCT_buf; //IDCT input, 
int *temp_IDCT_buf2; //IDCT output, for 4:1:1 we may have to do horizontal copying

#if 0
// only Pro2. the file relative info should be stored after init, and loaded again at each reload data.
static int iPro2_FilePos, iPro2_InBufLeft, iPro2_ByteRead, iPro2_BitLest;
static uchar* pPro2_BufOfs;
static uint uiPro2_BitBuf0, uiPro2_BitBuf1;
static int iPro2_RestartInterval;
static int iPro2_DcSize[JPGD_MAXCOMPONENTS];
static int iPro2_AcSize[JPGD_MAXCOMPONENTS];
// only Pro2. control the MCU lines to read and decode
static int iPro2_NextReadRow;
static int iPro2_McuBuf_MaxRow; // iPro2_McuBuf_MaxRow MCU lines per time. 2003-7-15.
// only Pro2. control the MCU lines(including each comps) to output
static int iPro2_McuBufLineOutput; // can store several MCU line for output.
static int iPro2_CurMcuLineOutput[JPGD_MAXCOMPONENTS];
#endif

// Coefficients are stored in this sequence in the data stream.
static unsigned char ZAG[64] =
{
  0,  1,  8, 16,  9,  2,  3, 10,
 17, 24, 32, 25, 18, 11,  4,  5,
 12, 19, 26, 33, 40, 48, 41, 34,
 27, 20, 13,  6,  7, 14, 21, 28,
 35, 42, 49, 56, 57, 50, 43, 36,
 29, 22, 15, 23, 30, 37, 44, 51,
 58, 59, 52, 45, 38, 31, 39, 46,
 53, 60, 61, 54, 47, 55, 62, 63,
};
#define INTERNAL_STATIC_ROUTINES
IMAGEDEC_INLINE uint get_bits_2(int numbits);
IMAGEDEC_STATIC void* mem_memcpy(void *target,void *source, int n)
{
 unsigned char *t;
 unsigned char *s;

 if(target && source && target != source && n > 0)
 {
  t = (unsigned char *)target;
  s = (unsigned char *)source;
  if(((unsigned long)t&0x03) == ((unsigned long)s&0x03))
  {
   while(((unsigned long)t&0x03) != 0 && n > 0)
   {
    *t++ = *s++;
    n--;
   }
   while(n >= 16)
   {
    ((unsigned long *)t)[0] = ((unsigned long *)s)[0];
    ((unsigned long *)t)[1] = ((unsigned long *)s)[1];
    ((unsigned long *)t)[2] = ((unsigned long *)s)[2];
    ((unsigned long *)t)[3] = ((unsigned long *)s)[3];
    t += 16;
    s += 16;
    n -= 16;
   }
   while(n >= 4)
   {
    *(unsigned long *)t = *(unsigned long *)s;
    t += 4;
    s += 4;
    n -= 4;
   }
  }
  while(n > 0)
  {
   *t++ = *s++;
   n--;
  }
 }

 return target;
}

//------------------------------------------------------------------------------
// Clear buffer to word values.
IMAGEDEC_STATIC void word_clear(void *p, ushort c, uint n)
{

	ushort *ps = (ushort *)p;
	unsigned char *ps1 = (unsigned char *)p;

	if((unsigned long)p&0x1)
	{
		while (n)
		{
			*ps1++ = (c&0xff);
			*ps1++ = ((c>>8)&0xff);
			n--;
		}
	}
	else
	{
		while (n)
		{
			*ps++ = c;
			n--;
		}

	}
}

IMAGEDEC_STATIC int read_input_data(unsigned char *buf, int buf_size, bool *eof)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	DWORD ret;
//	jpeg_printf("buf addr <%x>\n",buf);
	ret = phdl->in_stream.fread(phdl->fh,buf, buf_size);

//	jpg_buffer_pos+=ret;

//	*eof = (jpg_buffer_pos >= jpg_buffer_len)? 1: 0;
	
	return ret; 
}

// Refill the input buffer.
// This method will sit in a loop until (A) the buffer is full or (B)
// the stream's read() method reports and end of file condition.

#ifdef JPEG_DUMP_ORI_FILE
UINT8 *g_jpeg_test_addr = NULL;
UINT32 g_jpeg_test_size = 0;
#endif

#ifdef IMAGEDEC_SINGLE_INPUT_BUF
void imagedec_m33_fill_vld_buff(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	pImagedec_hw_vld_mgr pmgr = &phdl->in_stream.vld_mgr;
	UINT32 free_size = pmgr->buf_size - pmgr->valid_size;
	int bytes_read = 0;
	int tmp = 0;

	if(1 == image_data_end)
		return;
	
	if(free_size)
	{
		if((pmgr->uwrite + free_size - 1) > pmgr->buf_end_addr )	
		{
			tmp = pmgr->buf_end_addr - pmgr->uwrite + 1;
			bytes_read = read_input_data((unsigned char *)pmgr->uwrite,tmp,&eof_flag);
			bytes_read += read_input_data((unsigned char *)pmgr->buf_start_addr
				,free_size - tmp,&eof_flag);			         
		}
		else
		{
			bytes_read = read_input_data((unsigned char *)pmgr->uwrite,free_size,&eof_flag);        
		}
		osal_cache_flush(pmgr->uwrite, bytes_read);
		jpeg_printf("Fill data %d\n",bytes_read);
		if((UINT32)bytes_read < free_size)
			image_data_end = 1;
		pmgr->valid_size += bytes_read;
		pmgr->uwrite += bytes_read;
		while(pmgr->uwrite >= pmgr->buf_end_addr)
			pmgr->uwrite -= pmgr->buf_size;
	}
}

void prep_in_buffer(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	phdl->in_stream.in_buf_left = 0;
	phdl->in_stream.Pin_buf_ofs = phdl->in_stream.in_buf;
	int bytes_read;

	if (image_data_end == 1)
	{
		jpeg_printf("==Read loader error, image data is error!\n");
		//terminate(JPGD_STREAM_READ);
		return;
	}
	
	if (eof_flag)
		return;
	
	bytes_read = read_input_data(phdl->in_stream.in_buf + phdl->in_stream.in_buf_left,
		                          JPGD_INBUFSIZE - phdl->in_stream.in_buf_left,
			                      &eof_flag);
#ifdef JPEG_DUMP_ORI_FILE
	MEMCPY((void *)g_jpeg_test_addr,phdl->in_stream.in_buf,bytes_read);
	g_jpeg_test_addr += bytes_read;
	g_jpeg_test_size += bytes_read;
#endif
	jpeg_printf("read data <%d>\n",bytes_read);
	if (bytes_read <= 0)
	{
		jpeg_printf("==Read loader error, can not read data!\n");
		terminate(JPGD_STREAM_READ);
		return;
	}
	else if (bytes_read < (JPGD_INBUFSIZE - phdl->in_stream.in_buf_left))
		image_data_end = 1;

	phdl->in_stream.in_buf_left += bytes_read;

	phdl->in_stream.total_bytes_read += phdl->in_stream.in_buf_left;

	word_clear(phdl->in_stream.in_buf + phdl->in_stream.in_buf_left, 0xD9FF, 64);
}
#else
void imagedec_fill_in_buffer(pImagedec_hdl phdl,UINT8 buf_idx)
{
	pImagedec_in_buf_item pbuf_item = &( phdl->in_stream.buf_item[buf_idx]);
	UINT8 *buf_addr = pbuf_item->in_buf;
	int bytes_read = 0;
	JPEG_ENTRY;
#if 0
	if (image_data_end == 1)
	{
		jpeg_printf("==Read loader error, image data is error!\n");
		//terminate(JPGD_STREAM_READ);
		return;
	}
	if (eof_flag)
		return;
#endif

#if 1	// for debug	
	if(pbuf_item->busy)
	{
		jpeg_printf("buf busy<%d>\n",buf_idx);
		terminate(JPGD_IN_BUF_BUSY);
	}
#endif	

	bytes_read = read_input_data(buf_addr,JPGD_INBUFSIZE,&eof_flag);
	if (bytes_read <= 0)
	{
		jpeg_printf("can not read data!\n");
		pbuf_item->error = 1;
		return;
	}
	else if (bytes_read < JPGD_INBUFSIZE)
	{
		pbuf_item->end = 1;
	}
	pbuf_item->in_buf_left = bytes_read;
	pbuf_item->busy = 1;
	pbuf_item->end = 0;
	phdl->in_stream.total_bytes_read += pbuf_item->in_buf_left;
	word_clear(pbuf_item->in_buf + pbuf_item->in_buf_left, 0xD9FF, 64);
	jpeg_printf("fill buffer <%d>idx<%d>\n",bytes_read,buf_idx);
	JPEG_EXIT;
}

void prep_in_buffer(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	UINT8 idx = 1 - phdl->in_stream.buf_idx_using;
	ER s_flag;
	
	JPEG_ENTRY;
	if (image_data_end == 1)
	{
		jpeg_printf("==Read loader error, image data is error!\n");
		//terminate(JPGD_STREAM_READ);
		return;
	}
	
	if (eof_flag)
		return;
#if 1
	if(!phdl->in_stream.buf_item[idx].busy)
	{
		jpeg_printf("in buf free error <%d><%s>\n",idx,__FUNCTION__);
		terminate(JPGD_IN_BUF_BUSY);
	}
#endif
	jpeg_printf("swith to the buf idx <%d>\n",idx);
	phdl->in_stream.buf_item[idx].busy = 0;
	phdl->in_stream.in_buf = phdl->in_stream.buf_item[idx].in_buf;
	phdl->in_stream.in_buf_left = phdl->in_stream.buf_item[idx].in_buf_left;
	phdl->in_stream.Pin_buf_ofs = phdl->in_stream.in_buf;
	phdl->in_stream.buf_idx_using = idx;
	if(phdl->in_stream.buf_item[idx].error)
	{
		phdl->in_stream.buf_item[idx].error = 0;
		jpeg_printf("set the read error flag\n");
		terminate(JPGD_STREAM_READ);
		return;
	}
	image_data_end = phdl->in_stream.buf_item[idx].end;
	if(1 != image_data_end)
	{
		/*set the flag to make the task fill the buffer*/
		s_flag = osal_flag_set(g_imagedec_wait_flag[phdl->id],IMAGEDEC_FILL_IN_BUF_PTN);
		if(E_OK != s_flag)
		{
				jpeg_printf("set flag error<%s>\n",__FUNCTION__);
		}
	}
	JPEG_EXIT;
}
#endif

//////////////////////// control functions ///////////////////////////////////////
IMAGEDEC_STATIC int selected_comp (int comp_id) // if it is the right compenent to display
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	volatile int iReg = FALSE;

	if (1==phdl->decoder.comps_in_frame){ // if 1, gray pic.
		if (0 == iset_comp && 0 == comp_id) 
			iReg = TRUE;
	}
	else{
		if (0==iset_comp){ // Y
			if (0<=comp_id && (phdl->decoder.comps_in_frame-2) > comp_id)
				iReg = TRUE;
		}
		else if (1 == iset_comp){ // Cb
			if ((phdl->decoder.comps_in_frame-2) == comp_id) 
				iReg = TRUE;
		}
		else if (2 == iset_comp){ // Cb
			if ((phdl->decoder.comps_in_frame-1) == comp_id) 
				iReg = TRUE;
		}
	}
	return iReg;
}

//------------------------------------------------------------------------------
// Restart interval processing.
IMAGEDEC_STATIC void process_restart(void)
{
  int i, c = 0;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

  // Align to a byte boundry
  // FIXME: Is this really necessary? get_bits_2() never reads in markers!
  //get_bits_2(phdl->decoder.bits_left & 7);

  // Let's scan a little bit to find the marker, but not _too_ far.
  // 1536 is a "fudge factor" that determines how much to scan.
  for (i = 1536; i > 0; i--)
    if (get_char() == 0xFF)
      break;


  if (i == 0)
    terminate(JPGD_BAD_RESTART_MARKER);

  for ( ; i > 0; i--)
    if ((c = get_char()) != 0xFF)
      break;

  if (i == 0)
    terminate(JPGD_BAD_RESTART_MARKER);

  // Is it the expected marker? If not, something bad happened.
  if (c != (phdl->decoder.next_restart_num + M_RST0))
    terminate(JPGD_BAD_RESTART_MARKER);

  // Reset each phdl->decoder.component's DC prediction values.
  MEMSET(&phdl->decoder.last_dc_val, 0, phdl->decoder.comps_in_frame * sizeof(uint));

  phdl->decoder.eob_run = 0;

  phdl->decoder.restarts_left = phdl->decoder.restart_interval;

  phdl->decoder.next_restart_num = (phdl->decoder.next_restart_num + 1) & 7;

  // Get the bit buffer going again...

  {
    phdl->decoder.bits_left = 16;
    //bit.bit_buf = 0;
    get_bits_2(16);
    get_bits_2(16);
  }
}
//------------------------------------------------------------------------------
// This method throws back into the stream any bytes that where read
// into the bit buffer during initial marker scanning.
static void fix_in_buffer(void)
{
	/* In case any 0xFF's where pulled into the buffer during marker scanning */
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	if (phdl->decoder.bits_left == 16)
		stuff_char( (uchar)((phdl->decoder.bit.bit_buf >> 16) & 0xFF));

	if (phdl->decoder.bits_left >= 8)
		stuff_char( (uchar)((phdl->decoder.bit.bit_buf >> 24) & 0xFF));

	stuff_char( (uchar)(phdl->decoder.bit.bit_buf & 0xFF) );
	stuff_char( (uchar)((phdl->decoder.bit.bit_buf >> 8) & 0xFF) );
	phdl->decoder.bits_left = 16;
	//bit.bit_buf = 0;
	get_bits_2(16);
	get_bits_2(16);
}
//////////////////////// decoding relative functions ////////////////////////
//------------------------------------------------------------------------------
// The coeff_buf series of methods originally stored the coefficients
// into a "virtual" file which was located in EMS, XMS, or a disk file. A cache
// was used to make this process more efficient. Now, we can store the entire
// thing in RAM.
IMAGEDEC_STATIC Pcoeff_buf_t coeff_buf_open(
  int block_num_x, int block_num_y,
  int block_len_x, int block_len_y)
{
  Pcoeff_buf_t cb = (Pcoeff_buf_t)imagedec_malloc(sizeof(coeff_buf_t));

  if (IsTerminated()) 
  {
	  jpeg_printf("error\n");
	  return NULL; // error check and mem control. peter luo. 6-10.
  }

  cb->block_num_x = block_num_x;
  cb->block_num_y = block_num_y;

  cb->block_len_x = block_len_x;
  cb->block_len_y = block_len_y;

  cb->block_size = (block_len_x * block_len_y) * sizeof(BLOCK_TYPE);
  jpeg_printf("coeff_buf_open(),cb->block_size=%d, block_num_x=%d, block_num_y=%d\n",cb->block_size, block_num_x, block_num_y);
  
  cb->Pdata = (uchar *)imagedec_malloc(cb->block_size * block_num_x * block_num_y);
  jpeg_printf(" cb->Pdata = 0x%x size=%d\n", cb->Pdata,cb->block_size * block_num_x * block_num_y);
  if (IsTerminated()) return NULL; // error check and mem control. peter luo. 6-10.


  return cb;
}

//------------------------------------------------------------------------------
IMAGEDEC_STATIC  BLOCK_TYPE *coeff_buf_getp(
  Pcoeff_buf_t cb,
  int block_x, int block_y)
{
	BLOCK_TYPE *pOut = (BLOCK_TYPE *)(cb->Pdata + block_x * cb->block_size + block_y * (cb->block_size * cb->block_num_x));
	
  if (block_x >= cb->block_num_x)
    iterminate(JPGD_ASSERTION_ERROR);

  if (block_y >= cb->block_num_y)
    iterminate(JPGD_ASSERTION_ERROR);

  if (((unsigned long)pOut & 0xfffffff)>(g_imagedec_hdl[0].mem.mem_up_limit & 0xfffffff)) { // although it may not be the error of decoding itself, it can be the late flush of cache and mem, we only verify it. Peter luo. 2003-7-19.
      jpeg_printf("in %s, memory ERROR!\n",__FUNCTION__);//SDBBP();
      iterminate(JPGD_DECODE_ERROR1);
  }

//  return (BLOCK_TYPE *)(cb->Pdata + block_x * cb->block_size + block_y * (cb->block_size * cb->block_num_x));
  return pOut;
}

IMAGEDEC_STATIC void process_restart_hw_vld()
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	pImagedec_hw_vld_mgr pvld_mgr = &phdl->in_stream.vld_mgr;
	UINT32 tmp_read = pvld_mgr->uread;
	UINT32 read_bytes = 0;
	UINT8 code = 0;
	int i = 0;

	if(pvld_mgr->valid_size < JPEG_HW_VLD_MIN_DATA_SIZE)
		imagedec_m33_fill_vld_buff();

	if(pvld_mgr->used_bits)
	{
		if(tmp_read == pvld_mgr->buf_end_addr)
			tmp_read = pvld_mgr->buf_start_addr;
		else
			tmp_read++;
		read_bytes++;
	}

	for(i = 1536;i > 0;i--)
	{
		code = *(unsigned  char *)tmp_read;

		if(tmp_read == pvld_mgr->buf_end_addr)
			tmp_read = pvld_mgr->buf_start_addr;
		else
			tmp_read++;
		read_bytes++;

		if(0xFF == code)
			break;	
	}

	if ((0 == i) || (read_bytes > pvld_mgr->valid_size))
		terminate(JPGD_BAD_RESTART_MARKER);
	
	for (;i > 0;i--)
	{
		code = *(unsigned  char *)tmp_read;
		if(tmp_read == pvld_mgr->buf_end_addr)
			tmp_read = pvld_mgr->buf_start_addr;
		else
			tmp_read++;
		read_bytes++;
	
		if(0xFF != code)
			break;
	}
	
	if ((0 == i) || (read_bytes > pvld_mgr->valid_size))
		terminate(JPGD_BAD_RESTART_MARKER);

	// Is it the expected marker? If not, something bad happened.
	if (code != (phdl->decoder.next_restart_num + M_RST0))
		terminate(JPGD_BAD_RESTART_MARKER);

	phdl->decoder.restarts_left = phdl->decoder.restart_interval;
	phdl->decoder.next_restart_num = (phdl->decoder.next_restart_num + 1) & 7;
  
	pvld_mgr->valid_size -= read_bytes;
  	pvld_mgr->uread = tmp_read;
	pvld_mgr->used_bits = 0;
}

IMAGEDEC_STATIC void hw_vld_new_mcu(pImagedec_hdl phdl,UINT8 mcu_flag)
{
	pImagedec_hw_vld_mgr pvld_mgr = &phdl->in_stream.vld_mgr;

	if(pvld_mgr->valid_size < JPEG_HW_VLD_MIN_DATA_SIZE)
		imagedec_m33_fill_vld_buff();

	pvld_mgr->cfg_length = pvld_mgr->valid_size;
	pvld_mgr->cfg_offset = pvld_mgr->uread - pvld_mgr->buf_start_addr;

	pvld_mgr->cfg_control_bits &= ~JPEG_HW_VLD_FIRST;
	pvld_mgr->cfg_control_bits |= JPEG_HW_VLD_VALID;
	
	if(1 == mcu_flag)
		pvld_mgr->cfg_control_bits |= JPEG_HW_VLD_FIRST;
	else if(2 == mcu_flag)
		pvld_mgr->cfg_control_bits |= JPEG_HW_VLD_LAST | JPEG_HW_VLD_FIRST;

	jh_m33_config_hw_vld(pvld_mgr);
	jh_hw_vld_mcu_start();		
}

IMAGEDEC_STATIC void hw_vld_prepare_next_mcu(pImagedec_hdl phdl,UINT8 finish)
{
	pImagedec_hw_vld_mgr pvld_mgr = &phdl->in_stream.vld_mgr;
	UINT32 hw_offset = 0,hw_cur_posi = 0,cfg_end_addr=0,noused = 0;
    UINT32 valid_size = pvld_mgr->valid_size;
	UINT32 buf_start = pvld_mgr->buf_start_addr;
	UINT32 buf_size = pvld_mgr->buf_size;
	UINT32 buf_end = pvld_mgr->buf_end_addr;

	if(finish)
	{
		hw_offset = jh_hw_vld_get_offset();
	
		pvld_mgr->used_bits = (hw_offset & 0x07);

		hw_offset >>= 3;
		hw_cur_posi = buf_start + hw_offset;
		while(hw_cur_posi>buf_end)
			hw_cur_posi -= buf_size;

		cfg_end_addr = buf_start + pvld_mgr->cfg_offset + pvld_mgr->cfg_length;
		if(cfg_end_addr > buf_end)
			cfg_end_addr -= buf_size;	
		if(cfg_end_addr >= hw_cur_posi)
			noused = cfg_end_addr - hw_cur_posi;
		else
			noused = cfg_end_addr + buf_size - hw_cur_posi;

		if((hw_cur_posi > buf_end) ||(cfg_end_addr > buf_end) || (noused > buf_size ))
			{JPEG_ASSERT(0);}			
	}
	else
	{
		hw_cur_posi = buf_start + pvld_mgr->cfg_offset+pvld_mgr->cfg_length;
		if(hw_cur_posi>buf_end)
			hw_cur_posi -= buf_size;
		noused = 0;		
	}
	if(pvld_mgr->valid_size < pvld_mgr->cfg_length)
		{JPEG_ASSERT(0);}
	pvld_mgr->valid_size = pvld_mgr->valid_size - pvld_mgr->cfg_length + noused;	
	pvld_mgr->uread = hw_cur_posi;

	if(pvld_mgr->valid_size < JPEG_HW_VLD_MIN_DATA_SIZE)
		imagedec_m33_fill_vld_buff();
	
	if(finish)
	{
		UINT32 last_pos = 0;

		if(buf_start == pvld_mgr->uread)
			last_pos = pvld_mgr->buf_end_addr;
		else
			last_pos = pvld_mgr->uread - 1;

		if(0xFF == *(UINT8 *)last_pos)
		{
			if(pvld_mgr->valid_size > 0)
			{
				if((0 == pvld_mgr->used_bits) && (0 == *(UINT8 *)pvld_mgr->uread))
				{
					// skip one byte when meet the case 0xFF00
					jpeg_printf("Skip one byte when meet the special symbol 0xFF00\n");
					pvld_mgr->uread++;
					pvld_mgr->valid_size--;
					while(pvld_mgr->uread > pvld_mgr->buf_end_addr)
						pvld_mgr->uread -= pvld_mgr->buf_size;
				}
			}
		}
	}
}

#define HUFFMAN_DEC
IMAGEDEC_STATIC void decode_next_row_hw_vld(int last_row)
{
	int mcu_row;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	pImagedec_hw_vld_mgr pvld_mgr = &phdl->in_stream.vld_mgr;	
	UINT8 mcu_flag = 0;
	UINT32 cnt_cos = 0;

	for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
	{
		if(g_imagedec_m33_combine_hw_sw_flag && (phdl->imageout.hw_max_divide_h > 0))
		{
			UINT32 y_addr, c_addr;
			UINT32 x_base_offset;
			
			if(phdl->imageout.divide_mcu_row >= JPEG_HW_MAX_MCU_VALUE)
			{
				phdl->imageout.divide_mcu_row = 0;
				phdl->imageout.last_mcu_row = mcu_row;				
				x_base_offset = phdl->imageout.last_mcu_row<<(phdl->imageout.pixel_per_mcu_h - phdl->imageout.hw_h_pre);
				y_addr = phdl->imageout.hw_acc_frm_y + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xF);
				switch(phdl->decoder.scan_type)
				{
					case JPGD_YH1V1:
					case JPGD_YH2V1:
						x_base_offset &= ~0x1;
						c_addr = phdl->imageout.hw_acc_frm_c + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xE);
						break;
					case JPGD_YH4V1:
						x_base_offset >>= 1;
						c_addr = phdl->imageout.hw_acc_frm_c + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xE);			
						break;
					case JPGD_YH1V2:
					case JPGD_YH2V2:
						x_base_offset &= ~0x01;
						c_addr = phdl->imageout.hw_acc_frm_c + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xE);	
						break;
					default:
						return;	
				}
			
				jh_frame_addr(y_addr, c_addr);		
			}
			
			jh_reg_mcu_coord_x(phdl->imageout.divide_mcu_row);	
			phdl->imageout.divide_mcu_row++;			
		}
		else if(g_imagedec_m33_extend_hw_vld && (phdl->imageout.hw_max_divide_h > 0))
		{
			UINT32 y_addr, c_addr;
			UINT32 y_base_offset, x_base_offset;
			
			if(phdl->imageout.divide_mcu_row >= JPEG_HW_MAX_MCU_VALUE)
			{
				phdl->imageout.divide_mcu_row = 0;
				phdl->imageout.last_mcu_row = mcu_row;
				y_base_offset = phdl->imageout.last_mcu_col<<(phdl->imageout.pixel_per_mcu_v - phdl->imageout.hw_v_pre);
				x_base_offset = phdl->imageout.last_mcu_row<<(phdl->imageout.pixel_per_mcu_h - phdl->imageout.hw_h_pre);
				y_addr = phdl->imageout.hw_acc_frm_y + (y_base_offset & 0xFFF0) * phdl->imageout.hw_acc_stride 
					+ ((y_base_offset & 0x0F) << 4) + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xF);
				switch(phdl->decoder.scan_type)
				{
					case JPGD_YH1V1:
					case JPGD_YH2V1:
						x_base_offset &= ~0x1;
						c_addr = phdl->imageout.hw_acc_frm_c + (y_base_offset & 0xFFF0) * phdl->imageout.hw_acc_stride 
							+ ((y_base_offset & 0x0F) << 4) + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xE);
						break;
					case JPGD_YH4V1:
						x_base_offset >>= 1;
						c_addr = phdl->imageout.hw_acc_frm_c + (y_base_offset & 0xFFF0) * (phdl->imageout.hw_acc_stride>>1) 
							+ ((y_base_offset & 0x0F) << 4) + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xE);			
						break;
					case JPGD_YH1V2:
					case JPGD_YH2V2:
						y_base_offset >>= 1;
						x_base_offset &= ~0x01;
						c_addr = phdl->imageout.hw_acc_frm_c + (y_base_offset & 0xFFF0) * phdl->imageout.hw_acc_stride 
							+ ((y_base_offset & 0x0F) << 4) + ((x_base_offset & 0xFFF0)<<4) + (x_base_offset & 0xE);	
						break;
					default:
						return;	
				}
			
				jh_frame_addr(y_addr, c_addr);
				jh_reg_mcu_coord_y(phdl->imageout.divide_mcu_col);
			}
			
			jh_reg_mcu_coord_x(phdl->imageout.divide_mcu_row);	
			phdl->imageout.divide_mcu_row++;
		}
		else	
			jh_reg_mcu_coord_x(mcu_row);
		
		if ((phdl->decoder.restart_interval) && (phdl->decoder.restarts_left == 0))
		{
			process_restart_hw_vld();
			jh_reset_dc_value();
		}

#if 0
		if(last_row && (mcu_row >= phdl->decoder.mcus_per_row - 1))
			mcu_flag = 2;
		else
			mcu_flag = 1;
#else
		if(image_data_end)
			mcu_flag = 2;
		else
			mcu_flag = 1;
#endif
		
HW_CONTINUE:		
		hw_vld_new_mcu(phdl, mcu_flag);
		mcu_flag = 0;
		cnt_cos = 0;		
		while(jh_hw_jpeg_busy())
		{
			cnt_cos++;
			if(cnt_cos >= 2000)// change from 100) for the 120M mem clock solution
			{
				jpeg_printf("hw jpeg time out \n");
				return;
			}

			if(jh_hw_request_data())
			{
				jh_hw_clear_status();				
				break;
			}
		}

		if(jh_hw_request_data())
		{
			if((pvld_mgr->valid_size > 3) || (0 == image_data_end))
			{
				jpeg_printf("not enough data for current MCU %d \n", pvld_mgr->valid_size);	
				hw_vld_prepare_next_mcu(phdl,0);
				goto HW_CONTINUE;
			}
			else
			{
				jpeg_printf("mcu data lost\n");
				jh_hw_reset();
				return;
			}
		}
		else
			hw_vld_prepare_next_mcu(phdl,1);
		
		phdl->decoder.restarts_left--;
	}
	cur_decoding_mcu_row++;
}

// Retrieves a variable number of bits from the input stream.
// Markers will not be read into the input bit buffer. Instead,
// an infinite number of all 1's will be returned when a marker
// is encountered.
// FIXME: Is it better to return all 0's instead, like the older implementation?
// this function should be near the decode_nex_row(). it is not a inline function. 
// so it can decrease the operation of instrucion cache refresh
IMAGEDEC_INLINE uint get_bits_2(int numbits)
{
	uint i;
	uint c1;
	uint c2;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	i = (phdl->decoder.bit.bit_buf >> (16 - numbits)) & ((1 << numbits) - 1);
	if ((phdl->decoder.bits_left -= numbits) <= 0)
	{
		//    bit.bit_buf = rol(bit.bit_buf, numbits += bits_left);
		numbits = numbits + phdl->decoder.bits_left; // ...+= (negative num)... is not always right in X6304, so... Peter luo, 6-13.
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, numbits);
		//c1 = get_octet();
		//c2 = get_octet();
		c1 = get_octet2(); //change005
		c2 = get_octet2();
		phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf & 0xFFFF) | (((ulong)c1) << 24) | (((ulong)c2) << 16);
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, -phdl->decoder.bits_left);
		phdl->decoder.bits_left += 16;
	}
	else
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, numbits);

	return i;
}

IMAGEDEC_STATIC void decode_next_row_hw(void)
{
	Phuff_tables_t Ph;
	int prev_num_set;
	int component_id;
	int mcu_row, mcu_block;
	int row_block = 0;
	int r, s;
	int k;
	int *p; //bugfix001 use 32 bits for IDCT buffer
	QUANT_TYPE *q;
	int n;
	int kt;
	int i;
	int run = 0;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	UINT32 cnt_cos = 0;

	for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
	{
		cnt_cos = 0;
		while(jh_hw_jpeg_busy())
		{
			cnt_cos++;
			if(cnt_cos >= 1000)// change from 100) for the 120M mem clock solution
			{
				jpeg_printf("hw jpeg time out \n");
				return;
			}
		}
		jh_reg_mcu_coord_x(mcu_row);
		if ((phdl->decoder.restart_interval) && (phdl->decoder.restarts_left == 0))
		{
			process_restart();
			jh_reset_dc_value();
		}
		jh_mcu_start();
		for (mcu_block = 0; mcu_block < phdl->decoder.blocks_per_mcu; mcu_block++)
		{
			cnt_cos = 0;
			while(jh_hw_iqis_buf_full())
			{
				cnt_cos++;
				if(cnt_cos >= 1000)
				{
				jpeg_printf("hw jpeg time out2\n");
				return;
				}
			}
			if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.
			component_id = phdl->decoder.mcu_org[mcu_block];
			q = phdl->decoder.quant[phdl->decoder.comp_quant[component_id]];
			if ((s = huff_decode(phdl->decoder.h[phdl->decoder.comp_dc_tab[component_id]])) != 0)
			{
				r = get_bits_2(s);
				s = HUFF_EXTEND(r, s);
				jh_input_rlc_data(0,s);
			}
			else
			{
				jh_input_rlc_data(0,0);
			}
			s += phdl->decoder.last_dc_val[component_id];
			phdl->decoder.last_dc_val[component_id] = s;
			Ph = phdl->decoder.h[phdl->decoder.comp_ac_tab[component_id]];
			for (k = 1; k < 64; k++)
			{
				s = huff_decode(Ph);

				r = s >> 4;run = r;
				s &= 15;
				if (s)
				{
					k += r;

					r = get_bits_2(s);
					s = HUFF_EXTEND(r, s);
					jh_input_rlc_data(run,s);
				}
				else
				{
					if (r == 15)
					{
						jh_input_rlc_data(run,0);
						k += 15;
						if (k > 63) //change005
							terminate(JPGD_DECODE_ERROR4);
					}
					else
					{
						break;
					}
				}						
			}
			row_block++; 
			jh_unit_start();
		}
		phdl->decoder.restarts_left--;
	}
	cur_decoding_mcu_row++;
}

// Decodes and dephdl->decoder.quantizes the next row of coefficients.
IMAGEDEC_STATIC void decode_next_row(void)
{
	Phuff_tables_t Ph;
	int prev_num_set;
	int component_id;
	int mcu_row, mcu_block;
	int row_block = 0;
	int r, s;
	int k;
	int *p; //bugfix001 use 32 bits for IDCT buffer
	QUANT_TYPE *q;
	int n;
	int kt;
	int i;

	pImagedec_hdl phdl = &g_imagedec_hdl[0];

//	libc_printf("cur_decoding_mcu_row:%d\n",cur_decoding_mcu_row);
//	if(cur_decoding_mcu_row>=20)
//		return;

  // Clearing the entire row block buffer can take a lot of time!
  // Instead of clearing the entire buffer each row, keep track
  // of the number of nonzero entries written to each block and do
  // selective clears.
  //MEMSET(phdl->decoder.block_seg[0], 0, phdl->decoder.mcus_per_row * phdl->decoder.blocks_per_mcu * 64 * sizeof(BLOCK_TYPE));	

  for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
  {
	if ((phdl->decoder.restart_interval) && (phdl->decoder.restarts_left == 0))
	{
      		process_restart();
	}
    for (mcu_block = 0; mcu_block < phdl->decoder.blocks_per_mcu; mcu_block++)
    {
	  if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.
      component_id = phdl->decoder.mcu_org[mcu_block];
	  
      //p = phdl->decoder.block_seg[row_block];
	  //manson: I use a temporary working buffer for IDCT
	  MEMSET(temp_IDCT_buf, 0, 64 * sizeof(int)); //manson: please implement a fast memset for T6304

	  p = temp_IDCT_buf;

      q = phdl->decoder.quant[phdl->decoder.comp_quant[component_id]];

      if ((s = huff_decode(phdl->decoder.h[phdl->decoder.comp_dc_tab[component_id]])) != 0)
      {
        r = get_bits_2(s);
        s = HUFF_EXTEND(r, s);
      }
	  s += phdl->decoder.last_dc_val[component_id];
      phdl->decoder.last_dc_val[component_id] = s;

        p[0] = s * q[0];

   //john seem not use   prev_num_set = block_max_zag_set[row_block];

      Ph = phdl->decoder.h[phdl->decoder.comp_ac_tab[component_id]];

      for (k = 1; k < 64; k++)
      {
        s = huff_decode(Ph);

        r = s >> 4;
        s &= 15;
        if (s)
        {
          //if (r)
          //{
	        //manson: we will check after the loop
            //if ((k + r) > 63) // this should be the total num <= 63 as Robot said. Peter luo 6-13.
            //  terminate(JPGD_DECODE_ERROR); //change005

			//manson: I use memset instead
			/*
            if (k < prev_num_set)
            {
              n = min(r, prev_num_set - k);
              kt = k;
              while (n--)
                p[ZAG[kt++]] = 0;
            }
			*/
            k += r;
          //}

          r = get_bits_2(s);
          s = HUFF_EXTEND(r, s);
          //assert(k < 64);
          p[ZAG[k]] = s * q[k];
		//  if((cur_decoding_mcu_row==3)&&(mcu_row==22))
		 //  libc_printf("p[ZAG[k]]:%d %x\n",ZAG[k],p[ZAG[k]]);
        }
        else
        {
          if (r == 15)
          {
	        //manson: we will check after the loop

			//manson: I use memset instead
			/*
            if (k < prev_num_set)
            {
              int n = min(16, prev_num_set - k);		//bugfix Dec. 19, 2001 - was 15!
              int kt = k;
              while (n--)
                p[ZAG[kt++]] = 0;
            }
			*/
            k += 15;
            if (k > 63) //change005
              terminate(JPGD_DECODE_ERROR4);
          }
          else
          {
            //while (k < 64)
            //  p[ZAG[k++]] = 0;
            break;
          }
        }
        //if (k > 64) //manson: optimize for speed //change005
        //    terminate(JPGD_DECODE_ERROR);
      }

	  /*//manson: I use memset instead
      if (k < prev_num_set)
      {
        kt = k;
        while (kt < prev_num_set)
          p[ZAG[kt++]] = 0;
      }
      block_max_zag_set[row_block] = k;
      //block_num[row_block++] = k;
	  */
      row_block++; 
	  if(IMAGEDEC_REAL_SIZE != phdl->main.setting.mode)
		  IDCT_Scaling_Routine(phdl,0);
	  else 
	         IDCT_Duplication(phdl,mcu_row,mcu_block,0);
	  //idct(temp_IDCT_buf, temp_IDCT_buf2);
	//  if(mcu_row == 1)
	//  SDBBP();
//	  idct(temp_IDCT_buf, temp_IDCT_buf2);//SDBBP();
//	  idct4x4(temp_IDCT_buf, temp_IDCT_buf2);

//	  dump_data(mcu_row,mcu_block,tem1 );

	  if((cur_decoding_mcu_row==3)&&(mcu_row==22))
	  {
		//  jpeg_printf("dump_len1 = %d block:%d type:%d\n",dump_len,mcu_row,mcu_block);
		//  SDBBP();
	  }
	//  copy_data_to_frm(mcu_row,mcu_block,temp_IDCT_buf2 );
    }

   if(IMAGEDEC_REAL_SIZE == phdl->main.setting.mode)
	 imagedec_idct_dup_next_mcu(phdl);
    phdl->decoder.restarts_left--;
  }
//  libc_printf("decoder row:%d\n",cur_decoding_mcu_row);
//  if(cur_decoding_mcu_row==6)
//	SDBBP();
  cur_decoding_mcu_row++;

  /*update the next row status for the duplication*/
  if(IMAGEDEC_REAL_SIZE == phdl->main.setting.mode)
	 imagedec_idct_dup_next_row(phdl);	
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
IMAGEDEC_STATIC  	void decode_block_dc_refine(
  int component_id, int block_x, int block_y)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
  if (get_bits_2(1))
  {
//jpeg_printf("comid2:%d\n",component_id);
	  BLOCK_TYPE *p = coeff_buf_getp(phdl->decoder.dc_coeffs[component_id], block_x, block_y);

	 if (selected_comp (component_id) ||iPro2_progressive) // if not pro2, the selected Y, Cb, or Cr. peter luo 7-18.
	 //if (selected_comp (component_id)) // if not pro2, the selected Y, Cb, or Cr. peter luo 7-18.
    p[0] |= (1 << phdl->decoder.successive_low);
  }
}
//------------------------------------------------------------------------------
IMAGEDEC_STATIC  void decode_block_ac_first(
  int component_id, int block_x, int block_y)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
  int k, s, r;

  if (phdl->decoder.eob_run)
  {
    phdl->decoder.eob_run--;
    return;
  }
//jpeg_printf("comid1:%d\n",component_id);
  BLOCK_TYPE *p = coeff_buf_getp(phdl->decoder.ac_coeffs[component_id], block_x, block_y);

  for (k = phdl->decoder.spectral_start; k <= phdl->decoder.spectral_end; k++)
  {
    s = huff_decode(phdl->decoder.h[phdl->decoder.comp_ac_tab[component_id]]);

    r = s >> 4;
    s &= 15;

    if (s)
    {
      if ((k += r) > 63)
        terminate(JPGD_DECODE_ERROR2);

      r = get_bits_2(s);
      s = HUFF_EXTEND_P(r, s);

	if (selected_comp (component_id) || iPro2_progressive) // if not pro2, the selected Y, Cb, or Cr. peter luo 7-18.
	//if (selected_comp (component_id)) // if not pro2, the selected Y, Cb, or Cr. peter luo 7-18.
      p[ZAG[k]] = s << phdl->decoder.successive_low;
    }
    else
    {
      if (r == 15)
      {
        if ((k += 15) > 63)
          terminate(JPGD_DECODE_ERROR3);
      }
      else
      {
        phdl->decoder.eob_run = 1 << r;

        if (r)
          phdl->decoder.eob_run += get_bits_2(r);

        phdl->decoder.eob_run--;

        break;
      }
    }
  }
}

// The following methods decode the various types of blocks encountered
// in progressively encoded images.
IMAGEDEC_STATIC  void decode_block_dc_first(
  int component_id, int block_x, int block_y)
{
 pImagedec_hdl phdl = &g_imagedec_hdl[0];
  int s, r;
//jpeg_printf("comid3:%d\n",component_id);
  BLOCK_TYPE *p = coeff_buf_getp(phdl->decoder.dc_coeffs[component_id], block_x, block_y);

  if ((s = huff_decode(phdl->decoder.h[phdl->decoder.comp_dc_tab[component_id]])) != 0)
  {
    r = get_bits_2(s);
    s = HUFF_EXTEND_P(r, s);
  }

  phdl->decoder.last_dc_val[component_id] = (s += phdl->decoder.last_dc_val[component_id]);

  if (selected_comp (component_id) || iPro2_progressive) // if not pro2, the selected Y, Cb, or Cr. peter luo 7-18.
  //if (selected_comp (component_id)) // if not pro2, the selected Y, Cb, or Cr. peter luo 7-18.
  p[0] = s << phdl->decoder.successive_low;
}

//------------------------------------------------------------------------------
IMAGEDEC_STATIC  void decode_block_ac_refine(
  int component_id, int block_x, int block_y)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
  int s, k, r;
  int p1 = 1 << phdl->decoder.successive_low;
  int m1 = (-1) << phdl->decoder.successive_low;
  BLOCK_TYPE *p;

  // if pro2, we donot decode the refine part. peter luo. 7-18.
  if (!selected_comp (component_id) || iPro2_progressive) // the selected Y, Cb, or Cr. peter luo 6-5
	  return;
//jpeg_printf("comid4:%d\n",component_id);
  p = coeff_buf_getp( phdl->decoder.ac_coeffs[component_id], block_x, block_y);

  k =  phdl->decoder.spectral_start;

  if ( phdl->decoder.eob_run == 0)
  {
    for ( ; k <=  phdl->decoder.spectral_end; k++)
    {
      s = huff_decode( phdl->decoder.h[ phdl->decoder.comp_ac_tab[component_id]]);

      r = s >> 4;
      s &= 15;

      if (s)
      {
        if (s != 1)
          terminate(JPGD_DECODE_ERROR4);

        if (get_bits_2(1))
          s = p1;
        else
          s = m1;
      }
      else
      {
        if (r != 15)
        {
           phdl->decoder.eob_run = 1 << r;

          if (r)
             phdl->decoder.eob_run += get_bits_2(r);

          break;
        }
      }

      do
      {
        BLOCK_TYPE *this_coef = p + ZAG[k];

        if (*this_coef != 0)
        {
          if (get_bits_2(1))
          {
            if ((*this_coef & p1) == 0)
            {
              if (*this_coef >= 0)
                *this_coef += p1;
              else
                *this_coef += m1;
            }
          }
        }
        else
        {
          if (--r < 0)
            break;
        }

        k++;

      } while (k <=  phdl->decoder.spectral_end);

      if ((s) && (k < 64))
      {
        p[ZAG[k]] = s;
      }
    }
  }

  if ( phdl->decoder.eob_run > 0)
  {
    for ( ; k <=  phdl->decoder.spectral_end; k++)
    {
      BLOCK_TYPE *this_coef = p + ZAG[k];

      if (*this_coef != 0)
      {
        if (get_bits_2(1))
        {
          if ((*this_coef & p1) == 0)
          {
            if (*this_coef >= 0)
              *this_coef += p1;
            else
              *this_coef += m1;
          }
        }
      }
    }

     phdl->decoder.eob_run--;
  }
}
#define JPEG_HEADER
//------------------------------------------------------------------------------
// Read a Huffman code table.
IMAGEDEC_STATIC void read_dht_marker(void)
{
  int i, index, count;
  uint left;
  uchar l_huff_num[17];
  uchar l_huff_val[256];
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

  left = get_bits_1(16);

  if (left < 2)
    terminate(JPGD_BAD_DHT_MARKER);

  left -= 2;

  while (left)
  {
    index = get_bits_1(8);

    l_huff_num[0] = 0;

    count = 0;

    for (i = 1; i <= 16; i++)
    {
      l_huff_num[i] = get_bits_1(8);
      count += l_huff_num[i];
    }

    if (count > 255)
      terminate(JPGD_BAD_DHT_COUNTS);

    for (i = 0; i < count; i++)
      l_huff_val[i] = get_bits_1(8);

    i = 1 + 16 + count;

    if (left < (uint)i)
      terminate(JPGD_BAD_DHT_MARKER);

    left -= i;

    if ((index & 0x10) > 0x10)
      terminate(JPGD_BAD_DHT_INDEX);

    index = (index & 0x0F) + ((index & 0x10) >> 4) * (JPGD_MAXHUFFTABLES >> 1);

    if (index >= JPGD_MAXHUFFTABLES)
      terminate(JPGD_BAD_DHT_INDEX);

    if (!phdl->decoder.huff_num[index])
      phdl->decoder.huff_num[index] = (uchar *)imagedec_malloc(17);

    if (!phdl->decoder.huff_val[index])
      phdl->decoder.huff_val[index] = (uchar *)imagedec_malloc(256);

	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

    mem_memcpy(phdl->decoder.huff_num[index], l_huff_num, 17);
    mem_memcpy(phdl->decoder.huff_val[index], l_huff_val, 256);
  }
}
//------------------------------------------------------------------------------
// Read a phdl->decoder.quantization table.
IMAGEDEC_STATIC void read_dqt_marker(void)
{
  int n, i, prec;
  uint left;
  uint temp;
  pImagedec_hdl phdl = &g_imagedec_hdl[0];

  JPEG_ENTRY;
  left = get_bits_1(16);

  if (left < 2)
    terminate(JPGD_BAD_DQT_MARKER);

  left -= 2;

  while (left)
  {
    n = get_bits_1(8);
    prec = n >> 4;
    n &= 0x0F;
    if (n >= JPGD_MAXQUANTTABLES)
      terminate(JPGD_BAD_DQT_TABLE);

    if (!phdl->decoder.quant[n])
      phdl->decoder.quant[n] = (QUANT_TYPE *)imagedec_malloc(64 * sizeof(QUANT_TYPE));
	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

    // read phdl->decoder.quantization entries, in zag order
    for (i = 0; i < 64; i++)
    {
      temp = get_bits_1(8);

      if (prec)
        temp = (temp << 8) + get_bits_1(8);

        phdl->decoder.quant[n][i] = temp;
    }

    i = 64 + 1;

    if (prec)
      i += 64;

    if (left < (uint)i)
      terminate(JPGD_BAD_DQT_LENGTH);

    left -= i;
  }
  JPEG_EXIT;
}
//------------------------------------------------------------------------------
// Read the start of frame (SOF) marker.
IMAGEDEC_STATIC void read_sof_marker(void)
{
  int i;
  uint left;
 pImagedec_hdl phdl = &g_imagedec_hdl[0];

  JPEG_ENTRY;
  left = get_bits_1(16);

  if (get_bits_1(8) != 8)   /* precision: sorry, only 8-bit precision is supported right now */
    terminate(JPGD_BAD_PRECISION);

  phdl->decoder.image_y_size = get_bits_1(16);

  if ((phdl->decoder.image_y_size < 1) || (phdl->decoder.image_y_size > JPGD_MAX_HEIGHT))
    terminate(JPGD_BAD_HEIGHT);

  phdl->decoder.image_x_size = get_bits_1(16);

  if ((phdl->decoder.image_x_size < 1) || (phdl->decoder.image_x_size > JPGD_MAX_WIDTH))
    terminate(JPGD_BAD_WIDTH);

  phdl->decoder.comps_in_frame = get_bits_1(8);

  if (phdl->decoder.comps_in_frame > JPGD_MAXCOMPONENTS)
    terminate(JPGD_TOO_MANY_COMPONENTS);

  if (left != (uint)(phdl->decoder.comps_in_frame * 3 + 8))
    terminate(JPGD_BAD_SOF_LENGTH);

  for (i = 0; i < phdl->decoder.comps_in_frame; i++)
  {
    phdl->decoder.comp_ident[i]  = get_bits_1(8);
    phdl->decoder.comp_h_samp[i] = get_bits_1(4);
    phdl->decoder.comp_v_samp[i] = get_bits_1(4);
    phdl->decoder.comp_quant[i]  = get_bits_1(8);
  }
  jpeg_printf("jpeg w <%d>h<%d>\n",phdl->decoder.image_x_size,phdl->decoder.image_y_size);
  JPEG_EXIT;
}
//------------------------------------------------------------------------------
// Used to skip unrecognized markers.
IMAGEDEC_STATIC void skip_variable_marker(void)
{
  uint left;

  left = get_bits_1(16);

  if (left < 2)
    terminate(JPGD_BAD_VARIABLE_MARKER);

  left -= 2;

  while (left)
  {
	if (IsTerminated()) 	
	{
		return;
	} 
	get_bits_1(8);
	left--;
  }
}
//------------------------------------------------------------------------------
// Read a define restart interval (DRI) marker.
IMAGEDEC_STATIC void read_dri_marker(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
  if (get_bits_1(16) != 4)
    terminate(JPGD_BAD_DRI_LENGTH);

  phdl->decoder.restart_interval = get_bits_1(16);
  jpeg_printf("restart int <%d>\n",phdl->decoder.restart_interval);
}
//------------------------------------------------------------------------------
// Read a start of scan (SOS) marker.
IMAGEDEC_STATIC void read_sos_marker(void)
{
  uint left;
  int i, ci, n, c, cc;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

  left = get_bits_1(16);

  n = get_bits_1(8);

  phdl->decoder.comps_in_scan = n;

  left -= 3;

  if ( (left != (uint)(n * 2 + 3)) || (n < 1) || (n > JPGD_MAXCOMPSINSCAN) )
    terminate(JPGD_BAD_SOS_LENGTH);

  for (i = 0; i < n; i++)
  {
    cc = get_bits_1(8);
    c = get_bits_1(8);
    left -= 2;

    for (ci = 0; ci < phdl->decoder.comps_in_frame; ci++)
      if (cc == phdl->decoder.comp_ident[ci])
        break;

    if (ci >= phdl->decoder.comps_in_frame)
      terminate(JPGD_BAD_SOS_COMP_ID);

    phdl->decoder.comp_list[i]    = ci;
    phdl->decoder.comp_dc_tab[ci] = (c >> 4) & 15;
    phdl->decoder.comp_ac_tab[ci] = (c & 15) + (JPGD_MAXHUFFTABLES >> 1);
  }

  phdl->decoder.spectral_start  = get_bits_1(8);
  phdl->decoder.spectral_end    = get_bits_1(8);
  phdl->decoder.successive_high = get_bits_1(4);
  phdl->decoder.successive_low  = get_bits_1(4);

  if (!phdl->decoder.progressive_flag)
  {
    phdl->decoder.spectral_start = 0;
    phdl->decoder.spectral_end = 63;
  }

  left -= 3;

  while (left)                  /* read past whatever is left */
  {
    get_bits_1(8);
    left--;
  }
}

// Finds the next marker.
IMAGEDEC_STATIC int next_marker(void)
{
  uint c, bytes;

  bytes = 0;

  do
  {
    do
    {
      bytes++;

      c = get_bits_1(8);

    } while (c != 0xFF);

    do
    {
      c = get_bits_1(8);

    } while (c == 0xFF);

  } while (c == 0);

  // If bytes > 0 here, there where extra bytes before the marker (not good).

  return c;
}
//------------------------------------------------------------------------------
// Process markers. Returns when an SOFx, SOI, EOI, or SOS marker is
// encountered.
IMAGEDEC_STATIC int process_markers(pImagedec_hdl phdl)
{
  int c;
  JPEG_ENTRY;
  for ( ; ; )
  {
	if (IsTerminated()) return M_ERROR; // error check and mem control. peter luo. 6-10.

    c = next_marker();

    switch (c)
    {
      case M_SOF0:
      case M_SOF1:
      case M_SOF2:
      case M_SOF3:
      case M_SOF5:
      case M_SOF6:
      case M_SOF7:
//      case M_JPG:
      case M_SOF9:
      case M_SOF10:
      case M_SOF11:
      case M_SOF13:
      case M_SOF14:
      case M_SOF15:
      case M_SOI:
      case M_EOI:
      case M_SOS:
      {
	 JPEG_EXIT;
        return c;
      }
      case M_DHT:
      {
        read_dht_marker();
        break;
      }
      // Sorry, no arithmitic support at this time. Dumb patents!
      case M_DAC:
      {
        iterminate(JPGD_NO_ARITHMITIC_SUPPORT);
        break;
      }
      case M_DQT:
      {
        read_dqt_marker();
        break;
      }
      case M_DRI:
      {
        read_dri_marker();
        break;
      }
      //case M_APP0:  /* no need to read the JFIF marker */
      case M_JPG:
      case M_RST0:    /* no parameters */
      case M_RST1:
      case M_RST2:
      case M_RST3:
      case M_RST4:
      case M_RST5:
      case M_RST6:
      case M_RST7:
      case M_TEM:
      {
        iterminate(JPGD_UNEXPECTED_MARKER);
        break;
      }
      case M_APP1:
     {
		if(TRUE == phdl->app.item[c&0xF].active)
		{
			if(phdl->app.item[c&0xf].processor(phdl))
			{
				JPEG_EXIT;
				return c;
			}
		}
      }	
      default:    /* must be DNL, DHP, EXP, APPn, JPGn, COM, or RESn or APP0 */
      {
        skip_variable_marker();
        break;
      }

    }
  }
}

// Finds the start of image (SOI) marker.
// This code is rather defensive: it only checks the first 512 bytes to avoid
// false positives.
IMAGEDEC_STATIC void locate_soi_marker(void)
{
  uint lastchar, thischar;
  ulong bytesleft;
  pImagedec_hdl phdl = &g_imagedec_hdl[0];

  lastchar = get_bits_1(8);

  thischar = get_bits_1(8);

  /* ok if it's a normal JPEG file without a special header */

  if ((lastchar == 0xFF) && (thischar == M_SOI))
  {
	jpeg_printf("SOI shift 0\n");
	return;
  }

  bytesleft = 512;

  for ( ; ; )
  {
    if (--bytesleft == 0)
      terminate(JPGD_NOT_JPEG);

    lastchar = thischar;

    thischar = get_bits_1(8);

    if ((lastchar == 0xFF) && (thischar == M_SOI))
      break;
  }

  /* Check the next character after marker: if it's not 0xFF, it can't
     be the start of the next marker, so it probably isn't a JPEG */

  thischar = (phdl->decoder.bit.bit_buf >> 8) & 0xFF;

  if (thischar != 0xFF)
    terminate(JPGD_NOT_JPEG);
}

// Find a start of frame (SOF) marker.
IMAGEDEC_STATIC void locate_sof_marker(pImagedec_hdl phdl)
{
  int c;
  JPEG_ENTRY;

PARSING_JPEG_SOI:
  locate_soi_marker();
  jpeg_printf("marker SOI\n");
  c = process_markers(phdl);
  jpeg_printf("process maker <%x>\n",c);
  switch (c)
  {
    case M_SOF2:
      phdl->decoder.progressive_flag = TRUE;
      //jpeg_printf("progressive jpeg\n");
    case M_SOF0:  /* baseline DCT */
    case M_SOF1:  /* extended sequential DCT */
    {
      read_sof_marker();
      break;
    }
    case M_SOF9:  /* Arithmitic coding */
    {
      terminate(JPGD_NO_ARITHMITIC_SUPPORT);
      break;
    }
    case M_APP1:
    {
	/*decode the thumbnail in the APP1 section*/
	goto PARSING_JPEG_SOI;
    }
    default:
    {
	jpeg_printf("code <%x>\n",c);	
      terminate(JPGD_UNSUPPORTED_MARKER);
      break;
    }
  }
}
//------------------------------------------------------------------------------
// Find a start of scan (SOS) marker.
IMAGEDEC_STATIC int locate_sos_marker(pImagedec_hdl phdl)
{
  int c;

  c = process_markers(phdl);

  if (c == M_EOI)
    return FALSE;
  else if (c != M_SOS)
    iterminate(JPGD_UNEXPECTED_MARKER);

  read_sos_marker();

  return TRUE;
}
//------------------------------------------------------------------------------
// Creates the tables needed for efficient Huffman decoding.
IMAGEDEC_STATIC void make_huff_table(int index, Phuff_tables_t hs)
{
	int p, i, l, si;
	uchar huffsize[257];
	uint huffcode[257];
	uint code;
	uint subtree;
	int lastp;
	int nextfreeentry;
	int currententry;
	int code_size;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	p = 0;
	for (l = 1; l <= 16; l++)
	{
		for (i = 1; i <= phdl->decoder.huff_num[index][l]; i++) 
			huffsize[p++] = l;
		//manson: l and huffsize is codeword length. 
	}
	huffsize[p] = 0;
	lastp = p;
	code = 0;
	si = huffsize[0];
	p = 0;
	while (huffsize[p])
	{
		while (huffsize[p] == si)
		{
			huffcode[p++] = code;
			//manson: huffcode is the binary huffman codewords
			code++;
		}
		code <<= 1; //Manson: This guarenttee a shorter codeword can not be part of a longer codeword
		si++;
	}
#ifdef JPEG_NEW_HUF_TABLE
	UINT32 *tree = NULL;
	UINT32 sub_tree_size = 1<<JPEG_ROOT_BITS;
	UINT32 tree_len = JPEG_ROOT_BITS;
	UINT32 symbol = 0;
	UINT32 fill_start = 0,fill_end = 0;
	UINT32 root_code = 0;
	UINT16 root_entry = 0xFFFF;
	UINT8 *huf_value = NULL;
	UINT8 drop = 0;

	huf_value = (UINT8*)phdl->decoder.huff_val[index];
	tree = hs->huf_table = imagedec_malloc(sub_tree_size<<2);
	if(NULL == hs->huf_table)
	{
		terminate(JPGD_NOTENOUGHMEM);
	}
	for(p = 0;p < lastp;p++)
	{
		code_size = huffsize[p];
		code = huffcode[p];	
		if(code_size > JPEG_ROOT_BITS)
		{
			root_code = code>>(code_size - JPEG_ROOT_BITS);
			if(root_entry != root_code)
			{
				root_entry = root_code;
				drop = JPEG_ROOT_BITS;
				tree += sub_tree_size;

				for(i = p + 1;i < lastp;i++){
					if((huffcode[i]>>(huffsize[i] - drop))  != root_entry)
						break;
				}
				tree_len = huffsize[i - 1] - drop;
				sub_tree_size = 1<<tree_len;
				if(NULL == imagedec_malloc(sub_tree_size<<2))
				{
					terminate(JPGD_NOTENOUGHMEM);
				}
				symbol = 0;
				symbol = tree_len + JPEG_SUB_BASE;
				symbol |= 0xFF<<8;
				symbol |= (tree - hs->huf_table)<<16;
				hs->huf_table[root_entry] = symbol;
			}
		}

		code_size -= drop;
		symbol = 0;
		symbol |= code_size;
		symbol |= huf_value[p]<<8;

		code &= (1<<code_size) - 1;
		code_size = tree_len - code_size;
		fill_start = code<<code_size;
		fill_end = (1<<code_size) + fill_start;
		while(fill_start < fill_end)
		{
			tree[fill_start] = symbol;
			fill_start++;
		}
	}
#else
	MEMSET(hs->look_up, 0, sizeof(hs->look_up));
	MEMSET(hs->tree, 0, sizeof(hs->tree));
	MEMSET(hs->code_size, 0, sizeof(hs->code_size));
	nextfreeentry = -1;
	p = 0;
	while (p < lastp)
	{
		i = phdl->decoder.huff_val[index][p]; 
		//Manson: phdl->decoder.huff_val is the decoded value for each huffman codeword
		code = huffcode[p]; //manson: codeword
		code_size = huffsize[p]; //manson: codesize in bits
		hs->code_size[i] = code_size;
		if (code_size <= 8)
		{
			code <<= (8 - code_size); //manson: shift codeword to full 8 bits

			for (l = 1 << (8 - code_size); l > 0; l--)
			{
				hs->look_up[code] = i; //manson: all entries having the same high bits map to the same codeword
				code++;
			}
		}
		else
		{
			subtree = (code >> (code_size - 8)) & 0xFF; //manson: codeword size must be <= 16
			//manson: subtree = high 8 bits of the codeword, since a shorter codeword can not be
			//part of a longer codeword, the entry in the table must be empty. 
			currententry = hs->look_up[subtree];
			if (currententry == 0)
			//manson: 0 means this is the first codeword with that high 8 bits value
			{
				hs->look_up[subtree] = currententry = nextfreeentry;
				//assign a free entry for this group of codewords

				nextfreeentry -= 2;
			}
			code <<= (16 - (code_size - 8));
			for (l = code_size; l > 9; l--)
			{
				if ((code & 0x8000) == 0)
				currententry--;
				if (hs->tree[-currententry - 1] == 0)
				{
					hs->tree[-currententry - 1] = nextfreeentry;
					currententry = nextfreeentry;
					nextfreeentry -= 2;
				}
				else
					currententry = hs->tree[-currententry - 1];
				code <<= 1;
			}
			if ((code & 0x8000) == 0)
				currententry--;
			hs->tree[-currententry - 1] = i;
		}
		p++;
	}

	//manson: merge lookup and codesize for speed up
	for(i=0;i<256;i++)
	{
		if(hs->look_up[i] >= 0)
		{
			code_size = hs->code_size[hs->look_up[i]];
			hs->look_up_and_code_size[i] = (hs->look_up[i]&0xffff) + (code_size<<16);
		}
		else
			hs->look_up_and_code_size[i] = (hs->look_up[i]&0xffff);
	}
#endif
}
//------------------------------------------------------------------------------
// Verifies the phdl->decoder.quantization tables needed for this scan are available.
IMAGEDEC_STATIC void check_quant_tables(void)
{
	int i;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	for (i = 0; i < phdl->decoder.comps_in_scan; i++)
	{
		if (phdl->decoder.quant[phdl->decoder.comp_quant[phdl->decoder.comp_list[i]]] == NULL)
			terminate(JPGD_UNDEFINED_QUANT_TABLE);
	}
}
//------------------------------------------------------------------------------

IMAGEDEC_STATIC void build_hw_vld_table(pImagedec_hdl phdl, UINT8 tab_idx)
{
	UINT8 *num_buf = phdl->decoder.huff_num[tab_idx];
	UINT8 *val_buf = phdl->decoder.huff_val[tab_idx];
	int i = 0,j = 0,k = 0,nb = 0, code = 0, val = 0;

	phdl->decoder.hw_vld_max_code[tab_idx] = (UINT16 *)imagedec_malloc(16<<1);
	phdl->decoder.hw_vld_valptr[tab_idx] = (UINT8 *)imagedec_malloc(16);
	phdl->decoder.hw_vld_symbol[tab_idx] = (UINT8 *)imagedec_malloc(256);	
    jpeg_printf("build vld table %d max %x val %x sym %x\n",tab_idx
        ,phdl->decoder.hw_vld_max_code[tab_idx],phdl->decoder.hw_vld_valptr[tab_idx]
        ,phdl->decoder.hw_vld_symbol[tab_idx]);

	for(i = 1;i <= 16;i++)
	{
		nb = num_buf[i];
		val += nb;
		code = code + nb;
		if(0 == nb)
		{
			if(16 == i)
				phdl->decoder.hw_vld_max_code[tab_idx][i - 1] = 0;
			else
				phdl->decoder.hw_vld_max_code[tab_idx][i - 1] = -1;
			phdl->decoder.hw_vld_valptr[tab_idx][i - 1] = 0;
		}
		else
		{
			phdl->decoder.hw_vld_max_code[tab_idx][i - 1] = code - 1;
			phdl->decoder.hw_vld_valptr[tab_idx][i - 1] = val - 1;
		}
		code <<= 1;
		for(j = 0;j < nb;j++)
			phdl->decoder.hw_vld_symbol[tab_idx][k] = val_buf[k++];
	}
}

IMAGEDEC_STATIC void construct_hw_vld_table(pImagedec_hdl phdl)
{
	int tables_num = 0;
	int i = 0;

	// check the total huf table num
	for (i = 0; i < JPGD_MAXHUFFTABLES; i++)
	{
		if (phdl->decoder.huff_num[i])
			tables_num++;
	}
	if(tables_num > 4 || phdl->decoder.comps_in_frame > 3)
	{
		jpeg_printf("HW VLD don't support too many huffman table %d \n",tables_num);
		goto CRT_FAIL;
	}

	// build the hw table
	for (i = 0; i < JPGD_MAXHUFFTABLES; i++)
	{
		if (phdl->decoder.huff_num[i])
		{
			build_hw_vld_table(phdl,i);
			if(IsTerminated())
				return;
		}
	}	

	// set the hw vld table
	jh_m33_hw_set_hw_vld_table(phdl);
	return;
	
CRT_FAIL:
	g_imagedec_m33_enable_hw_vld = 0;
}

// Verifies that all the Huffman tables needed for this scan are available.
IMAGEDEC_STATIC void check_huff_tables(void)  
{
	int i;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	for (i = 0; i < phdl->decoder.comps_in_scan; i++)
	{
		if ((phdl->decoder.spectral_start == 0) && (phdl->decoder.huff_num[phdl->decoder.comp_dc_tab[phdl->decoder.comp_list[i]]] == NULL))
			terminate(JPGD_UNDEFINED_HUFF_TABLE);

		if ((phdl->decoder.spectral_end > 0) && (phdl->decoder.huff_num[phdl->decoder.comp_ac_tab[phdl->decoder.comp_list[i]]] == NULL))
			terminate(JPGD_UNDEFINED_HUFF_TABLE);
	}
	for (i = 0; i < JPGD_MAXHUFFTABLES; i++)
	{
		if (phdl->decoder.huff_num[i])
		{
			if (!phdl->decoder.h[i])
				phdl->decoder.h[i] = (Phuff_tables_t)imagedec_malloc(sizeof(huff_tables_t));

			if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

			make_huff_table(i, phdl->decoder.h[i]);
		}
	}
	
	for (i = 0; i < phdl->decoder.blocks_per_mcu; i++)
	{
		phdl->decoder.dc_huff_seg[i] = phdl->decoder.h[phdl->decoder.comp_dc_tab[phdl->decoder.mcu_org[i]]];
		phdl->decoder.ac_huff_seg[i] = phdl->decoder.h[phdl->decoder.comp_ac_tab[phdl->decoder.mcu_org[i]]];
		phdl->decoder.component[i]   = &phdl->decoder.last_dc_val[phdl->decoder.mcu_org[i]];
	}
}
//------------------------------------------------------------------------------
// Determines the phdl->decoder.component order inside each MCU.
// Also calcs how many MCU's are on each row, etc.
IMAGEDEC_STATIC void calc_mcu_block_order(void)
{
  int component_num, component_id;
  int max_h_samp = 0, max_v_samp = 0;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

  for (component_id = 0; component_id < phdl->decoder.comps_in_frame; component_id++)
  {
    if (phdl->decoder.comp_h_samp[component_id] > max_h_samp)
      max_h_samp = phdl->decoder.comp_h_samp[component_id];

    if (phdl->decoder.comp_v_samp[component_id] > max_v_samp)
      max_v_samp = phdl->decoder.comp_v_samp[component_id];
  }

  for (component_id = 0; component_id < phdl->decoder.comps_in_frame; component_id++)
  {
    phdl->decoder.comp_h_blocks[component_id] = ((((phdl->decoder.image_x_size * phdl->decoder.comp_h_samp[component_id]) + (max_h_samp - 1)) / max_h_samp) + 7) / 8;
    phdl->decoder.comp_v_blocks[component_id] = ((((phdl->decoder.image_y_size * phdl->decoder.comp_v_samp[component_id]) + (max_v_samp - 1)) / max_v_samp) + 7) / 8;
  }

  if (phdl->decoder.comps_in_scan == 1)
  {
    phdl->decoder.mcus_per_row = phdl->decoder.comp_h_blocks[phdl->decoder.comp_list[0]];
    phdl->decoder.mcus_per_col = phdl->decoder.comp_v_blocks[phdl->decoder.comp_list[0]];
  }
  else
  {
    phdl->decoder.mcus_per_row = (((phdl->decoder.image_x_size + 7) / 8) + (max_h_samp - 1)) / max_h_samp;
    phdl->decoder.mcus_per_col = (((phdl->decoder.image_y_size + 7) / 8) + (max_v_samp - 1)) / max_v_samp;
  }

//	jpeg_printf("@@@@@@! phdl->decoder.image_x_size=%d,phdl->decoder.image_y_size=%d, phdl->decoder.mcus_per_row=%d,phdl->decoder.mcus_per_col=%d\n",phdl->decoder.image_x_size, phdl->decoder.image_y_size, phdl->decoder.mcus_per_row,phdl->decoder.mcus_per_col);

  if (phdl->decoder.comps_in_scan == 1)
  {
    phdl->decoder.mcu_org[0] = phdl->decoder.comp_list[0];

    phdl->decoder.blocks_per_mcu = 1;
  }
  else
  {
    phdl->decoder.blocks_per_mcu = 0;

    for (component_num = 0; component_num < phdl->decoder.comps_in_scan; component_num++)
    {
      int num_blocks;

      component_id = phdl->decoder.comp_list[component_num];

      num_blocks = phdl->decoder.comp_h_samp[component_id] * phdl->decoder.comp_v_samp[component_id];

      while (num_blocks--) 
        phdl->decoder.mcu_org[phdl->decoder.blocks_per_mcu++] = component_id;
    }
  }
}

//---------------------------------progressive 1---------------------------------------
// decoding each comp of progressive JPG one by one, and then output, only for smaller picture. Peter luo. 2003-7-18.

// Decode a scan in a progressively encoded image.
IMAGEDEC_STATIC  void decode_scan(Pdecode_block_func decode_block_func)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
  int mcu_row, mcu_col, mcu_block;
  int block_x_mcu[JPGD_MAXCOMPONENTS], block_y_mcu[JPGD_MAXCOMPONENTS];

  MEMSET(block_y_mcu, 0, sizeof(block_y_mcu));

  for (mcu_col = 0; mcu_col < phdl->decoder.mcus_per_col; mcu_col++)
  {
    int component_num, component_id;

    MEMSET(block_x_mcu, 0, sizeof(block_x_mcu));

    for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
    {
      int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;

      if ((phdl->decoder.restart_interval) && (phdl->decoder.restarts_left == 0))
        process_restart();

      for (mcu_block = 0; mcu_block < phdl->decoder.blocks_per_mcu; mcu_block++)
      {
	/*check for the stop status*/
	if(g_imagedec_stop_flag[phdl->id])
		return; 
        component_id = phdl->decoder.mcu_org[mcu_block];
//jpeg_printf("path1: %d %d %d %d %d\n",component_id,block_x_mcu[component_id],block_y_mcu[component_id],block_x_mcu_ofs,block_y_mcu_ofs);
        decode_block_func(component_id,
                          block_x_mcu[component_id] + block_x_mcu_ofs,
                          block_y_mcu[component_id] +block_y_mcu_ofs);
		if (IsTerminated())
		{
			jpeg_printf("Decode terminate, %s()-%d\n", __FUNCTION__, __LINE__);
			return;
		}

        if (phdl->decoder.comps_in_scan == 1)
          block_x_mcu[component_id]++;
        else
        {
          if (++block_x_mcu_ofs == phdl->decoder.comp_h_samp[component_id])
          {
            block_x_mcu_ofs = 0;

            if (++block_y_mcu_ofs == phdl->decoder.comp_v_samp[component_id])
            {
              block_y_mcu_ofs = 0;

              block_x_mcu[component_id] += phdl->decoder.comp_h_samp[component_id];
            }
          }
        }
      }

      phdl->decoder.restarts_left--;
    }

    if (phdl->decoder.comps_in_scan == 1)
      block_y_mcu[phdl->decoder.comp_list[0]]++;
    else
    {
      for (component_num = 0; component_num < phdl->decoder.comps_in_scan; component_num++)
      {
        component_id = phdl->decoder.comp_list[component_num];

        block_y_mcu[component_id] += phdl->decoder.comp_v_samp[component_id];
      }
    }
  }
}
// Starts a new scan.
IMAGEDEC_STATIC int init_scan(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	if (!locate_sos_marker(phdl))
		return FALSE;
	calc_mcu_block_order();
	check_huff_tables();
	check_quant_tables();
	MEMSET(phdl->decoder.last_dc_val, 0, phdl->decoder.comps_in_frame * sizeof(uint));
	phdl->decoder.eob_run = 0;
	if (phdl->decoder.restart_interval)
	{
		phdl->decoder.restarts_left = phdl->decoder.restart_interval;
		phdl->decoder.next_restart_num = 0;
	}
	fix_in_buffer();
	return TRUE;
}

//---------------------------------progressive 2---------------------------------------
// decoding progressive JPG like sequential JPG, only for big picture. Peter luo. 2003-7-18.
// for those pictures with a size near the frame buffer, the disregard to the AC refine part couldnot be accepted(display quality),
// but the regard to it to the bigger ones could not be accepted(memory size limit), too.
// so, decode the pictures which are < frame buffer by method 1, and bigger ones by method 2.

// init some vars.
IMAGEDEC_STATIC void Pro2_init (void)
{
#if 0	
	int i, iMaxBytePerMcuRow=0;
	iPro2_NextReadRow=0;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	for (i=0;i<phdl->decoder.comps_in_frame;i++) 
		iMaxBytePerMcuRow += phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i]*1* phdl->decoder.comp_v_samp[i]*sizeof(BLOCK_TYPE)*(1+64);

	//	iPro2_McuBuf_MaxRow = 3; // for test only.
// john	iPro2_McuBuf_MaxRow = (int)(PRO2_MCUBUF_SIZE/iMaxBytePerMcuRow);

	if (iPro2_McuBuf_MaxRow<2) terminate(JPGD_NOTENOUGHMEM); // notice: must >= 2!!!! Peter luo 2003-7-19.
	if (iPro2_McuBuf_MaxRow > phdl->decoder.max_mcus_per_col) iPro2_McuBuf_MaxRow=phdl->decoder.mcus_per_col;

	jpeg_printf("@@@@ iPro2_McuBuf_MaxRow=%d\n",iPro2_McuBuf_MaxRow);

	iPro2_McuBufLineOutput=0;
#endif	
}

// store the position of the file pointer, etc.
IMAGEDEC_STATIC void Pro2_store_buf_info (void)
{
#if 0	
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	iPro2_InBufLeft = phdl->in_stream.in_buf_left;
	iPro2_ByteRead = phdl->in_stream.total_bytes_read;
	pPro2_BufOfs =  phdl->in_stream.Pin_buf_ofs;
//john	iPro2_FilePos = jpg_buffer_pos-JPGD_INBUFSIZE;
	iPro2_BitLest = phdl->decoder.bits_left;
	uiPro2_BitBuf0 = phdl->decoder.bit.bit_buf_64[0];
	uiPro2_BitBuf1 = phdl->decoder.bit.bit_buf_64[1];
	iPro2_RestartInterval = phdl->decoder.restart_interval;
#endif	
}

// load the position of the file pointer.
IMAGEDEC_STATIC void Pro2_load_buf_info (void)
{
#if 0
	int i;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	if (0 < jpg_buffer_pos-JPGD_INBUFSIZE) { // if the file < JPGD_INBUFSIZE, need not read again.
		phdl->in_stream.fseek(phdl->fh,iPro2_FilePos, SEEK_SET);
		prep_in_buffer(); // the data in the buffer read before is invalid and should be re-read now. 7-15.
	}

	phdl->in_stream.in_buf_left = iPro2_InBufLeft;
	phdl->in_stream.total_bytes_read = iPro2_ByteRead;
	phdl->in_stream.Pin_buf_ofs = pPro2_BufOfs;
	jpg_buffer_pos = iPro2_FilePos + JPGD_INBUFSIZE;
	phdl->decoder.bits_left = iPro2_BitLest;
	phdl->decoder.bit.bit_buf_64[0] = uiPro2_BitBuf0;
	phdl->decoder.bit.bit_buf_64[1] = uiPro2_BitBuf1; 
	phdl->decoder.restart_interval = iPro2_RestartInterval;

	for (i = 0; i < phdl->decoder.comps_in_frame; i++)
	{
		MEMSET (phdl->decoder.dc_coeffs[i]->Pdata,0, iPro2_DcSize[i]);
		MEMSET (phdl->decoder.ac_coeffs[i]->Pdata,0, iPro2_AcSize[i]);
	}
#endif
}

// Decode a scan in a progressively encoded image.
IMAGEDEC_STATIC void Pro2_decode_scan(
  Pdecode_block_func decode_block_func)
{
#if 0	
	int mcu_row, mcu_col, mcu_block;
	int block_x_mcu[JPGD_MAXCOMPONENTS], block_y_mcu[JPGD_MAXCOMPONENTS];
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
#if 1 //john
	MEMSET(phdl->decoder.block_y_mcu, 0, sizeof(phdl->decoder.block_y_mcu));

	for (mcu_col = 0; mcu_col < phdl->decoder.mcus_per_col; mcu_col++)  // the MCU row has been finished. 2003-7-15.
	{
		int component_num, component_id;

		//	if(mcu_col == iPro2_McuBuf_MaxRow || (mcu_col == iPro2_NextReadRow-1 && mcu_col != iPro2_McuBuf_MaxRow-1)) return; // only to get the very several cols we need.
		if(mcu_col == iPro2_NextReadRow+1) return; // only to get the very several cols we need.

		MEMSET(block_x_mcu, 0, sizeof(block_x_mcu));

		for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
		{
			int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;

			if ((phdl->decoder.restart_interval) && (phdl->decoder.restarts_left == 0))
				process_restart();

			for (mcu_block = 0; mcu_block < phdl->decoder.blocks_per_mcu; mcu_block++)
			{
				component_id = phdl->decoder.mcu_org[mcu_block];

				if( (0==mcu_col || mcu_col < iPro2_NextReadRow+2-iPro2_McuBuf_MaxRow) && 0==block_y_mcu_ofs) phdl->decoder.block_y_mcu[component_id]=0; // we should begin from the new valid read MCU line.

				decode_block_func(component_id,
				block_x_mcu[component_id] + block_x_mcu_ofs,
				phdl->decoder.block_y_mcu[component_id] + block_y_mcu_ofs);

				if (IsTerminated())
				{
					jpeg_printf("Decode terminate, %s()-%d\n", __FUNCTION__, __LINE__);
					return;
				}


				if (phdl->decoder.comps_in_scan == 1)
					block_x_mcu[component_id]++;
				else
				{
					if (++block_x_mcu_ofs == phdl->decoder.comp_h_samp[component_id])
					{
						block_x_mcu_ofs = 0;

						if (++block_y_mcu_ofs == phdl->decoder.comp_v_samp[component_id])
						{
							block_y_mcu_ofs = 0;
							block_x_mcu[component_id] += phdl->decoder.comp_h_samp[component_id];
						}
					}
				}
			}

			phdl->decoder.restarts_left--;     
		}

		if (phdl->decoder.comps_in_scan == 1) 
			phdl->decoder.block_y_mcu[phdl->decoder.comp_list[0]]++;
		else
		{
			for (component_num = 0; component_num < phdl->decoder.comps_in_scan; component_num++)
			{
				component_id = phdl->decoder.comp_list[component_num];
				phdl->decoder.block_y_mcu[component_id] += phdl->decoder.comp_v_samp[component_id];
			}
		}
	}
#endif
#endif
}

//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Decode several rows of MCU of progressively encoded image. // Peter luo 7-14.
// Progressive images only.
IMAGEDEC_STATIC int Pro2_dec_next_MCU_rows(void)
{
#if 0	
	int i, dc_only_scan, refinement_scan;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	Pdecode_block_func decode_block_func;
#if 1//john
	// load the position of the file pointer.
	Pro2_load_buf_info ();

	iPro2_NextReadRow+=iPro2_McuBuf_MaxRow-1;

	// each scan to the whole picture. Peter luo.
	for ( ; ; )
	{
		if (!init_scan())
		break;

		dc_only_scan    = (phdl->decoder.spectral_start == 0);
		refinement_scan = (phdl->decoder.successive_high != 0);

		if ((phdl->decoder.spectral_start > phdl->decoder.spectral_end) || (phdl->decoder.spectral_end > 63))
			iterminate(JPGD_BAD_SOS_SPECTRAL);

		if (dc_only_scan)
		{
		if (phdl->decoder.spectral_end)
			iterminate(JPGD_BAD_SOS_SPECTRAL);
		}
		else if (phdl->decoder.comps_in_scan != 1)  /* AC scans can only contain one phdl->decoder.component */
			iterminate(JPGD_BAD_SOS_SPECTRAL);

		if ((refinement_scan) && (phdl->decoder.successive_low != phdl->decoder.successive_high - 1))
			iterminate(JPGD_BAD_SOS_SUCCESSIVE);

		if (dc_only_scan)
		{
			if (refinement_scan)
				decode_block_func = decode_block_dc_refine;
			else
				decode_block_func = decode_block_dc_first;
		}
		else
		{
			if (refinement_scan)
				decode_block_func = decode_block_ac_refine;
			else
				decode_block_func = decode_block_ac_first;
		}

		Pro2_decode_scan(decode_block_func);

		if (IsTerminated()) return FALSE; // error check and mem control. peter luo. 6-10.

		phdl->decoder.bits_left = 16;
		get_bits_1(16);
		get_bits_1(16);
	}

	phdl->decoder.comps_in_scan = phdl->decoder.comps_in_frame;
	for (i = 0; i < phdl->decoder.comps_in_frame; i++)
	phdl->decoder.comp_list[i] = i;
	calc_mcu_block_order();
#endif
#endif
	return TRUE;
}


// Decode a progressively encoded image.
IMAGEDEC_STATIC void Pro2_init_progressive(void)
{
#if 0	
	int i;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	Pro2_init();
	if (phdl->decoder.comps_in_frame == 4)
	terminate(JPGD_UNSUPPORTED_COLORSPACE);

	// Allocate the coefficient buffers.
	for (i = 0; i < phdl->decoder.comps_in_frame; i++)
	{
		phdl->decoder.dc_coeffs[i] = coeff_buf_open(phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i],
		                          iPro2_McuBuf_MaxRow * phdl->decoder.comp_v_samp[i], 1, 1);
		phdl->decoder.ac_coeffs[i] = coeff_buf_open(phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i],
		                          iPro2_McuBuf_MaxRow * phdl->decoder.comp_v_samp[i], 8, 8);
		if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

		iPro2_DcSize[i] = phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i]*iPro2_McuBuf_MaxRow * phdl->decoder.comp_v_samp[i]*sizeof(BLOCK_TYPE);
		iPro2_AcSize[i] = phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i]*iPro2_McuBuf_MaxRow * 64*phdl->decoder.comp_v_samp[i]*sizeof(BLOCK_TYPE);
	}
	// store the position of the file pointer.
	Pro2_store_buf_info ();
#endif	
}

//------------------------------------------------------------------------------
// Loads and dephdl->decoder.quantizes the next row of (already decoded) coefficients.
// Progressive images only.
IMAGEDEC_STATIC void Pro2_load_next_row(void)
{
#if 0	
	int i;
	BLOCK_TYPE *p;
	QUANT_TYPE *q;
	BLOCK_TYPE *pAC;
	BLOCK_TYPE *pDC;
	int component_num, component_id;
	int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;
	int block_x_mcu[JPGD_MAXCOMPONENTS];
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	int mcu_row, mcu_block, row_block = 0;
	//int mcu_row, mcu_block, iCount=0, row_block=get_iblock_ordinal(); // peter luo 6-6
#if 1  //john
	MEMSET(block_x_mcu, 0, JPGD_MAXCOMPONENTS * sizeof(int));

	if ( 0==iPro2_McuBufLineOutput )
	{
		if ( 0==iPro2_NextReadRow)
			MEMSET(iPro2_CurMcuLineOutput, 0, JPGD_MAXCOMPONENTS * sizeof(int));
		else {
			for (i=0; i<phdl->decoder.comps_in_frame;i++) iPro2_CurMcuLineOutput[i]=phdl->decoder.comp_v_samp[i];
			iPro2_McuBufLineOutput++;
		}
		if(!Pro2_dec_next_MCU_rows() ) return;
	}


	for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
	{
		block_x_mcu_ofs = 0;
		block_y_mcu_ofs = 0;

		for (mcu_block = 0; mcu_block < phdl->decoder.blocks_per_mcu; mcu_block++)
		{
			component_id = phdl->decoder.mcu_org[mcu_block];

			p = phdl->decoder.block_seg[row_block];
			q = phdl->decoder.quant[phdl->decoder.comp_quant[component_id]];

			pAC = coeff_buf_getp(phdl->decoder.ac_coeffs[component_id],
			  block_x_mcu[component_id] + block_x_mcu_ofs,
			  iPro2_CurMcuLineOutput[component_id]+block_y_mcu_ofs);
			//                                  phdl->decoder.block_y_mcu[component_id] + block_y_mcu_ofs);
			pDC = coeff_buf_getp(phdl->decoder.dc_coeffs[component_id],
			  block_x_mcu[component_id] + block_x_mcu_ofs,
			  iPro2_CurMcuLineOutput[component_id]+block_y_mcu_ofs);
			//                                  phdl->decoder.block_y_mcu[component_id] + block_y_mcu_ofs);

			if (IsTerminated()) return; // error check and mem control. peter luo. 7-19.

			p[0] = pDC[0];
			mem_memcpy(&p[1], &pAC[1], 63 * sizeof(BLOCK_TYPE));

			for (i = 63; i > 0; i--)
				if (p[ZAG[i]])
					break;

			//block_num[row_block++] = i + 1;

			for ( ; i >= 0; i--)
				if (p[ZAG[i]])
					p[ZAG[i]] *= q[i];

			row_block++;

			if (phdl->decoder.comps_in_scan == 1)
				block_x_mcu[component_id]++;
			else
			{
				if (++block_x_mcu_ofs == phdl->decoder.comp_h_samp[component_id])
				{
					block_x_mcu_ofs = 0;

					if (++block_y_mcu_ofs == phdl->decoder.comp_v_samp[component_id])
					{
						block_y_mcu_ofs = 0;
						block_x_mcu[component_id] += phdl->decoder.comp_h_samp[component_id];
					}
				}
			}
		}
	}

	if (phdl->decoder.comps_in_scan == 1) {
		phdl->decoder.block_y_mcu[phdl->decoder.comp_list[0]]++;
		iPro2_CurMcuLineOutput[phdl->decoder.comp_list[0]]++; // loop use to the MCU buff.     
	}
	else
	{
		for (component_num = 0; component_num < phdl->decoder.comps_in_scan; component_num++)
		{
			component_id = phdl->decoder.comp_list[component_num];

			phdl->decoder.block_y_mcu[component_id] += phdl->decoder.comp_v_samp[component_id];
			iPro2_CurMcuLineOutput[component_id]+= phdl->decoder.comp_v_samp[component_id];
		}
	}

	iPro2_McuBufLineOutput++;
	if (iPro2_McuBufLineOutput==iPro2_McuBuf_MaxRow) iPro2_McuBufLineOutput=0; // loop use to the MCU buff.  

#endif
#endif
}
//-- End. -------------------------------------------------------------------------


#if 0
int dump_len  = 0;
void dump_data(int mcu_x, int block_type, int *idct_data)
{
	uchar* y_addr = (uchar*)0xa0400000;
	uchar* y_dst;
	int i;

	int * src  = idct_data;

	if(block_type<=3)
	{
		y_dst = y_addr + dump_len;
	//	jpeg_printf("y_dst:%x\n",y_dst);
		for(i=0;i<8;i++)
		{
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
		}
		dump_len += 64;
	}

}


void copy_data_to_frm(int mcu_x, int block_type, int *idct_data)
{
	int i,j,k;
	uchar* y_addr = (uchar*)__MM_FB_0_Y_ADDR;
	uchar* c_addr = (uchar*)__MM_FB_0_C_ADDR;
	uchar* y_dst,*c_dst;

	int offset;

	int * src  = idct_data;
	 
	int mcu_y = cur_decoding_mcu_row;

	if(mcu_x&0x1)
		offset = 16;
	else 
		offset = 0;


	if(block_type==0)
	{
		y_dst = y_addr+mcu_y*23*32*16+(mcu_x/2)*32*16 + offset;
		for(i=0;i<8;i++)
		{
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);

			y_dst+=24;
		}
	}

	else if(block_type==1)
	{
		y_dst = y_addr+mcu_y*23*32*16+(mcu_x/2)*32*16 + 8 + offset;
		for(i=0;i<8;i++)
		{
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			y_dst+=24;
		}
	}

	else if(block_type==2)
	{
		y_dst = y_addr+mcu_y*23*32*16+(mcu_x/2)*32*16+32*8  + offset;
		for(i=0;i<8;i++)
		{
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			y_dst+=24;
		}
	}

	else if(block_type==3)
	{
		y_dst = y_addr+mcu_y*23*32*16+(mcu_x/2)*32*16+32*8 + 8 + offset;
		for(i=0;i<8;i++)
		{
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			*y_dst++ = (uchar)(*src++);
			y_dst+=24;
		}
	}

	else if(block_type==4)
	{
		
		if((mcu_y&0x1)==0)
			c_dst = c_addr+(mcu_y/2)*46*16*16+(mcu_x/2)*32*16 + offset;
		else
			c_dst = c_addr+(mcu_y/2)*46*16*16+(mcu_x/2)*32*16 + 32*8 + offset;
			

//	libc_printf("mcu_x :%d mcu_y: %d c_dst:0x%x\n",mcu_x,mcu_y,c_dst );
		for(i=0;i<8;i++)
		{
			c_dst[0]= (uchar)(*src++);
			c_dst[2]= (uchar)(*src++);
			c_dst[4]= (uchar)(*src++);
			c_dst[6]= (uchar)(*src++);
			c_dst[8]= (uchar)(*src++);
			c_dst[10]= (uchar)(*src++);
			c_dst[12]= (uchar)(*src++);
			c_dst[14]= (uchar)(*src++);
			c_dst += 32;
		}
	}

	else if(block_type==5)
	{
		if((mcu_y&0x1)==0)
			c_dst = c_addr+(mcu_y/2)*46*16*16+(mcu_x/2)*32*16 + offset;
		else
			c_dst = c_addr+(mcu_y/2)*46*16*16+(mcu_x/2)*32*16 + 32*8 + offset;

		for(i=0;i<8;i++)
		{
			c_dst[1]= (uchar)(*src++);
			c_dst[3]= (uchar)(*src++);
			c_dst[5]= (uchar)(*src++);
			c_dst[7]= (uchar)(*src++);
			c_dst[9]= (uchar)(*src++);
			c_dst[11]= (uchar)(*src++);
			c_dst[13]= (uchar)(*src++);
			c_dst[15]= (uchar)(*src++);
			c_dst += 32;
		}
	}
}
#endif
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Reset everything to default/uninitialized state.
IMAGEDEC_STATIC void jpeg_init(void)
{
	int i;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
/*
	for(i=0;i<JPGD_MAXHUFFTABLES;i++)
	{
		phdl->decoder.huff_num[i] = phdl->decoder.huff_val[i] = 0;
	}
	error_code = 0;

	phdl->decoder.ready_flag = false;

	phdl->decoder.image_x_size = phdl->decoder.image_y_size = 0;

	phdl->decoder.progressive_flag = FALSE;

	MEMSET(phdl->decoder.huff_num, 0, sizeof(phdl->decoder.huff_num));
	MEMSET(phdl->decoder.huff_val, 0, sizeof(phdl->decoder.huff_val));
	MEMSET(phdl->decoder.quant, 0, sizeof(phdl->decoder.quant));

	phdl->decoder.scan_type = 0;

	phdl->decoder.comps_in_frame = 0;

	MEMSET(phdl->decoder.comp_h_samp, 0, sizeof(phdl->decoder.comp_h_samp));
	MEMSET(phdl->decoder.comp_v_samp, 0, sizeof(phdl->decoder.comp_v_samp));
	MEMSET(phdl->decoder.comp_quant, 0, sizeof(phdl->decoder.comp_quant));
	MEMSET(phdl->decoder.comp_ident, 0, sizeof(phdl->decoder.comp_ident));
	MEMSET(phdl->decoder.comp_h_blocks, 0, sizeof(phdl->decoder.comp_h_blocks));
	MEMSET(phdl->decoder.comp_v_blocks, 0, sizeof(phdl->decoder.comp_v_blocks));

	phdl->decoder.comps_in_scan = 0;
	MEMSET(phdl->decoder.comp_list, 0, sizeof(phdl->decoder.comp_list));
	MEMSET(phdl->decoder.comp_dc_tab, 0, sizeof(phdl->decoder.comp_dc_tab));
	MEMSET(phdl->decoder.comp_ac_tab, 0, sizeof(phdl->decoder.comp_ac_tab));

	phdl->decoder.spectral_start = 0;
	phdl->decoder.spectral_end = 0;
	phdl->decoder.successive_low = 0;
	phdl->decoder.successive_high = 0;

	phdl->decoder.max_mcu_x_size = 0;
	phdl->decoder.max_mcu_y_size = 0;

	phdl->decoder.blocks_per_mcu = 0;
	phdl->decoder.max_blocks_per_row = 0;
	phdl->decoder.mcus_per_row = 0;
	phdl->decoder.mcus_per_col = 0;

	MEMSET(phdl->decoder.mcu_org, 0, sizeof(phdl->decoder.mcu_org));

	phdl->decoder.total_lines_left = 0;
	phdl->decoder.mcu_lines_left = 0;

	phdl->decoder.dest_bytes_per_scan_line = 0;
	phdl->decoder.dest_bytes_per_pixel = 0;
	
	MEMSET(phdl->decoder.blocks, 0, sizeof(phdl->decoder.blocks));

	MEMSET(phdl->decoder.h, 0, sizeof(phdl->decoder.h));

	MEMSET(phdl->decoder.dc_coeffs, 0, sizeof(phdl->decoder.dc_coeffs));
	MEMSET(phdl->decoder.ac_coeffs, 0, sizeof(phdl->decoder.ac_coeffs));
	MEMSET(phdl->decoder.block_y_mcu, 0, sizeof(phdl->decoder.block_y_mcu));

	phdl->decoder.eob_run = 0;

	MEMSET(phdl->decoder.block_y_mcu, 0, sizeof(phdl->decoder.block_y_mcu));

	phdl->in_stream.Pin_buf_ofs = phdl->in_stream.in_buf;
	phdl->in_stream.in_buf_left = 0;
	eof_flag = false;
	tem_flag = 0;


//	MEMSET(phdl->in_stream.in_buf, 0, (JPGD_INBUFSIZE  );

	phdl->decoder.restart_interval = 0;
	phdl->decoder.restarts_left    = 0;
	phdl->decoder.next_restart_num = 0;

	phdl->decoder.max_mcus_per_row = 0;
	phdl->decoder.max_blocks_per_mcu = 0;
	phdl->decoder.max_mcus_per_col = 0;

	MEMSET(phdl->decoder.component, 0, sizeof(phdl->decoder.component));
	MEMSET(phdl->decoder.last_dc_val, 0, sizeof(phdl->decoder.last_dc_val));
	MEMSET(phdl->decoder.dc_huff_seg, 0, sizeof(phdl->decoder.dc_huff_seg));
	MEMSET(phdl->decoder.ac_huff_seg, 0, sizeof(phdl->decoder.ac_huff_seg));
	MEMSET(phdl->decoder.block_seg, 0, sizeof(phdl->decoder.block_seg));

	phdl->in_stream.total_bytes_read = 0;

  // Ready the input buffer.
	prep_in_buffer();

  // Prime the bit buffer.
	phdl->decoder.bits_left = 16;
	phdl->decoder.bit.bit_buf_64[0] = 0;
	phdl->decoder.bit.bit_buf_64[1] = 0;

	get_bits_1(16);
	get_bits_1(16);
	*/
	image_data_end = 0;
	JPEG_ENTRY;
	MEMSET(&phdl->decoder,0,sizeof(phdl->decoder));
	MEMSET(&phdl->mem.blocks,0,sizeof(phdl->mem.blocks));
	phdl->mem.mem_pos = phdl->mem.mem_start;
	phdl->in_stream.Pin_buf_ofs = phdl->in_stream.in_buf;
	phdl->in_stream.in_buf_left = 0;
	phdl->in_stream.buf_idx_using = 0;
	phdl->in_stream.total_bytes_read = 0;
#ifndef IMAGEDEC_SINGLE_INPUT_BUF	
	phdl->in_stream.buf_idx_using = 0;
	phdl->in_stream.buf_item[0].busy = 0;
	phdl->in_stream.buf_item[1].busy = 0;
	phdl->in_stream.buf_item[0].end = 0;
	phdl->in_stream.buf_item[1].end = 0;	
	phdl->in_stream.buf_item[0].error = 0;
	phdl->in_stream.buf_item[1].error = 0;	
	imagedec_fill_in_buffer(phdl,1 - phdl->in_stream.buf_idx_using);
#endif
	prep_in_buffer();
	phdl->decoder.bits_left = 16;
	get_bits_1(16);
	get_bits_1(16);
	phdl->decoder.block_seg = (BLOCK_TYPE **)imagedec_malloc(JPGD_MAXBLOCKSPERROW*sizeof(int));
	JPEG_EXIT;
}

//------------------------------------------------------------------------------
// Starts a frame. Determines if the number of components or sampling factors
// are supported.
IMAGEDEC_STATIC void init_frame(void)
{
	int i;
	uchar *q;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	if (phdl->decoder.comps_in_frame == 1)
	{
		phdl->decoder.scan_type = JPGD_GRAYSCALE;
		phdl->decoder.max_blocks_per_mcu = 1;
		phdl->decoder.max_mcu_x_size     = 8;
		phdl->decoder.max_mcu_y_size     = 8;
	}
	else if (phdl->decoder.comps_in_frame == 3)
	{
		if ( ((phdl->decoder.comp_h_samp[1] != 1) || (phdl->decoder.comp_v_samp[1] != 1)) ||
		((phdl->decoder.comp_h_samp[2] != 1) || (phdl->decoder.comp_v_samp[2] != 1)) )
			terminate(JPGD_UNSUPPORTED_SAMP_FACTORS);

		if ((phdl->decoder.comp_h_samp[0] == 1) && (phdl->decoder.comp_v_samp[0] == 1))
		{
			phdl->decoder.scan_type          = JPGD_YH1V1;
			phdl->decoder.max_blocks_per_mcu = 3;
			phdl->decoder.max_mcu_x_size     = 8;
			phdl->decoder.max_mcu_y_size     = 8;
		}
		else if ((phdl->decoder.comp_h_samp[0] == 2) && (phdl->decoder.comp_v_samp[0] == 1))
		{
			phdl->decoder.scan_type          = JPGD_YH2V1;
			phdl->decoder.max_blocks_per_mcu = 4;
			phdl->decoder.max_mcu_x_size     = 16;
			phdl->decoder.max_mcu_y_size     = 8;
		}
		else if ((phdl->decoder.comp_h_samp[0] == 1) && (phdl->decoder.comp_v_samp[0] == 2))
		{
			phdl->decoder.scan_type          = JPGD_YH1V2;
			phdl->decoder.max_blocks_per_mcu = 4;
			phdl->decoder.max_mcu_x_size     = 8;
			phdl->decoder.max_mcu_y_size     = 16;
		}
		else if ((phdl->decoder.comp_h_samp[0] == 2) && (phdl->decoder.comp_v_samp[0] == 2))
		{
			phdl->decoder.scan_type          = JPGD_YH2V2;
			phdl->decoder.max_blocks_per_mcu = 6;
			phdl->decoder.max_mcu_x_size     = 16;
			phdl->decoder.max_mcu_y_size     = 16;
		}
		else if ((phdl->decoder.comp_h_samp[0] == 4) && (phdl->decoder.comp_v_samp[0] == 1)) //4:1:1, added by Manson
		{
			phdl->decoder.scan_type          = JPGD_YH4V1;
			phdl->decoder.max_blocks_per_mcu = 6;
			phdl->decoder.max_mcu_x_size     = 32;
			phdl->decoder.max_mcu_y_size     = 8;
		}
		else 
			terminate(JPGD_UNSUPPORTED_SAMP_FACTORS);
	}
	else 
		terminate(JPGD_UNSUPPORTED_COLORSPACE);
#if 0
	if((phdl->decoder.scan_type!=JPGD_YH4V1)&&(phdl->decoder.scan_type!=JPGD_YH2V2)&&(phdl->decoder.scan_type!=JPGD_YH2V1))
	{
		//  libc_printf("Not support this phdl->decoder.scan_type:%d \n",phdl->decoder.scan_type);
		//  terminate(JPGD_UNSUPPORTED_SAMP_FACTORS);
	}
#endif
	phdl->decoder.max_mcus_per_row = (phdl->decoder.image_x_size + (phdl->decoder.max_mcu_x_size - 1)) / phdl->decoder.max_mcu_x_size;
	phdl->decoder.max_mcus_per_col = (phdl->decoder.image_y_size + (phdl->decoder.max_mcu_y_size - 1)) / phdl->decoder.max_mcu_y_size;
	phdl->decoder.mcus_of_col = phdl->decoder.max_mcus_per_col;

	/* these values are for the *destination* pixels: after conversion */

	jpeg_printf("-=-=-=-=-JPEG scan type: %d\n====", phdl->decoder.scan_type);
	//  if (phdl->decoder.scan_type == JPGD_GRAYSCALE)
	//    phdl->decoder.dest_bytes_per_pixel = 1;
	//  else
	phdl->decoder.dest_bytes_per_pixel = 4;

	phdl->decoder.dest_bytes_per_scan_line = ((phdl->decoder.image_x_size + 15) & 0xFFF0) * phdl->decoder.dest_bytes_per_pixel;

	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

	phdl->decoder.max_blocks_per_row = phdl->decoder.max_mcus_per_row * phdl->decoder.max_blocks_per_mcu;

	// Should never happen
	if (phdl->decoder.max_blocks_per_row > JPGD_MAXBLOCKSPERROW)
		terminate(JPGD_ASSERTION_ERROR);

	// Allocate the coefficient buffer, enough for one row's worth of MCU's
	//  libc_printf("max_block_per_row:%d\n",phdl->decoder.max_blocks_per_row);
	//  q = (uchar *)imagedec_malloc(phdl->decoder.max_blocks_per_row * sizeof(BLOCK_TYPE)*64 + 8);
	q = (uchar *)imagedec_malloc(((phdl->decoder.max_blocks_per_row * sizeof(BLOCK_TYPE))<<6) + 8);
	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.
	// Align to 8-byte boundry, for MMX code
	q = (uchar *)(((uint)q + 7) & ~7);
	// The phdl->decoder.block_seg[] array's name dates back to the
	// 16-bit assembler implementation. "seg" stood for "segment".
	for (i = 0; i < phdl->decoder.max_blocks_per_row; i++)
		// phdl->decoder.block_seg[i] = (BLOCK_TYPE *)(q + i * 64 * sizeof(BLOCK_TYPE));
		phdl->decoder.block_seg[i] = (BLOCK_TYPE *)(q + ((i * sizeof(BLOCK_TYPE))<<6));

	/* john for (i = 0; i < phdl->decoder.max_blocks_per_row; i++)
	block_max_zag_set[i] = 64;*/

	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

	phdl->decoder.total_lines_left = phdl->decoder.image_y_size;
	phdl->decoder.mcu_lines_left = 0;
	//  create_look_ups(); // peter
}
//------------------------------------------------------------------------------
IMAGEDEC_STATIC void init_sequential(void)
{
	if (!init_scan())
		terminate(JPGD_UNEXPECTED_MARKER);
}

// Decode a progressively encoded image.
IMAGEDEC_STATIC void init_progressive(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	int i;
	int dc_only_scan, refinement_scan;
	Pdecode_block_func decode_block_func;

  if (phdl->decoder.comps_in_frame == 4)
    terminate(JPGD_UNSUPPORTED_COLORSPACE);

  // Allocate the coefficient buffers.
  for (i = 0; i < phdl->decoder.comps_in_frame; i++)
  {
/*
    phdl->decoder.dc_coeffs[i] = coeff_buf_open(phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i],
                                  max_phdl->decoder.mcus_per_col * phdl->decoder.comp_v_samp[i], 1, 1);
    phdl->decoder.ac_coeffs[i] = coeff_buf_open(phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i],
                                  max_phdl->decoder.mcus_per_col * phdl->decoder.comp_v_samp[i], 8, 8);
//peter test                      max_phdl->decoder.mcus_per_col * phdl->decoder.comp_v_samp[i], 8, 8);

*/
	  if (1>i)
	{
	//	  libc_printf("phdl->decoder.max_mcus_per_row:%d \n",phdl->decoder.max_mcus_per_row);
	//	  libc_printf("phdl->decoder.comp_h_samp[i]:%d %d\n",phdl->decoder.comp_h_samp[i],phdl->decoder.comp_v_samp[i]);
		phdl->decoder.dc_coeffs[i] = coeff_buf_open(phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i],
									  phdl->decoder.max_mcus_per_col * phdl->decoder.comp_v_samp[i], 1, 1);
		phdl->decoder.ac_coeffs[i] = coeff_buf_open(phdl->decoder.max_mcus_per_row * phdl->decoder.comp_h_samp[i],
									  phdl->decoder.max_mcus_per_col * phdl->decoder.comp_v_samp[i], 8, 8);
	}
	else
	{
		phdl->decoder.dc_coeffs[i] = phdl->decoder.dc_coeffs[i-1];	
		phdl->decoder.ac_coeffs[i] = phdl->decoder.ac_coeffs[i-1];
	}

	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

  }
	
  for ( ; ; )
  {

    if (!init_scan())
      break;
	
	if (IsTerminated())
	{
		jpeg_printf("Decode terminate, %s()-%d\n", __FUNCTION__, __LINE__);
		return; // error check and mem control. peter luo. 6-10.
	}
	
    dc_only_scan    = (phdl->decoder.spectral_start == 0);
    refinement_scan = (phdl->decoder.successive_high != 0);

    if ((phdl->decoder.spectral_start > phdl->decoder.spectral_end) || (phdl->decoder.spectral_end > 63))
      terminate(JPGD_BAD_SOS_SPECTRAL);

    if (dc_only_scan)
    {
      if (phdl->decoder.spectral_end)
        terminate(JPGD_BAD_SOS_SPECTRAL);
    }
    else if (phdl->decoder.comps_in_scan != 1)  /* AC scans can only contain one phdl->decoder.component */
      terminate(JPGD_BAD_SOS_SPECTRAL);

    if ((refinement_scan) && (phdl->decoder.successive_low != phdl->decoder.successive_high - 1))
      terminate(JPGD_BAD_SOS_SUCCESSIVE);

    if (dc_only_scan)
    {
      if (refinement_scan)
        decode_block_func = decode_block_dc_refine;
      else
        decode_block_func = decode_block_dc_first;
    }
    else
    {
      if (refinement_scan)
        decode_block_func = decode_block_ac_refine;
      else
        decode_block_func = decode_block_ac_first;
    }
	/*check for the stop status*/
	if(g_imagedec_stop_flag[phdl->id])
		return; 
    decode_scan(decode_block_func);

    //get_bits_2(phdl->decoder.bits_left & 7);
    phdl->decoder.bits_left = 16;
    //bit_buf = 0;
    get_bits_1(16);
    get_bits_1(16);
  }

	phdl->decoder.comps_in_scan = phdl->decoder.comps_in_frame;

  for (i = 0; i < phdl->decoder.comps_in_frame; i++)
    phdl->decoder.comp_list[i] = i;


  calc_mcu_block_order();
}

/*
extern unsigned long lCpuFreq;
extern unsigned long current_counter2,lastclkcounter2, clk_elapse2, second_elapse2, remain_clk2;
extern unsigned long current_counter3,lastclkcounter3, clk_elapse3, second_elapse3, remain_clk3;
*/

//------------------------------------------------------------------------------
// Find the start of the JPEG file and reads enough data to determine
// its size, number of components, etc.
IMAGEDEC_STATIC void decode_init(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	phdl->file_endian = BIG_ENDIAN;
	if((IMAGEDEC_THUMBNAIL == phdl->main.setting.mode)&&(!g_imagedec_info_valid[phdl->id]))
	{
		jpeg_printf("quick parsing sof\n");
		jpeg_quick_parsing_sof(phdl);
		if (!imagedec_sub_file_pos_init(phdl)) 
		{
			jpeg_printf("fseek error\n");
			return;
		}
	}
	g_imagedec_exif_error_code[phdl->id] = 1;
	jpeg_init();
	if (IsTerminated()) 
		return; // error check and mem control. peter luo. 6-10.

	locate_sof_marker(phdl);
	if (IsTerminated()) 
		return; // error check and mem control. peter luo. 6-10.
	if(IMAGEDEC_THUMBNAIL != phdl->main.setting.mode)
	{
		phdl->info.precision = 24;
		phdl->info.width = phdl->decoder.image_x_size;
		phdl->info.height = phdl->decoder.image_y_size;
		
		if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
		{
			if(g_imagedec_osd_ins.on)
			{
				pImage_mem_ctrl pmem = &g_imagedec_hdl[0].mem;
				UINT32 n = 0;				
				pimagedec_osd_raw_pic pic = &g_imagedec_osd_ins.pic;

				/*allocate the raw pic buff built by Y and C buffer*/
				pic->width = phdl->decoder.image_x_size;
				pic->height = phdl->decoder.image_y_size;
				if((pic->width > MAX_WIDTH_BY_OSD) || (pic->height > MAX_HEIGHT_BY_OSD))
				{
					terminate(JPGD_OSD_UNSUPPORTED_PIC_SIZE);
				}
				else
				{
					pic->stride = (pic->width + 15) & 0xFFF0;
					pic->y_size = pic->stride * ((pic->height + 31) & 0xFFE0);
					pic->c_size = pic->y_size>>1;
					pic->y_buf = (UINT8 *)pmem->mem_pos;
					n = pic->y_size + 256;
					n = (n + 3)& ~3;
					pmem->mem_pos += n;                        
					pic->c_buf = (UINT8 *)pmem->mem_pos;
					n = pic->c_size + 256;
					n = (n + 3)& ~3;
					pmem->mem_pos += n;     
					pic->y_buf = (UINT8 *)((UINT32)(pic->y_buf + 255) & 0xFFFFFF00);
					pic->c_buf = (UINT8 *)((UINT32)(pic->c_buf + 255) & 0xFFFFFF00);
					jpeg_printf("osd pic y %x c %x\n",pic->y_buf,pic->c_buf);
				}
			}
		}
	}
}
//------------------------------------------------------------------------------
IMAGEDEC_STATIC void decode_start(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	if ((phdl->decoder.progressive_flag) || (IMAGEDEC_REAL_SIZE == phdl->main.setting.mode))
	{
		phdl->mem.mem_up_limit = (unsigned long)(((((UINT32)(g_imagedec_config[phdl->id].decoder_buf))&0x0FFFFFFF) | 0x80000000) \
			+ g_imagedec_config[phdl->id].decoder_buf_len); 
		phdl->main.hw_act_update = 1;
		phdl->main.api_cmd.hw_acc_flag = g_imagedec_enable_hw_acce[phdl->id];
	}
	init_frame();
	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

	//john	if(phdl->decoder.progressive_flag && PRO2_MCUBUF_SIZE < get_width()*get_height()*sizeof(BLOCK_TYPE)) // int16 quality per pixel when decoding, so...
	//		iPro2_progressive=1;
	//	else 
	iPro2_progressive=0;
		
	if (phdl->decoder.progressive_flag) 
	{
		if (iPro2_progressive)
		{
			jpeg_printf("-----BIG progressive JPG!\n");
			Pro2_init_progressive(); // support progressive JPG pictures now, so... Peter Luo, 2003-6-6
		}
		else
		{
			jpeg_printf("-----NORMAL progressive JPG!\n");
			init_progressive(); // support progressive JPG pictures now, so... Peter Luo, 2003-6-6
		}	
	}
	else
	{
		jpeg_printf("-----Sequential JPG!\n");
		init_sequential();
	}
	
	if (IsTerminated()) return;  

	struct Rect old_src;
	struct Rect old_dst;
	UINT32 width = 0;
	UINT32 height = 0;

	if(IMAGEDEC_FULL_SRN == phdl->main.setting.mode)
	{
		width = phdl->imageout.image_out_w;
		height = phdl->imageout.image_out_h;
		MEMCPY((void *)&old_src,&(phdl->imageout.de_src_rect),sizeof(struct Rect));
		MEMCPY((void *)&old_dst,&(phdl->imageout.de_dst_rect),sizeof(struct Rect));
	}

	g_imagedec_m33_combine_hw_sw_flag = 0;	
	phdl->decoder.out_type = phdl->decoder.scan_type;	
	//phdl->main.setting.hw_acc_flag = 0;
	if ((phdl->decoder.progressive_flag) || (IMAGEDEC_REAL_SIZE == phdl->main.setting.mode))
	{
		Init_Scaling_factors(phdl); 
		phdl->imageout.de_src_rect.uStartX = phdl->imageout.de_src_rect.uStartY = 0;
		phdl->imageout.de_src_rect.uWidth = 720;
		phdl->imageout.de_src_rect.uHeight = 576*5;
		phdl->imageout.de_dst_rect.uStartX = phdl->imageout.de_dst_rect.uStartY = 0;
		phdl->imageout.de_dst_rect.uWidth = 720;
		phdl->imageout.de_dst_rect.uHeight = 576*5;				
	}
	else
	{
		if(g_imagedec_enable_hw_acce[phdl->id])
		{
			jh_hw_init(phdl); 
		}
		
		if(phdl->main.setting.hw_acc_flag)
		{
			if(g_imagedec_m33_enable_hw_vld)
			{
				if(g_imagedec_m33_combine_hw_sw_flag)
				{	
					Init_Scaling_factors(phdl); 			
					phdl->imageout.de_src_rect.uStartX = phdl->imageout.de_src_rect.uStartY = 0;
					phdl->imageout.de_src_rect.uWidth = 720;
					phdl->imageout.de_src_rect.uHeight = 576*5;
					phdl->imageout.de_dst_rect.uStartX = phdl->imageout.de_dst_rect.uStartY = 0;
					phdl->imageout.de_dst_rect.uWidth = 720;
					phdl->imageout.de_dst_rect.uHeight = 576*5;				
				}				
				
				construct_hw_vld_table(phdl);
			}
			
			//temp_IDCT_buf = (int*)imagedec_malloc(64<<2);

			if(!g_imagedec_m33_combine_hw_sw_flag)
				jhcovs_m3329e_init(phdl);
		}
		
		if(!phdl->main.setting.hw_acc_flag)
		{
			phdl->mem.mem_up_limit = (unsigned long)(((((UINT32)(g_imagedec_config[phdl->id].decoder_buf))&0x0FFFFFFF) | 0x80000000) \
				+ g_imagedec_config[phdl->id].decoder_buf_len); 			
			Init_Scaling_factors(phdl); 
			phdl->main.hw_act_update = 1;
			phdl->main.api_cmd.hw_acc_flag = g_imagedec_enable_hw_acce[phdl->id];
			
			phdl->imageout.de_src_rect.uStartX = phdl->imageout.de_src_rect.uStartY = 0;
			phdl->imageout.de_src_rect.uWidth = 720;
			phdl->imageout.de_src_rect.uHeight = 576*5;
			phdl->imageout.de_dst_rect.uStartX = phdl->imageout.de_dst_rect.uStartY = 0;
			phdl->imageout.de_dst_rect.uWidth = 720;
			phdl->imageout.de_dst_rect.uHeight = 576*5;	
		}
	}
	
	if((IMAGEDEC_FULL_SRN == phdl->main.setting.mode) 
		&& (0 == phdl->imageout.frm[phdl->imageout.frm_idx_using].busy))
	{
		if((width != phdl->imageout.image_out_w) || (height != phdl->imageout.image_out_h)
			|| (0 != MEMCMP((void *)&old_src,(void *)&phdl->imageout.de_src_rect,sizeof(struct Rect)))
			|| (0 != MEMCMP((void *)&old_dst,(void *)&phdl->imageout.de_dst_rect,sizeof(struct Rect))))
		{
			jpeg_printf("clear buf when de with different scale parametes\n");
			jh_mem_set(phdl->imageout.old_frm.frm_y_addr,0x10,phdl->imageout.old_frm.frm_y_len);
			jh_mem_set(phdl->imageout.old_frm.frm_c_addr,0x80,phdl->imageout.old_frm.frm_c_len);						
			jh_mem_set(phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_addr,0x10,phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_y_len);
			jh_mem_set(phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_addr,0x80,phdl->imageout.frm[phdl->imageout.frm_idx_using].frm_c_len);			
		}
	}
}

// Loads and dephdl->decoder.quantizes the next row of (already decoded) coefficients.
// Progressive images only.
IMAGEDEC_STATIC  void load_next_row(void)
{
	int i;
	BLOCK_TYPE *p;
	QUANT_TYPE *q;
	BLOCK_TYPE *pAC;
	BLOCK_TYPE *pDC;
	int component_num, component_id;
	int block_x_mcu[JPGD_MAXCOMPONENTS];
	int block_x_mcu_ofs = 0, block_y_mcu_ofs = 0;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	int mcu_row, mcu_block, row_block = 0;
	//int mcu_row, mcu_block, iCount=0, row_block=get_iblock_ordinal(); // peter luo 6-6

  MEMSET(block_x_mcu, 0, JPGD_MAXCOMPONENTS * sizeof(int));

  for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
  {
    block_x_mcu_ofs = 0;
	block_y_mcu_ofs = 0;

    for (mcu_block = 0; mcu_block < phdl->decoder.blocks_per_mcu; mcu_block++)
    {
      component_id = phdl->decoder.mcu_org[mcu_block];

	  if (!selected_comp (component_id) ) goto next;// the selected Y, Cb, or Cr. peter luo 6-18

      p = phdl->decoder.block_seg[row_block];
      q = phdl->decoder.quant[phdl->decoder.comp_quant[component_id]];
//jpeg_printf("comid5:%d\n",component_id);
      pAC = coeff_buf_getp(phdl->decoder.ac_coeffs[component_id],
                                  block_x_mcu[component_id] + block_x_mcu_ofs,
                                  phdl->decoder.block_y_mcu[component_id] + block_y_mcu_ofs);

      pDC = coeff_buf_getp(phdl->decoder.dc_coeffs[component_id],
                                  block_x_mcu[component_id] + block_x_mcu_ofs,
                                  phdl->decoder.block_y_mcu[component_id] + block_y_mcu_ofs);

	  if (IsTerminated()) return; // error check and mem control. peter luo. 7-19.

      p[0] = pDC[0];
//	  jpeg_printf("p[0]:%x\n",p[0]);
      mem_memcpy(&p[1], &pAC[1], 63 * sizeof(BLOCK_TYPE));

        for (i = 63; i > 0; i--)
          if (p[ZAG[i]])
            break;

        //block_num[row_block++] = i + 1;

        for ( ; i >= 0; i--)
          if (p[ZAG[i]])
		  {
			  p[ZAG[i]] *= q[i];

	//	  jpeg_printf(" p[ZAG[i]]:%x\n", p[ZAG[i]]);
		  }

next:
      row_block++;
/*	  if (1 == phdl->decoder.comps_in_frame) // peter luo, 6-6
		  row_block++;
	  else { // YCbCr
		  if (0==iset_comp){ // Y
			  if(iCount==phdl->decoder.comps_in_frame-3) {iCount=0; row_block+=3;} // jmp over the cb and cr: 3
			  else {row_block++; iCount++;}
		  }
		  else row_block+=phdl->decoder.comps_in_frame; // Cb or Cr
	  }

*/

      if (phdl->decoder.comps_in_scan == 1)
        block_x_mcu[component_id]++;
      else
      {
        if (++block_x_mcu_ofs == phdl->decoder.comp_h_samp[component_id])
        {
          block_x_mcu_ofs = 0;

          if (++block_y_mcu_ofs == phdl->decoder.comp_v_samp[component_id])
          {
            block_y_mcu_ofs = 0;

            block_x_mcu[component_id] += phdl->decoder.comp_h_samp[component_id];
          }
        }
      }
    }
  }

  if (phdl->decoder.comps_in_scan == 1)
    phdl->decoder.block_y_mcu[phdl->decoder.comp_list[0]]++;
  else
  {
    for (component_num = 0; component_num < phdl->decoder.comps_in_scan; component_num++)
    {
      component_id = phdl->decoder.comp_list[component_num];

      phdl->decoder.block_y_mcu[component_id] += phdl->decoder.comp_v_samp[component_id];
    }
  }
}

//manson: for supporting progressive JPEG, we still have to do IDCT row by row
//But I used my fast IDCT and scaling instead. 
//Don't use this function for baseline JPEG
IMAGEDEC_STATIC void transform_row(void)
{
	int mcu_row;
	int mcu_block;
    BLOCK_TYPE *Psrc_ptr;
	int i;
	int j;
	int component_id;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

  {
 	  Psrc_ptr = phdl->decoder.block_seg[0];

	if (IsTerminated()) return; // error check and mem control. peter luo. 6-10.

    for (mcu_row = 0; mcu_row < phdl->decoder.mcus_per_row; mcu_row++)
    {
      for (mcu_block = 0; mcu_block < phdl->decoder.blocks_per_mcu; mcu_block++)
	  {
          component_id = phdl->decoder.mcu_org[mcu_block];

		  if (!selected_comp (component_id) ){
			  Psrc_ptr = Psrc_ptr+64;
			  continue;
		  }
		  for(j=0;j<64;j++){
			temp_IDCT_buf[j] = *Psrc_ptr;
			Psrc_ptr++;
		  }
		  if(IMAGEDEC_REAL_SIZE != phdl->main.setting.mode)
		  {
			  if(iset_comp==0)
				IDCT_Scaling_Routine(phdl,1);
		  	else if(iset_comp==2)
				IDCT_Scaling_Cb();
		  	else if(iset_comp==1)
				IDCT_Scaling_Cr();
		  }
		  else
		  {
		  //	if(0 == iset_comp)
			IDCT_Duplication(phdl,mcu_row,mcu_block,iset_comp);
		  }
	  }
	  
	  if(IMAGEDEC_REAL_SIZE == phdl->main.setting.mode)
		  imagedec_idct_dup_next_mcu(phdl);
	}
  }
}

// Find end of image (EOI) marker, so we can return to the user the
// exact size of the input stream.
IMAGEDEC_STATIC void find_eoi(pImagedec_hdl phdl)
{
  if (!phdl->decoder.progressive_flag)
  {
    // Attempt to read the EOI marker.
    //get_bits_2(phdl->decoder.bits_left & 7);

    // Prime the bit buffer
    phdl->decoder.bits_left = 16;
    //bit.bit_buf = 0;
    get_bits_1(16);
    get_bits_1(16);

    // The next marker _should_ be EOI
    process_markers(phdl);
  }

  phdl->in_stream.total_bytes_read -= phdl->in_stream.in_buf_left;
}

#define MAIN_API
#ifdef JPEG_NEW_HUF_TABLE
void* imagedec_malloc(int n )
{
	int i = 0; 
	void *q = NULL; 
	pImage_mem_ctrl pmem = &g_imagedec_hdl[0].mem;

	n = (n + 3)& ~3;
	q = (void *)pmem->mem_pos;
	pmem->mem_pos += n;

	if ( pmem->mem_up_limit < pmem->mem_pos)
	{
		jpeg_printf ("in %s, overused mem!!!\n",__FUNCTION__);
		jpeg_printf("mempos: 0x%x upper_limit:0x%x\n",pmem->mem_pos,pmem->mem_up_limit);
		iterminate(JPGD_NOTENOUGHMEM); // mem check by peter luo. 6-10
	}
	MEMSET(q, 0, n);
	pmem->blocks[i] = q;
	return ((void *)(((uint)q + 3) & ~3));
}
#else
void* imagedec_malloc(int n )
{
	int i;
	void *q;
	pImage_mem_ctrl pmem = &g_imagedec_hdl[0].mem;
//	libc_printf("mempos:%x n=%d\n",mem_pos,n);
  // Find a free slot. The number of allocated slots will
  // always be very low, so a linear search is good enough.
  for (i = 0; i < JPGD_MAXBLOCKS; i++)
  {
    if (pmem->blocks[i] == NULL)
      break;
  }

  if (i == JPGD_MAXBLOCKS)
  {
	 iterminate(JPGD_TOO_MANY_BLOCKS);
  }

  //q = (void*)malloc(n + 8);
	q = (void *)pmem->mem_pos;
	pmem->mem_pos += n+8;

	// peter luo control buffer mem. 6-9.
	if ( pmem->mem_up_limit < pmem->mem_pos)
	{
		jpeg_printf ("in %s, overused mem!!!\n",__FUNCTION__);
		jpeg_printf("mempos: 0x%x upper_limit:0x%x\n",pmem->mem_pos,pmem->mem_up_limit);
	    iterminate(JPGD_NOTENOUGHMEM); // mem check by peter luo. 6-10
	}
  MEMSET(q, 0, n + 8);

  pmem->blocks[i] = q;
//	libc_printf("alloc:%x n=%d\n",q,n);
  // Round to qword boundry, to avoid misaligned accesses with MMX code
  return ((void *)(((uint)q + 7) & ~7));
}
#endif

//////////////////////// output interface functions ////////////////////////
void set_comp (pImagedec_hdl phdl,  int icomp) // decide which phdl->decoder.component to display: 0:Y, 1: Cb, or 2: Cr
{
	icomp = (icomp>0)? icomp : 0; // use itself again
	iset_comp = (icomp<3)? icomp : 0; // between 0 ~ 3.
}

int max_ycbcr_comp (pImagedec_hdl phdl ) // not the block components, the y cb cr comps
{
	if (!phdl->decoder.progressive_flag) return 1;
	return (1==phdl->decoder.comps_in_frame)? 1: 3; // if 1, gray pic.
}

int IsProgressive (pImagedec_hdl phdl) // whether this JPG picture is progressive format.
{
	return phdl->decoder.progressive_flag;
}

int IsPro2_big (void) // whether this JPG picture is progressive format.
{
	return iPro2_progressive;
}

inline int IsTerminated (void) // whether inner error, or all the buffer memories are used.
{
	if(error_code)
		{JPEG_ASSERT(0);}
	return error_code; // add an error: all the buffer memories are used. Peter luo. 6-10
}

//------------------------------------------------------------------------------
// Call get_error_code() after constructing to determine if the stream
// was valid or not. You may call the get_width(), get_height(), etc.
// methods after the constructor is called.
// You may then either destruct the object, or begin decoding the image
// by calling begin(), then decode().
void imagedec_jpeg_init(void)
{
	error_code = 0;
	// new mem use. Peter luo 6-27. 
	//john	phdl->in_stream.in_buf = (uchar *)TMP_IN_BUF;  // peter luo, 5-21
	//in_buf = (uchar *)((TMP_IN_BUF&0xffffff) | 0xa0000000);  // frank, 2003.09.03
      // peter luo, 5-21
	decode_init();
}
//------------------------------------------------------------------------------
// If you wish to decompress the image, call this method after constructing
// the object. If JPGD_OKAY is returned you may then call decode() to
// fetch the scan lines.
int imagedec_jpeg_begin(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	if (phdl->decoder.ready_flag)
		return (JPGD_OKAY);

	if (error_code)
		return (JPGD_FAILED);

	decode_start();
	if (IsTerminated()) 
		return (JPGD_FAILED); // error check and mem control. peter luo. 6-10.
	if(IMAGEDEC_REAL_SIZE == phdl->main.setting.mode)
		imagedec_idct_dup_init(phdl);
	phdl->decoder.ready_flag = true;
	return (JPGD_OKAY);
}

// Returns the next scan line.
// Returns JPGD_DONE if all scan lines have been returned.
// Returns JPGD_OKAY if a scan line has been returned.
// Returns JPGD_FAILED if an error occured.
int imagedec_jpeg_decode(void)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	if ((error_code) || (!phdl->decoder.ready_flag))
		return (JPGD_FAILED);

 // jpeg_printf("mcus_of_col:%d\n",phdl->decoder.mcus_of_col);

  /*
  if (phdl->decoder.total_lines_left <= 0)
  {
	  jpeg_printf("======phdl->decoder.total_lines_left = %d, phdl->decoder.max_mcu_y_size = %d\n", phdl->decoder.total_lines_left, phdl->decoder.max_mcu_y_size);
	  return (JPGD_DONE);
  }
	*/
	if ((phdl->decoder.mcus_of_col <= 0) || (phdl->decoder.real_size_done))
		return (JPGD_DONE);
	if (phdl->decoder.progressive_flag) {
		if (iPro2_progressive)
			Pro2_load_next_row(); // support progressive JPG pictures now, so... Peter Luo, 2003-6-6
		else
			load_next_row(); // support progressive JPG pictures now, so... Peter Luo, 2003-6-6
	}
	else
	{
		if(phdl->main.setting.hw_acc_flag)
		{
			if(g_imagedec_m33_enable_hw_vld)
			{
				if(g_imagedec_m33_combine_hw_sw_flag)
				{
					if(phdl->imageout.combine_hw_mcu_col_cnt >= phdl->imageout.combine_hw_mcu_col_max)
					{
						phdl->imageout.combine_hw_mcu_col_cnt = 0;
						combine_sw_scale_row();
					}
					
					jh_frame_addr(phdl->imageout.hw_acc_frm_y, phdl->imageout.hw_acc_frm_c);
					jh_reg_mcu_coord_y(phdl->imageout.combine_hw_mcu_col_cnt);

					phdl->imageout.divide_mcu_row = 0;					
					decode_next_row_hw_vld((phdl->decoder.mcus_of_col <= 1));		

					if(phdl->decoder.mcus_of_col <= 1)
						combine_sw_scale_row();

					phdl->imageout.combine_hw_mcu_col_cnt++;
				}
				else 
				{
					if(g_imagedec_m33_extend_hw_vld && (phdl->imageout.hw_max_divide_v > 0))
					{
						UINT32 y_addr, c_addr;
						UINT32 y_base_offset;
						
						if(phdl->imageout.divide_mcu_col >= JPEG_HW_MAX_MCU_VALUE)
						{
							phdl->imageout.divide_mcu_col = 0;

							phdl->imageout.last_mcu_col = phdl->decoder.max_mcus_per_col - phdl->decoder.mcus_of_col;
							jpeg_printf("last_mcu_col %d\n", phdl->imageout.last_mcu_col);
						}
						
						y_base_offset = phdl->imageout.last_mcu_col<<(phdl->imageout.pixel_per_mcu_v - phdl->imageout.hw_v_pre);
						y_addr = phdl->imageout.hw_acc_frm_y + (y_base_offset & 0xFFF0) * phdl->imageout.hw_acc_stride 
							+ ((y_base_offset & 0x0F) << 4);

						switch(phdl->decoder.scan_type)
						{
							case JPGD_YH1V1:
							case JPGD_YH2V1:
								c_addr = phdl->imageout.hw_acc_frm_c + (y_base_offset & 0xFFF0) * phdl->imageout.hw_acc_stride 
									+ ((y_base_offset & 0x0F) << 4);
								break;
							case JPGD_YH4V1:
								c_addr = phdl->imageout.hw_acc_frm_c + (y_base_offset & 0xFFF0) * (phdl->imageout.hw_acc_stride>>1) 
									+ ((y_base_offset & 0x0F) << 4);			
								break;
							case JPGD_YH1V2:
							case JPGD_YH2V2:
								y_base_offset >>= 1;
								c_addr = phdl->imageout.hw_acc_frm_c + (y_base_offset & 0xFFF0) * phdl->imageout.hw_acc_stride 
									+ ((y_base_offset & 0x0F) << 4);	
								break;
							default:
								return (JPGD_FAILED);	
						}
						
						jh_frame_addr(y_addr, c_addr);
						jh_reg_mcu_coord_y(phdl->imageout.divide_mcu_col);
						
						phdl->imageout.divide_mcu_col++;

						phdl->imageout.divide_mcu_row = 0;
						phdl->imageout.last_mcu_row = 0;
					}
					else
						jh_reg_mcu_coord_y(phdl->decoder.max_mcus_per_col - phdl->decoder.mcus_of_col);
					decode_next_row_hw_vld((phdl->decoder.mcus_of_col <= 1));
				}
			}
			else
			{
				jh_reg_mcu_coord_y(phdl->decoder.max_mcus_per_col - phdl->decoder.mcus_of_col);
				decode_next_row_hw();
			}
			
		}
		else
			decode_next_row();

		if(!g_imagedec_m33_combine_hw_sw_flag)
			PrepareForNextRow(0); //update some parameters and get ready to process next row of input data
	}

	// Find the EOI marker if that was the last row.
	if (phdl->decoder.total_lines_left <= phdl->decoder.max_mcu_y_size)
		find_eoi(phdl);
	
	//manson: we don't do IDCT row by row, we do block by block
	//transform_row();
    if (phdl->decoder.progressive_flag) {
		transform_row();
		if(iset_comp==0)
			PrepareForNextRow(1); //update some parameters and get ready to process next row of input data
		else if (iset_comp==2)
			PrepareForNextRow(2); //update some parameters and get ready to process next row of input data
		else if (iset_comp==1)
			PrepareForNextRow(3); //update some parameters and get ready to process next row of input data

		if(IMAGEDEC_REAL_SIZE == phdl->main.setting.mode)
	 		imagedec_idct_dup_next_row(phdl);	
	}
	phdl->decoder.mcu_lines_left = phdl->decoder.max_mcu_y_size;

	 phdl->decoder.total_lines_left--;

  if (IsTerminated()) return (JPGD_FAILED); // error check and mem control. peter luo. 6-10.
  ////

  //phdl->decoder.total_lines_left -= phdl->decoder.max_mcu_y_size;
  phdl->decoder.mcus_of_col --;
  if(phdl->decoder.progressive_flag)
  {
  	phdl->info.coding_progress += (1<<16)/(phdl->decoder.max_mcus_per_col*3);
	//jpeg_printf("pro <%x> com <%d>\n",phdl->info.coding_progress,iset_comp);
  }
  else
  	 phdl->info.coding_progress = ((phdl->decoder.max_mcus_per_col - phdl->decoder.mcus_of_col)<<16)/(phdl->decoder.max_mcus_per_col);
  if(phdl->info.coding_progress - phdl->info.last_called_prog >= 3000)
  {
	phdl->info.last_called_prog = phdl->info.coding_progress;
	if(NULL != phdl->main.status)
		phdl->main.status((void *)&phdl->info.last_called_prog);
  }
  g_imagedec_info_valid[phdl->id] = 1;
 // jpeg_printf("col <%d>\n",phdl->decoder.mcus_of_col);
  return (JPGD_OKAY);
}

void jpeg_quick_parsing_sof(pImagedec_hdl phdl)
{
	int c = 0;
	JPEG_ENTRY;

	error_code = 0;
	jpeg_init();
	 locate_soi_marker();
	while(1)
	{
		if (IsTerminated()) 	
		{
			jpeg_printf("get jpg info error\n");
			return;
		}
		c = next_marker();
		if((M_SOF0 == c) || (M_SOF1 == c) || (M_SOF2 == c))
			break;
		else if((c >= M_RST0) && (c <= M_RST7))
			{terminate(JPGD_GET_INFO_ERROR);}	
		else
			{skip_variable_marker();}		
	}
	read_sof_marker();
	phdl->info.precision = 24;
	phdl->info.width = phdl->decoder.image_x_size;
	phdl->info.height = phdl->decoder.image_y_size;
	JPEG_EXIT;
}


