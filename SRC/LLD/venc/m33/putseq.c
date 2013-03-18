/* putseq.c, sequence level routines                                        */

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

void putseq()
{
	/* this routine assumes (N % M) == 0 */
	int i, j;
	int ipflag;


	putseqhdr();
	putseqext();
	putseqdispext();

	/* I frame */
	pict_type = I_TYPE;
	forw_hor_f_code = forw_vert_f_code = 15;
	back_hor_f_code = back_vert_f_code = 15;

	putgophdr(0,1);

	temp_ref = 0;
	pict_struct = FRAME_PICTURE;

	putpicthdr();
	if (!mpeg1)
		putpictcodext();	
	
	for (j=0; j<mb_height2; j++)//row
	{
	    for (i=0; i<mb_width; i++)
	    {
			motion_estimation(&mbinfo);
			dct_type_estimation(&mbinfo);
			transform(neworg,i,j,&mbinfo,g_blocks);			  
			putpict(i,j,&mbinfo);			
	    }
	}

	putseqend();
	
}
