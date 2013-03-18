
#ifndef __LIB_C_ALLOC_H__
#define __LIB_C_ALLOC_H__


#ifdef __cplusplus
extern "C"
{
#endif

#define MALLOC(size)				malloc(size)
#define REALLOC(ptr, len)			realloc(ptr, len)
#define FREE(ptr)					free(ptr)


#define	__malloc_size_t		unsigned int
#define	__malloc_ptrdiff_t	int
#define	__ptr_t		void *


#ifndef SKIP_ALLOC_DECLARE   //to avoid the conflict with the <stdlib.h>
__ptr_t malloc (__malloc_size_t size);
__ptr_t realloc (__ptr_t ptr, __malloc_size_t size);
void free (__ptr_t ptr);
__ptr_t calloc(__malloc_size_t nelem, __malloc_size_t elsize);

#endif //#ifndef SKIP_ALLOC_DECLARE


#ifdef __cplusplus
}
#endif

#endif /* __LIB_C_ALLOC_H__ */
