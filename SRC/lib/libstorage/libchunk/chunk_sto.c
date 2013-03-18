#include <types.h>
#include <retcode.h>
#include <sys_config.h>
#include <api/libchunk/chunk.h>
#include <api/libc/string.h>
#include <api/libc/printf.h>
#include <hld/sto/sto.h>

#ifndef STO_CHUNK_PRINTF
#define STO_CHUNK_PRINTF PRINTF
#endif

static unsigned long chunk_start = 0;
static unsigned long chunk_end =  0x100000;
static unsigned long chunk_maxlen = 0x100000;

static struct sto_device *sto = NULL;

unsigned long sto_fetch_long(unsigned long offset)
{
    unsigned char buf[4];

    ASSERT(sto != NULL);

    sto_get_data(sto, buf, offset, 4);

    return (buf[0] << 24) | (buf[1] << 16) | (buf[2] << 8) | buf[3];
}

/*
    Search in predefined Chunk Range for certain chunk with
    specified Chunk ID and Number. Meanwhile, it returns real
    ID of the chunk.
*/
unsigned long sto_chunk_goto(unsigned long *chid, unsigned long mask, \
      unsigned int n)
{
    unsigned long id;
    unsigned long offset;
    unsigned long addr = chunk_start;

    if (n == 0)
        return ERR_PARA;

    while (addr < chunk_end)
    {
        id = sto_fetch_long(addr + CHUNK_ID);
        if (id == 0 || id == 0xFFFFFFFF)
            break;
        if (!((id ^ *chid) & mask))
        {
            if (--n == 0)
            {
                *chid = id;
                return addr;
            }
        }
        offset = sto_fetch_long(addr + CHUNK_OFFSET);
        if (offset == 0 || offset > chunk_maxlen)
            break;
        addr += offset;
    }
    return ERR_FAILUE;
}


/*
    Count the number of certain chunk with specified Chunk ID
    in predefined Chunk Range.
*/
unsigned char sto_chunk_count(unsigned long chid, unsigned long mask)
{
    unsigned long id;
    unsigned long len;
    unsigned long offset;
    unsigned long addr = chunk_start;
    unsigned char n = 0;

    while (addr < chunk_end)
    {
        id = sto_fetch_long(addr + CHUNK_ID);
        if (id == 0 || id == 0xFFFFFFFF)
            break;

        offset = sto_fetch_long(addr + CHUNK_OFFSET);
        if (offset > chunk_maxlen)
            break;

        len = sto_fetch_long(addr + CHUNK_LENGTH);
        if ((offset != 0) && (len > offset))
            break;

        if (!((id ^ chid) & mask))
            n++;

        if (offset == 0)
            break;

        addr += offset;
    }
    return n;
}


/*
    Initial Chunk List area.
*/
void sto_chunk_init(unsigned long start, unsigned int length)
{
    sto = (struct sto_device *)dev_get_by_id(HLD_DEV_TYPE_STO, 0);
    if (length == 0)
        length = sto->totol_size;

    /* open it anyway */
    sto_open(sto);

    chunk_start = start;
    chunk_end = start + length;
    chunk_maxlen = length;
}

/*
    Get chunk list
*/
int sto_get_chunk_list(CHUNK_LIST* chunk_list)
{
    CHUNK_LIST* plist;
    unsigned long id;
    unsigned char name[16];
    unsigned long count;
    unsigned long addr;
    unsigned long i;

    ASSERT(sto != NULL);

    if(!chunk_list) return 0;

    count = sto_chunk_count(0, 0);
    if(!count) return 0;

    plist = chunk_list;

    for(i=1; i<=count; i++)
    {
        addr = sto_chunk_goto(&id, 0, i);
        if(addr & 0x80000000) return 0;

        sto_get_data(sto, name, addr + CHUNK_NAME, 16);

        plist->id = id;

        plist->upg_flag = 0;

        plist++;
    }

    return count;
}

