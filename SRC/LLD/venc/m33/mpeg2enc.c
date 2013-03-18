/* mpeg2enc.c, main() and parameter file reading                            */

/* Copyright (C) 1996, MPEG Software Simulation Group. All Rights Reserved. */

/*
 * Disclaimer of Warranty
 *
 * These software programs are available to the user without any license fee or
 * royalty on an "as is" basis.  The MPEG Software Simulation Group disclaims
 * any and all warranties, whether express, implied, or statuary, including any
 * implied warranties or merchantability or of fitness for a particular
 * purpose.  In no event shall the copyright-holder be liable for any
 * incidental, punitive, or consequential damages of any kind whatsoever
 * arising from the use of these programs.
 *
 * This disclaimer of warranty extends to the user of these programs and user's
 * customers, employees, agents, transferees, successors, and assigns.
 *
 * The MPEG Software Simulation Group does not represent or warrant that the
 * programs furnished hereunder are free of infringement of any third-party
 * patents.
 *
 * Commercial implementations of MPEG-1 and MPEG-2 video, including shareware,
 * are subject to royalty fees to patent holders.  Many of these patents are
 * general enough such that they are unavoidable regardless of implementation
 * design.
 *
 */

#define GLOBAL

#include "global.h"


/* private prototypes */


static UINT32 GetCMBOffset(UINT16 x, UINT16 y,  UINT16 uMBWidth)
{
	UINT16 c_x, c_y, uPrevLineMB;
	
	c_x = x/2;
	c_y = y/2;
	uPrevLineMB = (c_y/16) *(2*uMBWidth);

	return (c_y%16 <8? 2*(c_x/8) : 2*(c_x/8)+1) +uPrevLineMB;
}
static void GetOffset(UINT16 x, UINT16 y,  UINT16 uMBWidth, UINT32 *puYOffset, UINT32 *puCOffset)
{
	UINT16 uCurYMB, uCurCMB;
 	UINT32 uYMBDataSize, uCMBDataSize, uYInMBOffset, uCInMBOffset;
		
	uYMBDataSize = 16*16;
	uCMBDataSize = 16*8;
 	
	uCurYMB = (y/16)*uMBWidth + x/16;
	uCurCMB = GetCMBOffset(x,y,uMBWidth);
	uYInMBOffset = (y%16)*16 + (x%16);
	uCInMBOffset = ((y%16)/2)*16 + (x%16);
	*puYOffset =  uCurYMB*uYMBDataSize + uYInMBOffset;
	*puCOffset =  uCurCMB*uCMBDataSize + uCInMBOffset;

}
#if (CAPTURE_MODE==FRAME_CAPTURE)
// change a MB format yuv buffer to Line format
// MB/Y_C mode => Line/Y_Cb_Cr mode
void format_transform(UINT8 *uSrcYAddr, UINT8 *uSrcCAddr, 
						UINT8 *uDstYAddr, UINT8 *uDstCbAddr, UINT8 *uDstCrAddr,
						UINT16 uWidth, UINT16 uHeigth)
{
	UINT32 x, y ;
	UINT16 uMBWidth ;
	UINT16 uCont, 	// number of continuous pixels
		     uFilled; 	
	
	UINT16 i;
	//UINT8 *puCurYAddr, *puCurCAddr;
	UINT32 uYOffset, uCOffset;

//soc_printf("%8x\n%8x\n%8x\n%8x\n%8x\n%d\n%d\n",uSrcYAddr,uSrcCAddr,uDstYAddr,uDstCbAddr,uDstCrAddr,uWidth,uHeigth);

	uMBWidth = (uWidth+15) /16;

	for (y=0; y<uHeigth; y++)
	{
		// fill a line each time
		uFilled= 0;
		x = 0;
		while (uFilled < uWidth)
		{
			uCont = 16- x%16;
			GetOffset(x, y,  uMBWidth, &uYOffset, &uCOffset); 
			for (i=0; i<uCont; i++)	 	// Y
				 *(uDstYAddr+y*uWidth+x+i) = *(uSrcYAddr + uYOffset+i);
			if (y%2 == 0)
			{
				for (i=0; i<uCont/2; i++) 	// C
				{
					UINT32 uSrcOffset = (y/2)*(uWidth/2)+x/2+i;
					*(uDstCbAddr+uSrcOffset) = *(uSrcCAddr + uCOffset+2*i);
					*(uDstCrAddr+uSrcOffset) = *(uSrcCAddr + uCOffset+2*i+1);
				}
			}
			uFilled += uCont;
			x += uCont;
		}
		
	}

}
#else//(CAPTURE_MODE==FIELD_CAPTURE)
// change a MB format yuv buffer to Line format
// MB/Y_C mode => Line/Y_Cb_Cr mode
void format_transform(UINT8 *uSrcYAddr, UINT8 *uSrcCAddr, 
						UINT8 *uDstYAddr, UINT8 *uDstCbAddr, UINT8 *uDstCrAddr,
						UINT16 uWidth, UINT16 uHeigth)
{
	UINT32 x, y ;
	UINT16 uMBWidth ;
	UINT16 uCont, 	// number of continuous pixels
		     uFilled; 	
	
	UINT16 i;
	//UINT8 *puCurYAddr, *puCurCAddr;
	UINT32 uYOffset, uCOffset;

//soc_printf("%8x\n%8x\n%8x\n%8x\n%8x\n%d\n%d\n",uSrcYAddr,uSrcCAddr,uDstYAddr,uDstCbAddr,uDstCrAddr,uWidth,uHeigth);

	uMBWidth = (uWidth+15) /16;

	for (y=0; y<uHeigth; y++)
	{
		// fill a line each time
		uFilled= 0;
		x = 0;
		while (uFilled < uWidth)
		{
			uCont = 16- x%16;
			GetOffset(x, y*2,  uMBWidth, &uYOffset, &uCOffset); 
			for (i=0; i<uCont; i++)	 	// Y
				 *(uDstYAddr+y*uWidth+x+i) = *(uSrcYAddr + uYOffset+i);
			if (y%2 == 0)
			{
				for (i=0; i<uCont/2; i++) 	// C
				{
					UINT32 uSrcOffset = (y/2)*(uWidth/2)+x/2+i;
					*(uDstCbAddr+uSrcOffset) = *(uSrcCAddr + uCOffset+2*i);
					*(uDstCrAddr+uSrcOffset) = *(uSrcCAddr + uCOffset+2*i+1);
				}
			}
			uFilled += uCont;
			x += uCont;
		}
		
	}

}
#endif



