/******************************************************************************
 * File name: jpegdecoder.inl
 * Purpose  : inlines-- moved from .h file for clarity.
 * Author   : Peter	Luo,	peter@t2-design.com
 * Last updated: 10/JUNE/2003
 * 
 * Copyright (c) 2003 T-Square
 * All rights reserved.
 * 
 * No permission to use, copy, modify, and distribute this software and its
 * documentation for any purpose, without authorization by T-square, and 
 * without written agreement is hereby granted, provided that the above 
 * copyright notice and the following two paragraphs appear in all copies 
 * of this software.
 * 
 * In no event shall the author be liable to any party for direct, indirect, 
 * special, incidental, or consequential damages arising out of the use of 
 * this software and its documentation, even if the author has been advised 
 * of the possibility of such damage.
 *****************************************************************************/
//---- two functions moved from jpegdecoder.c to avoid the bit union redefinition problem 
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// Tables and macro used to fully decode the DPCM differences.
// (Note: In x86 asm this can be done without using tables.)
static int extend_test[16] =   /* entry n is 2**(n-1) */
  { 0, 0x0001, 0x0002, 0x0004, 0x0008, 0x0010, 0x0020, 0x0040, 0x0080,
    0x0100, 0x0200, 0x0400, 0x0800, 0x1000, 0x2000, 0x4000 };

static int extend_offset[16] = /* entry n is (-1 << n) + 1 */
  { 0, ((-1)<<1) + 1, ((-1)<<2) + 1, ((-1)<<3) + 1, ((-1)<<4) + 1,
    ((-1)<<5) + 1, ((-1)<<6) + 1, ((-1)<<7) + 1, ((-1)<<8) + 1,
    ((-1)<<9) + 1, ((-1)<<10) + 1, ((-1)<<11) + 1, ((-1)<<12) + 1,
    ((-1)<<13) + 1, ((-1)<<14) + 1, ((-1)<<15) + 1 };

/* // not used, so note it. Peter luo. 2003-6-10.
// used by huff_extend()
static int extend_mask[] =
{
  0,
  (1<<0), (1<<1), (1<<2), (1<<3),
  (1<<4), (1<<5), (1<<6), (1<<7),
  (1<<8), (1<<9), (1<<10), (1<<11),
  (1<<12), (1<<13), (1<<14), (1<<15),
  (1<<16),
};
*/
#define HUFF_EXTEND_TBL(x,s) ((x) < extend_test[s] ? (x) + extend_offset[s] : (x))

#define HUFF_EXTEND(x,s) HUFF_EXTEND_TBL(x,s)
#define HUFF_EXTEND_P(x,s) HUFF_EXTEND_TBL(x,s)
//------------------------------------------------------------------------------
#define rol(i,j)	((i << j) | (i >> (32 - j)))
// Inserts a previously retrieved character back into the input buffer.
static void stuff_char(uchar q)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	
	*(--phdl->in_stream.Pin_buf_ofs) = q;
	phdl->in_stream.in_buf_left++;
}

// Clamps a value between 0-255.
inline uchar clamp(int i)
{
  if (i & 0xFFFFFF00)
    i = (((~i) >> 31) & 0xFF);

  return (i);
}
//------------------------------------------------------------------------------
// Retrieve one character from the input stream.
static inline uint get_char(void)
{
	int t;
	uint c;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	
	// Any bytes remaining in buffer?
	if (!phdl->in_stream.in_buf_left)
	{
		// Try to get more bytes.
		prep_in_buffer();
		// Still nothing to get?
		if (!phdl->in_stream.in_buf_left)
		{
		// Padd the end of the stream with 0xFF 0xD9 (EOI marker)
		// FIXME: Is there a better padding pattern to use?
		t = tem_flag;
		tem_flag ^= 1;
		if (t)
			return (0xD9);
		else
			return (0xFF);
		}
	}
	c = *phdl->in_stream.Pin_buf_ofs++;
	phdl->in_stream.in_buf_left--;
	return (c);
}

#if 0
//------------------------------------------------------------------------------
// Same as previus method, except can indicate if the character is
// a "padd" character or not.
inline uint get_char_a(bool *Ppadding_flag)
{
	int t;
	uint c;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

  if (!phdl->in_stream.in_buf_left)
  {
    prep_in_buffer();
    if (!phdl->in_stream.in_buf_left)
    {
      *Ppadding_flag = true;
      t = tem_flag;
      tem_flag ^= 1;
      if (t)
        return (0xD9);
      else
        return (0xFF);
    }
  }

  *Ppadding_flag = false;

  c = *phdl->in_stream.Pin_buf_ofs++;
  phdl->in_stream.in_buf_left--;

  return (c);
}
//------------------------------------------------------------------------------

