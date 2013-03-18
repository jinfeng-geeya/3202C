//
// 2D IDCT
// Derived from an older version of the IJG's JPEG software.
// Downloadable from: www.ijg.org
// This module is going to be replaced with a faster (and
// uncopyrighted) version.
// I am unable to find the original file from which this code was derived.
// I have included the copyright notice included with latest IJG version of this
// module.
//

/*
 * jidctint.c
 *
 * Copyright (C) 1991-1998, Thomas G. Lane.
 * This file is part of the Independent JPEG Group's software.
 * For conditions of distribution and use, see the accompanying README file.
 *
 * This file contains a slow-but-accurate integer implementation of the
 * inverse DCT (Discrete Cosine Transform).  In the IJG code, this routine
 * must also perform dequantization of the input coefficients.
 *
 * A 2-D IDCT can be done by 1-D IDCT on each column followed by 1-D IDCT
 * on each row (or vice versa, but it's more convenient to emit a row at
 * a time).  Direct algorithms are also available, but they are much more
 * complex and seem not to be any faster when reduced to code.
 *
 * This implementation is based on an algorithm described in
 *   C. Loeffler, A. Ligtenberg and G. Moschytz, "Practical Fast 1-D DCT
 *   Algorithms with 11 Multiplications", Proc. Int'l. Conf. on Acoustics,
 *   Speech, and Signal Processing 1989 (ICASSP '89), pp. 988-991.
 * The primary algorithm described there uses 11 multiplies and 29 adds.
 * We use their alternate method with 12 multiplies and 32 adds.
 * The advantage of this method is that no data path contains more than one
 * multiplication; this allows a very simple and accurate implementation in
 * scaled fixed-point arithmetic, with a minimal number of shifts.
 */

/*----------------------------------------------------------------------------*/
typedef unsigned char  uchar;       /*  8 bits     */
typedef   signed short int16;       /* 16 bits     */
typedef   signed int   int32;       /* 32+ bits    */

/*----------------------------------------------------------------------------*/


#define CONST_BITS  13
#define PASS1_BITS  2
#define SCALEDONE ((int32) 1)
/*----------------------------------------------------------------------------*/
#define FIX_0_298631336  ((int32)  2446)        /* FIX(0.298631336) */
#define FIX_0_390180644  ((int32)  3196)        /* FIX(0.390180644) */
#define FIX_0_541196100  ((int32)  4433)        /* FIX(0.541196100) */
#define FIX_0_765366865  ((int32)  6270)        /* FIX(0.765366865) */
#define FIX_0_899976223  ((int32)  7373)        /* FIX(0.899976223) */
#define FIX_1_175875602  ((int32)  9633)        /* FIX(1.175875602) */
#define FIX_1_501321110  ((int32)  12299)       /* FIX(1.501321110) */
#define FIX_1_847759065  ((int32)  15137)       /* FIX(1.847759065) */
#define FIX_1_961570560  ((int32)  16069)       /* FIX(1.961570560) */
#define FIX_2_053119869  ((int32)  16819)       /* FIX(2.053119869) */
#define FIX_2_562915447  ((int32)  20995)       /* FIX(2.562915447) */
#define FIX_3_072711026  ((int32)  25172)       /* FIX(3.072711026) */
/*----------------------------------------------------------------------------*/
#define DESCALE(x,n)  (((x) + (SCALEDONE << ((n)-1))) >> n)
/*----------------------------------------------------------------------------*/
#define MULTIPLY(var,cnst)  ((var) * (cnst))
/*----------------------------------------------------------------------------*/

//#define clamp(i) if (i & 0xFF00) {i = (((~i) >> 15) & 0xFF);}
#define clamp(i) 