/*
    Get header of chunk with block_id
*/
int sto_get_chunk_header(unsigned long chid, CHUNK_HEADER *pheader)
{
    unsigned long addr;

    ASSERT(sto != NULL);

    addr = sto_chunk_goto(&chid, 0xFFFFFFFF, 1);
    if(addr & 0x80000000) return 0;

    pheader->id = chid;
    pheader->len = sto_fetch_long(addr + CHUNK_LENGTH);
    pheader->offset = sto_fetch_long(addr + CHUNK_OFFSET);
    pheader->crc = sto_fetch_long(addr + CHUNK_CRC);
    sto_get_data(sto, pheader->name, addr + CHUNK_NAME, 16);
    sto_get_data(sto, pheader->version, addr + CHUNK_VERSION, 16);
    sto_get_data(sto, pheader->time, addr + CHUNK_TIME, 16);
    sto_get_data(sto, pheader->reserved, addr + CHUNK_RESERVED, 64);

    return 1;
}

unsigned long sto_chunk_goto_ext(unsigned long index)
{
	unsigned long id;
	unsigned long offset;
	unsigned long addr = chunk_start;
	unsigned long i=0;

	for(addr=chunk_start; addr<chunk_end;i++)
	{
		if(index == i)
			return addr;
		
		offset = sto_fetch_long(addr + CHUNK_OFFSET);
		if (offset == 0 || offset > chunk_maxlen)
			break;
		addr += offset;
	}

	return ERR_FAILUE;
}

#if(SYS_SDRAM_SIZE > 2)
unsigned long sto_fetch_long_ext(unsigned long offset, unsigned int cnt)
{
    unsigned char buf[4];
	unsigned long val = 0;
	unsigned int i;

    ASSERT(sto != NULL && cnt>=1 && cnt<=4);

    sto_get_data(sto, buf, offset, cnt);

	val = 0;
	for(i=0;i<cnt;i++)
	{
		val <<= 8;
		val += buf[i];
	}

	return val;
}

int sto_subblock_count(unsigned long chid)
{
	unsigned long ch_id,address,end_address;
	unsigned char sbtype;
	unsigned long len,offset;
	CHUNK_HEADER ch_header;
	int n = 0;

	ch_id = chid;

	address = sto_chunk_goto(&ch_id,0xFFFFFFFF,1);
    if(address & 0x80000000) return 0;
	
	sto_get_chunk_header(ch_id,&ch_header);
	
	if(ch_header.offset != 0)
		end_address = address + ch_header.offset;
	else
		end_address = address + ch_header.len;

	address += CHUNK_HEADER_SIZE;

    while (address < end_address)
    {

		sbtype = sto_fetch_long_ext(address + SUBBLOCK_TYPE,1);
        if (sbtype == 0 || sbtype == 0xFF)
            break;

        offset = sto_fetch_long_ext(address + SUBBLOCK_OFFSET,3);
        len = sto_fetch_long_ext(address + SUBBLOCK_LENGTH,3);
		
        if (address + offset > end_address)
        {
        	n = 0;
            break;
        }
		else if ((offset != 0) && (len > offset))
        {
        	n = 0;
            break;
        }
		else
			n++;

        if (offset == 0)
            break;
		
        address += offset;
    }
	
    return n;	

}

unsigned long sto_subblock_goto(unsigned long chid,unsigned char *sbblock_type,unsigned char mask, unsigned int n)
{
	unsigned long ch_id,address,end_address;
	unsigned char sbtype;
	unsigned long len,offset;
	CHUNK_HEADER ch_header;
	
    if (n == 0)
        return ERR_PARA;

	ch_id = chid;

	address = sto_chunk_goto(&ch_id,0xFFFFFFFF,1);
    if(address & 0x80000000) 
		return ERR_FAILUE;
	
	sto_get_chunk_header(ch_id,&ch_header);

	if(ch_header.offset != 0)
		end_address = address + ch_header.offset;
	else
		end_address = address + ch_header.len;

	address += CHUNK_HEADER_SIZE;
	

	while (address < end_address)
	{
		sbtype = sto_fetch_long_ext(address + SUBBLOCK_TYPE,1);
        if (sbtype == 0 || sbtype == 0xFF)
            break;
		
        if (!((sbtype ^ *sbblock_type) & mask))
        {
            if (--n == 0)
            {
                *sbblock_type = sbtype;
                return address;
            }
        }

        offset = sto_fetch_long_ext(address + SUBBLOCK_OFFSET,3);
        len = sto_fetch_long_ext(address + SUBBLOCK_LENGTH,3);
		if (((offset != 0) && (len > offset))||(0 == offset))
            break;
        address += offset;
    }
	
    return ERR_FAILUE;	
}