//------------------------------------------------------------------------------
// Retrieves one character from the input stream, but does
// not read past markers. Will continue to return 0xFF when a
// marker is encountered.
// FIXME: Bad name?
inline uchar get_octet(void)
{
  bool padding_flag;
  int c = get_char_a(&padding_flag);

  if (c == 0xFF)
  {
    if (padding_flag)
      return (0xFF);

    c = get_char_a(&padding_flag);
    if (padding_flag)
    {
      stuff_char(0xFF);
      return (0xFF);
    }

    if (c == 0x00)
      return (0xFF);
    else
    {
      stuff_char(c);
      stuff_char(0xFF);
      return (0xFF);
    }
  }

  return (c);
}
#endif
//manson: I remove padding and move two functions into one
//I assume there is no reason that the decoder still ask for data when end of
//the file has been reached. 
static inline uchar get_octet2(void) //change005
{
	uchar c;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	
	//	unsigned long ra = GET_RA();
	/*
	//bool padding_flag; //change005
	if (!in_buf_left)
	prep_in_buffer();
	c = *Pin_buf_ofs++;
	in_buf_left--;
	if (c == 0xFF){
	if (!in_buf_left)
	prep_in_buffer();
	c = *Pin_buf_ofs++;
	in_buf_left--;
	if (c == 0x00)
	return (0xFF);
	else{
	stuff_char(c);
	stuff_char(0xFF);
	return (0xFF);
	}
	}
	*/
	if (!phdl->in_stream.in_buf_left)
	{
		prep_in_buffer();
		if (!phdl->in_stream.in_buf_left)
		{
			// Padd the end of the stream with 0xFF
			// FIXME: Is there a better padding pattern to use?
			return 0xFF;
		}
	}
	c = *phdl->in_stream.Pin_buf_ofs++;
	phdl->in_stream.in_buf_left--;
	if (c == 0xFF)
	{
		if (!phdl->in_stream.in_buf_left)
		{
			prep_in_buffer();
			if (!phdl->in_stream.in_buf_left)
				return 0xFF;
		}
		c = *phdl->in_stream.Pin_buf_ofs++;
		phdl->in_stream.in_buf_left--;
		if (c == 0x00)
			return (0xFF);
		else{
			//stuff_char(c);
			*(--phdl->in_stream.Pin_buf_ofs) = c;
			phdl->in_stream.in_buf_left++;
			// stuff_char(0xFF);
			*(--phdl->in_stream.Pin_buf_ofs) = 0xFF;
			phdl->in_stream.in_buf_left++;
			return (0xFF);
		}
	}
	return (c);
}

//------------------------------------------------------------------------------
// Retrieves a variable number of bits from the input stream.
// Does not recognize markers.
uint get_bits_1(int num_bits)
{
	uint i;
	uint c1;
	uint c2;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	i = (phdl->decoder.bit.bit_buf >> (16 - num_bits)) & ((1 << num_bits) - 1);

	if ((phdl->decoder.bits_left -= num_bits) <= 0)
	{
		//    bit.bit_buf = rol(bit.bit_buf, num_bits += bits_left);
		num_bits = num_bits + phdl->decoder.bits_left; // ...+= (negative num)... is not always right in X6304, so... Peter luo, 6-13.
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, num_bits);
		c1 = get_char();
		c2 = get_char();
		phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf & 0xFFFF) | (((ulong)c1) << 24) | (((ulong)c2) << 16);
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, -phdl->decoder.bits_left);
		phdl->decoder.bits_left += 16;
	}
	else
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, num_bits);

	return i;
}

/*add by sam for bmp decoder. num_bits <= 16*/
uint get_bits_1_lit_end(int num_bits)
{
	uint i;
	uint c1;
	uint c2;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	//  i = (phdl->decoder.bit.bit_buf >> (16 - num_bits)) & ((1 << num_bits) - 1);
	i = (phdl->decoder.bit.bit_buf)&((1<<num_bits) - 1);
	if ((phdl->decoder.bits_left -= num_bits) <= 0)
	{
		//    bit.bit_buf = rol(bit.bit_buf, num_bits += bits_left);
		num_bits = num_bits + phdl->decoder.bits_left; // ...+= (negative num)... is not always right in X6304, so... Peter luo, 6-13.
		phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf)>>num_bits;//rol(phdl->decoder.bit.bit_buf, num_bits);
		c1 = get_char();
		c2 = get_char();
		phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf & 0xFFFF) | (((ulong)c1) << 16) | (((ulong)c2) << 24);
		phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf)>>(-phdl->decoder.bits_left);//rol(phdl->decoder.bit.bit_buf, -phdl->decoder.bits_left);
		phdl->decoder.bits_left += 16;
	}
	else
		phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf)>>num_bits;//rol(phdl->decoder.bit.bit_buf, num_bits);

	return i;
}
//------------------------------------------------------------------------------

