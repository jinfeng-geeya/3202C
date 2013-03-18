/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: flash_data_s.c
 *
 *  Description: Provide serial flash ID and description.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  1.  2006.6.28   Justin Wu   0.1.000  Initial
 *  2.  2006.10.31   Wen Liu    0.2.000  Reorganization 
 ****************************************************************************/
const unsigned char sflash_deviceid[] =
{
//ENDIAN_LITTLE	
/* Index|DevID byte|bit offset*/
/*0*/	0x04, 8,	/* AT26F004 */
/*1*/	0x45, 8,	/* AT26DF081 */
/*2*/	0x46, 8,	/* AT26DF161 */
/*3*/	0x47, 8,	/* AT26DF321 */
/*4*/	0x13, 24,	/* A25L40P */
/*5*/	0x14, 24,	/* A25L80P */
/*6*/	0x15, 24,	/* A25L16P */
/*7*/	0x32, 56,	/* EN25B40B */
/*8*/	0x42, 56,	/* EN25B40T */
/*9*/	0x33, 56,	/* EN25B80B */
/*10*/	0x43, 56,	/* EN25B80T */
/*11*/	0x34, 56,	/* EN25B16B */
/*12*/	0x44, 56,	/* EN25B16T */	
/*13*/	0x8D, 16,	/* SST25VF040B */				
/*14*/	0x8E, 16,	/* SST25VF080B */
/*15*/	0x41, 16,	/* SST25VF016B */
/*16*/	0x4A, 16,	/* SST25VF032B */
/*17*/	0x4B, 16,	/* SST25VF064C */
	//The following should go after all others
/*18*/	0x12, 56,	/* EN25P40 */ /* M25P40 M25PE40 M45PE40*/  /* MX25L4005A */  /* S25FL004A */ /*W25X40*/
/*19*/	0x13, 56,	/* EN25P80 */ /* M25P80 M25PE80 M45PE80*/  /* MX25L8005 */   /* S25FL008A */ /*W25X80*/
/*20*/	0x14, 56,	/* EN25P16 */ /* M25P16 M25PE16 M45PE16 */ /* MX25L1605/A */ /* S25FL016A */ /*W25X16*/
/*21*/	0x15, 56,   /* M25P32 */ /* MX25L3205/A */ /* S25FL032A */ /*W25X32*/ /*ESMT F25L32PA*/ /*EN25Q32B*/ /*EN25F32*/
/*22*/	0x11, 16,       /*intel S33 2MB*/
/*23*/	0x31, 8,        /*EoN 16CN20C*/
/*24*/	0x8c, 24,       /*ESMT*/
/*25*/	0x16, 56,	/*MX25L6405D*//*S25FL064A*/ /*W25X64*//*MX25L6406E*/
/*26*/	0x17, 56,	/*MX25L12845E*//*S25F129PoxMF001*/
/*27*/	0x15, 16,		/* MX25L1635D*/
/*28*/	0x16, 16,		/* MX25L3235D*/ /*Nomunyx M25PX32 */
/*29*/	0x20, 0,	/*M25128P*/
/*30*/	0x35, 56, /*EN25B32B */
/*31*/	0x12, 16, /*intel S33 4MB*/
/*32*/	0x48, 8,	/* AT25DF641 */
};

const unsigned char sflash_id[] =
{
	7, 13, 14, 17,					/* start from index 0*/
	24, 26, 28, 					/* start from index 4*/
	24, 25, 26, 27, 28, 29,				/* start from index 7*/
	7, 13, 14, 17, 31,				/* start from index 13*/
	7, 13, 14, 17,30,13, 13, 31, 34, 30, 17, 32,	/* start from index 18*/
	33, 17, 31,			/* start from index 30*/
};

const unsigned short sflash_deviceid_num = \
	(sizeof(sflash_deviceid) / (2 * sizeof(unsigned char))) ;