//  extern int r,Xi,Xb,Xp,d0i,d0p,d0b; /* rate control */
//  extern double avg_act; /* rate control */
  
static void initparam()
{
	int i;
	int h,m,s,f;


	h=23;m=59;s=59;f=24;//fgets(line,254,fd); sscanf(line,"%d:%d:%d:%d",&h,&m,&s,&f);
	N=1;//fgets(line,254,fd); sscanf(line,"%d",&N);
	M=0;//fgets(line,254,fd); sscanf(line,"%d",&M);
	
	mpeg1=0;//fgets(line,254,fd); sscanf(line,"%d",&mpeg1);
	fieldpic=0;//fgets(line,254,fd); sscanf(line,"%d",&fieldpic);
	aspectratio=2;//fgets(line,254,fd); sscanf(line,"%d",&aspectratio);
	frame_rate_code=4;//fgets(line,254,fd); sscanf(line,"%d",&frame_rate_code);
	
	bit_rate=4000000.0;//fgets(line,254,fd); sscanf(line,"%lf",&bit_rate);
	vbv_buffer_size=6;//fgets(line,254,fd); sscanf(line,"%d",&vbv_buffer_size);   
	low_delay=0;//fgets(line,254,fd); sscanf(line,"%d",&low_delay);     
	constrparms=0;//fgets(line,254,fd); sscanf(line,"%d",&constrparms);
	mpeg_profile=4;//fgets(line,254,fd); sscanf(line,"%d",&profile);
	mpeg_level=8;//fgets(line,254,fd); sscanf(line,"%d",&level);
	prog_seq=0;//fgets(line,254,fd); sscanf(line,"%d",&prog_seq);
	chroma_format=1;//fgets(line,254,fd); sscanf(line,"%d",&chroma_format);
	video_format=2;//fgets(line,254,fd); sscanf(line,"%d",&video_format);
	color_primaries=5;//fgets(line,254,fd); sscanf(line,"%d",&color_primaries);
	transfer_characteristics=5;//fgets(line,254,fd); sscanf(line,"%d",&transfer_characteristics);
	matrix_coefficients=5;//fgets(line,254,fd); sscanf(line,"%d",&matrix_coefficients);
	display_horizontal_size=720;//fgets(line,254,fd); sscanf(line,"%d",&display_horizontal_size);
	display_vertical_size=480;//fgets(line,254,fd); sscanf(line,"%d",&display_vertical_size);
	dc_prec=2;//fgets(line,254,fd); sscanf(line,"%d",&dc_prec);
	topfirst=1;//fgets(line,254,fd); sscanf(line,"%d",&topfirst);

	frame_pred_dct = 1;
	q_scale_type = 0;
	intravlc = 1;
	altscan = 1;

	repeatfirst=0;//fgets(line,254,fd); sscanf(line,"%d",&repeatfirst);
	//prog_frame=1;//fgets(line,254,fd); sscanf(line,"%d",&prog_frame);

	/* make flags boolean (x!=0 -> x=1) */
	mpeg1 = !!mpeg1;
	fieldpic = !!fieldpic;
	low_delay = !!low_delay;
	constrparms = !!constrparms;
	prog_seq = !!prog_seq;
	topfirst = !!topfirst;

	frame_pred_dct = !!frame_pred_dct;
	q_scale_type = !!q_scale_type;
	intravlc = !!intravlc;
	altscan = !!altscan;

	repeatfirst = !!repeatfirst;
	prog_frame = !!prog_frame;


	frame_rate = 29970;

	/* timecode -> frame number */
	tc0 = h;
	tc0 = 60*tc0 + m;
	tc0 = 60*tc0 + s;
	tc0 = (int)(frame_rate*tc0/1000) + f;




	/* relational checks */


	if (!mpeg1 && constrparms)
	{
		  VENC_PRINTF("Warning: setting constrained_parameters_flag = 0\n");
		constrparms = 0;
	}

	if (prog_seq && !prog_frame)
	{
		  VENC_PRINTF("Warning: setting progressive_frame = 1\n");
		prog_frame = 1;
	}

	if (prog_frame && fieldpic)
	{
		  VENC_PRINTF("Warning: setting field_pictures = 0\n");
		fieldpic = 0;
	}

	if (!prog_frame && repeatfirst)
	{
		  VENC_PRINTF("Warning: setting repeat_first_field = 0\n");
		repeatfirst = 0;
	}

	if (prog_frame)
	{
		  if (!frame_pred_dct)
		  {
		      VENC_PRINTF("Warning: setting frame_pred_frame_dct[%d] = 1\n",i);
		    frame_pred_dct = 1;
		  }
	}

	if (prog_seq && !repeatfirst && topfirst)
	{
		  VENC_PRINTF("Warning: setting top_field_first = 0\n");
		topfirst = 0;
	}


}

