#ifndef CHUNK_H
#define CHUNK_H

#include <hld/sto/sto.h>

#define CHUNK_ID        0
#define CHUNK_LENGTH    4
#define CHUNK_OFFSET    8
#define CHUNK_CRC       12
#define CHUNK_NAME      16
#define CHUNK_VERSION   32
#define CHUNK_TIME      48
#define CHUNK_RESERVED 64
#define CHUNK_USER_VERSION  64
#define CHUNK_CODESTART 120
#define CHUNK_ENTRY     124
#define CHUNK_HEADER_SIZE       128

#define NO_CRC  0x4E435243

typedef struct{
	unsigned long id;
	unsigned long len;
	unsigned long offset;
	unsigned long crc;
	unsigned char name[16];
	unsigned char version[16];
	unsigned char time[16];
	unsigned char reserved[64];
}CHUNK_HEADER;

typedef struct{
	unsigned long id;
	unsigned int upg_flag;
}CHUNK_LIST;

typedef struct{
	unsigned long type;
	unsigned long index;
	unsigned long offset;
}CHUNK_LIST2;

#if(SYS_SDRAM_SIZE > 2)
#define SUBBLOCK_TYPE      0
#define SUBBLOCK_LENGTH    1
#define SUBBLOCK_OFFSET    4
#define SUBBLOCK_RESERVED 7
#define SUBBLOCK_HEADER_SIZE    16

typedef struct{
	unsigned char type;
	unsigned long len;
	unsigned long offset;
	unsigned char reserved[7];
}SUBBLOCK_HEADER;
#endif

unsigned long fetch_long(unsigned char *);
void store_long(unsigned char *, unsigned long);
unsigned char *chunk_goto(unsigned long *, unsigned long, \
      unsigned int);
unsigned char chunk_count(unsigned long, unsigned long);
void chunk_init(unsigned long, unsigned int);
int get_chunk_list(CHUNK_LIST* chunk_list);
int get_chunk_header(unsigned long chid, CHUNK_HEADER *pheader);

unsigned long sto_fetch_long(unsigned long);
unsigned long sto_chunk_goto(unsigned long *, unsigned long, \
      unsigned int);
unsigned char sto_chunk_count(unsigned long, unsigned long);
void sto_chunk_init(unsigned long, unsigned int);
int sto_get_chunk_list(CHUNK_LIST* chunk_list);
int sto_get_chunk_header(unsigned long chid, CHUNK_HEADER *pheader);

#if(SYS_SDRAM_SIZE > 2)
int sto_subblock_count(unsigned long chid);
unsigned long sto_subblock_goto(unsigned long chid,unsigned char *sbblock_type,unsigned char mask, unsigned int n);
int sto_get_subblock_header(unsigned long chid, unsigned char sbblock_type, SUBBLOCK_HEADER *pheader);
#endif

BOOL backup_chunk_data(UINT8 *pbuff, UINT32 length, UINT32 chunk_id);
#endif