int sto_get_subblock_header(unsigned long chid, unsigned char sbblock_type, SUBBLOCK_HEADER *pheader)
{
    unsigned long addr;

    ASSERT(sto != NULL);

    addr = sto_subblock_goto(chid,&sbblock_type, 0xFF, 1);
    if(addr & 0x80000000) return 0;

	pheader->type 		= sbblock_type;
	pheader->len 		= sto_fetch_long_ext(addr + SUBBLOCK_LENGTH, 3);
	pheader->offset 	= sto_fetch_long_ext(addr + SUBBLOCK_OFFSET, 3);

    sto_get_data(sto, pheader->reserved, addr + SUBBLOCK_RESERVED, 7);

    return 1;
}
#endif

void forlink()
{
}

/*
check sto flash chunk data
return 0:ok; else: [b31-b16]--chunk index, [b15-b0]--error type
*/
UINT32 sto_chunk_check(void)
{
	UINT32 i;
	UINT8 *buffer;
	UINT32 data_id;
	UINT32 data_len;
	UINT32 data_off;
	UINT32 data_crc;
	UINT32 crc;
	UINT32 pointer = 0;
	UINT32 index=0;

	do
	{
		if(pointer < chunk_start)
		{
			STO_CHUNK_PRINTF("@pointer[%08X] < chunk_start[%08x]\n", pointer, chunk_start);
			return (index<<16|1);
		}
		
		if(pointer + CHUNK_HEADER_SIZE > chunk_end)
		{
			STO_CHUNK_PRINTF("@pointer[%08X] +0x80 > chunk_end[%08x]\n", pointer, chunk_end);
			return (index<<16|2);
		}
	
		data_id  = sto_fetch_long((UINT32)pointer + CHUNK_ID);
		data_len = sto_fetch_long((UINT32)pointer + CHUNK_LENGTH);
		data_off = sto_fetch_long((UINT32)pointer + CHUNK_OFFSET);
		data_crc = sto_fetch_long((UINT32)pointer + CHUNK_CRC);

		STO_CHUNK_PRINTF("addr:0x%08x,id:0x%08x,len:0x%08x,offset:0x%08x,crc:0x%08x,\n",pointer,data_id,data_len,data_off,data_crc);

		if(pointer != chunk_start)
		{
			if ((((data_id >> 16) & 0xFF) + ((data_id >> 24) & 0xFF)) != 0xFF)
			{// chunk id
				STO_CHUNK_PRINTF("@pointer[%08X] data_id[%08x] error\n", pointer, data_id);
				return (index<<16|3);
			}
		}

		if((data_len > chunk_maxlen) ||((pointer+data_len+0x10) > chunk_end))
		{// chunk len
			STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_off[%08X] > flash size\n", pointer, data_id, data_off);
			return (index<<16|4);
		}
		
		if((data_off > chunk_maxlen) ||((pointer+data_off) > chunk_end))
		{
			STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer, data_id, data_len, data_off);
			return (index<<16|5);
		}

		if((data_off != 0) && (data_len > data_off))
		{
			STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer, data_id, data_len, data_off);
			return (index<<16|6);
		}
		
		if (data_crc != NO_CRC)
		{
			if ((buffer = (UINT8 *)MALLOC(data_len)) == NULL)
				ASSERT(0);
			sto_get_data(sto, buffer, pointer + 16, data_len);
			crc = MG_Table_Driven_CRC(0xFFFFFFFF, buffer, data_len);
			FREE(buffer);
			if (data_crc != crc)
			{
				STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] crc[%08X] != data_crc[%08X]\n",pointer, data_id, crc, data_crc);
				return (index<<16|7);
			}
		}
		pointer += data_off;

		index++;

	} while (data_off);

	return 0;
}

