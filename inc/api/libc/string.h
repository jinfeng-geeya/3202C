
#ifndef __LIB_C_STRING_H__
#define __LIB_C_STRING_H__

//#define ALI_DDK

//某些客户项目需要将别名定义在头文件
#ifdef ALI_LIBC_ALIAS
void* memcpy(void *dest, const void *src, unsigned int len) __attribute__((alias("libc_memcpy")));
void* memset(void *dest, int c, unsigned int  len) __attribute__((alias("libc_memset")));
int itoa(char * str, unsigned long val) __attribute__((alias("ali_itoa")));
int memcmp(const void *buf1, const void *buf2, unsigned int  len) __attribute__((alias("libc_memcmp")));
void * memmove(void * dest, const void * src, unsigned int n) __attribute__((alias("libc_memmove")));
char* strcpy(char *dest, const char *src) __attribute__((alias("libc_strcpy")));
char *strncpy( char *strDest, const char *strSource, unsigned int count )  __attribute__((alias("libc_strncpy")));
int strcmp(const char *s1, const char *s2) __attribute__((alias("libc_strcmp")));
unsigned long strlen(const char *src) __attribute__((alias("libc_strlen")));
char *strstr(const char *in, const char *str) __attribute__((alias("libc_strstr")));
long strtol(const char *nptr, char **endptr, register int base) __attribute__((alias("libc_strtol")));
unsigned long strtoul(const char *nptr, char **endptr, register int base) __attribute__((alias("libc_strtoul")));
int atoi(const char * str) __attribute__((alias("libc_atoi")));
#endif

//#ifdef ALI_DDK
#if 0
//libc_xxx带打印和参数检查
#define MEMCPY(dest, src, len)			libc_memcpy(dest, src, len)
#define MEMCMP(buf1,buf2, len)			libc_memcmp(buf1,buf2, len)
#define MEMSET(dest, c, len)			libc_memset(dest, c, len)
#define MEMMOVE(dest, src, len)			libc_memmove(dest, src, len)

#define STRCPY(dest,src)				libc_strcpy(dest,src)
#define STRNCPY(dest,src,n)				libc_strncpy(dest,src,n)
#define STRCMP(dest,src)				libc_strcmp(dest,src)
#define STRLEN(str)						libc_strlen(str)

#define ITOA(str, val)					libc_itoa(str, val)
#define ATOI(str)						libc_atoi(str)
#define ATOI16(str)						libc_atoi16(str)
#else	//SDK
#define MEMCPY(dest, src, len)			ali_memcpy(dest, src, len)
#define MEMCMP(buf1,buf2, len)			ali_memcmp(buf1,buf2, len)
#define MEMSET(dest, c, len)			ali_memset(dest, c, len)
#define MEMMOVE(dest, src, len)			ali_memmove(dest, src, len)

#define STRCPY(dest,src)				ali_strcpy(dest,src)
#define STRNCPY(dest,src,n)				ali_strncpy(dest,src,n)
#define STRCMP(dest,src)				ali_strcmp(dest,src)
#define STRLEN(str)						ali_strlen(str)

#define ITOA(str, val)					ali_itoa(str, val)
#define ATOI(str)						ali_atoi(str)
#define ATOI16(str)						ali_atoi16(str)
#endif

#define RAND(max_val)					ali_rand(max_val)

#ifdef __cplusplus
extern "C"
{
#endif

void* ali_memcpy(void *dest,const void *src, unsigned int len);
void* ali_memset(void *dest, int  c,unsigned int  len);
int ali_memcmp(const void *buf1,const void *buf2,unsigned int  len);
void * ali_memmove(void * dest,const void * src, unsigned int  n);


char* ali_strcpy(char *dest, const char *src);
char * ali_strncpy ( char * dest, const char * source, int count );
int ali_strcmp(const char *dest, const char *src);
unsigned long ali_strlen(const char *src);

int ali_itoa(char * str, unsigned long val);
int ali_atoi(const char * str);
unsigned long ali_atoi16(const char * str);



//porting functions
void* libc_memcpy(void *dest,const void *src, unsigned int len);
void* libc_memset(void *dest, int  c,unsigned int  len);
int libc_memcmp(const void *buf1,const void *buf2,unsigned int  len);
void * libc_memmove(void * dest,const void * src, unsigned int  n);


char* libc_strcpy(char *dest, const char *src);
char * libc_strncpy ( char * dest, const char * source, int count );
int libc_strcmp(const char *dest, const char *src);
unsigned long libc_strlen(const char *src);

int libc_itoa(char * str, unsigned long val);
int libc_atoi(const char * str);
unsigned long libc_atoi16(const char * str);

#ifdef __cplusplus
}
#endif

#endif /* __LIB_C_STRING_H__ */

