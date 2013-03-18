/***fastCRC.H*/
/***Fast CRC function head file*/
/***Justin wood*/
/***2000.7.13*/

#ifndef	__MG_CRC_H
#define __MG_CRC_H

#ifdef __cplusplus
extern "C"
{
#endif

/********************************** Var announce *************************************/
/* FCS fast lookup table:	in	"fastCRC.c"*/
//extern unsigned int MG_CRC_Table[256]; 

/********************************** Function announce *******************************/
/* From "len" length "buffer" data get CRC, "crc" is preset value of crc*/
unsigned int MG_Compute_CRC(register unsigned int crc,register unsigned char *bufptr,register int len);

/* Setup fast CRC compute table*/
void MG_Setup_CRC_Table();

/* Fast CRC compute*/
unsigned int MG_Table_Driven_CRC(register unsigned int crc,register unsigned char *bufptr,register int len);

/* FCS coder */
void MG_FCS_Coder(unsigned char *pucInData,int len);

/* FCS decoder */
int MG_FCS_Decoder(unsigned char *pucInData,int len);

#ifdef __cplusplus
}
#endif

#endif	/* __MG_CRC_H */
/**************************************** END *************************************/