/*
 * name		: sto_chunk_check_ext(UINT32, UINT32)
 * description	: check sto flash chunk data, and map it to sdram.
 * parameter	: image_addr		buffer address for image
 			  max_len		buffer max size for image
 * return value	: Error code
 */
 UINT32 sto_chunk_check_ext(UINT32 image_addr, UINT32 max_len)
{
	UINT32 i;
	UINT8 *buffer;
	UINT32 data_id;
	UINT32 data_len;
	UINT32 data_off;
	UINT32 data_crc;
	UINT32 crc;
	UINT32 pointer = 0;
	UINT32 index=0;

	buffer = (UINT8 *)image_addr;
	do
	{
		if(pointer < chunk_start)
		{
			STO_CHUNK_PRINTF("@pointer[%08X] < chunk_start[%08x]\n", pointer, chunk_start);
			return (index<<16|1);
		}
		
		if(pointer + CHUNK_HEADER_SIZE > chunk_end)
		{
			STO_CHUNK_PRINTF("@pointer[%08X] +0x80 > chunk_end[%08x]\n", pointer, chunk_end);
			return (index<<16|2);
		}

		sto_get_data(sto, buffer, pointer, 16);
		
		data_id  = fetch_long(buffer + CHUNK_ID);
		data_len = fetch_long(buffer + CHUNK_LENGTH);
		data_off = fetch_long(buffer + CHUNK_OFFSET);
		data_crc = fetch_long(buffer + CHUNK_CRC);

		STO_CHUNK_PRINTF("addr:0x%08x,id:0x%08x,len:0x%08x,offset:0x%08x,crc:0x%08x,\n",pointer,data_id,data_len,data_off,data_crc);

		if(pointer != chunk_start)
		{
			if ((((data_id >> 16) & 0xFF) + ((data_id >> 24) & 0xFF)) != 0xFF)
			{// chunk id
				STO_CHUNK_PRINTF("@pointer[%08X] data_id[%08x] error\n", pointer, data_id);
				return (index<<16|3);
			}
		}

		if((data_len > chunk_maxlen) ||((pointer+data_len+0x10) > chunk_end))
		{// chunk len
			STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_off[%08X] > flash size\n", pointer, data_id, data_off);
			return (index<<16|4);
		}
		
		if((data_off > chunk_maxlen) ||((pointer+data_off) > chunk_end))
		{
			STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer, data_id, data_len, data_off);
			return (index<<16|5);
		}

		if((data_off != 0) && (data_len > data_off))
		{
			STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] data_len[%08X] > data_off[%08X]\n", pointer, data_id, data_len, data_off);
			return (index<<16|6);
		}

		if (data_crc != NO_CRC)
		{
			if((UINT32)(buffer + data_len + 16) > image_addr + max_len)
			{
				STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] buffer[%08X] + data_len[%08X] > image_addr[%08X] + max_len[%08X]\n", buffer, data_len, image_addr, max_len);
				return (index<<16|8);
			}
			sto_get_data(sto, buffer + 16, pointer + 16, data_len);
			crc = MG_Table_Driven_CRC(0xFFFFFFFF, buffer + 16, data_len);
			
			if (data_crc != crc)
			{
				STO_CHUNK_PRINTF("@pointer[%08X] id[%08X] crc[%08X] != data_crc[%08X]\n",pointer, data_id, crc, data_crc);
				return (index<<16|7);
			}
		}
		pointer += data_off;
		buffer += data_off;
		index++;
	} while (data_off);
	return 0;
}
