/***FastCRC.c*/
/***Fast CRC code/decode functions*/
/***Justin Wu*/
/***2000.7.13*/
/***NOTE: The type of int must be 32bits weight. ***/
#include <sys_config.h>
#include <api/libc/fastCRC.h>

/************************************* Const define *********************************/
/* Globle CRC select switch */
// #define MG_CRC_32_ARITHMETIC
#define MG_CRC_32_ARITHMETIC_CCITT
/* #define MG_CRC_24_ARITHMETIC_CCITT
 * #define MG_CRC_24_ARITHMETIC */

#ifdef MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_BIT
#endif

#ifdef MG_CRC_32_ARITHMETIC
#define MG_CRC_32_BIT
#endif

#ifdef MG_CRC_24_ARITHMETIC_CCITT
#define MG_CRC_24_BIT
#endif

#ifdef MG_CRC_24_ARITHMETIC
#define MG_CRC_24_BIT
#endif

/* 0xbba1b5 = 1'1011'1011'1010'0001'1011'0101b
 <= x24+x23+x21+x20+x19+x17+x16+x15+x13+x8+x7+x5+x4+x2+1 */
#ifdef MG_CRC_24_ARITHMETIC_CCITT
#define MG_CRC_24_CCITT		0x00ddd0da
#endif

#ifdef MG_CRC_24_ARITHMETIC
#define MG_CRC_24			0x00ad85dd
#endif

/* 0x04c11db7 = 1'0000'0100'1100'0001'0001'1101'1011'0111b
 <= x32+x26+x23+x22+x16+x12+x11+x10+x8+x7+x5+x4+x2+x+1 */
#ifdef MG_CRC_32_ARITHMETIC_CCITT
#define MG_CRC_32_CCITT		0x04c11db7
#endif

#ifdef MG_CRC_32_ARITHMETIC
#define MG_CRC_32			0xedb88320
#endif

/************************************ Var define ************************************/
#if defined(FIXED_CRC_TABLE)
    unsigned int MG_CRC_Table[256];
#elif (SYS_SDRAM_SIZE == 2 && (SYS_PROJECT_FE == PROJECT_FE_DVBS||SYS_PROJECT_FE == PROJECT_FE_DVBS2) )
    #if(SYS_EPG_MODULE == SYS_FUNC_ON)
        unsigned int MG_CRC_Table[256];
    #else
        unsigned int *MG_CRC_Table = (unsigned int *)(__MM_CRC_BUF);
    #endif
#elif (SYS_SDRAM_SIZE == 2 && SUBTITLE_ON != 1)
    #ifdef  _DEBUG_VERSION_
      unsigned int *MG_CRC_Table = (unsigned int *)(__MM_CRC_BUF);
    #else
       unsigned int MG_CRC_Table[256];
    #endif
#else
    unsigned int MG_CRC_Table[256];
#endif


/********************************** Function define *********************************/

/*From "len" length "buffer" data get CRC, "crc" is preset value of crc*/
unsigned int MG_Compute_CRC(register unsigned int crc,
						register unsigned char *bufptr,
						register int len)
{
	register int i;

#ifdef MG_CRC_24_ARITHMETIC_CCITT
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)(*bufptr) << 16);
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 0x00800000)	/*Highest bit procedure*/
				crc = (crc << 1) ^ MG_CRC_24_CCITT;
			else
				crc <<= 1;
		}
	}
	return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_24_ARITHMETIC
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)*bufptr;
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 1)				/*Lowest bit procedure*/
				crc = (crc >> 1) ^ MG_CRC_24;
			else
				crc >>= 1;
		}
	}
	return(crc & 0x00ffffff);  /*Get lower 24 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)(*bufptr) << 24;
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 0x80000000)	/*Highest bit procedure*/
				crc = (crc << 1) ^ MG_CRC_32_CCITT;
			else
				crc <<= 1;
		}
	}
	return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif

#ifdef MG_CRC_32_ARITHMETIC
	while(len--)  /*Length limited*/
	{
		crc ^= (unsigned int)*bufptr;
		bufptr++;
		for(i = 0; i < 8; i++)
		{
			if(crc & 1)				/*Lowest bit procedure*/
				crc = (crc >> 1) ^ MG_CRC_32;
			else
				crc >>= 1;
		}
	}
	return(crc & 0xffffffff);  /*Get lower 32 bits FCS*/
#endif
}

