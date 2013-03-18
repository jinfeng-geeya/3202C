
#define UNZIP_BASE_SIZE 1846
#define UNZIP_LIT_SIZE 768

#ifdef _LZMA_OUT_READ
static unsigned char *m_dict_buffer;
static unsigned int m_dict_size;

void set_dictionary_buffer(unsigned char *buffer, unsigned int size)
{
    m_dict_buffer = buffer;
    m_dict_size = size;
}

unsigned int get_dictionary_size(unsigned char *in)
{
    unsigned int size;
    size = (in[4] << 24) | (in[3] << 16) | (in[2] << 8) | in[1];
    return size;
}

unsigned int get_unzip_size(unsigned char *in)
{
    unsigned int size;
    size = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];
    return size;
}

unsigned int get_internal_size(unsigned char *in)
{
    unsigned int lzmaInternalSize;
    int lc, lp, pb;
    unsigned char prop0 = in[0];
    if (prop0 >= ( 9 * 5 * 5))
    {
        //PRINTF("properties error\n");
        return 1;
    }

    for (pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5));
    for (lp = 0; prop0 >= 9; lp++, prop0 -= 9);
    lc = prop0;

    // 2) Calculate required amount for LZMA lzmaInternalSize:
    lzmaInternalSize = (UNZIP_BASE_SIZE + (UNZIP_LIT_SIZE << (lc + lp))) * 
          sizeof(unsigned short);

    return (lzmaInternalSize+100);
}

typedef int (*UN7ZIP_OUT_CALLBACK)(unsigned char **ptr_out, unsigned int *ptr_size);
int un7zip(unsigned char *in, unsigned int in_size, 
           unsigned char *buf, unsigned int buf_size, 
           unsigned char *out, unsigned int out_size, UN7ZIP_OUT_CALLBACK outCallback)
{
    unsigned int lzmaInternalSize, dictSize;
    int lc, lp, pb;
    unsigned int nowPos, unzipSize, outSizeProcessed;
    int ret;

    // 1) Read first byte of properties for LZMA compressed stream,
    // check that it has correct value and calculate three
    // LZMA property variables:
    unsigned char prop0 = in[0];
    if(prop0 >= ( 9 * 5 * 5))
    {
        //PRINTF("properties error\n");
        return LZMA_RESULT_DATA_ERROR;
    }

    for(pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5));
    for(lp = 0; prop0 >= 9; lp++, prop0 -= 9);
    lc = prop0;

    // 2) Calculate required amount for LZMA lzmaInternalSize:
    lzmaInternalSize = (UNZIP_BASE_SIZE + (UNZIP_LIT_SIZE << (lc + lp))) * sizeof(unsigned short) + 100;
    if(lzmaInternalSize > buf_size)
        return LZMA_RESULT_NOT_ENOUGH_MEM;

    // 3) Get decompressed data dictionary size:
    dictSize = get_dictionary_size(in);
    if(dictSize > m_dict_size)
        return LZMA_RESULT_NOT_ENOUGH_MEM;
        
    unzipSize = get_unzip_size(in);
    if(!outCallback && unzipSize > out_size)
        return LZMA_RESULT_NOT_ENOUGH_MEM;

    // 4) Init decoder
    ret = LzmaDecoderInit(buf, lzmaInternalSize, lc, lp, pb, 
                          m_dict_buffer, dictSize, in+13, in_size-13);
    if(ret != LZMA_RESULT_OK)
        return ret;

    // 4) Decompress data:
    for(nowPos=0; nowPos<unzipSize; nowPos+=outSizeProcessed)
    {
        UInt32 blockSize = unzipSize - nowPos;
        if(blockSize > out_size)
            blockSize = out_size;
        ret = LzmaDecode(buf, out, blockSize, &outSizeProcessed);
        if(ret != LZMA_RESULT_OK || !outSizeProcessed)
            break;
        if(outCallback)
        {
            out_size = outSizeProcessed;
            if(outCallback(&out, &out_size))
                break;
        }
    }
    if(!ret)
    {
        for(lc=0; lc<(int)sizeof(unsigned int); lc++)
            buf[lc] = ((unsigned char *)&nowPos)[lc];
    }
    return ret;
}
#else
int un7zip(unsigned char *in, unsigned char *out, \
      unsigned char *buf)
{
    unsigned int lzmaInternalSize;
    int lc, lp, pb;
    unsigned int outSize, outSizeProcessed;
    int ret;

    // 1) Read first byte of properties for LZMA compressed stream,
    // check that it has correct value and calculate three
    // LZMA property variables:
    unsigned char prop0 = in[0];
    if (prop0 >= ( 9 * 5 * 5))
    {
        //PRINTF("properties error\n");
        return 1;
    }

    for (pb = 0; prop0 >= (9 * 5); pb++, prop0 -= (9 * 5));
    for (lp = 0; prop0 >= 9; lp++, prop0 -= 9);
    lc = prop0;

    // 2) Calculate required amount for LZMA lzmaInternalSize:
    lzmaInternalSize = (UNZIP_BASE_SIZE + (UNZIP_LIT_SIZE << (lc + lp))) * 
          sizeof(unsigned short);

    // 3) Get decompressed data size:
    outSize = (in[8] << 24) | (in[7] << 16) | (in[6] << 8) | in[5];

    // 4) Decompress data:
    ret = LzmaDecode(buf, lzmaInternalSize,
          lc, lp, pb,
          in + 13, 0x800000,
          out, outSize, &outSizeProcessed);

    *(unsigned int *)buf = outSizeProcessed;
    return ret;
}
#endif
