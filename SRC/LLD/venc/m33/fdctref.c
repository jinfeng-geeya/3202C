/* fdctref.c, forward discrete cosine transform, double precision           */

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


/* global declarations */

/* private data */
static int c[8][8] = /* transform coefficients */
{
{353,353,353,353,353,353,353,353},
{490,415,277,97,-97,-277,-415,-490},
{461,191,-191,-461,-461,-191,191,461},
{415,-97,-490,-277,277,490,97,-415},
{353,-353,-353,353,353,-353,-353,353},
{277,-490,97,415,-415,-97,490,-277},
{191,-461,461,-191,-191,461,-461,191},
{97,-277,415,-490,490,-415,277,-97}
};

void fdct(blk)
short *blk;
{
	int i, j, k;  //double s;  //double tmp[64];
	int s,sh,sl;
	int tmp[64];  
	
  for (i=0; i<8; i++)    
  	for (j=0; j<8; j++)   
	{      s = 0;      
		for (k=0; k<8; k++)       
			s += c[j][k] * blk[8*i+k];      
		tmp[8*i+j] = s;
	  //printf("%f\n",tmp[8*i+j]);    
	  }  
 for (j=0; j<8; j++)    
 	for (i=0; i<8; i++)    
	 {     
	  	s = 0;     
		sh = 0;
		sl = 0;
	  	for (k=0; k<8; k++)        
      		{
			//s += c[i][k] * tmp[8*k+j];
			sh += (c[i][k]>>6) * tmp[8*k+j];
			sl += (c[i][k]&0x3f) * tmp[8*k+j];	 
		}			
	  	//s += c[i][k] * tmp[8*k+j]/(1000*1000);      
	  	//block[8*i+j] = (int)floor(s+0.499999);
	       //blk[8*i+j] = s;      
		sh = (sh*64)/(1000*1000);
		sl = (sl+500*1000)/(1000*1000);
		blk[8*i+j] = sh + sl;
		   
	  /*       * reason for adding 0.499999 instead of 0.5:       
	  * s is quite often x.5 (at least for i and/or j = 0 or 4)       
	  * and setting the rounding threshold exactly to 0.5 leads to an       
	  * extremely high arithmetic implementation dependency of the result;       
	  * s being between x.5 and x.500001 (which is now incorrectly rounded       
	  * downwards instead of upwards) is assumed to occur less often       
	  * (if at all)       */    
 	}
}