static void initquantmat()
{
  int i;

    /* use default intra matrix */
    load_iquant = 0;
    for (i=0; i<64; i++)
      intra_q[i] = enc_default_intra_quantizer_matrix[i];



    /* use default non-intra matrix */
    load_niquant = 0;
    for (i=0; i<64; i++)
      inter_q[i] = 16;

}

void mpeg2enc(UINT16 h_size,UINT16 v_size,UINT8* y_addr,UINT8* c_addr,UINT8* f_y_addr,UINT8* f_c_addr,UINT8* out_stream_buf,UINT32* out_stream_size)
{
	UINT32 dest_size = *out_stream_size;
	if(dest_size == 0) dest_size = 32768;
	
	#if (CAPTURE_MODE==FRAME_CAPTURE)
	prog_frame = 0;
	g_mquant = 18;
	#else //(CAPTURE_MODE==FIELD_CAPTURE)
	prog_frame = 1;
	g_mquant = 10;
	#endif

	do{

		g_bs_ptr = out_stream_buf;
		g_bs_len = 0;

		initparam();

		/* round picture dimensions to nearest multiple of 16 or 32 */
		mb_width = (h_size+15)/16;
	#if (CAPTURE_MODE==FRAME_CAPTURE)
		mb_height = prog_seq ? (v_size+15)/16 : 2*((v_size+31)/32);
	#else//(CAPTURE_MODE==FIELD_CAPTURE)
		mb_height = prog_seq ? (v_size/2+15)/16 : 2*((v_size/2+31)/32);
	#endif
		mb_height2 = fieldpic ? mb_height>>1 : mb_height; /* for field pictures */
		luma_width = 16*mb_width;
		luma_height = 16*mb_height;

		chrom_width = luma_width>>1;
		chrom_height = luma_height>>1;

		width2 = fieldpic ? luma_width<<1 : luma_width;
		chrom_width2 = fieldpic ? chrom_width<<1 : chrom_width;

		block_count = 6;//420

			
		neworg[0] = f_c_addr;
		neworg[1] = neworg[0]+luma_width*luma_height;
		neworg[2] = neworg[1]+chrom_width*chrom_height;	


		format_transform(y_addr,c_addr,neworg[0],neworg[1],neworg[2],luma_width,luma_height);

		initquantmat();
		initbits();

		putseq();
		//soc_printf("mquant:%d,bs_len:%d\n",g_mquant,g_bs_len);

		if(g_mquant == 62 && g_bs_len>dest_size) 
		{
			soc_printf("ERROR: compressed size exceed buffer size!\n");
			break;				
		} 

		if(g_bs_len*10/dest_size < 12)
			g_mquant +=2;
		else if(g_bs_len*10/dest_size < 15)
			g_mquant +=4;
		else if(g_bs_len*10/dest_size < 18)
			g_mquant +=6;
		else if(g_bs_len*10/dest_size < 21)
			g_mquant +=8;
		else			
			g_mquant +=10;
		
		if(g_mquant > 62) 
			g_mquant = 62;
		
	}while(g_bs_len > dest_size);
	
	*out_stream_size = g_bs_len;
	
	return ;
}