void idct4x4(int *data, int *Pdst_ptr) 
//manson: input is in *data. input is 8x8 array (the reason for not using 4x4 is 
//I don't want to waste the time reordering the ZAG table to map in 4x4. But we
//only process the top-left 4x4 block of the input 8x8 block.
//output is in *Pdst_ptr. it is in 4x4 format
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;

  //now process row IDCT
  //for (rowctr = 7; rowctr >= 0; rowctr--)
  dataptr = data;
  pOutput = Pdst_ptr;
  for (rowctr = 3; rowctr >= 0; rowctr--) //discard row 4~7
  {
    //if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
    //     dataptr[5] | dataptr[6] | dataptr[7]) == 0)
    if ((dataptr[1] | dataptr[2] | dataptr[3]) == 0) //discard sample 4~7
    {
      dcval = (dataptr[0] << PASS1_BITS);

	  //since we still output 8 sample rows, we output sample 0 2 4 6
      dataptr[0] = dataptr[2] = dataptr[4] = dataptr[6] = dcval;
	  
      dataptr += 8;       // advance pointer to next row 
      continue;
    }

    z2 = (int32) dataptr[2];

    z1 = MULTIPLY(z2, FIX_0_541196100);
    tmp2 = z1;
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = dataptr[0] << CONST_BITS;
    tmp1 = tmp0;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = 0;
    tmp1 = 0;
    tmp2 = (int32) dataptr[3];
    tmp3 = (int32) dataptr[1];

    z1 = tmp3;
    z2 = tmp2;
    z3 = tmp2;
    z4 = tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 = z1 + z3;
    tmp1 = z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

	//since we have 8x8 working buffer, the temporal row result is still in 8 sample format
    //manson: we don't have to do the DESCALE if we write in RISC assembly
	//but I don't want to do it now by myself
	dataptr[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[6] = DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[4] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);
    
    dataptr += 8;
  }

  dataptr = data;
  pOutput = Pdst_ptr;
  //now process columes IDCT
  for (rowctr = 3; rowctr >= 0; rowctr--) //only process colume 0 2 4 8, discard 1 3 5 7
  {
    if ((dataptr[8] | dataptr[16] | dataptr[24]) == 0) //sample 32 40 48 56 are 0s after row IDCT
    {
      dcval = DESCALE(dataptr[0], PASS1_BITS+3);
	  dcval += 128;
	  pOutput[0] = pOutput[4] = pOutput[8] = pOutput[12] = dcval; //manson: output is 4x4
      //dataptr++;
      dataptr = dataptr + 2; //only process colume 0 2 4 8, discard 1 3 5 7
      pOutput++;
      continue;
    }

	//manson: sample 32 40 48 56 are 0s after row IDCT
    z2 = (int32) dataptr[16];
    z3 = 0;

    z1 = MULTIPLY(z2, FIX_0_541196100);    tmp2 = z1;
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[0]) << CONST_BITS;
    tmp1 = tmp0;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = 0;
    tmp1 = 0;
    tmp2 = (int32) dataptr[24];
    tmp3 = (int32) dataptr[8];

	z1 = dataptr[8];
	z2 = dataptr[24];
	z3 = z2;
	z4 = z1;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);

    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 = z1 + z3;
    tmp1 = z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    //pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[48] = DESCALE(tmp11 - tmp2, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[16] = DESCALE(tmp12 + tmp1, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[32] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	//manson: we want to output in 4x4 format, so index*8 become index*4
    pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[12] = DESCALE(tmp11 - tmp2, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[4] = DESCALE(tmp12 + tmp1, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[8] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	//dataptr++;
	dataptr = dataptr + 2;//only process colume 0 2 4 8, discard 1 3 5 7
    pOutput++;
  }
}

