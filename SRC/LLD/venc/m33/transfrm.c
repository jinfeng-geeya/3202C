/* transfrm.c,  forward / inverse transformation                            */

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


/* subtract prediction from block data */
static void sub_pred(cur,lx,blk)
unsigned char *cur;
int lx;
short *blk;
{
  int i, j;

  for (j=0; j<8; j++)
  {
    for (i=0; i<8; i++)
      blk[i] = cur[i] - 0x80;// - pred[i];
    blk+= 8;
    cur+= lx;
    //pred+= lx;
  }
}

void transform(cur,i,j,mbi,blks)
unsigned char *cur[];
int i,j;
struct MbInfo *mbi;
//short blks[][64];
short (*blks)[64];
{
  int i1, j1, n, cc, offs, lx;

	i = i*16;
	j = j*16;
	
      for (n=0; n<block_count; n++)
      {
        cc = (n<4) ? 0 : (n&1)+1; /* color component index */
        if (cc==0)
        {
          /* luminance */
          if ((pict_struct==FRAME_PICTURE) && mbi->dct_type )
	   {
            /* field DCT */
            offs = i + ((n&1)<<3) + luma_width*(j+((n&2)>>1));
            lx = luma_width<<1;
          }
          else
          {
            /* frame DCT */
            offs = i + ((n&1)<<3) + width2*(j+((n&2)<<2));
            lx = width2;
          }				
        }
        else
        {
          /* chrominance */

          /* scale coordinates */
          i1 = (chroma_format==CHROMA444) ? i : i>>1;
          j1 = (chroma_format!=CHROMA420) ? j : j>>1;

          if ((pict_struct==FRAME_PICTURE) && mbi->dct_type
              && (chroma_format!=CHROMA420))
          {
            /* field DCT */
            offs = i1 + (n&8) + chrom_width*(j1+((n&2)>>1));
            lx = chrom_width<<1;
          }
          else
          {
            /* frame DCT */
            offs = i1 + (n&8) + chrom_width2*(j1+((n&2)<<2));
            lx = chrom_width2;
          }
        }

	 sub_pred(cur[cc]+offs,lx,blks[n]);
        fdct(blks[n]);
	}
}


void dct_type_estimation(mbi)
struct MbInfo *mbi;
{

      if (frame_pred_dct || pict_struct!=FRAME_PICTURE)
        mbi->dct_type = 0;
      else
      {
        /* interlaced frame picture */
          mbi->dct_type = 1; /* field DCT */
      }
}

