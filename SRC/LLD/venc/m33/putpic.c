/* putpic.c, block and motion vector encoding routines                      */

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
 
#include "global.h"


void putpict(i,j,mbi)
int i,j;
struct MbInfo *mbi;
{
  int comp, cc;
  int mb_type;
  //static int prev_mquant = 12;
  int prev_mquant = g_mquant;//12;
  int cbp, MBAinc;

  //prev_mquant = rc_start_mb(); /* initialize quantization parameter */
//  prev_mquant =20;

      /* macroblock loop */
      if (i==0)
      {
        /* slice header (6.2.4) */
        alignbits();
	putbits(SLICE_MIN_START+j,32); /* slice_start_code */
	
        /* quantiser_scale_code */
        putbits(q_scale_type ? map_non_linear_mquant[prev_mquant]
                             : prev_mquant >> 1, 5);  
		
        putbits(0,1); /* extra_bit_slice */

	/* reset predictors */
        for (cc=0; cc<3; cc++)
          dc_dct_pred[cc] = 0;
		
        MBAinc = i + 1; /* first MBAinc denotes absolute position */
      }

      mb_type = mbi->mb_type;

      mbi->mquant = g_mquant;//12;

      /* quantize macroblock */
      if (mb_type & MB_INTRA)
      {
        for (comp=0; comp<block_count; comp++)
          quant_intra(g_blocks[comp],g_blocks[comp],
                      dc_prec,intra_q,mbi->mquant);
        mbi->cbp = cbp = (1<<block_count) - 1;
      }
      else
      {
        cbp = 0;
        for (comp=0;comp<block_count;comp++)
          cbp = (cbp<<1) | quant_non_intra(g_blocks[comp],
                                           g_blocks[comp],
                                           inter_q,mbi->mquant);

        mbi->cbp = cbp;

        if (cbp)
          mb_type|= MB_PATTERN;
      }

      /* macroblock cannot be skipped */
      mbi->skipped = 0;

      /* there's no VLC for 'No MC, Not Coded':
       * we have to transmit (0,0) motion vectors
       */

	putaddrinc(1); /* macroblock_address_increment */
      //putaddrinc(MBAinc); /* macroblock_address_increment */
      //MBAinc = 1;

      putmbtype(pict_type,mb_type); /* macroblock type */


      if (pict_struct==FRAME_PICTURE && cbp && !frame_pred_dct)
        putbits(mbi->dct_type,1);


      for (comp=0; comp<block_count; comp++)
      {
        /* block loop */
        if (cbp & (1<<(block_count-1-comp)))
        {
          if (mb_type & MB_INTRA)
          {
            cc = (comp<4) ? 0 : (comp&1)+1;
            putintrablk(g_blocks[comp],cc);
          }
          else
            putnonintrablk(g_blocks[comp]);
        }
      }
	  
      /* reset predictors */
      if (!(mb_type & MB_INTRA))
        for (cc=0; cc<3; cc++)
          dc_dct_pred[cc] = 0;
      mbi->mb_type = mb_type;

}