void idct2x2(int *data, int *Pdst_ptr) 
//manson: input is in *data. input is 8x8 array (the reason for not using 4x4 is 
//I don't want to waste the time reordering the ZAG table to map in 4x4. But we
//only process the top-left 2x2 block of the input 8x8 block.
//output is in *Pdst_ptr. it is in 4x4 format
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;

  //now process row IDCT
  //for (rowctr = 7; rowctr >= 0; rowctr--)
  dataptr = data;
  pOutput = Pdst_ptr;
  for (rowctr = 1; rowctr >= 0; rowctr--) //discard row 4~7
  {
	//Don't treat the high frequency zero as special case because we only process the low frequency 2x2
    tmp10 = ((int32) dataptr[0]) << CONST_BITS;
    tmp13 = ((int32) dataptr[0]) << CONST_BITS;

    tmp3 = (int32) dataptr[1];

    z1 = (int32) dataptr[1];
    z4 = (int32) dataptr[1];
    z5 = MULTIPLY(z4, FIX_1_175875602);

    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 = z5;
    z4 += z5;

    tmp0 = z1+z3; 
    tmp3 += z1 + z4;

	//since we have 8x8 working buffer, the temporal row result is still in 8 sample format
    dataptr[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[4] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += 8;
  }

  dataptr = data;
  pOutput = Pdst_ptr;
  //now process columes IDCT
  for (rowctr = 1; rowctr >= 0; rowctr--) //only process colume 0 2 4 8, discard 1 3 5 7
  {

	//Don't treat the high frequency zero as special case because we only process the low frequency 2x2
	//manson: sample 16 24 32 40 48 56 are 0s after row IDCT
    tmp10 = ((int32) dataptr[0]) << CONST_BITS;
    tmp13 = ((int32) dataptr[0]) << CONST_BITS;

    tmp3 = (int32) dataptr[8];

	z1 = dataptr[8];
	z4 = z1;
    z5 = MULTIPLY(z4, FIX_1_175875602);

    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);

    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 = z5;
    z4 += z5;

    tmp0 = z1+z3;
    tmp3 += z1 + z4;

    //pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[32] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	//manson: we want to output in 2x2 format, so index*8 become index*2
    pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[2] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	//dataptr++;
	dataptr = dataptr + 4;//only process colume 0 4, discard 1 2 3 5 6 7
    pOutput++;
  }
}

void idct1x1(int *data, int *Pdst_ptr) 
{
	*Pdst_ptr = (*data>>3) + 128;
}

