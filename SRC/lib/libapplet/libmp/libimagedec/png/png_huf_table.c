/****************************************************************************(I)(S)
 *  (C)
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2008 Copyright (C)
 *  (C)
 *  File: png_huf_table.c
 *  (I)
 *  Description: create the huffman table for the inflate decompress
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

#if 0
PNG_INLINE UINT8 *GET_TMP_BUF(struct png_cfg *cfg,UINT32 *size)
{
	*size = (*size + 3) & ~0x3;
	if(cfg->dec_buf_size > *size){
		cfg->dec_buf_size -= *size;
		return (cfg->dec_buf_start + cfg->dec_buf_size);
	}
	return NULL;
}

PNG_INLINE void *FREE_TMP_BUF(struct png_cfg *cfg,UINT32 size)
{
	cfg->dec_buf_size += size;
}
#endif

#if 1
PNG_INLINE void SWAP16(UINT16 *p)
{
	UINT16 tmp = 0;
	
	tmp = *(p + 1);
	*(p + 1) = *p;
	*p = tmp;
}

PNG_INLINE void SWAP8(UINT8 *p)
{
	UINT8 tmp = 0;
	
	tmp = *(p + 1);
	*(p + 1) = *p;
	*p = tmp;
}
#else
PNG_INLINE void SWAP16(UINT16 *p,INT32 i,INT32 j)
{
	UINT16 tmp = 0;
	
	tmp = *(p + i);
	*(p + i) = *(p + j);
	*(p + j) = tmp;
}

PNG_INLINE void SWAP8(UINT8 *p,INT32 i,INT32 j)
{
	UINT8 tmp = 0;
	
	tmp = *(p + i);
	*(p + i) = *(p + j);
	*(p + j) = tmp;
}
#endif

#if 1
static void sort_len(UINT8 *code_len,INT32 num,UINT16 *symbol)
{
	INT32 idx = 0,idx2 = 0;
	UINT16 *sym = NULL;
	UINT8 *len = NULL;
	UINT8 swap = 0;

	if(num <= 1) return;
	
	for(idx = 1;idx < num;idx++)
	{
		sym = symbol;
		len = code_len;
		for(idx2 = num;idx2 > idx;idx2--)
		{
			if(*(len) > *(len + 1))
				swap = 1;
			else if((*len == *(len + 1)) && (*sym > *(sym + 1)))
				swap = 1;
			else
				swap = 0;
			if(swap){
				SWAP8(len);
				SWAP16(sym);
			}
			len++;sym++;
		}
	}
}
#else
static void sort_len(UINT8 *code_len,INT32 num,UINT16 *symbol)
{
	//quick sort algorithm
	INT32 idx1 = 1, idx2 = num - 1,idx = 0;
	UINT8 flag = 0;

	if(num <= 1)		return;

	//choose the middle index as the flag of quick sorting
	idx = (num - 1)>>1;
	flag = 0xFF & *(code_len + idx);
	if(0 != idx){
		SWAP8(code_len,0,idx);
		SWAP16(symbol,0,idx);
	}
	do{
		for(;idx2 > 0;idx2--){
			if(*(code_len + idx2) < flag){
				break;
			}
		}
		for(;idx1 < num - 1;idx1++){
			if(*(code_len + idx1) > flag){
				break;			
			}
		}
		if(idx1 >= idx2){
			break;
		}else{
			SWAP8(code_len,idx1,idx2);
			SWAP16(symbol,idx1,idx2);
			idx1++;idx2--;
		}
	}while(1);
	if(idx2 > 0){
		SWAP8(code_len,0,idx2);
		SWAP16(symbol,0,idx2);
	}
	idx = idx2;
	sort_len(code_len,idx,symbol);
	sort_len(code_len + idx + 1,num - idx - 1,symbol + idx + 1);
}
#endif
static INT16 code_len_sym[] = {16,17,18,0,8,7,9,6,10,5,11,4,12,3,13,2,14,1,15};

static UINT16 code_len_base[] = {3,3,11};
static UINT8 code_len_ebits[] = {2,3,7};

static UINT16 len_base[] = {3,4,5,6,7,8,9,10,11,13,15,17,19,23,27,31,35,43,51,59,67,83,99,115,131
	,163,195,227,258};

static UINT8 len_extra_bits[] = {0,0,0,0,0,0,0,0,1,1,1,1,2,2,2,2,3,3,3,3,4,4,4,4,5,5,5,5,0};

static UINT16 dis_base[] = {1,2,3,4,5,7,9,13,17,25,33,49,65,97,129,193,257,385,513,769,1025
	,1537,2049,3073,4097,6145,8193,12289,16385,24577};

static UINT8 dis_extra_bits[] = {0,0,0,0,1,1,2,2,3,3,4,4,5,5,6,6,7,7,8,8,9,9,10,10,11,11,12,12,13,13};

static UINT32 *huf_code = NULL;	// huffman code buffer
static UINT32 *len_count  = NULL;	// code num of each code lenght
static UINT32 *tree = NULL;			// pointer to the huffman tree
static INT16 *symbol = NULL;		// symbol buffer

static UINT32 code = 0;			// output code in the table
static UINT32 huf = 0; 				// huffman code
static UINT32 first_code = 0;		// first code with valid code lenght
static UINT32 idx = 0;				// count of the code index
static UINT32 inc = 0;				// temporal increase value
static UINT32 mask = 0;			// mask value 
static UINT32 sub_tree_size = 0;	// needed sub tree size
static UINT32 top = 0;				// top address of the root tree
static UINT32 entry = 0;			// entry to the tree
static UINT16 root_entry = 0;		// entry in the root tree for the sub tree
static UINT8 len_cnt = 0;			// count of the code lenght
static UINT8 min = 0; 		 		// mininum bit length
static UINT8 drop = 0;				// drop bits when enter the sub tree
static UINT8 tree_len = 0;			// bit lenght of the current tree

static INT16 SYM = 0;				// symbol value

static INT16 ext_min = 0;			// mininum of the extra code

static UINT16 *base = NULL;		// base value buffer
static UINT8 *ext_bits = NULL;		// extra bits buffer
	
PNG_RET png_huf_table(struct png_cfg *cfg,UINT8 type,UINT8 *len,UINT32 num,UINT32 **table,UINT8 bits)
{
	UINT32 size = 0;
	UINT32 work_size = 0;
	UINT32 i = 0;
	int j = 0;

	osal_task_dispatch_off();// here exist some global variables. this routine can be re-entered
	
	//get the work buffer for symbol&len_count&huf_code
	size = num<<1;
	symbol = (INT16 *)PNG_MALLOC(cfg,size);
	work_size += size;
	size = num<<2;
	huf_code = (UINT32 *)PNG_MALLOC(cfg,size);
	work_size += size;
	size = (PNG_MAX_BITS + 1)<<2;
	len_count = (UINT32 *)PNG_MALLOC(cfg,size);
	work_size += size;	
	if(NULL == symbol || NULL == len_count || NULL == huf_code){
		PNG_ASSERT(PNG_ERR_MALLOC);
		goto BUILD_TABLE_FAIL;
	}
	MEMSET((void *)len_count,0,size);	
	//init
	for(i = 0;i < num;i++){
		if(type == INF_HUF_TAB_CODE_LEN)
			*(symbol + i) = code_len_sym[i];
		else
			*(symbol + i) = i;
		*(huf_code + i) = 0;
		len_count[len[i]]++;
	}

	//mininum code length
	for(i = 1;i < PNG_MAX_BITS;i++){
		if(len_count[i])	
			break;
	}
	if(PNG_MAX_BITS != i)
		min = i;
	else
		goto BUILD_TABLE_FAIL;
	
	//check code length correctness and the table size	
	j = 1;
	for(i = 1;i < PNG_MAX_BITS;i++){
		j <<= 1;
		j -= len_count[i];
		if(j < 0)	goto BUILD_TABLE_FAIL;
	}
	tree = *table = (UINT32 *)PNG_MALLOC(cfg,4<<bits);
	if(NULL == *table){
		PNG_ASSERT(PNG_ERR_MALLOC);	
		goto BUILD_TABLE_FAIL;
	}
	MEMSET((void *)tree,0,4<<bits);
	//different type huffman table
	switch(type)
	{
		case INF_HUF_TAB_CODE_LEN:
			ext_min = 16;
			base = code_len_base;
			ext_bits = code_len_ebits;
			break;
		case INF_HUF_TAB_CODE:
			ext_min = 257;
			base = len_base;
			ext_bits = len_extra_bits;
			break;
		case INF_HUF_TAB_DIS:
			ext_min = 0;
			base = dis_base;
			ext_bits = dis_extra_bits;
			break;
		default:
			goto BUILD_TABLE_FAIL;
	}
	
	//build the huffman code	
	huf = 0;
	len_cnt = 0;
	sort_len(len,num,symbol);
	for(first_code = 0,i = 0;i < min;i++)
		first_code += len_count[i];
	for(idx = first_code;idx < (UINT32)num;idx++){
		*(huf_code + idx) = huf;
		len_cnt = len[idx];
		inc = 1<<(len_cnt - 1);
		while(huf&inc)
			inc >>= 1;
		if(!inc){
			huf = 0;
		}else{
			huf &= inc - 1;
			huf += inc;
		}
	}
	
	//reconstructe the huffman table
	drop = 0;
	mask = (1<<bits) - 1;
	root_entry = 0xFFFF;
	tree_len = bits;
	sub_tree_size = 1<<bits;
	for(idx = first_code;idx < (UINT32)num;idx++){
		len_cnt = len[idx];
		huf = *(huf_code + idx);
		if(len_cnt > bits && (root_entry != (huf&mask))){
			root_entry = huf&mask;
			drop = bits;
			tree += sub_tree_size;

			inc = len_cnt;
			for(i = idx + 1;i < num;i++){
				if((*(huf_code + i)&mask) != root_entry)
					break;
			}			
			inc = len[i - 1];
			inc -= bits;
			tree_len = inc;
			sub_tree_size = 1<<inc;	
			if(NULL == (UINT8 *)PNG_MALLOC(cfg,sub_tree_size<<2))
			{
				PNG_ASSERT(PNG_MALLOC);
				goto BUILD_TABLE_FAIL;
			}

			code = 0;
			code |= inc + INF_CTRL_SUB_BITS_BASE;
			code |= bits<<8;
			code |= (tree - *table)<<16;
			(*table)[root_entry] = code;
		}

		entry = huf>>drop;
		code = 0;
		SYM = *(symbol + idx);
		code |= (len_cnt - drop)<<8;
		if(SYM == 256){
			code |= INF_CTRL_BLK_END;
		}else if(SYM >= ext_min){
			SYM -= ext_min;
			code |= ext_bits[SYM];
			code |= base[SYM]<<16;
		}else{
			code |= INF_CTRL_CHAR;
			code |= SYM<<16;
		}

		inc = 1<<(len_cnt - drop);
		top = 1<<tree_len;
		do{
			top -= inc;
			tree[entry + top] = code;
		}while(0 != top);
	}

	//FREE_TMP_BUF(cfg,work_size);
	osal_task_dispatch_on();
	return PNG_TRUE;
		
BUILD_TABLE_FAIL:
	//FREE_TMP_BUF(cfg,work_size);
	osal_task_dispatch_on();
	return PNG_FALSE;
}

