/****************************************************************************
 *
 *  ALi (Shanghai) Corporation, All Rights Reserved. 2005 Copyright (C)
 *
 *  File: flash_data.c
 *
 *  Description: Provide general flash data for sto type device.
 *
 *  History:
 *      Date        Author      Version  Comment
 *      ====        ======      =======  =======
 *  0.              Justin Wu            Ref. code
 *  1.  2005.5.28   Liu Lan     0.1.000  Initial
 *
 ****************************************************************************/
const unsigned int tflash_cmdaddr[] =
{
	0xAAA, 0x555, 0x5555
};

const unsigned char flash_cmdaddr_num = \
	(sizeof(tflash_cmdaddr) / sizeof(unsigned int));

/* Item 0 in following array is just an position holder. */
const unsigned short tflash_sectors[] =
{
	0,		/*0*/
	11, 11, 19, 19, 35, 35, 8, /*1*/
	23, 23,		/*8*/
	128, 256, 512,	/*10*/
	16,         /* 13 reserved for 39VF088 */
	32,		/*14*/
	39, 39,	/*15*/
	64, 71, 71,	/*17*/
    263, 270,		/*20*/
    128,			/*22*/
    512,			/*23*/
    12, 12,		/*24*/
    20, 20,		/*26*/
    36, 36, 32,		/*28*/
    128, 64, 68, 256, /*31*/
    40, 72,  /*35*/
};

const unsigned char flash_sector_begin[] =
{
	0,		/*0*/
	0, 6, 0, 5, 0, 4, 3,	/*1*/
	10, 14,		/*8*/
	16, 16, 16,	/*10*/
	4,           /* 13 reserved for 39VF088 */
	3,		/*14*/
	10, 13,	/*15*/
	11, 10, 12,	/*17*/
	20, 25,		/*20*/
	24,			/*22*/
	40,			/*23*/
	28, 35,		/*24*/
	28, 34,		/*26*/
	28, 33, 12,	/*28*/
	21, 43, 44, 21, /*31*/
	57, 62,  /*35*/
};
#define _256K	18
#define _128K	17
#define _64K    16
#define _32K    15
#define _16K    14
#define _8K     13
#define _4K     12
#define _1K	10

const unsigned char flash_sector_map[] =
{
          1, _16K,   2,  _8K,   1, _32K,  7, _64K,  /* 0 */
          16, _64K,   8, _64K,                       /* 4 */
          7, _64K,   1, _32K,   2,  _8K,  1, _16K,  /* 6 */
          8,  _8K,  15, _64K,                       /* 10 */
          32, _64K,  16, _64K,                       /* 12 */
          15, _64K,   8,  _8K,				/* 14 */
          128,  _4K, 128,  _4K, 128, _4K, 128,  _4K,  /* 16 */
          8, _8K,  193, _64K,  63, _64K,  8,  _8K,	/* 20 */
          128, _128K,  8,  _8K, 254, _64K,  8,  _8K,	/* 24 */
          2, _4K, 1, _8K, 1, _16K,1, _32K,  7, _64K,	/* 28 */
          16, _64K,   8, _64K,                          /* 33 */
          7, _64K,   1, _32K,   1,  _16K,  1, _8K, 2, _4K, /* 35 */
          255, _1K, 255, _1K,2, _1K,			/* 40 */
          64, _256K,						/* 43 */
          2, _4K, 1, _8K, 1, _16K, 1, _32K, 7, _64K,	/* 44 */
          32, _64K, 16, _64K, 8, _64K,			/* 49 */
          7, _64K, 1, _32K, 1, _16K, 1, _8K, 2, _4K, /* 52 */
          4, _8K, 1, _32K, 30, _64K, 1, _32K, 4, _8K, /* 57 */
          4, _8K, 1, _32K, 62, _64K, 1, _32K, 4, _8K, /* 62 */
};