void idct8x4(int *data, int *Pdst_ptr) 
//manson: for CbCr in 4:2:2 format, only decimate by 2 vertically. 
//copy row process from idct(), copy column process from idct4x4
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;

  dataptr = data;
  pOutput = Pdst_ptr;
  for (rowctr = 3; rowctr >= 0; rowctr--)
  {
    if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
         dataptr[5] | dataptr[6] | dataptr[7]) == 0)
    {
      dcval = (dataptr[0] << PASS1_BITS);

      dataptr[0] = dataptr[1] = dataptr[2] = dataptr[3] = \
	  dataptr[4] = dataptr[5] = dataptr[6] = dataptr[7] = dcval;
	  
      dataptr += 8;       // advance pointer to next row 
      continue;
    }

    z2 = (int32) dataptr[2];
    z3 = (int32) dataptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[0] + (int32) dataptr[4]) << CONST_BITS;
    tmp1 = ((int32) dataptr[0] - (int32) dataptr[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (int32) dataptr[7];
    tmp1 = (int32) dataptr[5];
    tmp2 = (int32) dataptr[3];
    tmp3 = (int32) dataptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    dataptr[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[7] = DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    dataptr[1] = DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    dataptr[6] = DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[5] = DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    dataptr[3] = DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    dataptr[4] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += 8;
  }

  dataptr = data;
  pOutput = Pdst_ptr;
  //now process columes IDCT
  for (rowctr = 7; rowctr >= 0; rowctr--) //only process colume 0 2 4 8, discard 1 3 5 7
  {
    if ((dataptr[8] | dataptr[16] | dataptr[24]) == 0) //sample 32 40 48 56 are 0s after row IDCT
    {
      dcval = DESCALE(dataptr[0], PASS1_BITS+3);
	  dcval += 128;
	  pOutput[0] = pOutput[8] = pOutput[16] = pOutput[24] = dcval; //manson: output is 4x4
      dataptr++;
      pOutput++;
      continue;
    }

	//manson: sample 32 40 48 56 are 0s after row IDCT
    z2 = (int32) dataptr[16];
    z3 = 0;

    z1 = MULTIPLY(z2, FIX_0_541196100);    tmp2 = z1;
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[0]) << CONST_BITS;
    tmp1 = tmp0;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = 0;
    tmp1 = 0;
    tmp2 = (int32) dataptr[24];
    tmp3 = (int32) dataptr[8];

	z1 = dataptr[8];
	z2 = dataptr[24];
	z3 = z2;
	z4 = z1;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);

    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 = z1 + z3;
    tmp1 = z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    //pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[48] = DESCALE(tmp11 - tmp2, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[16] = DESCALE(tmp12 + tmp1, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[32] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	//manson: we want to output in 8x4 format
    pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[24] = DESCALE(tmp11 - tmp2, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[8] = DESCALE(tmp12 + tmp1, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[16] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	dataptr++;
    pOutput++;
  }
}

void idct4x2(int *data, int *Pdst_ptr) 
//manson: for CbCr in 4:2:2 format, vertically decimated by 4, horizontally decimated by 2
//copy row process from idct4x4(), copy column process from idct2x2
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;

  //now process row IDCT
  dataptr = data;
  pOutput = Pdst_ptr;
  for (rowctr = 1; rowctr >= 0; rowctr--) //discard row 4~7
  {
    //if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
    //     dataptr[5] | dataptr[6] | dataptr[7]) == 0)
    if ((dataptr[1] | dataptr[2] | dataptr[3]) == 0) //discard sample 4~7
    {
      dcval = (dataptr[0] << PASS1_BITS);

	  //since we still output 8 sample rows, we output sample 0 2 4 6
      dataptr[0] = dataptr[2] = dataptr[4] = dataptr[6] = dcval;
	  
      dataptr += 8;       // advance pointer to next row 
      continue;
    }

    z2 = (int32) dataptr[2];

    z1 = MULTIPLY(z2, FIX_0_541196100);
    tmp2 = z1;
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = dataptr[0] << CONST_BITS;
    tmp1 = tmp0;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = 0;
    tmp1 = 0;
    tmp2 = (int32) dataptr[3];
    tmp3 = (int32) dataptr[1];

    z1 = tmp3;
    z2 = tmp2;
    z3 = tmp2;
    z4 = tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 = z1 + z3;
    tmp1 = z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

	//since we have 8x8 working buffer, the temporal row result is still in 8 sample format
    //manson: we don't have to do the DESCALE if we write in RISC assembly
	//but I don't want to do it now by myself
	dataptr[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[6] = DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[4] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += 8;
  }

  dataptr = data;
  pOutput = Pdst_ptr;
  //now process columes IDCT
  for (rowctr = 3; rowctr >= 0; rowctr--) //only process colume 0 2 4 8, discard 1 3 5 7
  {

	//Don't treat the high frequency zero as special case because we only process the low frequency 2x2
	//manson: sample 16 24 32 40 48 56 are 0s after row IDCT
    tmp10 = ((int32) dataptr[0]) << CONST_BITS;
    tmp13 = ((int32) dataptr[0]) << CONST_BITS;

    tmp3 = (int32) dataptr[8];

	z1 = dataptr[8];
	z4 = z1;
    z5 = MULTIPLY(z4, FIX_1_175875602);

    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);

    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 = z5;
    z4 += z5;

    tmp0 = z1+z3;
    tmp3 += z1 + z4;

    //pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[32] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	//manson: we want to output in 4x2 format
    pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[4] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	dataptr = dataptr + 2;
    pOutput++;
  }
}

void idct2x1(int *data, int *Pdst_ptr) 
//manson: for CbCr in 4:2:2 format, vertically decimated by 8, horizontally decimated by 4
//copy row process from idct2x2(), copy column process from idct1x1
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;

  //now process row IDCT
  //for (rowctr = 7; rowctr >= 0; rowctr--)
  dataptr = data;
  pOutput = Pdst_ptr;

  //Don't treat the high frequency zero as special case because we only process the low frequency 2x2
  tmp10 = dataptr[0] << CONST_BITS;
  tmp13 = dataptr[0] << CONST_BITS;

  tmp3 = dataptr[1];

  z1 = dataptr[1];
  z4 = dataptr[1];
  z5 = MULTIPLY(z4, FIX_1_175875602);

  tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
  z1 = MULTIPLY(z1, - FIX_0_899976223);
  z4 = MULTIPLY(z4, - FIX_0_390180644);

  z3 = z5;
  z4 += z5;

  tmp0 = z1+z3; 
  tmp3 += z1 + z4;

  pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
  pOutput[1] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);
  //manson: if we want bit to bit exact output with the IDCT function,
  //we have to do the shifting and rounding in two step to simulate the rounding
  pOutput[0] = DESCALE(pOutput[0], PASS1_BITS+3);
  pOutput[0] = pOutput[0]+128;
  pOutput[1] = DESCALE(pOutput[1], PASS1_BITS+3);
  pOutput[1] = pOutput[1]+128;
}

