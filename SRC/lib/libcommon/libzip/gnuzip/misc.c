/*
 * misc.c
 *
 * This is a collection of several routines from gzip-1.0.3
 * adapted for Linux.
 *
 * malloc by Hannu Savolainen 1993 and Matthias Urlichs 1994
 * puts by Nick Holloway 1993, better puts by Martin Mares 1995
 * High loaded stuff by Hans Lermen & Werner Almesberger, Feb. 1996
 */

/*
 * gzip declarations
 */

//#define SHOW_DEBUG_MESSAGE

#define OF(args)  args
#define STATIC static
const char SA[] = "NIC";
//const unsigned int Compress_Start_Address = 0x12345678;
//const unsigned int Compress_Size = 0x12345678;
static unsigned int Compress_Start_Address = 0xbfc10010;
//#define Compress_Start_Address	(0xbfc10000 + 16)
#define Compress_Size			0xffffffff
const char END[] = "END";
//#define Decom_Start_Address 0x0200
static unsigned int Decom_Start_Address = 0x0200;
#undef memset
#undef memcpy
#define memzero(s, n)     memset ((s), 0, (n))

typedef unsigned char  uch;
typedef unsigned short ush;
typedef unsigned long  ulg;

#define WSIZE 0x8000		/* Window size must be at least 32k, */
/* and a power of two */

static uch *inbuf;	     /* input buffer */
static uch window[WSIZE];    /* Sliding window buffer */

static unsigned insize ;  /* valid bytes in inbuf */
static unsigned inptr;   /* index of next byte to be processed in inbuf */
static unsigned outcnt;  /* bytes in output buffer */

/* gzip flag byte */
#define ASCII_FLAG   0x01 /* bit 0 set: file probably ASCII text */
#define CONTINUATION 0x02 /* bit 1 set: continuation of multi-part gzip file */
#define EXTRA_FIELD  0x04 /* bit 2 set: extra field present */
#define ORIG_NAME    0x08 /* bit 3 set: original file name present */
#define COMMENT      0x10 /* bit 4 set: file comment present */
#define ENCRYPTED    0x20 /* bit 5 set: file is encrypted */
#define RESERVED     0xC0 /* bit 6,7:   reserved */

#define get_byte()  (inptr < insize ? inbuf[inptr++] : fill_inbuf())

#define Assert(cond,msg)
#define Trace(x)
#define Tracev(x)
#define Tracevv(x)
#define Tracec(c,x)
#define Tracecv(c,x)

static int  fill_inbuf(void);
static void flush_window(void);
static void error(char *m);

#if 0
static void *malloc(int size);
static void free(void *where);
#endif
static void gzip_mark(void **);
static void gzip_release(void **);

static unsigned long bytes_out;
static uch *output_data;
static unsigned long output_ptr;

extern int _end;
static long free_mem_ptr = (long)&_end;
static long free_mem_end_ptr ;

#define NULL 0
#define stderr 3

#if 0
static void* memset(void* s, int c, unsigned long n);
static void* memcpy(void* __dest, __const void* __src,
             unsigned long __n);
#endif
#include "inflate.c"

static void gzip_mark(void **ptr)
{
    *ptr = (void *) free_mem_ptr;
}

static void gzip_release(void **ptr)
{
    free_mem_ptr = (long) *ptr;
}

#if 0
static void *malloc(int size)
{
    void *p;

    if (size <0)
        error("Malloc error\n");
    if (free_mem_ptr <= 0)
        error("Memory error\n");

    free_mem_ptr = (free_mem_ptr + 3) & ~3;	/* Align */

    p = (void *)free_mem_ptr;
    free_mem_ptr += size;

    if (free_mem_ptr >= free_mem_end_ptr)
        error("\nOut of memory\n");

    return p;
}

static void free(void *where)
{	/* Don't care */
}
static void* memset(void* s, int c, unsigned long n)
{
    unsigned long i;
    char *ss = (char*)s;

    for (i=0;i<n;i++)
        ss[i] = c;
    return s;
}

static void* memcpy(void* __dest, __const void* __src,
             unsigned long __n)
{
    unsigned long i;
    char *d = (char *)__dest, *s = (char *)__src;

    for (i=0;i<__n;i++)
        d[i] = s[i];
    return __dest;
}
#endif

/* ===========================================================================
 * Fill the input buffer. This is called only when the buffer is empty
 * and at least one byte is really needed.
 */
static int fill_inbuf(void)
{
    if (insize != 0)
    {
        error("ran out of input data\n");
    }

    inbuf = (uch *)Compress_Start_Address;
    insize = Compress_Size;
    inptr = 1;
    return inbuf[0];
}

/* ===========================================================================
 * Write the output window window[0..outcnt-1] and update crc and bytes_out.
 * (Used for the decompressed data only.)
 */
static void flush_window_low(void)
{
    ulg c = crc;         /* temporary variable */
    unsigned n;
    uch *in, *out, ch;

    in = window;
    out = &output_data[output_ptr];
    for (n = 0; n < outcnt; n++)
    {
        ch = *out++ = *in++;
        c = crc_32_tab[((int)c ^ ch) & 0xff] ^ (c >> 8);
    }
    crc = c;
    bytes_out += (ulg)outcnt;
    output_ptr += (ulg)outcnt;
    outcnt = 0;
}


static void flush_window(void)
{
    flush_window_low();
}

static void error(char *x)
{
    /*	while(1);   Halt */
}

static void setup_normal_output_buffer(void)
{
    output_data = (char *)Decom_Start_Address; /* Points to 1M */
}

int decompress_kernel()
{
    bytes_out = 0;
    output_ptr = 0;
    insize = 0;
    inptr = 0;
    outcnt = 0;

    free_mem_end_ptr = Decom_Start_Address;

    setup_normal_output_buffer();
    makecrc();
    return gunzip();
}

int gzip_decompress(void *input, void *output)
{
    bytes_out = 0;
    output_ptr = 0;
    insize = 0;
    inptr = 0;
    outcnt = 0;
    Compress_Start_Address = (unsigned int)input;
    Decom_Start_Address = (unsigned int)output;

    setup_normal_output_buffer();
    makecrc();
    return gunzip();
}
