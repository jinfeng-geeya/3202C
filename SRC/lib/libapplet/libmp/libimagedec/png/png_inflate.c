/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png_inflate.c
 *  (I)
 *  Description: inflate decompress implement
 *  (S)
 *  History:(M)
 *      	Date        			Author         	Comment
 *      	====        			======		=======
 * 0.		2008.2.1			Sam		Create
 ****************************************************************************/
#include <sys_config.h>
#include <basic_types.h>
#include <mediatypes.h>
#include <os/tds2/itron.h>
#include <api/libc/printf.h>
#include <osal/osal_timer.h>
#include <api/libc/printf.h>
#include <api/libc/string.h>
#include <osal/osal.h>
#include <api/libmp/png.h>
#include "png_core.h"
#include "png_debug.h"

//bit operation
#define BIT_VARIABLES \
	INT16 byte = 0; \
	UINT32 bit_value = 0; \
	UINT32 bit_left = 0; \
	UINT32 byte_left = 0

#define BIT_RECOVER	\
	bit_value = inf->bit_value;	\
	bit_left = inf->bit_left;	\
	table = inf->table

#define BIT_SAVE	\
	inf->bit_value = bit_value;	\
	inf->bit_left = bit_left;	\
	inf->table = table

#define BIT_SET_LEN \
	byte_left = info->chunk[info->IDAT_idx].len; \
	if(0 == byte_left) \
		goto INF_DEC_FAIL
		
#define READ_BYTE \
	if(byte_left)	{ \
		byte_left--;\
		byte = png_in_read_byte(in); \
		bit_value |= ((UINT8)byte)<<bit_left; 	\
		bit_left = (-1 == byte) ? bit_left: bit_left + 8; \
	}
	
#define READ_2BYTES		READ_BYTE;READ_BYTE

#define NEXT_BITS(n) \
	if(bit_left < n)	 {READ_2BYTES;} \
	if(bit_left < n)	 { \
		PNG_PRINTF("Finish IDAT CHUNK Parse bit left %d byte %d need %d\n",bit_left,byte_left,n);	\
		break;\
	}
	
#define MOVE_BITS(n) \
	bit_value >>= n; \
	bit_left -= n
	
#define GET_BITS(n)		(bit_value & ((1<<n) - 1))

//block header
#define INF_READ_BLK_HEADR \
	NEXT_BITS(3); \
	data = GET_BITS(3); \
	inf->pars.BFINAL = data&1; \
	inf->pars.BTYPE = (data>>1)&3; \
	MOVE_BITS(3)
#define RESTORE_LAST_BYTE	\
	MOVE_BITS(bit_left%8);	\
	while(bit_left){	\
		bit_left -= 8;	\
		PNG_PRINTF("Restore last byte %x\n",((bit_value>>bit_left)&0xFF));	\
		png_in_write_byte(in,((bit_value>>bit_left)&0xFF));\
	}

#define INF_HUF_DEC_LEN(num,buf)	\
	while(inf->count < num)	\
	{	\
		NEXT_BITS(PNG_INF_LEN_HUF_MIN_BITS);	\
		data = table[GET_BITS(PNG_INF_LEN_HUF_MIN_BITS)];	\
		e_bits = data&0xFF;	\
		bits = (data>>8) & 0xFF;	\
		base = (data>>16) & 0xFFFF;	\
		if(INF_CTRL_CHAR == e_bits){	\
			MOVE_BITS(bits);		\
			*(buf + inf->count) = base;	\
			inf->pre_len = base;	\
			inf->count++;	\
		}	\
		else{	\
			e_bits += bits;	\
			NEXT_BITS(e_bits);	\
			data = base + (GET_BITS(e_bits)>>bits);	\
			MOVE_BITS(e_bits);	\
			e_bits -= bits;		\
			switch(e_bits){	\
				case 2:		\
					break;	\
				case 3:		\
				case 7:		\
					inf->pre_len = 0;		\
					break;	\
				default:		\
					goto INF_DEC_FAIL;	\
			}	\
			while(data--){	\
				*(buf + inf->count) = inf->pre_len;	\
				inf->count++;	\
			}\
		}\
	}