void idct8x2(int *data, int *Pdst_ptr) 
//for 4:1:1 format, vertical decimation factor is four times of horizontal decimation factor
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;

  dataptr = data;
  pOutput = Pdst_ptr;
  for (rowctr = 1; rowctr >= 0; rowctr--)
  {
    if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
         dataptr[5] | dataptr[6] | dataptr[7]) == 0)
    {
      dcval = (dataptr[0] << PASS1_BITS);

      dataptr[0] = dataptr[1] = dataptr[2] = dataptr[3] = \
	  dataptr[4] = dataptr[5] = dataptr[6] = dataptr[7] = dcval;
	  
      dataptr += 8;       // advance pointer to next row 
      continue;
    }

    z2 = (int32) dataptr[2];
    z3 = (int32) dataptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[0] + (int32) dataptr[4]) << CONST_BITS;
    tmp1 = ((int32) dataptr[0] - (int32) dataptr[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (int32) dataptr[7];
    tmp1 = (int32) dataptr[5];
    tmp2 = (int32) dataptr[3];
    tmp3 = (int32) dataptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    dataptr[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[7] = DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    dataptr[1] = DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    dataptr[6] = DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[5] = DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    dataptr[3] = DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    dataptr[4] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += 8;
  }

  dataptr = data;
  pOutput = Pdst_ptr;
  //now process columes IDCT
  for (rowctr = 7; rowctr >= 0; rowctr--) //only process colume 0 2 4 8, discard 1 3 5 7
  {

	//Don't treat the high frequency zero as special case because we only process the low frequency 2x2
	//manson: sample 16 24 32 40 48 56 are 0s after row IDCT
    tmp10 = ((int32) dataptr[0]) << CONST_BITS;
    tmp13 = ((int32) dataptr[0]) << CONST_BITS;

    tmp3 = (int32) dataptr[8];

	z1 = dataptr[8];
	z4 = z1;
    z5 = MULTIPLY(z4, FIX_1_175875602);

    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);

    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 = z5;
    z4 += z5;

    tmp0 = z1+z3;
    tmp3 += z1 + z4;

    //pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    //pOutput[32] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	//manson: we want to output in 8x2 format
    pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    pOutput[8] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;

	dataptr = dataptr + 1;
    pOutput++;
  }
}

void idct4x1(int *data, int *Pdst_ptr) 
//for 4:1:1 format, vertical decimation factor is four times of horizontal decimation factor
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;

  //now process row IDCT
  dataptr = data;
  pOutput = Pdst_ptr;
  if ((dataptr[1] | dataptr[2] | dataptr[3]) == 0) //discard sample 4~7
  {
      dcval = (dataptr[0] << PASS1_BITS);
      pOutput[0] = pOutput[1] = pOutput[2] = pOutput[3] = dcval;
  }

  z2 = (int32) dataptr[2];

  z1 = MULTIPLY(z2, FIX_0_541196100);
  tmp2 = z1;
  tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = dataptr[0] << CONST_BITS;
    tmp1 = tmp0;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = 0;
    tmp1 = 0;
    tmp2 = (int32) dataptr[3];
    tmp3 = (int32) dataptr[1];

    z1 = tmp3;
    z2 = tmp2;
    z3 = tmp2;
    z4 = tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 = z1 + z3;
    tmp1 = z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

	//since we have 8x8 working buffer, the temporal row result is still in 8 sample format
    //manson: we don't have to do the DESCALE if we write in RISC assembly
	//but I don't want to do it now by myself
	pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    pOutput[3] = DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    pOutput[1] = DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    pOutput[2] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

	//manson: if we want bit to bit exact output with the IDCT function,
	//we have to do the shifting and rounding in two step to simulate the rounding
	pOutput[0] = DESCALE(pOutput[0], PASS1_BITS+3);
	pOutput[0] = pOutput[0]+128;
	pOutput[1] = DESCALE(pOutput[1], PASS1_BITS+3);
	pOutput[1] = pOutput[1]+128;
	pOutput[2] = DESCALE(pOutput[2], PASS1_BITS+3);
	pOutput[2] = pOutput[2]+128;
	pOutput[3] = DESCALE(pOutput[3], PASS1_BITS+3);
	pOutput[3] = pOutput[3]+128;
}