/*Setup fast CRC compute table*/
void MG_Setup_CRC_Table()
{
	register int count;
	unsigned char zero=0;

	for(count = 0; count <= 255; count++) /*Comput input data's CRC, from 0 to 255*/
#ifdef MG_CRC_24_ARITHMETIC_CCITT
		MG_CRC_Table[count] = MG_Compute_CRC(count << 16,&zero,1);
#endif

#ifdef MG_CRC_24_ARITHMETIC
		MG_CRC_Table[count] = MG_Compute_CRC(count,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
		MG_CRC_Table[count] = MG_Compute_CRC(count << 24,&zero,1);
#endif

#ifdef MG_CRC_32_ARITHMETIC
		MG_CRC_Table[count] = MG_Compute_CRC(count,&zero,1);
#endif

}

/*Fast CRC compute*/
unsigned int MG_Table_Driven_CRC(register unsigned int crc,
			    register unsigned char *bufptr,
			    register int len)
{
	register int i;

	for(i = 0; i < len; i++)
#ifdef MG_CRC_24_ARITHMETIC_CCITT
		crc=(MG_CRC_Table[((crc >> 16) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_24_ARITHMETIC
		crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0x00ffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
		crc=(MG_CRC_Table[((crc >> 24) & 0xff) ^ bufptr[i]] ^ (crc << 8)) & 0xffffffff;
#endif

#ifdef MG_CRC_32_ARITHMETIC
		crc=(MG_CRC_Table[(crc & 0xff) ^ bufptr[i]] ^ (crc >> 8)) & 0xffffffff;
#endif
	return(crc);
}

void MG_FCS_Coder(unsigned char *pucInData,int len)
{
	unsigned int iFCS;

#ifdef MG_CRC_24_ARITHMETIC_CCITT
	iFCS = ~MG_Table_Driven_CRC(0x00ffffff,pucInData,len);
	pucInData[len + 2] = (unsigned char)iFCS&0xff;
	pucInData[len + 1] = (unsigned char)(iFCS >>  8) & 0xff;
	pucInData[len]     = (unsigned char)(iFCS >> 16) & 0xff;
#endif

#ifdef MG_CRC_24_ARITHMETIC
	iFCS = ~MG_Table_Driven_CRC(0x00ffffff,pucInData,len);
	pucInData[len]     = (unsigned char)iFCS&0xff;
	pucInData[len + 1] = (unsigned char)(iFCS >>  8) & 0xff;
	pucInData[len + 2] = (unsigned char)(iFCS >> 16) & 0xff;
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
	iFCS = MG_Table_Driven_CRC(0xffffffff,pucInData,len);
	pucInData[len + 3] = (unsigned char)iFCS & 0xff;
	pucInData[len + 2] = (unsigned char)(iFCS >>  8) & 0xff;
	pucInData[len + 1] = (unsigned char)(iFCS >> 16) & 0xff;
	pucInData[len]     = (unsigned char)(iFCS >> 24) & 0xff;
#endif

#ifdef MG_CRC_32_ARITHMETIC
	iFCS = ~MG_Table_Driven_CRC(0xffffffff,pucInData,len);
	pucInData[len]     = (unsigned char)iFCS & 0xff;
	pucInData[len + 1] = (unsigned char)(iFCS >>  8) & 0xff;
	pucInData[len + 2] = (unsigned char)(iFCS >> 16) & 0xff;
	pucInData[len + 3] = (unsigned char)(iFCS >> 24) & 0xff;
#endif
}

int MG_FCS_Decoder(unsigned char *pucInData,int len)
{
	unsigned int iFCS;

#ifdef MG_CRC_24_ARITHMETIC_CCITT
	pucInData[len - 1] = ~pucInData[len - 1];
	pucInData[len - 2] = ~pucInData[len - 2];
	pucInData[len - 3] = ~pucInData[len - 3];
	if ((iFCS = MG_Table_Driven_CRC(0x00ffffff,pucInData,len)) != 0)/*Compute FCS*/
	{
		return(iFCS);							/* CRC check error */
	}
	pucInData[len-1]='\0';
	pucInData[len-2]='\0';
	pucInData[len-3]='\0';
#endif

#ifdef MG_CRC_24_ARITHMETIC
	pucInData[len - 1] = ~pucInData[len - 1];
	pucInData[len - 2] = ~pucInData[len - 2];
	pucInData[len - 3] = ~pucInData[len - 3];
	if ((iFCS = MG_Table_Driven_CRC(0x00ffffff,pucInData,len)) != 0)/*Compute FCS*/
	{
		return(iFCS);							/* CRC check error */
	}
	pucInData[len-1]='\0';
	pucInData[len-2]='\0';
	pucInData[len-3]='\0';
#endif

#ifdef MG_CRC_32_ARITHMETIC_CCITT
/*	pucInData[len - 1] = ~pucInData[len - 1];
	pucInData[len - 2] = ~pucInData[len - 2];
	pucInData[len - 3] = ~pucInData[len - 3];
	pucInData[len - 4] = ~pucInData[len - 4];
*/	if ((iFCS = MG_Table_Driven_CRC(0xffffffff,pucInData,len)) != 0)/*Compute FCS*/
	{
		return(iFCS);							/* CRC check error */
	}
/*	pucInData[len - 1] = '\0';
	pucInData[len - 2] = '\0';
	pucInData[len - 3] = '\0';
	pucInData[len - 4] = '\0';
*/
#endif

#ifdef MG_CRC_32_ARITHMETIC
	pucInData[len - 1] = ~pucInData[len - 1];
	pucInData[len - 2] = ~pucInData[len - 2];
	pucInData[len - 3] = ~pucInData[len - 3];
	pucInData[len - 4] = ~pucInData[len - 4];
	if ((iFCS = MG_Table_Driven_CRC(0xffffffff,pucInData,len)) != 0)/*Compute FCS*/
	{
		return(iFCS);							/* CRC check error */
	}
	pucInData[len - 1] = '\0';
	pucInData[len - 2] = '\0';
	pucInData[len - 3] = '\0';
	pucInData[len - 4] = '\0';
#endif

	return(0);								/* CRC check OK */
}

/***************************************** END ***************************************/