PNG_RET png_inflate_pass_done(struct png_core *core)
{
	struct png_inflate *inf = &core->inf;
	
	return (1 == inf->scan.finish)?PNG_TRUE:PNG_FALSE;
}

PNG_RET png_inflate_dec(struct png_core *core)
{
	struct png_inflate *inf = &core->inf;
	struct png_inf_scan *scan = &inf->scan;
	struct png_cfg *cfg = &inf->cfg;
	struct png_in *in = &core->in;
	struct png_info *info = &core->info;
	UINT32 *table = NULL;
	UINT32 data = 0,base = 0;
	UINT8 bits = 0,e_bits = 0;
	UINT32 i = 0;
	BIT_VARIABLES;
	BIT_SET_LEN;
	BIT_RECOVER;

	PNG_ENTER;
INF_DEC_BEGIN:
	switch(inf->status){
		case INF_BLK_HEADER:
			inf->deflate_blk_num++;
			PNG_PRINTF("Start a new deflate block %d\n",inf->deflate_blk_num);
			MEMCPY((void *)cfg,(void *)&core->cfg,sizeof(struct png_cfg));
			INF_READ_BLK_HEADR;
			switch(inf->pars.BTYPE){
				case PNG_INF_BLK_NONE_COMMPRESSION:
					inf->status = INF_NO_COM_LEN;
					goto INF_DEC_BEGIN;
				case PNG_INF_BLK_FIXED_HUF_TABLE:
					PNG_PRINTF("Fixed huffman table final %d\n", inf->pars.BFINAL);
					inf->code_table_bits = 9;
					inf->code_table_num = 288;
					inf->dis_table_bits = 5;
					inf->dis_table_num = 32;
					inf->code_len = (UINT8 *)PNG_MALLOC(cfg,inf->code_table_num);
					inf->dis_len = (UINT8 *)PNG_MALLOC(cfg,inf->dis_table_num);
					if(NULL == inf->code_len || NULL == inf->dis_len){
						PNG_ASSERT(PNG_ERR_MALLOC);
						goto INF_DEC_FAIL;
					}
					for(i = 0;i < inf->code_table_num;i++){
						if(i < 144)
							*(inf->code_len + i) = 8;
						else if(i < 256)
							*(inf->code_len + i) = 9;							
						else if(i < 280)
							*(inf->code_len + i) = 7;							
						else
							*(inf->code_len + i) = 8;
					}
					for(i = 0;i < inf->dis_table_num;i++){
						*(inf->dis_len + i) = 5;
					}
					inf->status =  INF_CODE_TABLE;
					break;
				case PNG_INF_BLK_DYN_HUF_TABLE:
					PNG_PRINTF("Dynamic huffman table\n");
					inf->status =  INF_TAB_LEN;
					break;
				default:
					PNG_ASSERT(PNG_ERR_BLK_TYPE);
					goto INF_DEC_FAIL;
			}
			goto INF_DEC_BEGIN;
		case INF_TAB_LEN:
			NEXT_BITS(14);
			inf->code_table_num = 257 + GET_BITS(5);
			MOVE_BITS(5);
			inf->dis_table_num = 1 + GET_BITS(5);
			MOVE_BITS(5);
			inf->len_tab_num = 4 + GET_BITS(4);
			MOVE_BITS(4);
			inf->count = 0;
			inf->len_len = (UINT8 *)PNG_MALLOC(cfg,inf->len_tab_num);
			inf->code_len = (UINT8 *)PNG_MALLOC(cfg,inf->code_table_num);
			inf->dis_len = (UINT8 *)PNG_MALLOC(cfg,inf->dis_table_num);
			if(NULL == inf->len_len || NULL == inf->code_len || NULL == inf->dis_len){
				PNG_ASSERT(PNG_ERR_MALLOC);
				goto INF_DEC_FAIL;
			}
			inf->status = INF_LEN_TABLE;
		case INF_LEN_TABLE:
			while(inf->count < inf->len_tab_num){
				NEXT_BITS(3);
				*(inf->len_len + inf->count) = GET_BITS(3);
				MOVE_BITS(3);
				inf->count++;
			}
			if(inf->count == inf->len_tab_num){
				inf->len_tab_bits = PNG_INF_LEN_HUF_MIN_BITS;
				PNG_PRINTF("Infalte: create code len table root tree num %d bits %d\n",inf->len_tab_num\
					,inf->len_tab_bits);
				if(!png_huf_table(cfg,INF_HUF_TAB_CODE_LEN,inf->len_len,inf->len_tab_num,&(inf->len_table.code)\
					,inf->len_tab_bits)){
					goto INF_DEC_FAIL;
				}
				inf->count = 0;
				inf->status = INF_DEC_CODE_LEN;
				inf->len_table.size = inf->len_tab_num;
				table = inf->len_table.code;
				goto INF_DEC_BEGIN;
			}
			break;
		case INF_DEC_CODE_LEN:
			MEMSET((void *)inf->code_len,0,inf->code_table_num);
			INF_HUF_DEC_LEN((UINT32)inf->code_table_num,inf->code_len);
			if(inf->count >= inf->code_table_num){
				inf->count = 0;
				inf->code_table_bits = PNG_INF_CODE_HUF_MIN_BITS;
				inf->status = INF_DEC_DIS_LEN;
				goto INF_DEC_BEGIN;
			}
			break;
		case INF_DEC_DIS_LEN:
			MEMSET((void *)inf->dis_len,0,inf->dis_table_num);
			INF_HUF_DEC_LEN((UINT32)inf->dis_table_num,inf->dis_len);
			if(inf->count >= inf->dis_table_num){
				inf->count = 0;
				inf->dis_table_bits = PNG_INF_DIS_HUF_MIN_BITS;
				inf->status = INF_CODE_TABLE;
				goto INF_DEC_BEGIN;
			}
			break;
		case INF_CODE_TABLE:
			PNG_PRINTF("Infalte: create code table root tree num %d bits %d\n",inf->code_table_num\
					,inf->code_table_bits);
			if(!png_huf_table(cfg,INF_HUF_TAB_CODE,inf->code_len,inf->code_table_num,&(inf->sym_table.code)\
				,inf->code_table_bits)){
				goto INF_DEC_FAIL;
			}
			inf->sym_table.size = inf->code_table_num;
			inf->status = INF_DIS_TABLE;
		case INF_DIS_TABLE:
			PNG_PRINTF("Infalte: create distance table root tree num %d bits %d\n",inf->dis_table_num\
					,inf->dis_table_bits);
			if(!png_huf_table(cfg,INF_HUF_TAB_DIS,inf->dis_len,inf->dis_table_num,&(inf->dis_table.code)\
				,inf->dis_table_bits)){
				goto INF_DEC_FAIL;
			}
			inf->dis_table.size = inf->dis_table_num;			
			table = inf->sym_table.code;
			PNG_PRINTF("Start inflate decompressing\n");
			inf->status = INF_LITER_CODE;
		case INF_LITER_CODE:
#ifdef PNG_MEM_DOWN
			if((((UINT32)scan->write) - ((UINT32)scan->read)) >= scan->line_size){
				png_filter_scan_line(core);
			}
#endif
			NEXT_BITS(inf->code_table_bits);
			data = table[GET_BITS(inf->code_table_bits)];
			e_bits = data & 0xFF;
			bits = (data>>8) & 0xFF;
			base = (data>>16) & 0xFFFF;	
			if(e_bits > INF_CTRL_SUB_BITS_BASE && e_bits < INF_CTRL_CHAR){
				e_bits -= INF_CTRL_SUB_BITS_BASE;
				e_bits += bits;
				NEXT_BITS(e_bits);
				data = table[base + (GET_BITS(e_bits)>>bits)];
				e_bits = data & 0xFF;
				bits += (data>>8) & 0xFF;
				base = (data>>16) & 0xFFFF;
			}
			MOVE_BITS(bits);
			if(INF_CTRL_BLK_END == e_bits){
				PNG_PRINTF("Stop deflate block decompress file offset 0x%x final %d\n", core->in.file.offset
					, inf->pars.BFINAL);
				if(!inf->pars.BFINAL)
				{
					inf->status = INF_BLK_HEADER;
					goto INF_DEC_BEGIN;
				}

#ifdef PNG_MEM_DOWN
				PNG_PRINTF("Left scan size %d line size %d\n", (int)(scan->write - scan->read)
					, scan->line_size);
				while((((UINT32)scan->write) - ((UINT32)scan->read)) >= scan->line_size){
					png_filter_scan_line(core);
				}
#endif				
				core->flag &= ~PNG_INFO_CMF;
				RESTORE_LAST_BYTE;
				break;
			}
			else if(INF_CTRL_CHAR == e_bits){
				*(scan->write++) = base; 
				scan->count++;
				goto INF_DEC_BEGIN;
			}
			else{
				inf->len_base = base;
				inf->len_ext_bits = e_bits;
			}
			inf->status = INF_LEN_CODE;
		case INF_LEN_CODE:
			inf->len = inf->len_base;
			if(inf->len_ext_bits > 0){
				NEXT_BITS(inf->len_ext_bits);
				inf->len += GET_BITS(inf->len_ext_bits);
				MOVE_BITS(inf->len_ext_bits);
			}
			table = inf->dis_table.code;
			inf->status = INF_DIS_CODE;
		case INF_DIS_CODE:
			NEXT_BITS(inf->dis_table_bits);
			data = table[GET_BITS(inf->dis_table_bits)];
			e_bits = data & 0xFF;
			bits = (data>>8) & 0xFF;
			base = (data>>16) & 0xFFFF;	
			if(e_bits > INF_CTRL_SUB_BITS_BASE && e_bits < INF_CTRL_CHAR){
				e_bits -= INF_CTRL_SUB_BITS_BASE;
				e_bits += bits;
				NEXT_BITS(e_bits);
				data = table[base + (GET_BITS(e_bits)>>bits)];
				e_bits = data & 0xFF;
				bits += (data>>8) & 0xFF;
				base = (data>>16) & 0xFFFF;
			}
			MOVE_BITS(bits);
			inf->dis_base = base;
			inf->dis_ext_bits = e_bits;
			inf->status = INF_DIS_EXTRA;
		case INF_DIS_EXTRA:
			inf->dis = inf->dis_base;
			if(inf->dis_ext_bits > 0){
				NEXT_BITS(inf->dis_ext_bits);
				inf->dis += GET_BITS(inf->dis_ext_bits);
				MOVE_BITS(inf->dis_ext_bits);
			}
			inf->count = 0;
			inf->status = INF_WRITE_DATA;
		case INF_WRITE_DATA:
			data = inf->len;
			while(data)
			{
				if(data > inf->dis){
					PNG_MEMCPY((void *)(scan->write + inf->count),(void *)(scan->write - inf->dis),inf->dis);	
					data -= inf->dis;
					inf->count += inf->dis;
				}
				else{
					PNG_MEMCPY((void *)(scan->write + inf->count),(void *)(scan->write - inf->dis),data);
					data = 0;
				}
			}
			scan->write += inf->len;
			scan->count += inf->len;
			table = inf->sym_table.code;
			inf->count = 0;
			inf->status = INF_LITER_CODE;
			goto INF_DEC_BEGIN;
		case INF_NO_COM_LEN:
		case INF_NO_COM:
			//to be added later
		default:
			PNG_ASSERT(PNG_ERR_INF_STATUS);
			break;
	}
	if(!(core->flag & PNG_INFO_CMF))
		inf->scan.finish = inf->pars.BFINAL;
	BIT_SAVE;
	PNG_EXIT;
	return PNG_TRUE;
		
INF_DEC_FAIL:
	png_core_set_error(PNG_ERR_INF);
	return PNG_FALSE;
}