void idct(int *data, int *Pdst_ptr) //bugfix001 use 32 bits for IDCT buffer
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int dcval;
  int *dataptr; //bugfix001 use 32 bits for IDCT buffer
  int *pOutput;
  int rowctr;
  int i,j;
  /*
	//force high frequency zero to simulate 4x4 IDCT
	dataptr = data;
	for(i=0;i<8;i++){
		for(j=0;j<8;j++){
			//if((i>=2)||(j>=2)) //2x2
			//if((i>=4)||(j>=4)) //4x4
			//if(i>=4)  //8x4
			//if((i>=2)||(j>=4)) //4x2
			//if((i>=1)||(j>=2)) //2x1
			//if(i>=2) //8x2
			if((i>=1)||(j>=4)) //4x1
				*dataptr = 0; 
			dataptr++;
		}
	}
*/
  dataptr = data;
  pOutput = Pdst_ptr;
  for (rowctr = 7; rowctr >= 0; rowctr--)
  {
    if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
         dataptr[5] | dataptr[6] | dataptr[7]) == 0)
    {
      dcval = (dataptr[0] << PASS1_BITS);

      dataptr[0] = dataptr[1] = dataptr[2] = dataptr[3] = \
	  dataptr[4] = dataptr[5] = dataptr[6] = dataptr[7] = dcval;
	  
      dataptr += 8;       // advance pointer to next row 
      continue;
    }

    z2 = (int32) dataptr[2];
    z3 = (int32) dataptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[0] + (int32) dataptr[4]) << CONST_BITS;
    tmp1 = ((int32) dataptr[0] - (int32) dataptr[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (int32) dataptr[7];
    tmp1 = (int32) dataptr[5];
    tmp2 = (int32) dataptr[3];
    tmp3 = (int32) dataptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    dataptr[0] = DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[7] = DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    dataptr[1] = DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    dataptr[6] = DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[5] = DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    dataptr[3] = DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    dataptr[4] = DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

//	libc_printf("dataptr:%x\n",dataptr);SDBBP();

    dataptr += 8;
  }

  dataptr = data;
  pOutput = Pdst_ptr;
  for (rowctr = 8-1; rowctr >= 0; rowctr--)
  {
    if ((dataptr[8] | dataptr[16] | dataptr[24] |
         dataptr[32] | dataptr[40] | dataptr[48] |
         dataptr[56]) == 0)
    {
      dcval = DESCALE((int32) dataptr[0], PASS1_BITS+3);
	  //manson: we will do clamp1ing at the last output 
	  dcval += 128;
	  pOutput[0] = pOutput[8] = pOutput[16] = pOutput[24] = \
	  pOutput[32] = pOutput[40] = pOutput[48] = pOutput[56] = dcval; //manson: keep 16 bits
//	   libc_printf("dcval:%x\n",dcval);
      dataptr++;
      pOutput++;
      continue;
    }

    z2 = (int32) dataptr[16];
    z3 = (int32) dataptr[48];


    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[0] + (int32) dataptr[32]) << CONST_BITS;
    tmp1 = ((int32) dataptr[0] - (int32) dataptr[32]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (int32) dataptr[56];
    tmp1 = (int32) dataptr[40];
    tmp2 = (int32) dataptr[24];
    tmp3 = (int32) dataptr[8];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    pOutput[0] = DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[0]);
    pOutput[56] = DESCALE(tmp10 - tmp3, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[56]);
    pOutput[8] = DESCALE(tmp11 + tmp2, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[8]);
    pOutput[48] = DESCALE(tmp11 - tmp2, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[48]);
    pOutput[16] = DESCALE(tmp12 + tmp1, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[16]);
    pOutput[40] = DESCALE(tmp12 - tmp1, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[40]);
    pOutput[24] = DESCALE(tmp13 + tmp0, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[24]);
    pOutput[32] = DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;clamp(pOutput[32]);

/*	libc_printf("pOutput[0]:%x\n",pOutput[0]);
	libc_printf("pOutput[56]:%x\n",pOutput[56]);
	libc_printf("pOutput[8]:%x\n",pOutput[8]);
	libc_printf("pOutput[48]:%x\n",pOutput[48]);
	libc_printf("pOutput[16]:%x\n",pOutput[16]);
	libc_printf("pOutput[40]:%x\n",pOutput[40]);
	libc_printf("pOutput[24]:%x\n",pOutput[24]);
	libc_printf("pOutput[32]:%x\n",pOutput[32]);*/
	dataptr++;
    pOutput++;
  }
