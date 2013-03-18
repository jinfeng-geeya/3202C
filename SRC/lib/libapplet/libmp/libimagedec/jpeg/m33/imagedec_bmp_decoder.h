#ifndef __IMAGEDEC_BMP_DECODER_HEADER__
#define __IMAGEDEC_BMP_DECODER_HEADER__
				
/*parameter types*/
#define LONG long
#define WORD unsigned short
#define DWORD unsigned long

#define BMP_FAILED -1
#define BMP_DONE 	1
#define BMP_ROW_OK 0


/*Pal size*/
#define TV_HOR_COL_NUM		720
#define TV_VER_LINE_NUM	576

/*RGB2YCbCr coefficient*/
#define Y_COF_R		0x41CA//0.257<<16	
#define Y_COF_G		0x8106//0.504<<16	
#define Y_COF_B		0x1916//0.098<<16
#define C_COF_Cb	0x2049b// 2.018<<16
#define C_COF_Cr		0x19893// 1.596<<16

#define CB_COF_R	0x25e3
#define CB_COF_G	0x4A7E
#define CB_COF_B	0x7062
#define CR_COF_R	0x7062
#define CR_COF_G	0x5e35
#define CR_COF_B	0x122D

#define BI_RGB 	0
#define BI_RLE8 	1
#define BI_RLE4	2
#define BI_BITFIELDS 3

typedef enum 
{
	ENC_NOR,
	ENC_1ST,
	ENC_2ND,
	ENC_3RD,
	ABS_MODE,
}RLE_MODE;

typedef struct tagBITMAPFILEHEADER {
        WORD    bfType;
        DWORD   bfSize;
        WORD    bfReserved1;
        WORD    bfReserved2;
        DWORD   bfOffBits;
} BITMAPFILEHEADER, *PBITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER{
        DWORD      biSize;
        LONG       biWidth;
        LONG       biHeight;
        WORD       biPlanes;
        WORD       biBitCount;
        DWORD      biCompression;
        DWORD      biSizeImage;
        LONG       biXPelsPerMeter;
        LONG       biYPelsPerMeter;
        DWORD      biClrUsed;
        DWORD      biClrImportant;
} BITMAPINFOHEADER, *PBITMAPINFOHEADER;

#endif