#ifndef JPEG_NEW_HUF_TABLE
inline uint get_bits_2_1bit()
{
  uint i;
  uint c1;
  uint c2;
  //int numbits = 1;
  pImagedec_hdl phdl = &g_imagedec_hdl[0];

  if((phdl->decoder.bit.bit_buf & 0x8000)==0)
	  i = 1;
  else
	  i = 0;

  phdl->decoder.bits_left --;
  if (phdl->decoder.bits_left <= 0)
  {
//    bit.bit_buf = rol(bit.bit_buf, numbits += bits_left);
	//numbits = numbits + bits_left; // ...+= (negative num)... is not always right in X6304, so... Peter luo, 6-13.
    phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, 1); 

    c1 = get_octet2(); 
    c2 = get_octet2();

    phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf & 0xFFFF) | (((ulong)c1) << 24) | (((ulong)c2) << 16);


    phdl->decoder.bits_left = 16;
  }
  else
    phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, 1);

  //return ((i << j) | (i >> (32 - j))); //cycular shift left

  return i;
}
#endif
//------------------------------------------------------------------------------
//manson: optimize for speed. In most case we don't need the return value
static inline void get_bits_2_discard(int numbits) //change005
{
	//uint i;
	uint c1;
	uint c2;
	pImagedec_hdl phdl = &g_imagedec_hdl[0];

	//i = (bit.bit_buf >> (16 - numbits)) & ((1 << numbits) - 1);
	if ((phdl->decoder.bits_left -= numbits) <= 0)
	{
		//    bit.bit_buf = rol(bit.bit_buf, numbits += bits_left);
		numbits = numbits + phdl->decoder.bits_left; // ...+= (negative num)... is not always right in X6304, so... Peter luo, 6-13.
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, numbits); 
		//c1 = get_octet();
		//c2 = get_octet();
		c1 = get_octet2();
		c2 = get_octet2();
		phdl->decoder.bit.bit_buf = (phdl->decoder.bit.bit_buf & 0xFFFF) | (((ulong)c1) << 24) | (((ulong)c2) << 16);
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, -phdl->decoder.bits_left);
		phdl->decoder.bits_left += 16;
	}
	else
		phdl->decoder.bit.bit_buf = rol(phdl->decoder.bit.bit_buf, numbits);
	//return ((i << j) | (i >> (32 - j))); //cycular shift left
	//return i;
}

// Decodes a Huffman encoded symbol.
#ifdef JPEG_NEW_HUF_TABLE
static inline int huff_decode(Phuff_tables_t ph)
{
	pImagedec_hdl phdl = &g_imagedec_hdl[0];
	UINT32 symbol = 0;
	int cnt = (phdl->decoder.bit.bit_buf>>JPEG_SHIFT_BITS)&JPEG_SHIFT_MSK;

	symbol = ph->huf_table[cnt];
	if(((UINT8)(symbol&0xFF)) > JPEG_SUB_BASE){
		get_bits_2_discard(JPEG_ROOT_BITS);
		cnt = (symbol&0xFF) - JPEG_SUB_BASE;
		cnt = (phdl->decoder.bit.bit_buf>>(16 - cnt)) & ((1<<cnt) - 1);
		cnt = cnt + (symbol>>16);
		symbol = ph->huf_table[cnt];
	}
	get_bits_2_discard(symbol&0xFF);
	symbol = (symbol>>8)&0xFF;
	return symbol;
}
#else
static inline int huff_decode(Phuff_tables_t Ph)
{
  int symbol;
  unsigned int lookup_and_codesize;
  pImagedec_hdl phdl = &g_imagedec_hdl[0];

  lookup_and_codesize = Ph->look_up_and_code_size[(phdl->decoder.bit.bit_buf >> 8) & 0xFF];
  symbol = (short)(lookup_and_codesize);

  // Check first 8-bits: do we have a complete symbol?
  //if ((symbol = Ph->look_up[(bit.bit_buf >> 8) & 0xFF]) < 0)
  if(symbol < 0)
  {
    // Decode more bits, use a tree traversal to find symbol.
    //get_bits_2(8); //discard 8 bits
	get_bits_2_discard(8); //change005
    do
    {
      //symbol = Ph->tree[~symbol + (1 - get_bits_2(1))];
      symbol = Ph->tree[~symbol + get_bits_2_1bit()]; //1- has been done in get_bits_2_1bit()
    } while (symbol < 0);
  }
  else{
    //get_bits_2(Ph->code_size[symbol]); //discard bits according to codeword size
	//get_bits_2_discard(Ph->code_size[symbol]); //change005
	get_bits_2_discard(lookup_and_codesize>>16); //change005
  }
  return symbol;
}
#endif