/*
  //drop to simulate decimated IDCT
  //pOutput = Pdst_ptr;
  //for(i=0;i<2;i++){
 	//for(j=0;j<2;j++){ //2x2
		//*(Pdst_ptr+i*2+j) = *(Pdst_ptr+(i*4)*8+j*4);
  //for(i=0;i<4;i++){
	  //for(j=0;j<4;j++){ //4x4
		//*(Pdst_ptr+i*4+j) = *(Pdst_ptr+(i*2)*8+j*2);
  //for(i=0;i<4;i++){
	  //for(j=0;j<8;j++){  //8x4
		//*(Pdst_ptr+i*8+j) = *(Pdst_ptr+(i*2)*8+j);
  //for(i=0;i<2;i++){
	  //for(j=0;j<4;j++){ //4x2
		//*(Pdst_ptr+i*4+j) = *(Pdst_ptr+(i*4)*8+j*2);
  //for(j=0;j<2;j++){ //2x1
  		//*(Pdst_ptr+j) = *(Pdst_ptr+j*4);
  //for(i=0;i<2;i++){
	  //for(j=0;j<8;j++){  //8x2
		//*(Pdst_ptr+i*8+j) = *(Pdst_ptr+(i*4)*8+j);
  //for(j=0;j<4;j++){ //4x1
	//*(Pdst_ptr+j) = *(Pdst_ptr+j*2);
  //}
*/
}

