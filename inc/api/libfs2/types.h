#ifndef _FS_TYPES_H_
#define _FS_TYPES_H_


#include <sys/types.h>

#ifndef __cplusplus
#define __BEGIN_DECLS
#define __END_DECLS
#else
#define __BEGIN_DECLS extern "C" {
#define __END_DECLS }
#endif


//conflict with <mips-sde/sde/include/machine/types.h>.
//mips-sde compiler for linux
#ifndef __MACHINE_TYPES_H_

typedef char int8_t;
typedef short int16_t;
typedef long int32_t;
typedef long long int64_t;


typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef unsigned long long uint64_t;

#endif //#ifndef __MACHINE_TYPES_H_


typedef char int8;
typedef short int16;
typedef long int32;
typedef long long int64;



typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned long long uint64;


typedef unsigned char uchar;


#define ino_t unsigned long long		
#define off_t long long	


#endif