/*
void idct(int16 *data, uchar *Pdst_ptr)
{
  int32 tmp0, tmp1, tmp2, tmp3;
  int32 tmp10, tmp11, tmp12, tmp13;
  int32 z1, z2, z3, z4, z5;
  int16 *dataptr;
  int rowctr;

  //Just for test
//  MEMSET(Pdst_ptr, 0x77, 64);
//  return;
  //
  dataptr = data;
  for (rowctr = 8-1; rowctr >= 0; rowctr--)
  {
    if ((dataptr[1] | dataptr[2] | dataptr[3] | dataptr[4] |
         dataptr[5] | dataptr[6] | dataptr[7]) == 0)
    {
      int16 dcval = (int16) (dataptr[0] << PASS1_BITS);

      dataptr[0] = dcval;
      dataptr[1] = dcval;
      dataptr[2] = dcval;
      dataptr[3] = dcval;
      dataptr[4] = dcval;
      dataptr[5] = dcval;
      dataptr[6] = dcval;
      dataptr[7] = dcval;

      dataptr += 8;       // advance pointer to next row 
      continue;
    }

    z2 = (int32) dataptr[2];
    z3 = (int32) dataptr[6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[0] + (int32) dataptr[4]) << CONST_BITS;
    tmp1 = ((int32) dataptr[0] - (int32) dataptr[4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (int32) dataptr[7];
    tmp1 = (int32) dataptr[5];
    tmp2 = (int32) dataptr[3];
    tmp3 = (int32) dataptr[1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    dataptr[0] = (int16) DESCALE(tmp10 + tmp3, CONST_BITS-PASS1_BITS);
    dataptr[7] = (int16) DESCALE(tmp10 - tmp3, CONST_BITS-PASS1_BITS);
    dataptr[1] = (int16) DESCALE(tmp11 + tmp2, CONST_BITS-PASS1_BITS);
    dataptr[6] = (int16) DESCALE(tmp11 - tmp2, CONST_BITS-PASS1_BITS);
    dataptr[2] = (int16) DESCALE(tmp12 + tmp1, CONST_BITS-PASS1_BITS);
    dataptr[5] = (int16) DESCALE(tmp12 - tmp1, CONST_BITS-PASS1_BITS);
    dataptr[3] = (int16) DESCALE(tmp13 + tmp0, CONST_BITS-PASS1_BITS);
    dataptr[4] = (int16) DESCALE(tmp13 - tmp0, CONST_BITS-PASS1_BITS);

    dataptr += 8;
  }

  dataptr = data;
  for (rowctr = 8-1; rowctr >= 0; rowctr--)
  {
    int16 i;

    if ((dataptr[8*1] | dataptr[8*2] | dataptr[8*3] |
         dataptr[8*4] | dataptr[8*5] | dataptr[8*6] |
         dataptr[8*7]) == 0)
    {
      int16 dcval = (int16) DESCALE((int32) dataptr[0], PASS1_BITS+3);

      if ((dcval += 128) < 0)
        dcval = 0;
      else if (dcval > 255)
        dcval = 255;

      Pdst_ptr[8*0] = (uchar)dcval;
      Pdst_ptr[8*1] = (uchar)dcval;
      Pdst_ptr[8*2] = (uchar)dcval;
      Pdst_ptr[8*3] = (uchar)dcval;
      Pdst_ptr[8*4] = (uchar)dcval;
      Pdst_ptr[8*5] = (uchar)dcval;
      Pdst_ptr[8*6] = (uchar)dcval;
      Pdst_ptr[8*7] = (uchar)dcval;

      dataptr++;
      Pdst_ptr++;
      continue;
    }

    z2 = (int32) dataptr[8*2];
    z3 = (int32) dataptr[8*6];

    z1 = MULTIPLY(z2 + z3, FIX_0_541196100);
    tmp2 = z1 + MULTIPLY(z3, - FIX_1_847759065);
    tmp3 = z1 + MULTIPLY(z2, FIX_0_765366865);

    tmp0 = ((int32) dataptr[8*0] + (int32) dataptr[8*4]) << CONST_BITS;
    tmp1 = ((int32) dataptr[8*0] - (int32) dataptr[8*4]) << CONST_BITS;

    tmp10 = tmp0 + tmp3;
    tmp13 = tmp0 - tmp3;
    tmp11 = tmp1 + tmp2;
    tmp12 = tmp1 - tmp2;

    tmp0 = (int32) dataptr[8*7];
    tmp1 = (int32) dataptr[8*5];
    tmp2 = (int32) dataptr[8*3];
    tmp3 = (int32) dataptr[8*1];

    z1 = tmp0 + tmp3;
    z2 = tmp1 + tmp2;
    z3 = tmp0 + tmp2;
    z4 = tmp1 + tmp3;
    z5 = MULTIPLY(z3 + z4, FIX_1_175875602);

    tmp0 = MULTIPLY(tmp0, FIX_0_298631336);
    tmp1 = MULTIPLY(tmp1, FIX_2_053119869);
    tmp2 = MULTIPLY(tmp2, FIX_3_072711026);
    tmp3 = MULTIPLY(tmp3, FIX_1_501321110);
    z1 = MULTIPLY(z1, - FIX_0_899976223);
    z2 = MULTIPLY(z2, - FIX_2_562915447);
    z3 = MULTIPLY(z3, - FIX_1_961570560);
    z4 = MULTIPLY(z4, - FIX_0_390180644);

    z3 += z5;
    z4 += z5;

    tmp0 += z1 + z3;
    tmp1 += z2 + z4;
    tmp2 += z2 + z3;
    tmp3 += z1 + z4;

    i = (int16) DESCALE(tmp10 + tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*0] = (uchar)i;

    i = (int16) DESCALE(tmp10 - tmp3, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*7] = (uchar)i;

    i = (int16) DESCALE(tmp11 + tmp2, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*1] = (uchar)i;

    i = (int16) DESCALE(tmp11 - tmp2, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*6] = (uchar)i;

    i = (int16) DESCALE(tmp12 + tmp1, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*2] = (uchar)i;

    i = (int16) DESCALE(tmp12 - tmp1, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*5] = (uchar)i;

    i = (int16) DESCALE(tmp13 + tmp0, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*3] = (uchar)i;

    i = (int16) DESCALE(tmp13 - tmp0, CONST_BITS+PASS1_BITS+3) + 128;
    clamp1(i)
    Pdst_ptr[8*4] = (uchar)i;

    dataptr++;
    Pdst_ptr++;
  }
}
*/